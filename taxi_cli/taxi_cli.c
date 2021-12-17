#pragma warning(disable:4996)
#pragma comment(lib, "ws2_32.lib")
#pragma comment (lib, "libmysql.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <Windows.h>
#include <process.h>
#include <mysql.h>	
#include <time.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

#define MYSQLUSER "root"				
#define MYSQLPASSWORD "1234"		
#define MYSQLIP "localhost" 

//ȸ������ �Լ�
void join(MYSQL* connection, MYSQL* conn);
//�α��� �Լ�(id�� key�� ����)
int login(MYSQL* connection, MYSQL* conn);
void stat(MYSQL* connection, MYSQL* conn, int id_num);
typedef struct soc_tel {
    int x;
    int y;
    char tel_name[12];
    int check;
}soc_tel;
int SendMsg(SOCKET sock);//������ �����Լ�
int RecvMsg(SOCKET sock, MYSQL* connection, MYSQL* conn, int id_num);
char msg[BUF_SIZE];

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    SOCKET sock;
    SOCKADDR_IN serverAddr;
    MYSQL* connection = NULL, conn;
    char IP[15] = "127.0.0.1", port[1024] = "4000";
    char menu[3][20] = { "�α���","ȸ������","����" };
    char menu2[3][20] = { "����","�������","����" };
    int i, a,id_check,b;
 
    mysql_init(&conn);
    connection = mysql_real_connect(&conn, MYSQLIP, MYSQLUSER, MYSQLPASSWORD, "taxi", 3306, (char*)NULL, 0);

    if (connection == NULL)
    {
        fprintf(stderr, "Mysql connection error : %s", mysql_error(&conn));
        return 1;
    }
    else {
        mysql_query(&conn, "set names euckr"); //�ѱۃ��� ����
    }

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
        printf("          �ý� ���� ���α׷�(��� ����)\n");
        for (i = 0;i < 3;i++) {
            printf("%d. %s\n", i + 1, menu[i]);
        }
        printf("\n �Է� :");
        fflush(stdout);
        scanf("%d", &a);
        if (a == 3) {
            printf("�����մϴ�. \n");
            closesocket(sock);//���� ����  
            WSACleanup();
            return 0;
        }
        else if (a == 2) {
            join(connection, &conn);
        }
        else if (a == 1) {
            system("CLS");
            id_check = login(connection, &conn); 
            if(id_check>=0){
                while (1) {
                    system("CLS"); 
                    for (i = 0;i < 3;i++) {
                        printf("%d. %s\n", i + 1, menu2[i]);
                    }
                    printf("\n �Է� :");
                    fflush(stdout);
                    scanf("%d", &b);
                    if (b == 1) {
                        SendMsg(sock);// ���� �Լ�.   
                        RecvMsg(sock, connection, &conn,id_check);//�����Լ�
                    }
                    else if (b == 2) {
                        stat(connection, &conn, id_check);
                    }
                    else if (b == 3) {
                        printf("�α׾ƿ��մϴ� \n");
                        break;
                    }
                }
            }
        }
    }
    return 0;
}

int SendMsg(SOCKET sock) {//���ۿ� �������Լ�
    soc_tel soc_tel_cli;
    int x, y;
    char tel_name[12]="111";

    printf("���� ��� ���� �Է��� �ּ���(x���� y��ǥ �Է�)");
    fflush(stdout);
    scanf("%d %d", &x, &y); 

    soc_tel_cli.check = 1;
    soc_tel_cli.x = x;
    soc_tel_cli.y = y;
    sprintf(soc_tel_cli.tel_name, tel_name);

    send(sock, (char*)&soc_tel_cli, sizeof(soc_tel_cli), 0);//nameMsg�� �������� �����Ѵ�.

    return 0;
}
int RecvMsg(SOCKET sock, MYSQL* connection, MYSQL* conn,int id_num) {
    MYSQL_RES* sql_result = NULL;
    int strLen = 0, query_stat,coin;
    char massage[BUF_SIZE];
    char query[255];
    soc_tel* cli_soc;
    sql_result = mysql_store_result(connection);
    strLen = recv(sock, massage, sizeof(massage), 0);
    massage[strLen] = '\0';
    cli_soc = (soc_tel*)massage;
    printf("���� ����� ���� ��ġ��%d, %d �Դϴ�.\n", cli_soc->x, cli_soc->y);
    printf("��ȭ��ȣ : %s\n", cli_soc->tel_name);
    printf("�� ������ ���� �Է�\n");
    getche();
    system("cls");
    printf("���� �Է�: ");
    fflush(stdout);
    scanf("%d", &coin);
    sprintf_s(query, 255, "insert into coin(login_id,coin_add) values(%d,%d)", id_num, coin);

    query_stat = mysql_query(connection, query);
    if (query_stat != 0)
    {
        fprintf(stderr, "Mysql query error : %s", mysql_error(&conn));
        return;
    }
    mysql_free_result(sql_result);
}
void stat(MYSQL* connection, MYSQL* conn, int id_num) {
    MYSQL_RES* sql_result = NULL;
    MYSQL_ROW   sql_row;
    int  query_stat, i=0,check=0,sum,wh_ch=0;
    char menu3[3][30] = { "�Ѵ� ���Գ���","���� ���� ����","����" };
    char coin_data[255][30] = { NULL, }, query[255], coin_check[255][30] = { NULL, };
    char coin_data2[255][30] = { NULL, };
    time_t t = time(NULL);
    struct tm tm = *localtime(&t); 
    char* ptr; 
    printf("%d", id_num);
    while (1) {
        system("cls");
        for (i = 0;i < 3;i++) {
            printf("%d, %s\n", i + 1, menu3[i]);
        }
        printf("  �Է�: ");
        fflush(stdout);
        scanf("%d", &check);

        if (check == 1 || check == 2) { 
            sprintf_s(query, 255, "select * from coin where login_ID=%d", id_num);  
            query_stat = mysql_query(connection, query);  
            if (query_stat != 0)
            {
                fprintf(stderr, "Mysql query error : %s", mysql_error(&conn));
                return 0;
            } 
            sql_result = mysql_store_result(connection); 
            i = 0;
            wh_ch = 0;
            while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
            { 
                strcpy(coin_data[i], sql_row[1]);  
                strcpy(coin_data2[i], sql_row[1]);
                strcpy(coin_check[i], sql_row[2]);  
                i++;
                wh_ch++;
            }           
            mysql_free_result(sql_result);
            if (check == 1) {
                system("cls");
                i = 0;
                sum = 0;
                while (i<wh_ch) {  
                    ptr = strtok(coin_data[i], " ");  
                    ptr = strtok(ptr, "-");
                    ptr = strtok(NULL, "-"); 
                    if (tm.tm_mon+1 == 1) {
                        if (12 == atoi(ptr) || tm.tm_mon + 1 == atoi(ptr)) { 
                            printf("%s: %s\n", coin_data2[i], coin_check[i]); 
                            sum += atoi(coin_check[i]);
                        }
                    }
                    else if (tm.tm_mon == atoi(ptr) || tm.tm_mon + 1 == atoi(ptr)) { 
                        printf("%s: %s\n", coin_data2[i], coin_check[i]); 
                        sum += atoi(coin_check[i]);                      
                    }                   
                    i++;
                } 
                printf("�� �ݾ�: %d\n", sum);
                printf("������ : ���� �Է�\n");
                getche();
            }
            else if (check == 2) {
                system("cls");
                i = 0;
                sum = 0;
                while (i < wh_ch) {
                    ptr = strtok(coin_data[i], " ");
                    ptr = strtok(ptr, "-");  
                    if (tm.tm_year+1900 == atoi(ptr)) {
                        printf("%s: %s\n", coin_data2[i], coin_check[i]);
                        sum += atoi(coin_check[i]);
                    }
                    i++;
                } 
                printf("�� �ݾ�: %d\n", sum);
                printf("������ : ���� �Է�\n");
                getche();
            }
        }
        else if (check == 3) {
            break;
        }
    }
}
void join(MYSQL* connection, MYSQL* conn) {
    MYSQL_RES* sql_result = NULL;
    MYSQL_ROW   sql_row;
    int       query_stat, check = 0, check2 = 0, i = 0;
    char id[20];
    char id_check[20];
    char pw[20];
    char query[255];
    char id_ok[255][20] = { NULL };
    char a[2];
    check = 0;
    while (check == 0) {
        system("cls");
        printf("--------ȸ������--------\n\n");
        printf("id  �Է�: ");
        fflush(stdout);
        scanf_s("%s", &id, 20);

        query_stat = mysql_query(connection, "select * from login");
        if (query_stat != 0)
        {
            fprintf(stderr, "Mysql query error : %s", mysql_error(&conn));
            return 1;
        }
        sql_result = mysql_store_result(connection);
        i = 0;
        while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
        {
            strcpy_s(id_ok[i], 20, sql_row[1], 20);
            i++;
        }
        mysql_free_result(sql_result);
        i = 0;
        for (i = 0;i < 255;i++) {
            if (strcmp(id, id_ok[i]) == 0) {
                printf("�ߺ��� ���̵� �Դϴ�.\n");
                printf("���� �Է�\n");
                getche();
                check2++;
                return;
            }
        } 
        printf("idȮ��  �Է�: ");
        fflush(stdout);
        scanf_s("%s", &id_check, 20);
        if ((strcmp(id, id_check)) == 0) {
            printf("���̵� Ȯ�� �Ϸ�\n");
            break;
        }
        else {
            printf("idȮ���� Ʋ�Ƚ��ϴ�.\n");
            printf("���� �Է�\n");
            getche();
        }
    }
    printf("pw �Է�: ");
    fflush(stdout);
    scanf_s("%s", &pw, 20);
    sprintf_s(query, 255, "insert into login(r_id,password) values('%s','%s')", id, pw);

    query_stat = mysql_query(connection, query);
    if (query_stat != 0)
    {
        fprintf(stderr, "Mysql query error : %s", mysql_error(&conn));
        return;
    }
    else {
        printf("ȸ������ �Ϸ�\n");
        return 0;
    }
    mysql_free_result(sql_result);
    return 0;
}
//�α��� �Լ� 
int login(MYSQL* connection, MYSQL* conn) {
    MYSQL_RES* sql_result = NULL;
    MYSQL_ROW   sql_row;
    int query_stat, check_id = 0;
    char check_id_ch[10], a[10];
    char id[20];
    char pw[20];

    printf("id  �Է�: ");
    fflush(stdout);
    scanf_s("%s", id, 20);
    query_stat = mysql_query(connection, "select * from login");
    if (query_stat != 0)
    {
        fprintf(stderr, "Mysql query error : %s", mysql_error(&conn));
        return 0;
    }
    sql_result = mysql_store_result(connection);
    while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
    {
        if (strcmp(id, sql_row[1]) == 0) {
            strcpy_s(check_id_ch, 10, sql_row[0], 10);
        }
    }
    mysql_free_result(sql_result);

    check_id = atoi(check_id_ch);
    if (check_id <= 0) {
        printf("���� ���̵� �Դϴ�\n");
        printf("���ư����� �ƹ����� �Է�");
        fflush(stdout);
        scanf_s("%s", a, 10);
        return 0;
    }
    printf("pw �Է�: ");
    fflush(stdout);
    scanf_s("%s", &pw, 20);
    query_stat = mysql_query(connection, "select * from login");
    if (query_stat != 0)
    {
        fprintf(stderr, "Mysql query error : %s", mysql_error(&conn));
        return 0;
    }
    sql_result = mysql_store_result(connection);
    while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
    {
        if (strcmp(check_id_ch, sql_row[0]) == 0) {
            if (strcmp(pw, sql_row[2]) == 0) {
                printf("�α��� ����\n");
                return check_id;
            }
            else {
                printf("��й�ȣ�� �ٸ��ϴ�\n");
                printf("���ư����� �ƹ����� �Է�");
                fflush(stdout);
                scanf_s("%s", a, 10);
                return 0;
            }
        }
    }
    mysql_free_result(sql_result);
}