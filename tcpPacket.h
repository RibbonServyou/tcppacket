#pragma once

/** @file tcpPacket.h
*  @brief ����libeventʵ�ֵĶ��̴߳�����ؽ����뷢�ͽӿڡ�
*         
*  //ʹ��˵��
*  @author xuhuajie
*  @date 2017/08/21
*
*  @note ��ʷ��¼��
*  @note V1.0.0.0 ����
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
	int            _port;          //�������Ķ˿ں�
	short          _workNum;       //�������߳���Ŀ
	unsigned int   _connNum;       //ÿ�������̵߳�������
	int            _readTimeOut;   //��ȡ�ĳ�ʱʱ��
	int            _writeTimeOut;  //д��ĳ�ʱʱ��
	int            _readBuffLen;   //ҵ������������ܳ���
	int            _WriteBuffLen;  //ҵ�����������ͳ���,�ѷ���

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

