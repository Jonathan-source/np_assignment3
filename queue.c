#include "queue.h"

node_t *head = NULL;
node_t *tail = NULL;

void enqueue(client_t * client)
{
    node_t * new_node = malloc(sizeof(node_t));
    new_node->client = client;
    new_node->next = NULL;

    if(tail == NULL) 
        head = new_node;
    else
        tail->next = new_node;

    tail = new_node;
}

client_t * dequeue()
{
    if(head == NULL) return NULL;

    client_t * client = head->client;
    node_t * temp = head;
    head = head->next;

    if(head == NULL) tail = NULL;

    free(temp);
    return client;
}