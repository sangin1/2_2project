#pragma comment(lib, "ws2_32.lib")
#include <stdio.h>
#include<Windows.h>

typedef struct Point3D {
	int x, y, z;
}Point3D;

unsigned long WINAPI MyThread(Point3D *arg) {
	Point3D* pt = (Point3D*)arg;
	while (1) {
		printf("running :%d %d %d\n", pt->x, pt->y, pt->z);
		Sleep(1000);
	}
	return 0;
}

int main() {
	Point3D pt1 = { 10,20,30 };
	unsigned long ThreadID_1;
	HANDLE hThread1 = CreateThread(NULL, 0, MyThread, &pt1, 0, &ThreadID_1);

	Point3D pt2 = { 40,50,60 };
	unsigned long ThreadID_2;
	HANDLE hThread2 = CreateThread(NULL, 0, MyThread, &pt2, 0, &ThreadID_1);

	while (1) {
		printf("pr\n");
		Sleep(1000);
	}
	return 0;
}