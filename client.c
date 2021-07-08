#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <winsock.h>

#define MAXLINE 1024

typedef unsigned int bool;
#define false 0
#define true 1

SOCKET sd, broadcast_sd;
struct sockaddr_in serv, bc;
char input[MAXLINE];
char str_tcp[MAXLINE];
char str_bc[MAXLINE];
char reject[100] = "Server is currently full, please try again later.\n";
char userName[MAXLINE];
WSADATA wsadata;
int n, m, bc_len, timeout = 50, BROADCAST = 0103;
bool GM = false;
bool start = false;

void *recvMess(void *argu)
{
    while(1) 
	{
		if(GM == true)
		{
			break;
		}
        memset(str_tcp, "\0", 1024);
		n = recv(sd, str_tcp, MAXLINE, 0); //接收來自其他client的訊息
		if(strcmp(str_tcp, reject) == 0) //判斷訊息是否為斷線通知 
		{
			GM = true;
			printf("%s", reject);
			system("pause");
			closesocket(sd); //關閉TCP socket
   			WSACleanup();  // 結束 WinSock DLL 的使用
   			break;
		}
		else if(n != -1)
		{
			printf("%s\n", str_tcp);
			if(strcmp(str_tcp, "Result: 4A0B") == 0)
			{
				printf("**********\n");
				printf("*You Win!*\n");
				printf("**********\n");
   				GM = true;
   				break;
			}
		}      
    }
}

void *recvMessBroadcast(void *argu)
{
    while(1) 
	{
		memset(str_bc, "\0", 1024);
        m = recvfrom(broadcast_sd, str_bc, MAXLINE, 0,(LPSOCKADDR) &bc, &bc_len);
		if(m != -1)
		{
			if(strcmp(str_bc, "Game over!") == 0)
			{
				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
				printf("!%s", str_bc);
				m = recvfrom(broadcast_sd, str_bc, MAXLINE, 0,(LPSOCKADDR) &bc, &bc_len);
				printf("%s!\n", str_bc);
				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
   				GM = true;
   				system("pause");
   				break;
			}
			else if(strcmp(str_bc, "start") == 0)
			{
				start = true;
				system("cls");
			}
			else
			{
				system("cls");
				printf("%s", str_bc);
			}
		}
            
    }
}

int main(int argc, char** argv) 
{
	int n, i, j, total = 0;
    pthread_t thread1;
    pthread_t thread2;

   	WSAStartup(0x101, (LPWSADATA) &wsadata); // 呼叫 WSAStartup() 註冊 WinSock DLL 的使用
   	
   	sd = socket(AF_INET, SOCK_STREAM, 0); //開啟一個 TCP socket.
   	broadcast_sd = socket(AF_INET, SOCK_DGRAM, 0);
   	
   	serv.sin_family       = AF_INET;
   	serv.sin_addr.s_addr  = inet_addr("127.0.0.1");
   	serv.sin_port         = htons(5678);

	bc.sin_family       = AF_INET;//設定接收廣播用的socket 
    bc.sin_addr.s_addr  = 0;   
    bc.sin_port         = htons(BROADCAST);
    
    bc_len = sizeof(bc);
	if(bind(broadcast_sd, (LPSOCKADDR) &bc, bc_len) < 0)
	{
   		int error_code;
   		while((error_code = WSAGetLastError()) == 10048)
   		{
   			++BROADCAST;
   			bc.sin_port = htons(BROADCAST);
   			bind(broadcast_sd, (LPSOCKADDR) &bc, bc_len);
		}
   	}
	char broadcast = 'a';
	if(setsockopt(broadcast_sd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
		printf("setsockopt() error!\n");
		
	if(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
        printf("setsockopt() error!\n");
   	connect(sd, (LPSOCKADDR) &serv, sizeof(serv)); // 連接至 echo server
	
	pthread_create(&thread1, NULL, &recvMess, NULL);
	pthread_create(&thread2, NULL, &recvMessBroadcast, NULL);

	while(1)
	{
		
		bool invalid = false;
		if(start == true)
		{
			printf("Your guess : ");
			gets(input);
			if(GM == true)
				break;
			if(strlen(input) != 4)
			{
				printf("Invalid input!(input should be 4 variables)\n");
				invalid = true;
			}
			if(invalid == true)
			{
				continue;
			}
			
			for(i = 0; i < 4; ++i)
			{
				if(isdigit(input[i]) == 0)
				{
					printf("Invalid input!(input should be all digits)\n");
					invalid = true;
					break;
				}
			}
			if(invalid == true)
			{
				continue;
			}
			
			for(i = 0; i < 3; ++i)
			{
				for(j = i + 1; j < 4; ++j)
				{
					if(input[i] == input[j])
					{
						printf("Invalid input!(all digits should not repeat)\n");
						invalid = true;
						break;
					}
				}
				if(invalid == true)
				{
					break;
				}
			}
			if(invalid == true)
			{
				continue;
			}
			
			if(invalid == false)
			{
				send(sd, input, strlen(input)+1, 0); //傳送訊息
			}
		}
	}
	closesocket(sd); //關閉TCP socket
	return 0;
}

