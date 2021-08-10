#include <stdlib.h>
#include <stdio.h>   
#include <string.h>
#include <arpa/inet.h>  
#include <sys/types.h> 		
#include <sys/socket.h>	
#include <unistd.h>	 
#include <netinet/in.h>	
#include <pthread.h>

/* You will to add includes here */
#define BUFFER_SIZE 256
#define DISCONNECT 0

typedef struct thread_data {
    int sockfd;
    int connected;
} thread_data_t;

// Global variable for threads.
thread_data_t * g_data = NULL;

// Mutex for thread synchronization. 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void * send_message(void * arg);
void * recv_message(void * arg);




//================================================
// Entry point of the client application.
//================================================
int main(int argc, char * argv[])
{   
    /*
    * Check command line arguments. 
    */
    if(argc != 3) {
		printf("Syntax: %s <IP>:<PORT> <NICK> \n", argv[0]);
		exit(EXIT_FAILURE);
	}

    char delim[] = ":";
    char * pHost = strtok(argv[1], delim);
    char * pPort = strtok(NULL, delim);
    char * pNickname = argv[2];
    int iPort = atoi(pPort);
    printf("Host %s, and port %d. Nickname: %s.\n", pHost, iPort, pNickname);


    /* 
    * Socket: create the parent TCP socket. 
    */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
	    perror("socket()");
        exit(EXIT_FAILURE);
    } 


    /*
    * Fill in the server's address and data.
    */
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_addr.s_addr = inet_addr(pHost);
    servAddr.sin_port = htons(iPort);
    servAddr.sin_family = AF_INET;


    /* 
    * Connect: connect to remote server
    */    
    int status = connect(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr));
    if(status < 0) {
	    perror("connect()");
        exit(EXIT_FAILURE);
    }


    char buffer[BUFFER_SIZE];    
    memset(&buffer, 0, sizeof(buffer));


    /* 
    * Recieve: recieve protocol from server.
    */
    status = recv(sockfd, buffer, sizeof(buffer), 0);
    if(status < 0) {
	    perror("recv()");
        exit(EXIT_FAILURE);
    } 
    printf("%s", buffer);


    // Check if protocol is supported.
    const char * protocol = "HELLO 1\n";
    if(strcmp(buffer, protocol) != 0) {
	    perror("protocol not supported.");
        exit(EXIT_FAILURE);
    }
 
    // Add the cmd 'NICK' to message.
    const char * pCmd = "NICK ";
    memset(&buffer, 0, sizeof(buffer));
    if (strlen(pCmd) + strlen(pNickname) < sizeof(buffer)) {
        strcpy(buffer, pCmd);
        strcat(buffer, pNickname);
    }


    /* 
    * Send: send nickname to server.
    */ 
    status = send(sockfd, buffer, strlen(buffer), 0);
    if(status < 0) {
	    perror("send()");
        exit(EXIT_FAILURE);
    } 
    printf("%s\n", buffer);


    /* 
    * Recieve: recieve validation of NICK from server.
    */  
    memset(&buffer, 0, sizeof(buffer));
    status = recv(sockfd, buffer, sizeof(buffer), 0);
    if(status < 0) {
	    perror("recv()");
        exit(EXIT_FAILURE);
    }

    // Check if nickname was valid.
    const char * pValidation = "OK\n";
    if(strcmp(buffer, pValidation) != 0) {
        perror("server did not accept nickname.");
        exit(EXIT_FAILURE);
    }
    printf("%s", buffer);


    //======================= CONNECTED =======================//
    printf("Connected to server and ready to chat ('exit' to leave chat)...\n");

    // Data for threads.
    g_data = (thread_data_t*)malloc(sizeof(thread_data_t));
    g_data->sockfd = sockfd;
    g_data->connected = status;


    /* 
    * Thread: new thread that handle send.
    */
    pthread_t thread_send;
    status = pthread_create(&thread_send, NULL, &send_message, (void *) g_data);
    if(status < 0) {
        perror("pthread_create()");
        exit(EXIT_FAILURE);
    }


    /* 
    * Thread: new thread that handle recieve.
    */
    pthread_t thread_recv;
    status = pthread_create(&thread_recv, NULL, &recv_message, (void *) g_data);
    if(status < 0) {
        perror("pthread_create()");
        exit(EXIT_FAILURE);
    }


    /* 
    * Cleanup.
    */    
    pthread_join(thread_send, NULL);
    pthread_join(thread_recv, NULL);
    pthread_mutex_destroy(&mutex);
    if(g_data != NULL)
        free(g_data);
    close(sockfd);

    return EXIT_SUCCESS;
}


/*
* Thread function that handles sending messages.
*/
void * send_message(void * arg)
{
    thread_data_t * data = (thread_data_t *)arg;
    char buffer[BUFFER_SIZE];

    while(data->connected)
    {
        memset(&buffer, 0, sizeof(buffer));
        fgets(buffer, BUFFER_SIZE, stdin);

        if(strcmp(buffer, "exit\n") == 0) 
        {   
            pthread_mutex_lock(&mutex);
            data->connected = DISCONNECT;
            pthread_mutex_unlock(&mutex);
        }
        
        if(send(data->sockfd, buffer, strlen(buffer), 0) < 0) {
	        perror("Thread send()");
        }
    } 

    perror("Terminating send_message():");
    return NULL;
}


/*
* Thread function that handles recieveing messages.
*/
void * recv_message(void * arg)
{
    thread_data_t * data = (thread_data_t *)arg;
    char buffer[BUFFER_SIZE];

    while(data->connected)
    {
        memset(&buffer, 0, sizeof(buffer));
        if(recv(data->sockfd, buffer, sizeof(buffer), 0) < 0) // SOCK_NONBLOCK?
        { 
	        perror("Thread recv()");
            pthread_mutex_lock(&mutex);
            data->connected = DISCONNECT;
            pthread_mutex_unlock(&mutex);
        }

        printf("%s", buffer);
    }

    perror("Terminating recv_message():");
    return NULL;
}
