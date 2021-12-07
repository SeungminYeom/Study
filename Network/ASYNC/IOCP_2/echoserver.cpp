#include "echoserver.h"

void EchoServer::OnConnect(int idx)
{
	cout << "client index(" << idx << ") is connected" << endl;
	RecvMsg(idx);
}

void  EchoServer::OnClose(int idx)
{
	cout << "\nclient index(" << idx << ") is closed" << endl;
}

void  EchoServer::OnReceive(int idx, int size, char* pData)
{
	pData[size] = '\0';
	cout << "\nclient index(" << idx << ") : " << pData << endl;
	PacketData echopacket;
	echopacket.PacketCopy(idx, size, pData);
	lock_guard<mutex> guard(m_PacketLock);
	m_PacketDataQue.push_back(echopacket);
}

void EchoServer::Run(int n)
{
	m_IsPacketRun = true;
	m_PacketThread = thread([this]() {ProcessPacket(); });
	ServerStart(n);
}

void EchoServer::Exit()
{
	m_IsPacketRun = false;
	if (m_PacketThread.joinable())
		m_PacketThread.join();
	DestroyThread();
}

void EchoServer::ProcessPacket()
{
	while (m_IsPacketRun)
	{
		auto  packet = DequePacketData();
		if (packet.m_DataSize != 0)
			SendMsg(packet.m_SessionIndex, packet.m_DataSize, packet.m_pPacketData);
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

PacketData EchoServer::DequePacketData()
{
	PacketData  Packet;
	lock_guard<mutex> guard(m_PacketLock);
	if (m_PacketDataQue.empty())
		return PacketData();
	Packet.PacketCopy(m_PacketDataQue.front());
	m_PacketDataQue.front().RemovePacket();
	m_PacketDataQue.pop_front();
	return Packet;
}