#ifndef QUEUE_H
#define QUEUE_H
#include "queues.cpp"
#include <cstdio>
#include <cstdlib>
typedef struct node node_t;

// function declarations
void enqueue(int *client_socket);
int* dequeue();

#endif
