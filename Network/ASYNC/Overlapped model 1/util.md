# util.cpp

```C++
#include "Overlapped1.h";

#define lock(a) EnterCriticalSection(&a)
#define unlock(a) LeaveCriticalSection(&a)

bool Server_Init(Server* ser) {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa)) {
		err_display("WSAStartup");
		return false;
	}

	SOCKET s_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s_sock == INVALID_SOCKET) {
		err_display("socket");
		return false;
	}

	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8000);
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(s_sock, (SOCKADDR*)&saddr, sizeof(saddr))) {
		err_display("bind");
		return false;
	}

	//Server의 데이터 초기화
	ser->listen_sock = s_sock;
	ser->noc = 0;
	InitializeCriticalSection(&ser->cs);

	return true;
}

//세션 생성 함수
Session* Add_Session(Server* ser, SOCKET s) {
	Session* ses = new Session;

	//Session 정보 초기화
	ses->rwsa.buf = ses->rbuf;
	ses->swsa.buf = ses->sbuf;

	ses->sock = s;
	ses->server = ser;

	ses->event = WSACreateEvent();

	//Server 정보 업데이트
	lock(ser->cs);
	ser->sarr.push_back(ses); //서버에 방금 만든 세션 포인터를 추가
	ser->earr.push_back(ses->event); //이벤트 객체 추가
	ser->noc++;
	unlock(ser->cs);

	return ses;
}

void Close_Session(Session* ses) {
	//socket, event handle 값 백업
	SOCKET s = ses->sock;
	WSAEVENT e = ses->event;

	Server* ser = ses->server;
	lock(ser->cs);

	//세션 정보 제거
	ser->sarr.erase(find(ser->sarr.begin(), ser->sarr.end(), ses));
	//event 객체 제가
	ser->earr.erase(find(ser->earr.begin(), ser->earr.end(), e));
	//감시해야 할 이벤트 객체가 하나 줄음
	ser->noc--;

	unlock(ser->cs);

	closesocket(s);
	WSACloseEvent(e);
}

//Packet 수신 함수
void Clean_Session(Session* ses, int size, IOType type) {
	ZeroMemory(&ses->over, sizeof(ses->over)); //0으로 초기화
	ses->over.hEvent = ses->event;

	if (type == IOType::Read) {
		ses->rwsa.len = size - 1;
		ses->handle = Read_Process;
	}
	else {
		ses->swsa.len = size;
		ses->handle = Write_Process;
	}
}

void Packet_Recv(Session* ses) {
	Clean_Session(ses, 80, IOType::Read);

	DWORD trans;
	DWORD flag = 0;
	int ret = WSARecv(ses->sock, &ses->rwsa, 1, &trans, &flag, &ses->over, NULL);
	if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		err_display("WSARecv");
}

void Packet_Send(Session* ses, int size) {
	Clean_Session(ses, size, IOType::Write);

	DWORD trans;
	int ret = WSASend(ses->sock, &ses->rwsa, 1, &trans, 0, &ses->over, NULL);
	if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		err_display("WSASend");
}

//에코 서비스 관련 함수
void Read_Process(Session* ses, DWORD trans) {
	if (trans == 0) {
		cout << "normal clase case" << endl;
		Close_Session(ses);
		return;
	}
	//수신 데이터의 마지막에 널문자를 넣어 문자열로 만듦
	ses->rbuf[trans] = '\0';
	cout << ses->rbuf << endl;
	
	//수신 데이터를 클라이언트에 다시 보냄
	CopyMemory(ses->sbuf, ses->rbuf, trans);
	Packet_Send(ses, trans);
}

void Write_Process(Session* ses, DWORD trans) {
	Packet_Recv(ses);
}

void err_display(const char* mes)
{
	LPVOID out;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
		WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&out, 0, NULL);
	cout << mes << " : " << (LPSTR)out << endl;
	LocalFree(out);
}
```