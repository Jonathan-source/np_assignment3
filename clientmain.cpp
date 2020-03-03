#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h> 		
#include <sys/socket.h>		 
#include <netinet/in.h>	
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>


#define MAXLINE 1024

char NICK[12];


using namespace std;

int main(int argc, char *argv[])
{
  
	if(argc != 3)
	{
		printf("Usage: %s <ip>:<port>\n", argv[0]);
		return EXIT_FAILURE;
	}
	unsigned short int SERVER_PORT = atoi(argv[2]);


// VARIABLES  
//=====================================================================================================

	int sockfd;

  	int check = 0;

	char buffer[MAXLINE];

	struct sockaddr_in servAddr;

	bool isConnected = true;



// TCP SOCKET.
//=====================================================================================================
sockfd = socket(AF_INET, SOCK_STREAM, 0);
if(sockfd < 0)
{
	perror("[-] Failed creating socket.\n");
	return EXIT_FAILURE;
}
printf("[+] Socket was created.\n");


memset(&servAddr, '\0', sizeof(servAddr));
servAddr.sin_family = AF_INET;
servAddr.sin_port = htons(SERVER_PORT);
servAddr.sin_addr.s_addr = inet_addr(argv[1]);

check = connect(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr));
if (check < 0)
{
	perror("[-] Error while connecting.\n");
	return EXIT_FAILURE;
}
printf("[+] Sucessfully connected to %s:%d\n", inet_ntoa(servAddr.sin_addr), SERVER_PORT);



//=====================================================================================================
// LOOP
//=====================================================================================================
while(isConnected)
{ 	
	/* Clear the buffer */
	memset(&buffer, '\0', sizeof(buffer));

	check = recv(sockfd, buffer, MAXLINE, 0);
	if(check < 0)
	{	
		perror("[-] Error while receiving data.\n");
		return EXIT_FAILURE;
	}
	printf("%s\n", buffer);

	/* Clear the buffer */
	memset(&buffer, '\0', sizeof(buffer));	

	scanf("%s", &buffer[0]);
	check = send(sockfd, buffer, strlen(buffer), 0);
	if(check < 0)
	{
		perror("[-] Error while sending data.\n");
		return EXIT_FAILURE;
	}
}

	return 0;
}



void assign_NICK()
{



}