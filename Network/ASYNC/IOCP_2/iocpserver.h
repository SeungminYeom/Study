#pragma once
#include "session.h"
#include <vector>
#include <thread>
#include <iostream>

using std::vector;
using std::thread;
using std::cout;
using std::endl;

const int MAXClient = 20;
const UINT16 IOCPPORT = 8000;

class IOCPServer {
	vector<Session*> m_sessionarr;
	SOCKET m_listensock;
	vector<thread> m_workerthreads;
	thread m_accepterthread;
	HANDLE m_iocphandle;

	bool m_IsWorkerRun;
	bool m_IsAccepterRun;
public:
	IOCPServer() {
		m_iocphandle = INVALID_HANDLE_VALUE;
		m_listensock = INVALID_SOCKET;
		m_IsWorkerRun = false; 
		m_IsAccepterRun = false;
	}
	~IOCPServer() { WSACleanup(); }
	bool InitSocket();
	bool BindListen();
	bool ServerStart(int);
	void CreateSession(int);
	bool CreateWorkerThread();
	bool CreateAccepterThread();
	void DestroyThread();
	void WorkerThread();
	void AccepterThread();
	void CloseSession(Session*);
	void RecvMsg(int);
	bool SendMsg(int, int, char*);
	Session* GetEmptySession();
	Session* GetSessionPtr(int);
	virtual void OnConnect(int) {};
	virtual void OnClose(int) {};
	virtual void OnReceive(int, int, char*) {};
};