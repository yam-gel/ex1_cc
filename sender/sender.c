#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>

#include <stdlib.h>

//#include <Windows.h>
#include <winsock2.h>



#pragma comment (lib, "ws2_32.lib")
void ReadFile_1(SOCKET s)
{
	FILE* fp = NULL;
	fp = fopen("my_file.txt", "r");
	int sent;
	unsigned char buffer[13];
	char str[8];
	while (!feof(fp))
	{
		fread(buffer, 1, sizeof(buffer), fp);
		int MSG1 = ((int)buffer[0] << 18) + ((int)buffer[1] << 10) + ((int)buffer[2] << 2) + (((int)buffer[3] & 0xc0) >> 6);
		int MSG2 = (((int)buffer[3] & 0x3f) << 20) + ((int)buffer[4] << 12) + ((int)buffer[5] << 4) + (((int)buffer[6] & 0xf0) >> 4);
		int MSG3 = (((int)buffer[6] & 0x0f) << 22) + ((int)buffer[7] << 14) + ((int)buffer[8] << 6) + (((int)buffer[9] & 0xfc) >> 2);
		int MSG4 = (((int)buffer[9] & 0x03) << 24) + ((int)buffer[10] << 16) + ((int)buffer[11] << 8) + (int)buffer[12];
		printf("%x %x %x %x\n", MSG1, MSG2, MSG3, MSG4);
		_itoa(MSG4, str, 16);
		sent = send(s, &MSG3, sizeof(MSG3), 0);
		printf("%d bytes sent successfully\n", sent);

	}
}

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
	int port=0;
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
	
	ReadFile_1(s);
	
	int close_status = closesocket(s);
	WSACleanup();
	
	return 0;
}
