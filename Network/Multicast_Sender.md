# Multicast Sender
2021.10.21 네트워크 수업

```c++
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

using std::cout;
using std::endl;

void err_display(const char*);
void Insert_IP(PCSTR, PVOID);

int main() {
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa))
	{
		err_display("WSAStartup");
		return -1;
	}


	// 소켓 생성
	SOCKET c_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (c_sock == INVALID_SOCKET)
	{
		err_display("socket()");
		return -1;
	}

	// 서버의 IP주소 및 포트 지정
	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8000);
	Insert_IP("236.0.0.1", &saddr.sin_addr);

	int ttl = 0;
	int ttlSize = sizeof(ttl);

	if (getsockopt(c_sock, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, &ttlSize) == SOCKET_ERROR)
	{
		err_display("getsockopt(ttl)");
		return -1;
	}
	cout << ttl << endl;

	ttl = 16;
	if (setsockopt(c_sock, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl)) == SOCKET_ERROR)
	{
		err_display("setsockopt(ttl)");
		return -1;
	}

	// 파일 열기
	FILE* fptr = NULL;
	fopen_s(&fptr, "news.txt", "r");
	if (fptr == NULL) {
		cout << "File not found" << endl;
		return -1;
	}

	char buf[80];
	int sendlen;
	while (1)
	{
		fgets(buf, 79, fptr);
		// 문장을 입력하고 리턴을 하면 \n 붙고, 자동으로 \0 입력됨

		sendlen = strlen(buf); // \0까지의 길이
		sendto(c_sock, buf, sendlen, 0, (SOCKADDR*)&saddr, sizeof(saddr));

		//file의 끝에 fptr이 있으면 종료시킴
		if (feof(fptr)) {
			//tcp는 안되고 udp만 가능
			sendto(c_sock, buf, 0, 0, (SOCKADDR*)&saddr, sizeof(saddr));
			break;
		}
		Sleep(500); // 밀리세컨드, 1초
	}



	closesocket(c_sock);

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