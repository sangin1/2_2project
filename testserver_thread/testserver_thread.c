#pragma warning(disable:4996)
#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <process.h>  //������ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h> //���� 
#include <Windows.h>  
#define BUF_SIZE 100
#define MAX_CLNT 256

unsigned WINAPI client_con(void* arg); //����������� �Ű������� �ƱԸ�Ʈ�� �޴´�.
void all_msg(char* msg, int len); // �޼��� ���� �Լ�

int cli_count = 0;  //���ӵ� Ŭ���̾�Ʈ�� ī��Ʈ
SOCKET cli_socket[MAX_CLNT]; //Ŭ���̾�Ʈ ���� ���� ���� �迭
HANDLE hMutex; //�����带 ������ ȣ��� �����ڿ������� �����ϱ� ���� ���ؽ� ����

int main() {
	WSADATA wsaData;  //���� �ʱ�ȭ �Լ��� ���� ����
	SOCKET serverSock, clientSock;  //���� ����ü
	SOCKADDR_IN serverAddr, clientAddr; //�����ּ� ����ü
	int clientAddrSize;
	HANDLE hThread; //������ ����ü

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){ //���� �ʱ�ȭ(ws2_32.lib ���̺귯�� �ʱ�ȭ �� ��������) (2,2)�� ���� ����
		printf("wasstartup error");
		exit(1);
	}
	hMutex = CreateMutex(NULL, FALSE, NULL); //���ؽ� ����
	serverSock = socket(PF_INET, SOCK_STREAM, 0); //���� ����
	memset(&serverAddr, 0, sizeof(serverAddr)); //serverAddr �ʱ�ȭ
	serverAddr.sin_family = AF_INET;   //������
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); //�ּ� (����������ǻ���� �ּҷ� �ڵ� �Է�)
	serverAddr.sin_port = htons(4000); //��Ʈ ����

	if (bind(serverSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {  //���Ͽ� �ּ� �Ҵ�
		printf("bind error");
		exit(1);
	}
	if (listen(serverSock, 5) == SOCKET_ERROR) {   //�ٸ� ������ ���� �غ�
		printf("listen error");
		exit(1);
	}
	while (1) {
		clientAddrSize = sizeof(clientAddr);
		clientSock = accept(serverSock, (SOCKADDR*)&clientAddr, &clientAddrSize); //Ŭ���̾�Ʈ ���� �ޱ�(����)
		WaitForSingleObject(hMutex, INFINITE);  //������ ���ؽ���ɾ ���������� ���
		cli_socket[cli_count++] = clientSock;   //���ؽ��� �ؾ� ���� Ŭ���̾�Ʈ�� ���� ����(�ڿ�)�� ����ҋ� ��ġ�� �ʴ´�.
		ReleaseMutex(hMutex); //���ؽ� ����
		hThread = (HANDLE)_beginthreadex(NULL, 0, client_con, (void*)&clientSock, 0, NULL); //������ ����(������ �Լ��� ����)
		printf("������ IP: %s\n", inet_ntoa(clientAddr.sin_addr));
	}
	closesocket(serverSock);
	WSACleanup(); //���ϻ�� ���� �ý��� ��
	return 0;
}
unsigned WINAPI client_con(void* arg) {
	SOCKET clientSock = *((SOCKET*)arg);  //�Ű������� ���� ���� ����
	int strLen = 0, i;
	char msg[BUF_SIZE];

	while ((strLen = recv(clientSock, msg, sizeof(msg), 0)) != 0) {  //Ŭ���̾�Ʈ�κ��� �޼����� �޴´ٸ�(����Ǿ� �ִٸ�)
		all_msg(msg, strLen); //��� ����� Ŭ���̾�Ʈ�� �޼��� ���� �Լ�
	}
	WaitForSingleObject(hMutex, INFINITE); 
	for (i = 0;i < cli_count;i++) {
		if (clientSock == cli_socket[i]) {//���� ���� clientSock���� �迭�� ���� ���ٸ�
			while (i++ < cli_count - 1)//Ŭ���̾�Ʈ ���� ��ŭ
				cli_socket[i] = cli_socket[i + 1];//������ �����. 
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