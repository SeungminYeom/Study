#pragma once
#include "iodata.h"
#include <WS2tcpip.h>
#include <mutex>
#include <queue>

using std::queue;
using std::mutex;
using std::lock_guard;

const int MAXRBUF = 1024;
const int MAXSBUF = 2048;
const int MAXTHREAD = 8;

class Session {
	int m_index;
	IoData m_rio;				// 수신 전용
	char m_rbuf[MAXRBUF];		// 수신 전용
	SOCKET m_sock;
	mutex  m_sendlock;
	queue<IoData*> m_sendque;
public:
	Session(int);
	bool OnConnect(HANDLE, SOCKET);
	void CloseSocket();
	bool PacketRecv();
	bool SendMsg(int, char*);
	bool PacketSend();
	void SendCompleted();

	int GetIndex() { return m_index; }
	SOCKET& GetSock() { return m_sock; }
	bool IsConnected() { return m_sock != INVALID_SOCKET; }
	char* GetRbuf() { return m_rbuf; }
};
