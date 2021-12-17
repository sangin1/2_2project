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

unsigned WINAPI SendMsg(void* arg);//������ �����Լ�
unsigned WINAPI RecvMsg(void* arg);//������ �����Լ�
  
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
    printf("ip �Է�");
    gets(IP);
    printf("port �Է�");
    scanf("%s", &port);
    sock = socket(PF_INET, SOCK_STREAM, 0);//���� ����

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(IP);
    serverAddr.sin_port = htons(atoi(port));

    if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {//������ ���� 
        printf("connect error");
        exit(1);
    } 

    sendThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&sock, 0, NULL);// ���� �����尡 ���� .
    recvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&sock, 0, NULL);// ���� �����尡 ���� .

    WaitForSingleObject(sendThread, INFINITE);//���� �����尡 �����ɶ����� ��ٸ���./
    WaitForSingleObject(recvThread, INFINITE);//���� �����尡 �����ɶ����� ��ٸ���. 
    closesocket(sock);//���� ����  
    WSACleanup(); 
    return 0;
}

unsigned WINAPI SendMsg(void* arg) {//���ۿ� �������Լ�
    SOCKET sock = *((SOCKET*)arg);//������ ������ �����Ѵ�.
    char nameMsg[NAME_SIZE + BUF_SIZE];
    while (1) {//�ݺ�
        fgets(msg, BUF_SIZE, stdin);//�Է��� �޴´�.
        if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {//q�� �Է��ϸ� �����Ѵ�.
            closesocket(sock);
            exit(0);
        }
        send(sock, msg, strlen(nameMsg), 0);//nameMsg�� �������� �����Ѵ�.
    }
    return 0;
}

unsigned WINAPI RecvMsg(void* arg) {
    SOCKET sock = *((SOCKET*)arg);//������ ������ �����Ѵ�.
    char nameMsg[NAME_SIZE + BUF_SIZE];
    int strLen;
    while (1) {//�ݺ�
        strLen = recv(sock, nameMsg, NAME_SIZE + BUF_SIZE - 1, 0);//�����κ��� �޽����� �����Ѵ�.
        if (strLen == -1)
            return -1;
        nameMsg[strLen] = 0;//���ڿ��� ���� �˸��� ���� ����
        fputs(nameMsg, stdout);//�ڽ��� �ֿܼ� ���� �޽����� ����Ѵ�.
    }
    return 0;
}
 