#include "iocpserver.h"

bool IOCPServer::InitSocket()
{
	WSADATA wsa;
	WORD ver = MAKEWORD(2, 2);
	if (WSAStartup(ver, &wsa))
		return false;
	m_listensock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_listensock == INVALID_SOCKET)
		return false;
	return true;
}

bool IOCPServer::BindListen()
{
	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(IOCPPORT);
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(m_listensock, (SOCKADDR*)&saddr, sizeof(saddr)))
		return false;
	if (listen(m_listensock, SOMAXCONN))
		return false;
	return true;
}
//ServerStart
//이 함수는 에코서버에서 호출되며 IOCPServer를 가동하기 위함
//1. 세션 객체를 주어진 갯수(MAXClient)만큼 생성
//2. IOCP를 생성하여 m_iocphandle 멤버 변수에 저장
//3. Worker 스레드를 생성(GetQueuedCompletionStatus 함수를 실행하여 I/O 처리하는 노동자 스레드 생성)
//4. Accepter 스레드 생성 (accept를 담당)
bool IOCPServer::ServerStart(int maxsession)
{
	//1. 세션 객체를 미리 생성함 : 세션객체의 생성자에서 하는 초기화도 고려해야 함
	CreateSession(maxsession);

	//2.IOCP 객체 생성
	m_iocphandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MAXTHREAD);
	if (m_iocphandle == nullptr)
		return false;

	//3. worker thread 생성
	bool bRet = CreateWorkerThread();
	if (bRet == false)
		return false;

	//4. accepter thread 생성
	bRet = CreateAccepterThread();
	if (bRet == false)
		return false;
	return true;
}

//DestroyThread 함수
//이 함수는 에코서버에서 서버의 서비스를 끝낼 때 호출
void IOCPServer::DestroyThread()
{
	//Worker 스레드 종료를 위한 flag 셋팅
	m_IsWorkerRun = false;

	//IOCP 핸들 종료
	CloseHandle(m_iocphandle);

	//Worker 스레드가 종료될 때까지 기다림
	for (auto& th : m_workerthreads)
		if (th.joinable())
			th.join();

	//Accepter 스레드 종료를 위한 flag 셋팅
	m_IsAccepterRun = false;

	//Listen socket 종료
	closesocket(m_listensock);

	//Accepter 스레드가 종료될 때까지 기다림
	if (m_accepterthread.joinable())
		m_accepterthread.join();

}

void IOCPServer::CreateSession(int max)
{
	for (int i = 0; i < max; i++)
		m_sessionarr.push_back(new Session(i));
}

bool IOCPServer::CreateWorkerThread()
{
	m_IsWorkerRun = true;
	for (int i = 0; i < MAXTHREAD; i++)
		m_workerthreads.emplace_back([this]() {WorkerThread(); });
	return true;
}

bool IOCPServer::CreateAccepterThread()
{
	m_IsAccepterRun = true;
	m_accepterthread = thread([this]() {AccepterThread(); });
	return true;
}

void IOCPServer::WorkerThread()
{
	Session* ses = nullptr;
	WSAOVERLAPPED* over = nullptr;
	bool bRet = false;
	IoData* iod = nullptr;
	DWORD trans;

	while (m_IsWorkerRun)
	{
		bRet = GetQueuedCompletionStatus(m_iocphandle, &trans, (PULONG_PTR)&ses, &over, INFINITE);
		if (bRet == false)
		{
			if (over == nullptr)
			{
				cout << "[error] GetQueuedCompletionStatus() : " << WSAGetLastError() << endl;
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
			else {
				cout << "client(" << ses->GetIndex() << ") Abnormal Close case " << endl;
				CloseSession(ses);
			}
			continue;
		}//if false
		iod = (IoData*)over;
		switch (iod->m_iotype) {
		case IoType::Recv:
			if (trans == 0)
			{
				cout << "client(" << ses->GetIndex() << ") normal  close request" << endl;
				CloseSession(ses);
				break;
			}
			OnReceive(ses->GetIndex(), trans, ses->GetRbuf());
			ses->PacketRecv();
			break;
		case  IoType::Send:
			ses->SendCompleted();
			break;

		}//swithc
	}
}

void IOCPServer::AccepterThread()
{
	SOCKADDR_IN caddr;
	int namelen = sizeof(caddr);
	SOCKET c_sock;

	Session* ses = nullptr;
	while (m_IsAccepterRun)
	{
		ses = GetEmptySession();
		if (ses == nullptr)
		{
			cout << "Full Session occupied" << endl;
			return;
		}
		c_sock = accept(m_listensock, (SOCKADDR*)&caddr, &namelen);
		if (c_sock == INVALID_SOCKET)
			return;

		if (ses->OnConnect(m_iocphandle, c_sock) == false)
		{
			CloseSession(ses);
			return;
		}
		OnConnect(ses->GetIndex());
	}
}

void IOCPServer::CloseSession(Session* ses)
{
	auto idx = ses->GetIndex();
	ses->CloseSocket();
	OnClose(idx);
}

void IOCPServer::RecvMsg(int idx)
{
	Session* ses = GetSessionPtr(idx);
	ses->PacketRecv();
}

bool IOCPServer::SendMsg(int idx, int size, char* pData)
{
	auto ses = GetSessionPtr(idx);
	return ses->SendMsg(size, pData);
}

Session* IOCPServer::GetSessionPtr(int idx)
{
	return m_sessionarr[idx];
}

Session* IOCPServer::GetEmptySession()
{
	for (auto ses : m_sessionarr)
		if (ses->IsConnected() == false)
			return ses;
	return nullptr;
}