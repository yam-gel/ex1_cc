#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
//#include <Windows.h>
#include <winsock2.h>
#pragma comment (lib, "ws2_32.lib")



//function : Adds hamming into each elemnet of the array
//***********************************************************************
void add_hamming(int* arr) {
	for (int i = 0; i < 8; i++) {
		*(arr + i) = num2hamming(*(arr + i));
	}
	printf("CHECKK");
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
//function : fill batch without redundency
//*******************************************************************************
fill_batch(char* batch, int* arr) {
	int counter = 0;
	char curr = 0;
	int index = 0;
	int i, j;
	for (i = 0; i < 31; i++) {
		for (j = 0; j < 8; j++) {
			if (counter < 31) { //TODO: CHECK IF NEEDS TO BE 31?
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

		*(batch + 30 - i) = (reverse_char(curr));
		//*(batch + i) = curr;
	}
}

//*********************************************************************
//Function : Recive 26bits int, return 31bit Hamming code
//*********************************************************************
 
int num2hamming(int num){
	int num_b[26]={0};
	int code_b[31]={0};
	int c=1,res=0,exp=0;
	int code=0;
	for (int i=0;i<26;i++){
		num_b[i]=num%2;
		num>>=1;
	}

	for (int i = 1; i < 32; i++) {
		if (i == c) {
			c *= 2;
			exp++;
			continue;
		}
		else code_b[i - 1] = num_b[i-1-exp];
	}
	exp = 0;
	c = 1;
	for (int i=1;i<32;i++){
		if (i==c){
			res=0;
			for (int j=1;j<32;j++){
				if ((((j & i) >> exp) & 1) == 1) 
					res ^= code_b[j-1];
			}
			code_b[i-1]=res;
			c*=2;
			exp++;
		}
	}

	for (int i=31;i>=0;i--){
		code <<= 1;
		code+=code_b[i];
	}
	return code;

}
//***********************************************************************

//***********************************************************************
// function : Splits the buffer into 4 pieces of 26bits and assigns it to the relevant half of the array
//***********************************************************************
void split_buffer(char* buffer, int* arr, int half) {
	*arr = ((int)buffer[0] << 18) + ((int)buffer[1] << 10) + ((int)buffer[2] << 2) + (((int)buffer[3] & 0xc0) >> 6);
	*(arr + 1 + 4 * half) = (((int)buffer[3] & 0x3f) << 20) + ((int)buffer[4] << 12) + ((int)buffer[5] << 4) + (((int)buffer[6] & 0xf0) >> 4);
	*(arr + 2 + 4 * half) = (((int)buffer[6] & 0x0f) << 22) + ((int)buffer[7] << 14) + ((int)buffer[8] << 6) + (((int)buffer[9] & 0xfc) >> 2);
	*(arr + 3 + 4 * half) = (((int)buffer[9] & 0x03) << 24) + ((int)buffer[10] << 16) + ((int)buffer[11] << 8) + (int)buffer[12];
}
//************************************

//*********************************************************************
//Function : Recieve socket and a buffer of 31B and sends it to the line and returns the number of sent Bytes
//*********************************************************************
int Send31Bytes(SOCKET s, char *buffer)
{
	int sent_total = send(s, buffer, 31, 0); //add define for 31?
	
	if (sent_total!=31)
		printf("ERROR only %d Bytes were sent!!!\n", sent_total);
	printf("%d Bytes sent successfully\n", sent_total);
	printf("sent message was: \n");
	for (int i = 0; i < 31; i++)
	{
		printf("%c", buffer[i]);
	}
	return sent_total;

	//FILE* fp = NULL;
	//fp = fopen("my_file1.txt", "r");
	//int sent1, sent2, sent3, sent4;
	//unsigned char buffer[13];
	//char str[8];
	//fread(buffer, 1, sizeof(buffer), fp);
	//while (!feof(fp))
	//{
	//	int i = 0;
	//	/*int MSG1 = ((int)buffer[0] << 18) + ((int)buffer[1] << 10) + ((int)buffer[2] << 2) + (((int)buffer[3] & 0xc0) >> 6);
	//	int MSG2 = (((int)buffer[3] & 0x3f) << 20) + ((int)buffer[4] << 12) + ((int)buffer[5] << 4) + (((int)buffer[6] & 0xf0) >> 4);
	//	int MSG3 = (((int)buffer[6] & 0x0f) << 22) + ((int)buffer[7] << 14) + ((int)buffer[8] << 6) + (((int)buffer[9] & 0xfc) >> 2);
	//	int MSG4 = (((int)buffer[9] & 0x03) << 24) + ((int)buffer[10] << 16) + ((int)buffer[11] << 8) + (int)buffer[12];*/
	//	//printf("%x %x %x %x\n", MSG1, MSG2, MSG3, MSG4);
	//	//_itoa(MSG4, str, 16);
	//	sent1 = send(s, &buffer, sizeof(buffer), 0);
	//	while (i < 13)
	//	{
	//		printf("%c", buffer[i]);
	//		i++;
	//	}
	//	printf("\n");
	//	/*sent2 = send(s, &MSG2, sizeof(MSG2), 0);
	//	sent3 = send(s, &MSG3, sizeof(MSG3), 0);
	//	sent4 = send(s, &MSG4, sizeof(MSG4), 0);*/
	//	printf("%d bytes sent successfully\n", sent1);
	//	fread(buffer, 1, sizeof(buffer), fp);
	//}
}
//***********************************************************************



//*******************************************************************************
//function : Reads file in 13bytes chunks, splits, code to hamming,and sends it 
//*******************************************************************************
void file_reader(FILE* fp, SOCKET s) {
	unsigned char buffer[13];
	unsigned char batch[31];
	int splitted[8] = { 0 };
	while (!feof(fp)) {
		for (int i = 0; i < 2; i++) {
			fread(buffer, 1, sizeof(buffer), fp);
			split_buffer(buffer, splitted, i);
		}
		add_hamming(splitted);
		fill_batch(batch, splitted);
		//ADD SEND
	}
}

//*******************************************************************************
//function : TEST 1 BATCH 
//*******************************************************************************
void test_1() {
	int splitted[8] = { 1,1,1,1,1,1,1,1 };
	//int try_0[8] = {0x7fffffff,0x7fffffff,0x7fffffff,0x7fffffff,0x7fffffff,0x7fffffff,0x7fffffff,0x7fffffff};
	unsigned char batch[31];
	add_hamming(splitted);
	fill_batch(batch, splitted);
	printf("DONE");
}

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	int sent_counter = 0;
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


	/// ****THIS IS TEMPORARY TO CHECK THE SENDING FUNC ****///
	char buffer[31];
	FILE* fp = NULL;
	fp = fopen("my_file1.txt", "r");
	fread(buffer, 1, sizeof(buffer), fp);

	sent_counter+=Send31Bytes(s, buffer);
	/// *******************************************
	
	int close_status = closesocket(s);
	WSACleanup();

	return 0;
	
	
}
