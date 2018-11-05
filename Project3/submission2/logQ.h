#ifndef logQ_h
#define logQ_h

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct node{
  struct node * nextNode;
  char * logEntry;
}Node;

typedef struct queue{
    //lock
    Node* firstNode;
    Node* lastNode;
    pthread_mutex_t lock;
    pthread_cond_t fill;
    int size;
}LogQ;

Node* newNode(char* logEntry);
void destroyLogQ(LogQ* targetQueue);
char* pop(LogQ* targetQ);
LogQ* newLogQ();
void push(LogQ* targetQueue, char * stringToAdd);
int isEmpty(LogQ* targetQueue);
void printQ(LogQ* targetQ);
void workersFinished(LogQ *logQ);

#endif /* logQ_h */
