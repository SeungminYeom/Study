#pragma once
#include "iocpserver.h"
#include "packet.h"
#include <deque>

using  std::deque;

class EchoServer :public IOCPServer
{
	bool m_IsPacketRun;
	thread m_PacketThread;
	mutex m_PacketLock;
	deque<PacketData> m_PacketDataQue;
public:
	void OnConnect(int);
	void OnClose(int);
	void OnReceive(int, int, char*);
	void  Run(int);
	void ProcessPacket();
	void Exit();
	PacketData DequePacketData();
};
