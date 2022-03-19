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
//Function : Recive 31bytes buffer and flips any n'th bit in it, returns the remainder for next block
//*********************************************************************
int add_deterministic_noise(char* buffer, int n, int remainder, int* flip_counter)
{

	int temp=remainder;
	char* p = buffer;
	unsigned char mask = 0x80;
	for (int i = 0; i < 31; i++)
	{
		if (temp <= 8)
		{
			for (int j=0; j<8; j++)
			{
				temp--;
				if (temp == 0)
				{
					*p = *p ^ mask;
					*flip_counter += 1;
					temp = n;
				}
				mask >>= 1;
			}
			mask = 0x80;
		}
		else
			temp -= 8;
		p++;
	}
	
	return temp;
}
//*********************************************************************

//*********************************************************************
// Function : flips bit with probablity of n/(2^16)
//*********************************************************************
void add_random_noise(char* chunk, int seed, int n, int *flip_counter) {
	int noise_prob = 0;
	int count = 0;
	int flip_mask = 1;
	srand(seed);
	for (int i = 0; i < 31; i++) {
		for (int j = 0; j < 8; j++) {
			noise_prob = (rand() % (65536 / 2));
			if (noise_prob <= (n / 2)) {
				*(chunk + i) ^= (flip_mask << j);
				*flip_counter += 1;
			}
		}
	}
}
//*********************************************************************

//*********************************************************************
//Function : Gets sender socket and recever socket and transfer the data
//*********************************************************************
int recieve_and_send(SOCKET sender, SOCKET reciever, char* noise_type, int noise_parameter, int seed)
{
	
	char buffer[31];
	int recieved = recv(sender, buffer, 31, 0);
	int sent = 0;//send(reciever, &buffer, sizeof(buffer), 0);
	int retransmitted = 0, remainder_for_next_block=noise_parameter, flipped = 0;
	while (recieved != 0)
	{
		
		if (noise_type=='d')
		//This is adding deterministic noise, remainder is the remmainder of bits to count before fliping bit in next block
			remainder_for_next_block = add_deterministic_noise(buffer, noise_parameter, remainder_for_next_block, &flipped);
		else
			add_random_noise(buffer, seed, noise_parameter, &flipped);
		sent= send(reciever, &buffer, sizeof(buffer), 0);
		if (recieved == sent)
			retransmitted += recieved;
		recieved = recv(sender, buffer, 31, 0);
	}
	printf("retransmitted %d bytes, flipped %d bits\n", retransmitted, flipped);
	return retransmitted;
}
//*********************************************************************

int main(int argc, char* argv[])
{	

	char noise_type=0;
	int noise_parameter, seed=-1;
	sscanf(argv[1], "-%c", &noise_type);
	sscanf(argv[2], "%d", &noise_parameter);
	if (argc > 3)
		sscanf(argv[3], "%d", &seed);
	


	WSADATA wsaData;
	int init_result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (init_result != NO_ERROR)
	{
		fprintf(stderr, "Error as WSAStartup()\n");
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
	
	my_addr_s.sin_family = AF_INET;
	my_addr_s.sin_addr.s_addr = INADDR_ANY;
	my_addr_s.sin_port = 0;
	int connect_status = bind(s_sender, (SOCKADDR*)&my_addr_s, sizeof(my_addr_s));
	connect_status = listen(s_sender, MAX_CLIENTS);
	//check connection success
	if (connect_status == -1)
	{
		fprintf(stderr, "connection failed\n");
		return -1;
	}
	
	getsockname(s_sender, (struct sockaddr*)&my_addr_s, &add_len);
	printf("sender socket: IP address = %s port = %d\n", inet_ntoa(my_addr_s.sin_addr), ntohs(my_addr_s.sin_port));
	
	//reciever socket address
	my_addr_r.sin_family = AF_INET;
	my_addr_r.sin_addr.s_addr = INADDR_ANY;
	my_addr_r.sin_port = 0;
	connect_status = bind(s_receiver, (SOCKADDR*)&my_addr_r, sizeof(my_addr_r));
	connect_status = listen(s_receiver, MAX_CLIENTS);
	//check connection success
	if (connect_status == -1)
	{
		fprintf(stderr, "connection failed 2");
		return -1;
	}
	getsockname(s_receiver, (struct sockaddr*)&my_addr_r, &add_len);
	printf("receiver socket: IP address = %s port = %d\n", inet_ntoa(my_addr_r.sin_addr), ntohs(my_addr_r.sin_port));

	//comment
	SOCKET client1, client2;
	int close_status;
	char cont_flag[10];
	while (1)
	{
		client1 = accept(s_sender, (SOCKADDR*)&sender_addr, &add_len);
		client2 = accept(s_receiver, (SOCKADDR*)&reciever_addr, &add_len);
		recieve_and_send(client1, client2, noise_type, noise_parameter, seed);
		close_status = closesocket(client1);
		close_status = closesocket(client2);
		
		printf("continue? (yes/no)\n");
		scanf("%s", cont_flag);
		if (strcmp(cont_flag, "no") == 0)
			break;
	}

	close_status = closesocket(s_sender);
	close_status = closesocket(s_receiver);
	
	WSACleanup();

	return 0;
}