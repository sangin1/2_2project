#pragma warning(disable:4996)
#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <Windows.h>
#include <process.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

unsigned WINAPI SendMsg(void* arg);//쓰레드 전송함수
unsigned WINAPI RecvMsg(void* arg);//쓰레드 수신함수
  
char msg[BUF_SIZE];

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    SOCKET sock;
    SOCKADDR_IN serverAddr;
    HANDLE sendThread, recvThread; 
    char IP[15], port[1024]; 
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("wsastartup error");
        exit(1);
    }
    printf("ip 입력");
    gets(IP);
    printf("port 입력");
    scanf("%s", &port);
    sock = socket(PF_INET, SOCK_STREAM, 0);//소켓 생성

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(IP);
    serverAddr.sin_port = htons(atoi(port));

    if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {//서버에 접속 
        printf("connect error");
        exit(1);
    } 

    sendThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&sock, 0, NULL);// 전송 쓰레드가 실행 .
    recvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&sock, 0, NULL);// 수신 쓰레드가 실행 .

    WaitForSingleObject(sendThread, INFINITE);//전송 쓰레드가 중지될때까지 기다린다./
    WaitForSingleObject(recvThread, INFINITE);//수신 쓰레드가 중지될때까지 기다린다. 
    closesocket(sock);//소켓 종료  
    WSACleanup(); 
    return 0;
}

unsigned WINAPI SendMsg(void* arg) {//전송용 쓰레드함수
    SOCKET sock = *((SOCKET*)arg);//서버용 소켓을 전달한다.
    char nameMsg[NAME_SIZE + BUF_SIZE];
    while (1) {//반복
        fgets(msg, BUF_SIZE, stdin);//입력을 받는다.
        if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {//q를 입력하면 종료한다.
            closesocket(sock);
            exit(0);
        }
        send(sock, msg, strlen(nameMsg), 0);//nameMsg를 서버에게 전송한다.
    }
    return 0;
}

unsigned WINAPI RecvMsg(void* arg) {
    SOCKET sock = *((SOCKET*)arg);//서버용 소켓을 전달한다.
    char nameMsg[NAME_SIZE + BUF_SIZE];
    int strLen;
    while (1) {//반복
        strLen = recv(sock, nameMsg, NAME_SIZE + BUF_SIZE - 1, 0);//서버로부터 메시지를 수신한다.
        if (strLen == -1)
            return -1;
        nameMsg[strLen] = 0;//문자열의 끝을 알리기 위해 설정
        fputs(nameMsg, stdout);//자신의 콘솔에 받은 메시지를 출력한다.
    }
    return 0;
}
 