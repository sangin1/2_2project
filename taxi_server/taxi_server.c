#pragma warning(disable:4996)
#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <process.h>  //������ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h> //���� 
#include <Windows.h>  
#include <math.h>
#define BUF_SIZE 100
#define MAX_CLNT 256

typedef struct soc_tel {
	int x;
	int y;
	char tel_name[12];
	int check;
}soc_tel;
typedef struct ser_soc {
	soc_tel s_t;
	SOCKET cli_socket;
}ser_soc;

typedef struct Node  
{
	ser_soc data;
	struct Node* next;
}Node;

typedef struct ListNode
{
	ser_soc data;
	struct ListNode* next;
	struct ListNode* prev;
}ListNode;

typedef struct Queue  
{
	Node* front;  
	Node* rear;  
	int count; 
}Queue;

void InitQueue(Queue* queue);//ť �ʱ�ȭ
int IsEmpty(Queue* queue); //ť�� ������� Ȯ��
void Enqueue(Queue* queue, ser_soc data); //ť�� ����
ser_soc Dequeue(Queue* queue); //ť���� ����
unsigned WINAPI client_con(void* arg); //����������� �Ű������� �ƱԸ�Ʈ�� �޴´�. 
unsigned WINAPI search(void* arg); 

int cli_count = 0;  //���ӵ� Ŭ���̾�Ʈ�� ī��Ʈ
ser_soc cli_socket[MAX_CLNT]; //Ŭ���̾�Ʈ ���� ���� ���� �迭
HANDLE hMutex; //�����带 ������ ȣ��� �����ڿ������� �����ϱ� ���� ���ؽ� ����

int ListSize = 0;
ListNode* head;
ListNode* tail; 
void add_after(ListNode* pre, ser_soc item);
void removed(ListNode* p) {
	if (p->prev == NULL && p->next == NULL)	// p�� ������ ���
	{
		head = NULL;
		tail = NULL;
		free(p);
		ListSize--;
	}
	else if (p->prev == NULL)	// p�� head�� ���
	{
		head = p->next;
		head->prev = NULL;
		free(p);
		ListSize--;
	}
	else if (p->next == NULL)	// p�� tail�� ���
	{
		tail = p->prev;
		tail->next = NULL;
		free(p);
		ListSize--;
	}
	else	// p�� �߰��� ��ġ�� ����� ���
	{
		p->prev->next = p->next;
		p->next->prev = p->prev;
		free(p);
		ListSize--;
	}
}

Queue queue;
int main() {
	WSADATA wsaData;  //���� �ʱ�ȭ �Լ��� ���� ����
	SOCKET serverSock, clientSock;  //���� ����ü
	SOCKADDR_IN serverAddr, clientAddr; //�����ּ� ����ü
	int clientAddrSize;
	HANDLE hThread; //������ ����ü 
	HANDLE sThread;
	InitQueue(&queue);
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { //���� �ʱ�ȭ(ws2_32.lib ���̺귯�� �ʱ�ȭ �� ��������) (2,2)�� ���� ����
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
	sThread = (HANDLE)_beginthreadex(NULL, 0, search, (void*)&clientSock, 0, NULL); //������ ����(������ �Լ��� ����)
	while (1) {
		clientAddrSize = sizeof(clientAddr);
		clientSock = accept(serverSock, (SOCKADDR*)&clientAddr, &clientAddrSize); //Ŭ���̾�Ʈ ���� �ޱ�(����)
		WaitForSingleObject(hMutex, INFINITE);  //������ ���ؽ���ɾ ���������� ���
		cli_socket[cli_count++].cli_socket = clientSock;   //���ؽ��� �ؾ� ���� Ŭ���̾�Ʈ�� ���� ����(�ڿ�)�� ����ҋ� ��ġ�� �ʴ´�.
		ReleaseMutex(hMutex); //���ؽ� ����
		hThread = (HANDLE)_beginthreadex(NULL, 0, client_con, (void*)&clientSock, 0, NULL); //������ ����(������ �Լ��� ����)
	}
	closesocket(serverSock);
	WSACleanup(); //���ϻ�� ���� �ý��� ��
	return 0;
}
unsigned WINAPI client_con(void* arg) {
	SOCKET clientSock = *((SOCKET*)arg);  //�Ű������� ���� ���� ����
	int strLen = 0, i;
	char massage[BUF_SIZE];
	soc_tel *cli_soc;
	ser_soc cliser_soc;
	while ((strLen = recv(clientSock, massage, sizeof(massage), 0)) != 0) {  //Ŭ���̾�Ʈ�κ��� �޼����� �޴´ٸ�(����Ǿ� �ִٸ�)
		massage[strLen] = '\0';
		cli_soc = (soc_tel*)massage;
		cliser_soc.cli_socket = clientSock;
		cliser_soc.s_t = *cli_soc;
		if (cli_soc->check == 0) {
			Enqueue(&queue, cliser_soc);//��Ŭ���̾�Ʈ���� ���� ������ ť�� ���� 
		}
		else if (cli_soc->check == 1) {
			add_after(NULL, cliser_soc);			
		}
	}
	WaitForSingleObject(hMutex, INFINITE);
	for (i = 0;i < cli_count;i++) {
		if (clientSock == cli_socket[i].cli_socket) {//���� ���� clientSock���� �迭�� ���� ���ٸ�
			while (i++ < cli_count - 1)//Ŭ���̾�Ʈ ���� ��ŭ
				cli_socket[i] = cli_socket[i + 1];//������ �����. 
		}
	}
	cli_count--;
	ReleaseMutex(hMutex);
	closesocket(clientSock);
	return 0;
}
unsigned WINAPI search(void* arg) {
	ser_soc cli_data;
	int i,check=0,st1,st2;
	double check_street=100000,a;
	ListNode* search_list;
	while (1) {
		check_street = 100000;
		check = 0;
		if (queue.count != 0 && ListSize != 0) {
			cli_data = Dequeue(&queue);
			search_list = head;
			if (ListSize != 1) {
				for (i = 1;i <= ListSize;i++) {
					st1 = cli_data.s_t.x - search_list->data.s_t.x;
					st2 = cli_data.s_t.y - search_list->data.s_t.y;
					a = sqrt(pow(st1, st1) + pow(st2, st2));
					if (check_street > a) {
						check_street = a;
						check = i;
					}
					search_list = search_list->next;
				}
				search_list = head;
				for (i = 1;i <= check;i++) {
					if (i != 1) {
						search_list = search_list->next;
					}
				}
			}
			send(search_list->data.cli_socket, (char*)&cli_data, sizeof(cli_data), 0);
			removed(search_list);
		}
	}
}
void InitQueue(Queue* queue)
{
	queue->front = queue->rear = NULL;  
	queue->count = 0; 
}

int IsEmpty(Queue* queue)
{
	return queue->count == 0;     
}

void Enqueue(Queue* queue, ser_soc data)
{
	Node* now = (Node*)malloc(sizeof(Node));  
	now->data = data; 
	now->next = NULL;

	if (IsEmpty(queue))//ť�� ������� ��
	{
		queue->front = now;//�� ���� now�� ����       
	}
	else//������� ���� ��
	{
		queue->rear->next = now;//�� ���� ������ now�� ����
	}
	queue->rear = now;//�� �ڸ� now�� ����   
	queue->count++;//���� ������ 1 ����
}

ser_soc Dequeue(Queue* queue)
{
	ser_soc re;
	Node* now;
	if (IsEmpty(queue))//ť�� ����� ��
	{
		printf("queue error");
	}
	now = queue->front;//�� ���� ��带 now�� ���
	re = now->data;//��ȯ�� ���� now�� data�� ����
	queue->front = now->next;//�� ���� now�� ���� ���� ����
	free(now);//now �Ҹ�
	queue->count--;//���� ������ 1 ����
	return re;
}
void add_after(ListNode* pre, ser_soc item)
{
	ListNode* new_node = (ListNode*)malloc(sizeof(ListNode));
	new_node->data = item;
	new_node->prev = NULL;
	new_node->next = NULL;
	if (pre == NULL && head == NULL)	// empty list
	{
		head = new_node;
		tail = new_node;
	}
	else if (pre == NULL) // ���� ����Ʈ�� �� �տ� ����
	{
		new_node->next = head;	// ���ο� ��尡 ����Ʈ�� �� �տ� ��ġ
		head->prev = new_node;	// �� �� ����� ���� ��尡 ���ο� ���
		head = new_node;		// ���ο� ��尡 �� �� ��ġ�� ����
	}
	else if (pre == tail) // ����Ʈ�� �� �ڿ� ����
	{
		new_node->prev = tail;
		tail->next = new_node;
		tail = new_node;
	}
	else // �� �յ� �� �ڵ� �ƴ� ��ġ�� ����� �ڿ� ����
	{
		new_node->prev = pre;
		pre->next->prev = new_node;
		new_node->next = pre->next;
		pre->next = new_node;
	}
	ListSize++;	// ����� ���� ����
}
