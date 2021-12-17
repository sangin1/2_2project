#include <mysql.h>	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#pragma warning(disable:4996)
#pragma comment (lib, "libmysql.lib")

#define MYSQLUSER "root"				
#define MYSQLPASSWORD "1234"		
#define MYSQLIP "localhost" 

//ȸ������ �Լ�
void join(MYSQL* connection, MYSQL* conn);
//�α��� �Լ�(id�� key�� ����)
int login(MYSQL* connection, MYSQL* conn);

int main(void) {
	MYSQL* connection = NULL, conn;
    MYSQL_RES* sql_result = NULL;
    MYSQL_ROW   sql_row;
    int       query_stat;
    char menu[3][10] = { "ȸ������","�α���","����" };
    int i,check;
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
    query_stat = mysql_query(connection, "select * from coin");
    if (query_stat != 0)
    {
        fprintf(stderr, "Mysql query error : %s", mysql_error(&conn));
        return 0;
    }
    sql_result = mysql_store_result(connection);
    while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
    {
        printf("%s\n", sql_row[1]);
    }
    mysql_free_result(sql_result);
    while (1) {
        for (i = 0;i < 3;i++) {
            printf("%d %s\n", i, menu[i]);
        }
        printf("����: ");
        fflush(stdout);
        scanf_s("%d", &check, 1);
        if (check == 0) {
            join(connection, &conn);
        }
        else if (check == 1) {
            login(connection, &conn);
        }
        else if (check == 2) {
            break;
        }
    }
}
//ȸ������ �Լ�
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
                break;
            }
        }
        if (check2 == 1) {
            check = 0;
            continue;
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
                return 1;
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