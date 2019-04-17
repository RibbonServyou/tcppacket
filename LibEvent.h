#pragma once
#include "CLibEventData.h"
#include "UserCallBack.h"
#include <map>

class CLibEvent
{
public:
	CLibEvent(void);
	~CLibEvent(void);
private:
	//当前服务器对象
	Server m_Server;
public:
	bool StartServer(int port, short workernum, unsigned int connnum, int read_timeout, int write_timeout,
		int inbuffmax,int outbuffmax,ITcpPacketNotify* notify);
	void StopServer();
	bool SendPacket(int fd, const char* packet, int packetlen);
private:
	static void DoAccept(struct evconnlistener *listener, evutil_socket_t fd,struct sockaddr *sa, int socklen, void *user_data);
	static void DoError(struct bufferevent *bev, short error, void *ctx);
	static void CloseConn(Conn *pConn);
	static void DoRead(struct bufferevent *bev, void *ctx);
	static void ThreadServer(void* lPVOID);
	static void ThreadWorkers(void*  lPVOID);
	

 	ITcpPacketNotify * m_notify;
	std::recursive_mutex m_cs;

};
