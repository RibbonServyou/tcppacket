#include "stdafx.h"
#include "TcpPacketImp.h"


CTcpPacketImp::CTcpPacketImp()
{
}


CTcpPacketImp::~CTcpPacketImp()
{
}

bool CTcpPacketImp::Start(_TcpPacketConfig & config, ITcpPacketNotify* notify)
{
	return m_libEvent.StartServer(config._port, config._workNum, config._connNum, config._readTimeOut, config._writeTimeOut,
		config._readBuffLen,config._WriteBuffLen,notify);
}

void CTcpPacketImp::Stop()
{
	return m_libEvent.StopServer();
}

bool CTcpPacketImp::SendPacket(int fd, const char* packet, int packetlen)
{
	return m_libEvent.SendPacket(fd,packet,packetlen);
}

