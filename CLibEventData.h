#pragma once
#include <event2/bufferevent.h>
#include <event2/bufferevent_compat.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer_compat.h>
#include <event2/http_struct.h>
#include <event2/bufferevent.h>
#include <event2/thread.h>
#include <thread>
#include <mutex>

struct _Conn;
struct _Worker;
class CLibEvent;
//���������Է�װ����
struct _Server
{
	bool bStart;
	short nPort;
	short workernum;
	unsigned int connnum;
	volatile int nCurrentWorker;
	int read_timeout;
	int write_timeout;		
	struct evconnlistener *pListener;
	struct event_base *pBase;
	std::thread hThread;
	_Worker *pWorker;		
};


//���Ӷ����б�
struct _ConnList
{
	_ConnList()
	{
		head=NULL;
		tail=NULL;
		plistConn=NULL;
	}
	_Conn *head;
	_Conn *tail;
	_Conn *plistConn;
};
//���Ӷ���
struct _Conn
{
	_Conn()
	{
		fd=NULL;
		bufev=NULL;
		index=-1;
		in_buf_len=0;
		//out_buf_len=0;
		in_buff_max = 0;
		//out_buff_max = 0;
		owner=NULL;
		next=NULL;
		pImp = NULL;
		in_buf=NULL;
		//out_buf=NULL;
	}
	~_Conn()
	{
		delete[]in_buf;
		//delete[]out_buf;
		bufferevent_free(bufev);
		bufev = NULL;
	}
	struct bufferevent *bufev;
	evutil_socket_t fd;
	int index;
	char *in_buf;
	int in_buf_len;
	//char *out_buf;
	//int out_buf_len;
	int in_buff_max;
	//int out_buff_max;
	_Worker *owner;
	_Conn *next;
	CLibEvent* pImp;
};
//�����̷߳�װ����.
struct _Worker
{
	_Worker()
	{
		pWokerbase=NULL;
		pListConn=NULL;
		
	}
	~_Worker()
	{
		
	}
	struct event_base *pWokerbase;
	std::thread hThread;
	_ConnList *pListConn;
	std::recursive_mutex cs;
	_Conn* GetFreeConn()
	{
		_Conn*pItem=NULL;
		cs.lock();
		if(pListConn->head!=pListConn->tail)
		{
			pItem=pListConn->head;
			pListConn->head=pListConn->head->next;
		}
		cs.unlock();
		return pItem;
	}
	void PutFreeConn(_Conn *pItem)
	{
		cs.lock();
		pListConn->tail->next=pItem;
		pListConn->tail=pItem;
		pItem->next = NULL;
		cs.unlock();
	}
};


typedef struct _Server Server;
typedef struct _Worker Worker;
typedef struct _Conn Conn;
typedef struct _ConnList ConnList;
