# main.cpp

```C++
#pragma comment(lib, "ws2_32.lib")
#include "Overlapped1.h";
#include <process.h>

int main() {
	Server server;
	if (Server_Init(&server) == false) {
		cout << "Server_Init error" << endl;
		return -1;
	}

	_beginthreadex(NULL, 0, ProActor, &server, 0, NULL);

	if (listen(server.listen_sock, SOMAXCONN)) {
		err_display("listen");
		return -1;
	}

	SOCKADDR_IN caddr;
	int namelen = sizeof(caddr);
	SOCKET c_sock;

	Session* ses = NULL;
	while (1) {
		c_sock = accept(server.listen_sock, (SOCKADDR*)&caddr, &namelen);
		if (c_sock == INVALID_SOCKET) {
			err_display("accept");
			break;
		}

		//Session 생성
		ses = Add_Session(&server, c_sock);

		//Packet 수신
		Packet_Recv(ses);

	}



	closesocket(server.listen_sock);


	return 0;
}
```