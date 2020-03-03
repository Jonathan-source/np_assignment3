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
  
  if(argc != 2)
	{
		perror("error, arguments are missing.");
	}
	unsigned short int SERVER_PORT = atoi(argv[1]);


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

	/* Create an TCP socket. */
  	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("could not create socket.");
	}
	printf("\n[+] Socket was created. File descriptor: %d\n", sockfd);

	/* Fill in the server's address and data. */
	memset(&servAddr, 0, sizeof(servAddr));  		
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(SERVER_PORT);				
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  	/* Bind socket to an address. */
	int status = bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr));
	if (status < 0)
	{
		perror("failed to bind socket.");
	}
	printf("[+] Socket was successfully bound to %s:%d\n", inet_ntoa(servAddr.sin_addr), SERVER_PORT);
	
	/* Listen for connections. */
	if(listen(sockfd, 1) < 0)
	{	
		perror("listen failed.");
	}
	printf("Listenning, waiting for connection on port: %i...\n\n", SERVER_PORT);


	while(isConnected)
	{ 
		memset(&buffer, \0, sizeof(buffer));


		if((connfd = accept(sockfd, (struct sockaddr *) &cliAddr, &cliLen)) < 0)
		{
			perror("could not connect with the client.");
		}

		cliLen = sizeof(cliAddr);

		
		connfd = accept(sockfd, (struct sockaddr *) &cliAddr, &cliLen); 
		if (connfd< 0) 
    	{ 
			perror("server acccept failed."); 
    	} 