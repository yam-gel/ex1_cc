#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>

#include <stdlib.h>

//#include <Windows.h>
#include <winsock2.h>
#pragma comment (lib, "ws2_32.lib")

// ***********************************************************************************************
//This functions input is encoded block, the function returns the decoded block in 26 LSBs of int ***IF there was error correction it will be represented in 27th bit, see below
// ***********************************************************************************************

int decoder(int block)
{
	int array_of_bits[32] = { 0 };
	int mask = 1, error_index=0;
	//build array of 31 bits
	for (int i = 1; i < 32; i++)
	{
		if (block & mask)
			array_of_bits[i] = 1;
		mask <<= 1;
	}
	int pairities_index[5] = { 1,2,4,8,16 }, counter=0;
	//find index of error if exist
	for (int i = 0; i < 5; i++)
	{
		for (int j = 1; j < 32; j++)
		{
			if (j & pairities_index[i] && array_of_bits[j]==1)
				counter  ^= 1;
		}
		if (counter == 1)
			error_index += pairities_index[i];
		counter = 0;
	}
	if (error_index != 0)
		array_of_bits[error_index] ^= 1; //flip error bit
	//rebuild int of block after correction, in 26 LSBs of result
	unsigned int marker = 1, power = 0, result=0;
	for (int i = 1; i < 32; i++)
	{
		if (i == marker)
			marker <<= 1;
		else
		{
			result += array_of_bits[i] << power;
			power++;
		}
	}
	//If there was error correction in the block the 27th bit of result will be 1
	if (error_index != 0)
		result ^= 1 << 26;
	return (result);
}

//*********************************************************************
//Function : Recieve 31 Bytes block and write it to buffer
//*********************************************************************
int Write_to_buffer(SOCKET client, char *buffer)
{
	int splitted[8] = { 0 };
	char* head = buffer;
	int total_recieved=0, recieved = recv(client, buffer, 31, 0);
	while (recieved != 0 && recieved<=31)
	{
		total_recieved += recieved;
		head += recieved;
		if (recieved == 31) //It means we got a full block
			break;
		recieved = recv(client, head, sizeof(head), 0);
	}
	/*prints the recieved characters
	for (int i = 0; i < 31; i++)
	{
		printf("%c ", buffer[i]);
	}*/
	if (total_recieved != 31)
		printf("ERROR  %d Bytes recieved!!!", total_recieved);
	return total_recieved;
}

//********************************************************************

//********************************************************************
// FUNCTIONS : reverse num
//********************************************************************

unsigned int reverse_num(unsigned int num) {
	unsigned int reversed = 0;
	for (int i = 0; i < 32; i++) {
		reversed <<= 1;
		reversed += (num % 2);
		num /= 2;
	}
	return reversed/2;
}

//********************************************************************
// FUNCTIONS : splitting 31Bytes to 8 Integers
//********************************************************************

void split_chunk2numbers(int *arr,unsigned char *chunk) {
	int counter = 0;
	int curr=0;
	int index = 0;
	int mask = 0b010000000;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 31; j++) {
			if (counter < 8) {
				curr <<= 1;
				curr += (((*(chunk + index)) & mask)>>(7-counter));
				//(*(chunk + index)) /= 2;
				mask >>= 1;
				counter++;
			}
			else{
				j--;
				counter = 0;
				index++;
				mask = 0b010000000;
			}
		}
		//*(arr+7-i) = (reverse_num(curr));
		*(arr +i) = curr;
		curr = 0;
	}
}

//*******************************************************************************
//function : reverse char
// //*******************************************************************************
unsigned char reverse_char(unsigned char ch) {
	unsigned char reversed = 0;
	for (int i = 0; i < 8; i++) {
		reversed <<= 1;
		reversed += (ch % 2);
		ch /= 2;
	}
	return reversed;
}

//*******************************************************************************
//function : fill 26byte messeage to write from decoeded
//*******************************************************************************
fill_2_write(char* batch, int* arr) {
	int counter = 0;
	unsigned char curr = 0;
	int index = 0;
	int i, j;
	for (i = 0; i < 26; i++) {
		for (j = 0; j < 8; j++) {
			if (counter < 26) { //TODO: CHECK IF NEEDS TO BE 31?
				curr <<= 1;
				curr += (arr[index] % 2);
				arr[index] /= 2;
				counter++;
			}
			else {
				counter = 0;
				index++;
				j--;
			}
		}

		*(batch + 25 - i) = (reverse_char(curr));
		//*(batch + i) = curr;
	}
}
//********************************************************************

//*******************************************************************************
//function : Recieves a socket and writes to file all recieved data + prints number of recieved and wrote bytes
//*******************************************************************************
void one_file_reciever(SOCKET s, FILE *fp)
{
	
	char buffer[31];
	int splitted[8] = { 0 }, errors_corrected=0;
	unsigned char to_write[26];

	
	int total_recieved = 0, total_written = 0, recieved = 0;
	recieved = recv(s, buffer, 31, 0);
	while (recieved != 0)
	{
		total_recieved += recieved;
		
		split_chunk2numbers(splitted, buffer);
		for (int i = 0; i < 8; i++) {
			splitted[i] = decoder(splitted[i]);
			// This is decdode of was correction or not in a block
			if (splitted[i] & (1 << 26))
			{
				splitted[i] ^= (1 << 26);
				errors_corrected += 1;
			}
		}
		fill_2_write(to_write, splitted);
		fwrite(to_write, 1, sizeof(to_write), fp);
		total_written += sizeof(to_write);
		recieved = recv(s, buffer, sizeof(buffer), 0);
	}
	printf("received: %d bytes\n", total_recieved);
	printf("wrote: %d bytes\n", total_written);
	printf("corrected: %d errors\n", errors_corrected);
	fclose(fp);
	/// *******************************************
}
//********************************************************************

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
		fprintf(stderr,"connection failed");
		return -1;
	}

	FILE* fp = NULL;
	char file_name[100];
	int close_status;
	printf("enter file name:\n");
	scanf("%s", file_name);
	while (strcmp(file_name, "quit") != 0)
	{
		fp = fopen(file_name, "wb");
		if (fp == NULL)
		{
			fprintf(stderr, "ERROR can't open file");
			return - 1;
		}
		one_file_reciever(s, fp);

		close_status = closesocket(s);


		s = socket(AF_INET, SOCK_STREAM, 0);
		connect_status = connect(s, (SOCKADDR*)&remote_addr, sizeof(remote_addr));
		//check connection success
		if (connect_status == -1)
		{
			fprintf(stderr, "connection failed");
			return -1;
		}

		printf("enter file name:\n");
		scanf("%s", file_name);
	}

	WSACleanup();
	return 0;
}
