#pragma once

/** @file tcpPacket.h
*  @brief 基于libevent实现的多线程处理缓冲池接收与发送接口。
*         
*  //使用说明
*  @author xuhuajie
*  @date 2017/08/21
*
*  @note 历史记录：
*  @note V1.0.0.0 创建
*/
#ifndef _TCPPACKET_H__
#define _TCPPACKET_H__

#pragma once

#if (defined(_WIN32) || defined(_WIN64))
#  ifdef TCPPACKET_EXPORTS
#    define TCPPACKET_API __declspec(dllexport)
#  else
#    define TCPPACKET_API __declspec(dllimport)
#  endif

#else
#  define TCPPACKET_API
#endif

#include "UserCallBack.h"
#include <string.h>
typedef struct _TcpPacketConfig
{
	int            _port;          //服务器的端口号
	short          _workNum;       //工作的线程数目
	unsigned int   _connNum;       //每个工作线程的连接数
	int            _readTimeOut;   //读取的超时时间
	int            _writeTimeOut;  //写入的超时时间
	int            _readBuffLen;   //业务类包的最大接受长度
	int            _WriteBuffLen;  //业务类包的最大发送长度,已废弃

	_TcpPacketConfig()
	{
		_connNum = 100;
		_workNum = 5;
		_readTimeOut = 120;
		_writeTimeOut = 120;
		_readBuffLen = 1024;
		_WriteBuffLen = 1024;
		_port = 8000;
	}

} TcpPacketConfig;

class ITcpPacketManager
{
public:
	virtual bool Start(_TcpPacketConfig & config, ITcpPacketNotify* notify) = 0;
	virtual void Stop() = 0;
	virtual bool SendPacket(int fd,const char* packet,int packetlen) = 0; 
};


TCPPACKET_API ITcpPacketManager* CreateTcpPacketManager();

TCPPACKET_API void DestroyTcpPacketManager(ITcpPacketManager* manager);





















#endif

