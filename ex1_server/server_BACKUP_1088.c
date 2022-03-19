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

//*********************************************************************
//Function : Recive 31bytes buffer and flips any n'th bit in it, returns the remainer for next block
//*********************************************************************
int add_deterministic_noise(char* buffer, int n)
{
<<<<<<< HEAD
	int remainder = 0, temp = n;
=======
	int remainder = 0, temp=n;
>>>>>>> 10215b7eecf3183e5d23ddeca04bd1e3c3a5a3f9
	char* p = buffer;
	unsigned char mask = 0x80;
	for (int i = 0; i < 31; i++)
	{
		if (temp <= 8)
		{
<<<<<<< HEAD
			for (int j = 0; j < 8; j++)
=======
			for (int j=0; j<8; j++)
>>>>>>> 10215b7eecf3183e5d23ddeca04bd1e3c3a5a3f9
			{
				temp--;
				if (temp == 0)
				{
					*p = *p ^ mask;
					temp = n;
				}
				mask >>= 1;
			}
			mask = 0x80;
		}
		else
		{
			temp -= 8;
		}
		p++;
	}
}
//*********************************************************************


//*********************************************************************
//Function : Recive 31bits int , and flip bit with n/635536 probabilty
//*********************************************************************
int num_after_rand_noise(int num, int seed, int n) {
	int flip_mask = 1;
	int num_noised=num;
	int noise_prob = 0;
	int count = 0;
	srand(seed);
	for (int i = 0; i < 31; i++) {
		noise_prob = rand() % (65536/2);
		//printf("%0d\n", noise_prob);
		if (noise_prob <= (n/2)) {
			num_noised ^= flip_mask;
			printf("%0d\n", noise_prob);
			count++;
		}
		flip_mask <<= 1;
	}
	printf("%0d", count);
}
//*********************************************************************

//*********************************************************************
// Function : flips bit with probablity of n/(2^16)
//*********************************************************************
void add_random_noise(char* chunk, int seed, int n) {
	int noise_prob = 0;
	int count = 0;
	int flip_mask = 1;
	srand(seed);
	for (int i = 0; i < 31; i++) {
		for (int j = 0; j < 8; j++) {
			noise_prob = (rand() % (65536 / 2));
			if (noise_prob <= (n / 2)) {
				*(chunk + i) ^= (flip_mask << j);
			}
		}
	}
}


//*********************************************************************
//Function : Recieve 31 Bytes of data and writes it to buffer
//*********************************************************************
void Write_to_buffer(SOCKET client, char *buffer)
{
	char buffer1[31];
	int recieved = recv(client, buffer, 31, 0);
	
	//while (recieved != 0)
	//{
	//	printf("recieved %d Bytes, %c\n", recieved, buffer[0]);
	//	//buffer1 = buffer1+ recieved;
	//	recieved = recv(client, buffer, 31, 0);
	//}

	//********* I am not sure if we need to add loop of recieve??????????
}


//*********************************************************************

int main(int argc, char* argv[])
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
	char IP[200] = "127.0.0.1";
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
	//char MSG[256];
	int MSG=0;
	SOCKET client1 = accept(s_sender, (SOCKADDR*)&sender_addr, &add_len);
	/// START OF CHANGES TO TRY AND READ TO FILE
	//int recieved = recv(client1, &MSG, sizeof(MSG), 0);
	char buffer[31];
	Write_to_buffer(client1, buffer);

	for (int i = 0; i < 31; i++)
	{
		printf("%c", buffer[i]);
	}

	//printf("%d %s\n",MSG, inet_ntoa(sender_addr.sin_addr));
	//END OF CHANGES
	
	//reciever socket address
	my_addr_r.sin_family = AF_INET;
	my_addr_r.sin_addr.s_addr = inet_addr("127.0.0.1");
	my_addr_r.sin_port = htons(port2);
	connect_status = bind(s_receiver, (SOCKADDR*)&my_addr_r, sizeof(my_addr_r));
	connect_status = listen(s_receiver, MAX_CLIENTS);
	//check connection success
	if (connect_status == -1)
	{
		printf("connection failed 2");
		return -1;
	}
	//comment

	SOCKET client2 = accept(s_receiver, (SOCKADDR*)&reciever_addr, &add_len);
	int sent = send(client2, &buffer, sizeof(buffer), 0);
	
	int close_status = closesocket(s_sender);
	close_status = closesocket(s_receiver);
	close_status = closesocket(client1);
	close_status = closesocket(client2);
	
	WSACleanup();
	

	return 0;
}