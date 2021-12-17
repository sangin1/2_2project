#pragma warning(disable:4996)
#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <process.h>  //쓰레드 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h> //소켓 
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

void InitQueue(Queue* queue);//큐 초기화
int IsEmpty(Queue* queue); //큐가 비었는지 확인
void Enqueue(Queue* queue, ser_soc data); //큐에 보관
ser_soc Dequeue(Queue* queue); //큐에서 꺼냄
unsigned WINAPI client_con(void* arg); //쓰레드생성시 매개변수는 아규먼트로 받는다. 
unsigned WINAPI search(void* arg); 

int cli_count = 0;  //접속된 클라이언트수 카운트
ser_soc cli_socket[MAX_CLNT]; //클라이언트 소켓 정보 저장 배열
HANDLE hMutex; //쓰레드를 여러개 호출시 공유자원문제를 방지하기 위한 뮤텍스 생성

int ListSize = 0;
ListNode* head;
ListNode* tail; 
void add_after(ListNode* pre, ser_soc item);
void removed(ListNode* p) {
	if (p->prev == NULL && p->next == NULL)	// p가 유일한 노드
	{
		head = NULL;
		tail = NULL;
		free(p);
		ListSize--;
	}
	else if (p->prev == NULL)	// p가 head인 경우
	{
		head = p->next;
		head->prev = NULL;
		free(p);
		ListSize--;
	}
	else if (p->next == NULL)	// p가 tail인 경우
	{
		tail = p->prev;
		tail->next = NULL;
		free(p);
		ListSize--;
	}
	else	// p가 중간에 위치한 노드인 경우
	{
		p->prev->next = p->next;
		p->next->prev = p->prev;
		free(p);
		ListSize--;
	}
}

Queue queue;
int main() {
	WSADATA wsaData;  //윈속 초기화 함수를 위한 변수
	SOCKET serverSock, clientSock;  //소켓 구조체
	SOCKADDR_IN serverAddr, clientAddr; //소켓주소 구조체
	int clientAddrSize;
	HANDLE hThread; //쓰레드 구조체 
	HANDLE sThread;
	InitQueue(&queue);
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { //윈속 초기화(ws2_32.lib 라이브러리 초기화 및 가져오기) (2,2)는 윈속 버전
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
	sThread = (HANDLE)_beginthreadex(NULL, 0, search, (void*)&clientSock, 0, NULL); //쓰래드 생성(쓰래드 함수를 실행)
	while (1) {
		clientAddrSize = sizeof(clientAddr);
		clientSock = accept(serverSock, (SOCKADDR*)&clientAddr, &clientAddrSize); //클라이언트 소켓 받기(연결)
		WaitForSingleObject(hMutex, INFINITE);  //릴리즈 뮤텍스명령어를 받을떄까지 대기
		cli_socket[cli_count++].cli_socket = clientSock;   //뮤텍스를 해야 여러 클라이언트가 같은 변수(자원)을 사용할떄 겹치지 않는다.
		ReleaseMutex(hMutex); //뮤텍스 중지
		hThread = (HANDLE)_beginthreadex(NULL, 0, client_con, (void*)&clientSock, 0, NULL); //쓰래드 생성(쓰래드 함수를 실행)
	}
	closesocket(serverSock);
	WSACleanup(); //소켓사용 종료 시스템 콜
	return 0;
}
unsigned WINAPI client_con(void* arg) {
	SOCKET clientSock = *((SOCKET*)arg);  //매개변수로 받은 정보 저장
	int strLen = 0, i;
	char massage[BUF_SIZE];
	soc_tel *cli_soc;
	ser_soc cliser_soc;
	while ((strLen = recv(clientSock, massage, sizeof(massage), 0)) != 0) {  //클라이언트로부터 메세지를 받는다면(연결되어 있다면)
		massage[strLen] = '\0';
		cli_soc = (soc_tel*)massage;
		cliser_soc.cli_socket = clientSock;
		cliser_soc.s_t = *cli_soc;
		if (cli_soc->check == 0) {
			Enqueue(&queue, cliser_soc);//고객클라이언트에서 받은 정보를 큐에 저장 
		}
		else if (cli_soc->check == 1) {
			add_after(NULL, cliser_soc);			
		}
	}
	WaitForSingleObject(hMutex, INFINITE);
	for (i = 0;i < cli_count;i++) {
		if (clientSock == cli_socket[i].cli_socket) {//만약 현재 clientSock값이 배열의 값과 같다면
			while (i++ < cli_count - 1)//클라이언트 개수 만큼
				cli_socket[i] = cli_socket[i + 1];//앞으로 땡긴다. 
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

	if (IsEmpty(queue))//큐가 비어있을 때
	{
		queue->front = now;//맨 앞을 now로 설정       
	}
	else//비어있지 않을 때
	{
		queue->rear->next = now;//맨 뒤의 다음을 now로 설정
	}
	queue->rear = now;//맨 뒤를 now로 설정   
	queue->count++;//보관 개수를 1 증가
}

ser_soc Dequeue(Queue* queue)
{
	ser_soc re;
	Node* now;
	if (IsEmpty(queue))//큐가 비었을 때
	{
		printf("queue error");
	}
	now = queue->front;//맨 앞의 노드를 now에 기억
	re = now->data;//반환할 값은 now의 data로 설정
	queue->front = now->next;//맨 앞은 now의 다음 노드로 설정
	free(now);//now 소멸
	queue->count--;//보관 개수를 1 감소
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
	else if (pre == NULL) // 연결 리스트의 맨 앞에 삽입
	{
		new_node->next = head;	// 새로운 노드가 리스트의 맨 앞에 위치
		head->prev = new_node;	// 맨 앞 노드의 이전 노드가 새로운 노드
		head = new_node;		// 새로운 노드가 맨 앞 위치를 차지
	}
	else if (pre == tail) // 리스트의 맨 뒤에 삽입
	{
		new_node->prev = tail;
		tail->next = new_node;
		tail = new_node;
	}
	else // 맨 앞도 맨 뒤도 아닌 위치의 노드의 뒤에 삽입
	{
		new_node->prev = pre;
		pre->next->prev = new_node;
		new_node->next = pre->next;
		pre->next = new_node;
	}
	ListSize++;	// 노드의 개수 증가
}
