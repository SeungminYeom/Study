#include "session.h"

Session::Session(int idx)
{
	m_index = idx;
	m_sock = INVALID_SOCKET;
	m_rio.m_wsabuf.buf = m_rbuf;
}
bool Session::OnConnect(HANDLE iocphandle, SOCKET sock)
{
	m_sock = sock;
	HANDLE hiocp = CreateIoCompletionPort((HANDLE)sock, iocphandle, (ULONG_PTR)this, MAXTHREAD);
	if (hiocp == nullptr)
		return false;
	//PacketRecv();
	return true;
}

void Session::CloseSocket()
{
	closesocket(m_sock);
	m_sock = INVALID_SOCKET;
}

bool Session::PacketRecv()
{
	ZeroMemory(&m_rio.m_over, sizeof(m_rio.m_over));
	m_rio.m_wsabuf.len = MAXRBUF - 1;
	m_rio.m_iotype = IoType::Recv;
	DWORD trans;
	DWORD flag = 0;
	int nRet = WSARecv(m_sock, &m_rio.m_wsabuf, 1, &trans, &flag, &m_rio.m_over, NULL);
	if (nRet == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		return false;
	return true;
}

bool Session::SendMsg(int size, char* pData)
{
	auto sio = new IoData;
	ZeroMemory(sio, sizeof(IoData));
	sio->m_iotype = IoType::Send;
	sio->m_wsabuf.buf = new char[size];
	sio->m_wsabuf.len = size;
	CopyMemory(sio->m_wsabuf.buf, pData, size);

	lock_guard<mutex> guard(m_sendlock);
	m_sendque.push(sio);
	if (m_sendque.size() == 1)
		PacketSend();
	return true;
}

bool Session::PacketSend()
{
	auto sio = m_sendque.front();
	DWORD trans;
	int nRet = WSASend(m_sock, &sio->m_wsabuf, 1, &trans, 0, &sio->m_over, NULL);
	if (nRet == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		return false;
	return true;
}

void Session::SendCompleted()
{
	lock_guard<mutex> guard(m_sendlock);
	delete[] m_sendque.front()->m_wsabuf.buf;
	delete m_sendque.front();
	m_sendque.pop();

	if (m_sendque.empty() == false)
		PacketSend();
}