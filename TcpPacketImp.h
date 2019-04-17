#pragma once
#include "tcpPacket.h"
#include "LibEvent.h"
class CTcpPacketImp :
	public ITcpPacketManager
{
public:
	CTcpPacketImp();
	~CTcpPacketImp();

	virtual bool Start(_TcpPacketConfig & config, ITcpPacketNotify* notify);
	virtual void Stop();
	virtual bool SendPacket(int fd, const char* packet, int packetlen);


private:
	CLibEvent m_libEvent;
};

