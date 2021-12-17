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
typedef struct soc_tel {
    int x;
    int y;
    char tel_name[12];
    int check;
}soc_tel;
int SendMsg(SOCKET sock);//������ �����Լ�

char msg[BUF_SIZE];

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    SOCKET sock;
    SOCKADDR_IN serverAddr;
    char IP[15] = "127.0.0.1", port[1024] = "4000";
    char menu[2][20] = { "�ý� �θ���","����" };
    int i,a;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("wsastartup error");
        exit(1);
    }
    sock = socket(PF_INET, SOCK_STREAM, 0);//���� ����

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(IP);
    serverAddr.sin_port = htons(atoi(port));
    if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {//������ ���� 
        printf("connect error");
        exit(1);
    }
    while (1) {
        system("CLS");
        printf("-----------------------------------------\n");
        printf("          �ý� ���� ���α׷�\n");
        for (i = 0;i < 2;i++) {
            printf("%d. %s\n", i+1, menu[i]);
        }
        printf("\n �Է� :");
        fflush(stdout);
        scanf("%d", &a);
        if (a == 2) {
            closesocket(sock);//���� ����  
            WSACleanup();
            return 0;
        }
        else if (a == 1) {
            system("CLS"); 
            SendMsg(sock);// ���� �Լ� .   
        }
    } 
    return 0;
}

int SendMsg(SOCKET sock) {//���ۿ� �������Լ�
    soc_tel soc_tel_cli;
    int x, y;
    char tel_name[12];

    printf("���� ��� ���� �Է��� �ּ���(x����y ��ǥ �Է�)");
    fflush(stdout);
    scanf("%d %d", &x, &y);
    printf("��ȭ��ȣ�� �Է��� �ּ���.(-���� 11�ڸ�)");
    fflush(stdout);
    scanf("%s", tel_name);

    soc_tel_cli.check = 0;
    soc_tel_cli.x = x;
    soc_tel_cli.y = y;
    sprintf(soc_tel_cli.tel_name, tel_name);

    send(sock, (char*)&soc_tel_cli, sizeof(soc_tel_cli), 0);//nameMsg�� �������� �����Ѵ�.
    
    return 0;
}