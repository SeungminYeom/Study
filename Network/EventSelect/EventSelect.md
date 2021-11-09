# EventSelect
2021.11.04 네트워크 수업


``` C++
#pragma comment (lib,"ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

using std::cout;
using std::endl;

void err_display(const char*);
void Print_IP(LPVOID);
void Add_Session(int);
void Close_Session(int);
void Echo_Serv(int);

SOCKET sarr[10];
WSAEVENT earr[10];

int noc; //소켓의 갯수 == 이벤트 객체의 갯수

int main() {
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa)) { // ws2_32.dll 초기화
        err_display("WSAStartup");
        return -1;
    }

    //소켓 생성
    SOCKET s_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (s_sock == INVALID_SOCKET) {
        err_display("socket()");
        return -1;
    }


    SOCKADDR_IN saddr; //local address
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8000);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    //socket option REUSEADDR

    if (bind(s_sock, (SOCKADDR*)&saddr, sizeof(saddr))) {  // bind() != 0 
        err_display("bind()");
        return -1;
    }

    //소켓과 이벤트 객체 배열 정리
    sarr[noc] = s_sock; //문지기 소켓 배열 처리
    earr[noc] = WSACreateEvent(); //이벤트 객체 생성 및 배열 정리
    WSAEventSelect(sarr[noc], earr[noc], FD_ACCEPT); //소켓과 이벤트 객체 짝짓기 및 이벤트 설정
    noc++;

    //listen : 소켓을 listen 상태로 만들고, 동시에 listen buffer를 생성
    if (listen(s_sock, SOMAXCONN)) {
        err_display("listen()");
        return -1;
    }

    int ret1, ret2;
    WSANETWORKEVENTS event;
    while (1) {
        //wait function - WWFME : 이벤트 객체의 signalled 상태 체크
        ret1 = WSAWaitForMultipleEvents(noc, earr, false, WSA_INFINITE, false);
        if (ret1 == WSA_WAIT_FAILED) {
            cout << "WSAWFME failed case" << endl;
            continue;
        }
        ret1 -= WSA_WAIT_EVENT_0;

        for (int idx = ret1; idx < noc; idx++) {
            ret2 = WSAWaitForMultipleEvents(1, &earr[idx], true, 0, false);
            if (ret2 == WSA_WAIT_TIMEOUT) continue;

            //네트워크 이벤트는 발생했으므로 해당 네트워크 이벤트를 자세히 조사
            if (WSAEnumNetworkEvents(sarr[idx], earr[idx], &event) == SOCKET_ERROR) {
                cout << "WSAENE error" << endl;
                err_display("WSAEnumNetworkEvents");
                continue;
            }
            //accept 처리
            if (event.lNetworkEvents & FD_ACCEPT) {
                if (event.iErrorCode[FD_ACCEPT_BIT] != 0) {
                    cout << "accept error" << endl;
                    break;
                }

                Add_Session(idx);
                continue;
            }
            if (event.lNetworkEvents & FD_READ) {
                if (event.iErrorCode[FD_READ_BIT] != 0) {
                    cout << "FD_READ error" << endl;
                    err_display("FD_READ");
                    closesocket(s_sock);
                    Close_Session(idx);
                    continue;
                }
                Echo_Serv(idx);
                continue;
            }
            //연결 종료 처리
            if (event.lNetworkEvents & FD_CLOSE) {
                if (event.iErrorCode[FD_CLOSE_BIT] != 0) {
                    cout << "abnormal close case" << endl;
                    Close_Session(idx);
                    break;
                }

                //정상 종료
                cout << "normal close case" << endl;
                Close_Session(idx);
                continue;
            }
        }   //for
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

void Print_IP(LPVOID addr)
{
    char buf[20];

    if (!inet_ntop(AF_INET, addr, buf, 20))  // if(inetntop() == null)
    {
        err_display("inetntop");
        return;
    }

    cout << "ip addr : " << buf << endl;
}

void Add_Session(int idx) {
    SOCKADDR_IN caddr;
    int namelen = sizeof(caddr);

    SOCKET s = accept(sarr[idx], (SOCKADDR*)&caddr, &namelen); //IP와 port로 뭘 할 땐 s의 getpeer로 얻는다.
    sarr[noc] = s; //소켓 처리
    earr[noc] = WSACreateEvent();
    WSAEventSelect(sarr[noc], earr[noc], FD_READ | FD_CLOSE);
    noc++;
}

void Close_Session(int idx) {
    //제거하려는 소켓/이벤트 객체 백업
    SOCKET s = sarr[idx];
    WSAEVENT e = earr[idx];

    //해당 소켓과 이벤트 객체 제거
    if (idx != noc - 1) {
        sarr[idx] = sarr[noc - 1];
        earr[idx] = earr[noc - 1];
    }
    closesocket(s);
    WSACloseEvent(e);
    noc--;
}

void Echo_Serv(int idx) {
    char buf[80];
    int recvlen;

    recvlen = recv(sarr[idx], buf, 80, 0);
    buf[recvlen] = '\0';
    cout << "client : " << buf << endl;
    send(sarr[idx], buf, recvlen, 0);
}
```