#pragma once
#include <Windows.h>

struct PacketData {
	int m_SessionIndex;
	int m_DataSize;
	char* m_pPacketData;

	void PacketCopy(PacketData& packet)
	{
		m_SessionIndex = packet.m_SessionIndex;
		m_DataSize = packet.m_DataSize;
		m_pPacketData = new char[m_DataSize];
		CopyMemory(m_pPacketData, packet.m_pPacketData, m_DataSize);
	}
	void PacketCopy(int idx, int size, char* pData)
	{
		m_SessionIndex = idx;
		m_DataSize = size;
		m_pPacketData = new char[m_DataSize];
		CopyMemory(m_pPacketData, pData, m_DataSize);
	}
	void RemovePacket() { delete m_pPacketData; }
};