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

	int listener;
	int newfd;

	int rc = 0;
	int yes = 1;

	int recvdBytes = 0;
	char buffer[MAXLINE];

	struct sockaddr_in servAddr;
	struct sockaddr_in remoteAddr;

	socklen_t addrLen = sizeof(remoteAddr);

	bool isConnected = true;
	int i, j; 


// TCP SOCKET.
//=====================================================================================================

	/* Create a TCP socket. */
  	listener = socket(AF_INET, SOCK_STREAM, 0);
	if (listener < 0)
	{
		perror("[-] Could not create socket.\n");
	}
	printf("\n[+] Socket was created. File descriptor: %d\n", listener);

	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0){
		perror("setsockopt failed.\n");
		return EXIT_FAILURE;
	}

	/* Fill in the server's address and data. */
	memset(&servAddr, 0, sizeof(servAddr));  		
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(SERVER_PORT);				
	servAddr.sin_addr.s_addr = INADDR_ANY;

  	/* Bind socket to an address. */
	rc = bind(listener, (struct sockaddr*)&servAddr, sizeof(servAddr));
	if (rc < 0)
	{
		perror("[-] Failed to bind socket.");
		return EXIT_FAILURE;
	}
	printf("[+] Socket was successfully bound to %s:%d\n", inet_ntoa(servAddr.sin_addr), SERVER_PORT);
	
	/* Listen for connections. */
	if(listen(listener, MAX_CLIENTS) < 0)
	{	
		perror("[-] Listen() failed.\n");
		return EXIT_FAILURE;
	}
	printf("Listenning, waiting for connection on port: %u...\n\n", SERVER_PORT);

	/* Create the master and copy file descriptor set and zero it. */
	fd_set master;
	FD_ZERO(&master);

	/* Add our first socket; the listening socket */
	FD_SET(listener, &master);

//=====================================================================================================
// LOOP
//=====================================================================================================
	while(isConnected)
	{ 
		
		/* Clear the buffer */
		memset(&buffer, '\0', sizeof(buffer));

		/*
			Make a copy of the file descriptor set, because calling select() is destructive.
			The copy contains the sockets that are accepting connections requests or messages.
			When it is passed into select(), the clients sending a message at that time will be copied.
			The other sockets will be lost, hence we need to make a copy!
		*/

		fd_set copy = master;	

		/* Check who's talking to the server */
		int fdmax = select(0, &copy, NULL, NULL, NULL);
		
		/* Loop through all the current connections | potential connections. */
		for (i = 0; i < fdmax; i++)
		{
			if(FD_ISSET(i, &copy))
			{
				if(i == listener)
				{
					// Accept a new connection
					addrLen = sizeof(remoteAddr);
					newfd = accept(listener, (struct sockaddr*)&remoteAddr, &addrLen);
					if(newfd < 0)
					{
						perror("accept() failed.\n");
					} 
					else {
						// // Add the new connection to the list of connected clients.
						FD_SET(newfd, &master);
						// Updatera max.
						if(newfd > fdmax)
						{
							fdmax = newfd;
						}
						printf("[+] New connection accepted from %s:%d, FD: %d\n", 
							inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port), newfd);

						sprintf(buffer, "Welcome to TCP Chat Server!\n");
						send(newfd, buffer, strlen(buffer), 0);
					}
				} 
				else {
					/* Receive message */
					memset(&buffer, '\0', sizeof(buffer));
					recvdBytes = recv(i, buffer, MAXLINE, 0);
					if(recvdBytes == 0)
					{	
						printf("Selectserver: socket %d hung up\n", i);
						// error.
						close(i);
						FD_CLR(i, &master);
					}
					// Send to msg to all other clients.
					for(j = 0; j <= fdmax; j++){
						if(FD_ISSET(j, &master)){

							if(j != listener && j != i){
								if(send(j, buffer, recvdBytes, 0) < 0){
									perror("error while sending to all clients.\n");
								}
							}
						}
					}
				}
			}
		}
	}
return	0;
}