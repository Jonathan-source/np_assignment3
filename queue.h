#ifndef QUEUE_H
#define QUEUE_H

#include <netinet/in.h>	
#include <stdlib.h>

struct client 
{
    int connfd;
    struct sockaddr_in addr;
};
typedef struct client client_t;

struct node 
{
    struct node * next;
    client_t * client;
};
typedef struct node node_t;

void enqueue(client_t * client);
client_t * dequeue();

#endif