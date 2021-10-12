# CriticalSection 
+ 2021.10.07 네트워크 수업

## CriticalSection 코드
크리티컬 섹션에서 키 반납하지 않고 return하는 코드
```C++
#include <WinSock2.h>
#include <process.h>
#include <iostream>

using std::cout;
using std::endl;

int counter;

CRITICAL_SECTION cs; //임계영역 키 생성

unsigned __stdcall func(LPVOID param) {
	int start = *((int*)param);
	int end = *((int*)param + 1);

	for (int i = start; i < end; i++) {

		// 키 획득
		EnterCriticalSection(&cs);

		counter++;
		cout << counter << endl;

		// 키 반납X
		//LeaveCriticalSection(&cs);
	}

	return 0;
}

int main() {
	HANDLE th[16]; // CPU의 스레드 수를 저장하기 위한 배열 선언
	int arg[] = { 0, 100000 }; // 스레드 당 십만번의 연산을 위해 범위 지정

	//생성된 키를 초기화
	InitializeCriticalSection(&cs);

	SYSTEM_INFO info;
	GetSystemInfo(&info); // 컴퓨터의 스레드 수를 저장

	cout << "Number of Processors is" << info.dwNumberOfProcessors << endl; // 화면에 컴퓨터의 스레드 수 출력

	clock_t stime = clock(); // 연산에 걸린 시간 계산하기 위해 시작 시간 저장

	for (DWORD i = 0; i < info.dwNumberOfProcessors; i++) // 스레드의 수만큼 반복문 실행
		th[i] = (HANDLE)_beginthreadex(NULL, 0, &func, arg, 0, NULL);

	WaitForMultipleObjects(info.dwNumberOfProcessors, th, true, INFINITE);

	clock_t etime = clock(); // 연산에 걸린 시간 계산하기 위해 종료 시간 저장


	// 연산에 걸린 시간 = 종료시간 - 시작시간
	cout << "main:" << counter << ", time : " << (double)(etime - stime) / CLOCKS_PER_SEC << endl;

	return 0;
}
```

## 실행 결과
![Alt text](./img/mutex.PNG)

+ 키를 반납하지 않아 1번 스레드의 연산이 끝난 후 다른 스레드가 동작하지 않는다.