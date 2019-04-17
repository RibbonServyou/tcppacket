// tcpPacket.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "tcpPacketImp.h"


TCPPACKET_API ITcpPacketManager* CreateTcpPacketManager()
{
	ITcpPacketManager* tcpImg = new CTcpPacketImp;;
	if (tcpImg)
	{
		return tcpImg;
	}
	else
	{
		return NULL;
	}
}

TCPPACKET_API void DestroyTcpPacketManager(ITcpPacketManager* manager)
{
	if (manager)
	{
		delete manager;
		manager = NULL;
	}
}



