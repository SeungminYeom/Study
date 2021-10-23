# Multicast Receiver
2021.10.21 네트워크 수업

```c++
#pragma comment (lib,"ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

using std::cout;
using std::endl;

void err_display(const char*);
void Insert_IP(PCSTR, PVOID);

int main()
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa)) //ws2_32.dll 초기화
	{
		err_display("WSAStartup");
		return -1;
	}

	//소켓 생성
	SOCKET s_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s_sock == INVALID_SOCKET)
	{
		err_display("socket()");
		return -1;
	}

	//local address
	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8000);
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(s_sock, (SOCKADDR*)&saddr, sizeof(saddr))) // bind() != 0
	{
		err_display("bind()");
		return -1;
	}
	//bind() : LocalAddress(IP ADDR + Port NUM) + Socket 의 Association
	//bind가 필요로 하는 Parameter = Socket Descripter, Local Address(SOCKADDR_IN), Size

	IP_MREQ mreq;
	Insert_IP("236.0.0.1", &mreq.imr_multiaddr); // inet_pton(AF_INET, "236.0.0.1", &mreq.imr_multiaddr);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY); // Insert_IP("127.0.0.1", &mreq.imr_interface);
	if (setsockopt(s_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) == SOCKET_ERROR)
	{
		err_display("setsockopt(add_membership)");
		return -1;
	}

	SOCKADDR_IN caddr;
	int namelen = sizeof(caddr);

	char buf[80];
	int recvlen;

	while (1)
	{
		//수신
		recvlen = recvfrom(s_sock, buf, 80, 0, (SOCKADDR*)&caddr, &namelen);
		if (recvlen == 0)
		{
			cout << "client connection normal close case" << endl;
			err_display("recvform()");
			break;
		}
		if (recvlen == SOCKET_ERROR)
		{
			cout << "client connection abnormal(RST) close case" << endl;
			err_display("recvform()");
			break;
		}

		//수신 데이터를 화면에 출력
		//수신 데이터를 문자열로 만들기 위해 강제적으로 마지막에 널 문자 입력
		buf[recvlen] = '\0';
		cout << buf << endl;
	}

	closesocket(s_sock);

	WSACleanup();
	return 0;
}

void err_display(const char* mes)
{
	LPVOID out;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
		WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&out, 0, NULL);
	cout << mes << " : " << (LPSTR)out << endl;
	LocalFree(out);
}

void Insert_IP(PCSTR str, PVOID addr) {
	if (InetPton(AF_INET, str, addr) != 1) {
		err_display("InetPtons()");
	}
}
```