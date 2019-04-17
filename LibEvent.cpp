#include "StdAfx.h"
#include "LibEvent.h"
#include <string>
#include <iostream>
#include <signal.h>

#ifdef WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include<netinet/in.h>  
#include<sys/socket.h>  
#include<unistd.h> 
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#endif

using namespace std;

CLibEvent::CLibEvent(void)
{	
#ifdef WIN32
	ZeroMemory(&m_Server, sizeof(m_Server));
	WSADATA WSAData;
	WSAStartup(0x0201, &WSAData);
#else
	memset(&m_Server, 0, sizeof(m_Server));
#endif // WIN32
}

CLibEvent::~CLibEvent(void)
{
#ifdef WIN32
	WSACleanup();
#endif // WIN32
}

bool CLibEvent::StartServer(int port, short workernum, unsigned int connnum, int read_timeout, int write_timeout,
	int inbuffmax, int outbuffmax, ITcpPacketNotify* notify)
{	
	m_notify = notify;
	m_Server.bStart=false;
	m_Server.nCurrentWorker=0;
	m_Server.nPort=port;
	m_Server.workernum=workernum;
	m_Server.connnum=connnum;
	m_Server.read_timeout=read_timeout;
	m_Server.write_timeout=write_timeout;
#ifdef WIN32
	evthread_use_windows_threads();
#else
	evthread_use_pthreads();
#endif // WIN32

	m_Server.pBase=event_base_new();
	if (m_Server.pBase==NULL)
	{
		return false;
	}
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(m_Server.nPort);
	m_Server.pListener=evconnlistener_new_bind(m_Server.pBase,DoAccept,(void*)this,LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, 1024,(struct sockaddr*)&sin,sizeof(sin));
	if (m_Server.pListener==NULL)
	{
		return false;
	}	
	
	m_Server.pWorker=new Worker[workernum];
	for (int i=0;i<workernum;i++)
	{
		m_Server.pWorker[i].pWokerbase=event_base_new();
		if (m_Server.pWorker[i].pWokerbase== NULL)
		{
			delete []m_Server.pWorker;
			return false;
		}
		//初始化连接对象
		{
			m_Server.pWorker[i].pListConn=new ConnList();
			if (m_Server.pWorker[i].pListConn==NULL)
			{
				return false;
			}
			m_Server.pWorker[i].pListConn->plistConn=new Conn[m_Server.connnum+1];
			m_Server.pWorker[i].pListConn->head=&m_Server.pWorker[i].pListConn->plistConn[0];
			m_Server.pWorker[i].pListConn->tail=&m_Server.pWorker[i].pListConn->plistConn[m_Server.connnum];
			for (unsigned j=0; j<m_Server.connnum; j++) {
				m_Server.pWorker[i].pListConn->plistConn[j].index=j;
				m_Server.pWorker[i].pListConn->plistConn[j].next=&m_Server.pWorker[i].pListConn->plistConn[j+1];
			}
			m_Server.pWorker[i].pListConn->plistConn[m_Server.connnum].index=m_Server.connnum;
			m_Server.pWorker[i].pListConn->plistConn[m_Server.connnum].next=NULL;
			//设置当前事件
			Conn *p=m_Server.pWorker[i].pListConn->head;
			while (p!=NULL)
			{
				p->bufev=bufferevent_socket_new(m_Server.pWorker[i].pWokerbase,-1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
				if (p->bufev==NULL) 
				{
					return false;
				}
				bufferevent_setcb(p->bufev, DoRead, NULL, DoError, p);
				bufferevent_setwatermark(p->bufev, EV_READ, 0, 0);        //使用默认的高低水位
				bufferevent_enable(p->bufev, EV_READ);
				p->owner=&m_Server.pWorker[i];
				p->pImp = this;
				p->in_buff_max = inbuffmax;
				p->in_buf = new char[inbuffmax];
				p=p->next;
			}
		}
		m_Server.pWorker[i].hThread = std::thread(ThreadWorkers, &m_Server.pWorker[i]);
	}
	m_Server.hThread= std::thread(ThreadServer,&m_Server);
	m_Server.bStart=true;
	return true;
}

void CLibEvent::StopServer()
{
	if (m_Server.bStart)
	{
		struct timeval delay = { 2, 0 };
		event_base_loopexit(m_Server.pBase, NULL);
		m_Server.hThread.join();
		if (m_Server.pWorker)
		{
			for (int i=0;i<m_Server.workernum;i++)
			{
				event_base_loopexit(m_Server.pWorker[i].pWokerbase, NULL);
				m_Server.pWorker[i].hThread.join();
			}
			for (int i=0;i<m_Server.workernum;i++)
			{
				if (m_Server.pWorker[i].pListConn)
				{
					delete []m_Server.pWorker[i].pListConn->plistConn;
					delete m_Server.pWorker[i].pListConn;
					m_Server.pWorker[i].pListConn=NULL;
				}
				event_base_free(m_Server.pWorker[i].pWokerbase);
			}			
			delete[]m_Server.pWorker;
			m_Server.pWorker=NULL;
		}	
		evconnlistener_free(m_Server.pListener);
		event_base_free(m_Server.pBase);
	}
	m_Server.bStart=false;	
}

void CLibEvent::DoRead(struct bufferevent *bev, void *ctx)
{
    struct evbuffer * input=bufferevent_get_input(bev);
    if (evbuffer_get_length(input)) 
	{
		Conn *c = (Conn*) ctx;
		while (evbuffer_get_length(input))
		{
			//超过emMaxBuffLen还没有被消费掉，无能为力了。
			if (c->in_buf_len >= c->in_buff_max)
			{
				const char* msg = "no more buff";
				c->pImp->m_notify->OnError(c->fd,105,&msg);
				CloseConn(c);
				return;
			}

			//拷贝缓冲池的内存到Conn，最大缓冲不超过emMaxBuffLen
			c->in_buf_len += evbuffer_remove(input, c->in_buf + c->in_buf_len, c->in_buff_max - c->in_buf_len);
			//抛给业务层去解析包
			while (true)
			{
				int packlen = 0, ignore = 0;
				bool bRet = c->pImp->m_notify->OnAnalyzePacket(c->fd, c->in_buf,c->in_buf_len,packlen, ignore);
				if (!bRet)  //可能要舍弃一些脏数据
				{
					if (ignore > 0)
					{
						c->in_buf_len -= ignore;    //缓冲长度变少
						if (c->in_buf_len>0)
						{
							memmove(c->in_buf, c->in_buf + ignore, c->in_buf_len);
						}
						
					}
					else
					{
						//解析包失败了，往往是长度不够，跳出此循环继续读缓冲数据
						break;
					}
				}
				else
				{
					if (packlen>c->in_buf_len)
					{
						//用户解析的时候未考虑长度
						break;
					}
					//解析成功,通知业务层处理
					int out_len = 0;
					c->pImp->m_notify->OnPacketArrived(c->fd, c->in_buf, packlen,0, out_len);
					/*if (c->out_buf_len !=0)
					{
						//回复报文
						SendPacket(c->fd, c->out_buf, c->out_buf_len);
						//移除数据
						c->out_buf_len = 0;
					}*/
					//移除这个包文
					c->in_buf_len -= packlen;    //缓冲长度变少
					if (c->in_buf_len>0)
					{
						memmove(c->in_buf, c->in_buf + packlen, c->in_buf_len);
					}
					
				}
			}

		}

    }
}

void CLibEvent::CloseConn(Conn *pConn)
{
	pConn->in_buf_len = 0;
	//pConn->out_buf_len = 0;
	bufferevent_setfd(pConn->bufev, -1);
	pConn->pImp->m_notify->OnDisConnected(pConn->fd);
	evutil_closesocket(pConn->fd);


	pConn->fd = NULL;

	bufferevent_set_timeouts(pConn->bufev, NULL, NULL);  //取消读写超时时间
	bufferevent_enable(pConn->bufev, EV_READ );

	if (pConn->owner) //server
	{
		pConn->owner->PutFreeConn(pConn);
	}


}

void CLibEvent::DoError(struct bufferevent *bev, short error, void *ctx)
{
	Conn *c=(Conn*)ctx;
	if (error & BEV_EVENT_CONNECTED)
	{
		int fd = bufferevent_getfd(bev);
		c->fd = fd;
		evutil_make_socket_nonblocking(fd);
		bufferevent_setfd(c->bufev, c->fd);
		//转发发送事件
		c->pImp->m_notify->OnConnected(c->fd);
		bufferevent_enable(c->bufev, EV_READ | EV_WRITE);
		return;
	}
	if (error&EVBUFFER_TIMEOUT) 
	{
		c->pImp->m_notify->OnTimeOutError(c->fd);
	}else if (error&EVBUFFER_ERROR) 
	{
		int err = EVUTIL_SOCKET_ERROR();
		const char* msg = evutil_socket_error_to_string(err);
		c->pImp->m_notify->OnError(c->fd,err,&msg);
	}
	else if (error & EVBUFFER_EOF)
	{
		const char* msg = "close";
		c->pImp->m_notify->OnError(c->fd, 0, &msg);
	}
	CloseConn(c);
}

void CLibEvent::DoAccept(struct evconnlistener *listener, evutil_socket_t fd,struct sockaddr *sa, int socklen, void *user_data)
{
	//此处为监听线程的event.不做处理.
	CLibEvent* pThis = (CLibEvent*)user_data;
	Server *pServer = &(pThis->m_Server);
	//主线程处做任务分发.
	int nCurrent=pServer->nCurrentWorker++%pServer->workernum;
	//当前线程所在ID号
	Worker &pWorker=pServer->pWorker[nCurrent];
	//通知线程开始读取数据,用于分配哪一个线程来处理此处的event事件
	Conn *pConn=pWorker.GetFreeConn();
	if (pConn==NULL)
	{
		const char* msg = "!!!!连接数已经被用完!!!!";
		pThis->m_notify->OnError(fd, 2,&msg);
		return;
	}	 
	pConn->fd=fd;

	

	evutil_make_socket_nonblocking(pConn->fd);
	bufferevent_setfd(pConn->bufev, pConn->fd);
	//转发发送事件
	pThis->m_notify->OnConnected(pConn->fd);
	struct sockaddr_in *sin = (sockaddr_in*)sa;
	char* ClientIP =new char[256];    //客户端的IP地址
	memset(ClientIP, 0, 256);
	memcpy(ClientIP, inet_ntoa(sin->sin_addr), 20);
	pThis->m_notify->OnError(pConn->fd, 0, (const char**)&ClientIP);//临时用下，确认ip
	delete[]ClientIP;
}

void CLibEvent::ThreadServer(void* lPVOID)
{
	Server * pServer=reinterpret_cast<Server *>(lPVOID);
	if (pServer==NULL)
	{
		return;
	}
	event_base_dispatch(pServer->pBase);
	return ;
}

void CLibEvent::ThreadWorkers(void* lPVOID)
{
	Worker *pWorker=reinterpret_cast<Worker *>(lPVOID);
	if (pWorker==NULL)
	{
		return ;
	}
	event_base_dispatch(pWorker->pWokerbase);
	
	return ;
}

bool CLibEvent::SendPacket(int fd, const char* packet, int packetlen)
{
	int nLeft, nWritten;
	const char* pBuf = packet;
	nLeft = packetlen;

	//加个锁
	m_cs.lock();
	while (nLeft > 0)
	{
		nWritten = send(fd, pBuf, nLeft, 0);
		if (nWritten == SOCKET_ERROR)
		{
			int err = EVUTIL_SOCKET_ERROR();
			const char* msg = evutil_socket_error_to_string(err);
			m_notify->OnError(fd, err, &msg);
			m_cs.unlock();
			return false;
		}
		nLeft -= nWritten;
		pBuf += nWritten;
	}
	m_cs.unlock();
	return true;
	

}


