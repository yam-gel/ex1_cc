#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>

#include <stdlib.h>

//#include <Windows.h>
#include <winsock2.h>
#pragma comment (lib, "ws2_32.lib")

// ***********************************************************************************************
//This functions input is encoded block, the function returns the decoded block in 26 LSBs of int 
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
	printf("\n%d",error_index);
	if (error_index != 0)
	{
		array_of_bits[error_index] ^= 1; //flip error bit
		printf("error detected in bit number %d", error_index);
	}
	//build array of 31 bits
	int marker = 1, power = 0, result=0;
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
	printf("\n%d", result);
	return(result);
}

//*********************************************************************
//Function : Recieve 31 Bytes block and write it to buffer
//*********************************************************************
int Write_to_buffer(SOCKET client, char *buffer)
{
	int splitted[8] = { 0 };
	char* head = buffer;
	int total_recieved=0, recieved = recv(client, buffer, 31, 0);
	while (recieved != 0 && recieved<31)
	{
		total_recieved += recieved;
		head += recieved;
		recieved = recv(client, head, sizeof(head), 0);
	}

	for (int i = 0; i < 31; i++)
	{
		printf("%c ", buffer[i]);
	}
	if (total_recieved != 31)
		printf("ERROR  %d Bytes were sent!!!", total_recieved);
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
int main(int argc, char* argv[])
{

	WSADATA wsaData;
	int recieved_counter=0;
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

	/// ****THIS IS TEMPORARY TO CHECK THE SENDING FUNC ///
	char buffer[31];
	int splitted[8] = { 0 };

	unsigned char to_write[26];
	FILE* fp = NULL;
	fp = fopen("output.txt", "w");
	recieved_counter += Write_to_buffer(s, buffer);
	split_chunk2numbers(splitted, buffer);
	for (int i = 0; i < 8; i++) {
		splitted[i] = decoder(splitted[i]);
	}
	fill_2_write(to_write, splitted);
	printf("recieved message is: \n");	
	for (int i = 0; i < 26; i++)
	{
		printf("%c\n", to_write[i], i);
	}
	fwrite(to_write, 1, sizeof(to_write), fp);
	/// *******************************************

	int close_status = closesocket(s);
	WSACleanup();
	return 0;
	
	
	/*unsigned char check[31] = {0,0,0,0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	unsigned char check2[31] = { 0,0,0,14,0,0,0,28,0,0,0,56,0,0,0,112,0,0,0,224,0,0,1,192,0,0,3,128,0,0,7 };
	unsigned int splitted[8] = { 0 };
	split_chunk2numbers(splitted,check2);
	for (int i = 0; i < 8; i++) {
		splitted[i] = decoder(splitted[i]);
	}
	unsigned char try26[26] = { 0 };
	fill_2_write(try26, splitted);*/
	return 0;
}
