#pragma once

#include <WinSock2.h>

enum class IoType {
	Recv, Send
};

struct IoData {
	WSAOVERLAPPED m_over;
	WSABUF m_wsabuf;
	IoType m_iotype;
};