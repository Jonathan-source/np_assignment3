#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h> 		
#include <sys/socket.h>		 
#include <netinet/in.h>	
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include <calcLib.h>
#include "protocol.h"

#define MAXLINE 1024

using namespace std;



int main(int argc, char *argv[])
{
  
  if(argc != 3)
	{
		perror("error, arguments are missing.");
	}
	unsigned short int SERVER_PORT = atoi(argv[2]);


// VARIABLES  
//=====================================================================================================

	int sockfd;
	int connfd;
	int byteSent = 0;
  	int byteRcvd = 0;

	char buffer[MAXLINE];

	struct sockaddr_in servAddr;
	struct sockaddr_in cliAddr;

	bool isConnected = true;



// TCP SOCKET.
//=====================================================================================================
sockfd = socket(AF_INET, SOCK_STREAM, 0);
if(sockfd <0)
{

}
printf("[+] Socket was created.\n");

memset(&servAddr, '\0', sizeof(servAddr));
servAddr.sin_family = AF_INET;
servAddr.sin_port = htons(SERVER_PORT);
servAddr.sin_addr.s_addr = inet_addr(argv[1]);


int ret =connect(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr));
if (ret < 0){

}
printf("[+] Sucessfully connected to the server.\n");


while(isConnected)
{
	printf("Client: \t");
	scanf("%s", &buffer[0]);
	send(sockfd, buffer, strlen(buffer), 0);

	if(strcmp(buffer, ":exit")== 0){
		printf("[-] Disconnected from the server.\n");
		isConnected = false;
	}

	if(recv(sockfd, buffer, MAXLINE, 0) < 0){

	} else {
		printf("Server:\t%s\n", buffer);
	}
}













	return 0;
}