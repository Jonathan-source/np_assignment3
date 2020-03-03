#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h> 		
#include <sys/socket.h>		 
#include <netinet/in.h>	
#include <sys/time.h>
#include <unistd.h>

#define MAXLINE 1024
#define MAX_CLIENTS 5
using namespace std;



int main(int argc, char *argv[])
{
  
  if(argc != 2)
	{
		perror("error, arguments are missing.\nManual: ./server Port.\n");
		return EXIT_FAILURE;
	}
	unsigned short int SERVER_PORT = atoi(argv[1]);


// VARIABLES  
//=====================================================================================================

	int sockfd;
	int newSocket;

	int rc = 0;

	char buffer[MAXLINE];

	struct sockaddr_in servAddr;
	struct sockaddr_in newAddr;

	socklen_t cliLen = sizeof(newAddr);

	pid_t pid;

	bool isConnected = true;



// TCP SOCKET.
//=====================================================================================================

	/* Create an TCP socket. */
  	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("[-] Could not create socket.\n");
	}
	printf("\n[+] Socket was created. File descriptor: %d\n", sockfd);

	/* Fill in the server's address and data. */
	memset(&servAddr, 0, sizeof(servAddr));  		
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(SERVER_PORT);				
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  	/* Bind socket to an address. */
	rc = bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr));
	if (rc < 0)
	{
		perror("[-] Failed to bind socket.");
		return EXIT_FAILURE;
	}
	printf("[+] Socket was successfully bound to %s:%d\n", inet_ntoa(servAddr.sin_addr), SERVER_PORT);
	
	/* Listen for connections. */
	if(listen(sockfd, MAX_CLIENTS) < 0)
	{	
		perror("[-] Listen() failed.\n");
		return EXIT_FAILURE;
	}
	printf("Listenning, waiting for connection on port: %u...\n\n", SERVER_PORT);


//=====================================================================================================
// LOOP
//=====================================================================================================
	while(isConnected)
	{ 
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &cliLen);
		if(newSocket < 0)
		{
			isConnected = false;
			return EXIT_FAILURE;
		}
		printf("[+] Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));


		if((pid = fork()) == 0)
		{
			close(sockfd);
		
			while(1)
			{
				/* Clear the buffer */
				memset(&buffer, '\0', sizeof(buffer));

				recv(newSocket, buffer, MAXLINE, 0);
				printf("Client: %s\n", buffer);
				send(newSocket, buffer, strlen(buffer), 0);
			}
		}
	}
	close(newSocket);
}

	return	0;
}