#include <stdlib.h>
#include <stdio.h>   
#include <string.h>
#include <arpa/inet.h>  
#include <sys/types.h> 		
#include <sys/socket.h>	
#include <unistd.h>	 
#include <pthread.h>
#include <ctype.h>

/* You will to add includes here */
#include "queue.h"
#include "linkedlist.h"

#define BUFFER_SIZE 256
#define DEBUG
#define ERROR -1
#define DISCONNECT -1
#define SERVER_BACKLOG 100
#define MAX_CLIENTS 5
#define NICK_MIN_CHARS 2
#define NICK_MAX_CHARS 12
#define THREAD_POOL_SIZE 10

// TODO: Linked List...?
client_t * g_clients[MAX_CLIENTS];
int g_num_clients = 0;

// Thread related stuff.
pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

// A thread function to handle connections.
void * handle_connection(void * arg);

// A thread function to handle the thread pool.
void * thread_function(void * arg)
{
  while(1) 
  {
    client_t * pClient = NULL;
    pthread_mutex_lock(&mutex); 
    if((pClient = dequeue()) == NULL) {
      pthread_cond_wait(&condition_var, &mutex);
      pClient = dequeue();
    }
    pthread_mutex_unlock(&mutex);

    if(pClient != NULL) {
      handle_connection((void *)pClient);
    }
  }
}

// A thread function to send message to all clients,
// if any messages has been recieved.
void * send_message(void * arg)
{
  while(1)
  {
    char buffer[BUFFER_SIZE];
    for(int i = 0; i < MAX_CLIENTS; i++) 
    {
      if(g_clients[i] != NULL) 
      {
        memset(&buffer, 0, sizeof(buffer));
        if(recv(g_clients[i]->connfd, buffer, sizeof(buffer), SOCK_NONBLOCK) == ERROR)
        {
	        printf("could not recv from client. Removing it.");
          pthread_mutex_lock(&lock);
          free(g_clients[i]);
          g_clients[i] = NULL;
          g_num_clients--;
          pthread_mutex_unlock(&lock);
	      } 

        if(strlen(buffer) > 0) 
        {
          if(send(g_clients[i]->connfd, buffer, sizeof(buffer), 0) == ERROR)
          {
            printf("could not recv from client. Removing it.");
            pthread_mutex_lock(&lock);
            free(g_clients[i]);
            g_clients[i] = NULL;
            g_num_clients--;
            pthread_mutex_unlock(&lock);
          }
        }
      }
    }
  }
  return NULL;
}


void error(const char * msg){
  fprintf(stderr,"[Error]: %s \n", msg);  
}

#ifdef DEBUG 
  void DebugText(const char * msg){
    printf("[Debug]: %s \n", msg); 
  } 
#endif


void get_request(const int *sockfd);




//==========================================================
// Entry point of the server application.
//==========================================================
int main(int argc, char *argv[])
{ 
  if(argc != 2) 
	{
		error("incorrect arguments");
		printf("Syntax: %s <IP>:<PORT> \n", argv[0]);
		return EXIT_FAILURE;
	}

  char delim[] = ":";
  char * cHost = strtok(argv[1], delim);
  char * cPort = strtok(NULL, delim);

  int iPort = atoi(cPort);
#ifdef DEBUG 
  printf("Host %s, and port %d.\n", cHost, iPort);
#endif


  //======================= INITIALIZE =======================//


  // Create TCP socket.
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd == ERROR) {
	  error("could not create socket.");
    return EXIT_FAILURE;
  }

  DebugText("TCP socket created.");

  // Fill in the server's address and data.
  struct sockaddr_in servAddr;
  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_addr.s_addr = inet_addr(cHost);
  servAddr.sin_port = htons(iPort);
  servAddr.sin_family = AF_INET;

  // Bind socket.
  if(bind(sockfd, (struct sockaddr * ) &servAddr, sizeof(servAddr)) == ERROR){
	  error("could not bind socket to a socket address.");
    return EXIT_FAILURE;
  }

  DebugText("Socket bound to socket address.");

  // Listen for connections...
  if(listen(sockfd, SERVER_BACKLOG) == ERROR){
	  error("could not invoke listen().");
    return EXIT_FAILURE;
  }

  //======================= CONNECTED =======================//
  DebugText("Listenning for client connections...");

  for(int i = 0; i < MAX_CLIENTS; i++)
    g_clients[i] = NULL;

  // Initialize handle_connection() thread pool.
  for(int i = 0; i < THREAD_POOL_SIZE; i++)
    pthread_create(&thread_pool[i], NULL, &thread_function, NULL); 

  // Initialize send_message() thread pool.
  pthread_t send_message_thread;
  pthread_create(&send_message_thread, NULL, &send_message, NULL); 


  while(1) {
    get_request(&sockfd);
  }
  



  // Clean up.
  for(int i = 0; i < MAX_CLIENTS; i++)
  {
    if(g_clients[i] != NULL)
      free(g_clients[i]);
  }

  for(int i = 0; i < THREAD_POOL_SIZE; i++)
    pthread_join(thread_pool[i], NULL);

  pthread_join(send_message_thread, NULL);

  pthread_mutex_destroy(&mutex);
  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&condition_var);
  close(sockfd);

  return EXIT_SUCCESS;
}


void get_request(const int * sockfd)
{
  // Check if max clients are reached.
  if(g_num_clients < MAX_CLIENTS)
  {
    client_t * client = malloc(sizeof(client_t));
    socklen_t clientLen = sizeof(client->addr);

    // Accept connection.
    client->connfd = accept(*sockfd, (struct sockaddr *) &client->addr, &clientLen);
    if(client->connfd == ERROR)	{
      error("could not connect with the client.");
      free(client);
    }
    else {
      pthread_mutex_lock(&mutex);
      enqueue(client);  // Add client to handle_connection-queue.
      pthread_cond_signal(&condition_var);
      pthread_mutex_unlock(&mutex);
    }
  }
  else
    DebugText("Maximum clients reached.");
}



// Validate client. Releases its memory upon not being correctly validated.
void * handle_connection(void * arg)
{
  client_t * client = (client_t *)arg;

  // Send protocol.	
	if(send(client->connfd, "HELLO 1\n", sizeof("HELLO 1\n"), 0) == ERROR){
		error("could not send message to client.");
    free(client);
    return NULL;
	}

#ifdef DEBUG 
  printf("Client %s:%d connected, waiting for confirmation...\n", inet_ntoa(client->addr.sin_addr), client->addr.sin_port);
#endif
	
  // Recieve ACK from client (NICK nickname).
  char buffer[BUFFER_SIZE];
  memset(&buffer, 0, sizeof(buffer));
	if(recv(client->connfd, buffer, sizeof(buffer), 0) == ERROR){
	  error("could not recieve confirmation from client.");
    free(client);
    return NULL;
	}
	printf("%s\n", buffer);

  // Parse string.
  char delim[] = " ";
  char * pCmd = strtok(buffer, delim);
  char * pNickname = strtok(NULL, delim);
  printf("cmd: %s name: %s \n", pCmd, pNickname);

  // Check if ACK is correct (NICK).
  const char * pACK = "NICK";
  if(strcmp(buffer, pACK) != 0)
  {
	  send(client->connfd, "ERROR malformed command.\n", sizeof("ERROR malformed command.\n"), 0);
    free(client);
    return NULL;
  }

  // Check if nickname is between 2-12 characters.
  int nickname_length = strlen(pNickname);
  if(nickname_length < NICK_MIN_CHARS || nickname_length > NICK_MAX_CHARS) {
    send(client->connfd, "ERROR invalid nickname.\n", sizeof("ERROR invalid nickname.\n"), 0);
    free(client);
    return NULL;
  }

  // Check if nickname has valid characters (A-Za-z0-9\_).
  for(int i = 0; i < nickname_length; i++)
  {
    if(pNickname[i] == '\\' || pNickname[i] == '_') 
      continue;
    
    // Function to verify whether input is either a decimal digit
    // or an uppercase or lowercase letter.
    if(isalnum(pNickname[i]) == 0) {
      send(client->connfd, "ERROR invalid nickname.\n", sizeof("ERROR invalid nickname.\n"), 0);
      free(client);
      return NULL;
    }
  }

  // Send OK.	
	if(send(client->connfd, "OK\n", sizeof("OK\n"), 0) == ERROR){
		error("could not send message to client.");
    free(client);
    return NULL;
	}

  // Client is validated, add client to 'list'.
  pthread_mutex_lock(&lock);
  for(int i = 0; i < MAX_CLIENTS; i++)
  {
    if(g_clients[i] == NULL)
    {
      g_clients[i] = client;
      g_num_clients++;
      break; 
    }
  }
  pthread_mutex_unlock(&lock);

  return NULL;
}
