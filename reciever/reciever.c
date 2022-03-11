#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>

#include <stdlib.h>

//#include <Windows.h>
#include <winsock2.h>




#pragma comment (lib, "ws2_32.lib")
//int decoder(int block)
//{
//	int result = 0, mask=0;
//	int parities[4];
//	for (int i = 0; i < 5; i++)
//	{
//		parities[i] = block & mask;
//		mask <<= 1;
//		result
//	}
//}

int main(int argc, char* argv[])
{

	WSADATA wsaData;
	int init_result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (init_result != NO_ERROR)
	{
		printf("Error as WSAStartup()\n");
		return -1;
	}

	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in remote_addr;

	char IP[200];
	int port = 0;
	sscanf(argv[1], "%s", IP);
	sscanf(argv[2], "%d", &port);

	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = inet_addr(IP);
	remote_addr.sin_port = htons(port);
	int connect_status = connect(s, (SOCKADDR*)&remote_addr, sizeof(remote_addr));
	//check connection success
	if (connect_status == -1)
	{
		printf("connection failed");
		return -1;
	}

	//char MSG[256];
	int MSG=0;
	int recieved = recv(s, &MSG, 256, 0);

	if (recieved)
		printf("%d", MSG);

	int close_status = closesocket(s);
	WSACleanup();
	return 0;
}
