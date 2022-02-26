#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <WS2tcpip.h>
//#include <Windows.h>
#include <winsock2.h>

#define MAX_CLIENTS 2


#pragma comment (lib, "ws2_32.lib")


int main()
{

	WSADATA wsaData;
	int init_result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (init_result != NO_ERROR)
	{
		printf("Error as WSAStartup()\n");
		return -1;
	}
	
	SOCKET s_sender = socket(AF_INET, SOCK_STREAM, 0);
	SOCKET s_receiver = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in my_addr_s;
	struct sockaddr_in my_addr_r;

	struct sockaddr_in sender_addr;
	struct sockaddr_in reciever_addr;
	
	socklen_t add_len=256;

	//sender socket addres
	char IP[200] = "127.50.0.1";
	int port1 = 6342, port2=6343;

	my_addr_s.sin_family = AF_INET;
	my_addr_s.sin_addr.s_addr = inet_addr(IP);
	my_addr_s.sin_port = htons(port1);
	int connect_status = bind(s_sender, (SOCKADDR*)&my_addr_s, sizeof(my_addr_s));
	connect_status = listen(s_sender, MAX_CLIENTS);
	//check connection success
	if (connect_status == -1)
	{
		printf("connection failed");
		return -1;
	}

	printf("sender socket: %s port %d\n", IP, port1);
	printf("receiver socket: %s port %d\n", IP, port2);
	char MSG[256];
	SOCKET client1 = accept(s_sender, (SOCKADDR*)&sender_addr, &add_len);
	int recieved = recv(client1, MSG, sizeof(MSG), 0);

	printf("%s\n", inet_ntoa(sender_addr.sin_addr));
	
	//reciever socket address
	my_addr_r.sin_family = AF_INET;
	my_addr_r.sin_addr.s_addr = inet_addr("127.50.0.1");
	my_addr_r.sin_port = htons(6343);
	connect_status = bind(s_receiver, (SOCKADDR*)&my_addr_r, sizeof(my_addr_r));
	connect_status = listen(s_receiver, MAX_CLIENTS);
	//check connection success
	if (connect_status == -1)
	{
		printf("connection failed 2");
		return -1;
	}
	

	


	
	
	SOCKET client2 = accept(s_receiver, (SOCKADDR*)&reciever_addr, &add_len);
	int sent = send(client2, MSG, sizeof(MSG), 0);
	
	

	int close_status = closesocket(s_sender);
	close_status = closesocket(s_receiver);
	close_status = closesocket(client1);
	close_status = closesocket(client2);
	
	WSACleanup();
	return 0;
}
//THIS IS MY COMMENT BS
