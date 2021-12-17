#pragma warning(disable:4996)
#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <process.h>  //쓰레드 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h> //소켓 
#include <Windows.h>  
#define BUF_SIZE 100
#define MAX_CLNT 256

unsigned WINAPI client_con(void* arg); //쓰레드생성시 매개변수는 아규먼트로 받는다.
void all_msg(char* msg, int len); // 메세지 전송 함수

int cli_count = 0;  //접속된 클라이언트수 카운트
SOCKET cli_socket[MAX_CLNT]; //클라이언트 소켓 정보 저장 배열
HANDLE hMutex; //쓰레드를 여러개 호출시 공유자원문제를 방지하기 위한 뮤텍스 생성

int main() {
	WSADATA wsaData;  //윈속 초기화 함수를 위한 변수
	SOCKET serverSock, clientSock;  //소켓 구조체
	SOCKADDR_IN serverAddr, clientAddr; //소켓주소 구조체
	int clientAddrSize;
	HANDLE hThread; //쓰레드 구조체

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){ //윈속 초기화(ws2_32.lib 라이브러리 초기화 및 가져오기) (2,2)는 윈속 버전
		printf("wasstartup error");
		exit(1);
	}
	hMutex = CreateMutex(NULL, FALSE, NULL); //뮤텍스 생성
	serverSock = socket(PF_INET, SOCK_STREAM, 0); //소켓 생성
	memset(&serverAddr, 0, sizeof(serverAddr)); //serverAddr 초기화
	serverAddr.sin_family = AF_INET;   //연결방식
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); //주소 (서버실행컴퓨터의 주소로 자동 입력)
	serverAddr.sin_port = htons(4000); //포트 설정

	if (bind(serverSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {  //소켓에 주소 할당
		printf("bind error");
		exit(1);
	}
	if (listen(serverSock, 5) == SOCKET_ERROR) {   //다른 소켓을 받을 준비
		printf("listen error");
		exit(1);
	}
	while (1) {
		clientAddrSize = sizeof(clientAddr);
		clientSock = accept(serverSock, (SOCKADDR*)&clientAddr, &clientAddrSize); //클라이언트 소켓 받기(연결)
		WaitForSingleObject(hMutex, INFINITE);  //릴리즈 뮤텍스명령어를 받을떄까지 대기
		cli_socket[cli_count++] = clientSock;   //뮤텍스를 해야 여러 클라이언트가 같은 변수(자원)을 사용할떄 겹치지 않는다.
		ReleaseMutex(hMutex); //뮤텍스 중지
		hThread = (HANDLE)_beginthreadex(NULL, 0, client_con, (void*)&clientSock, 0, NULL); //쓰래드 생성(쓰래드 함수를 실행)
		printf("접속한 IP: %s\n", inet_ntoa(clientAddr.sin_addr));
	}
	closesocket(serverSock);
	WSACleanup(); //소켓사용 종료 시스템 콜
	return 0;
}
unsigned WINAPI client_con(void* arg) {
	SOCKET clientSock = *((SOCKET*)arg);  //매개변수로 받은 정보 저장
	int strLen = 0, i;
	char msg[BUF_SIZE];

	while ((strLen = recv(clientSock, msg, sizeof(msg), 0)) != 0) {  //클라이언트로부터 메세지를 받는다면(연결되어 있다면)
		all_msg(msg, strLen); //모든 연결된 클라이언트로 메세지 전달 함수
	}
	WaitForSingleObject(hMutex, INFINITE); 
	for (i = 0;i < cli_count;i++) {
		if (clientSock == cli_socket[i]) {//만약 현재 clientSock값이 배열의 값과 같다면
			while (i++ < cli_count - 1)//클라이언트 개수 만큼
				cli_socket[i] = cli_socket[i + 1];//앞으로 땡긴다. 
		}
	}
	cli_count--;
	ReleaseMutex(hMutex);
	closesocket(clientSock);
	return 0;
}

void all_msg(char* msg, int len) {
	int i;
	WaitForSingleObject(hMutex, INFINITE);
	for (i = 0;i < cli_count;i++) {
		send(cli_socket[i], msg, len, 0);
	}
	ReleaseMutex(hMutex);
}