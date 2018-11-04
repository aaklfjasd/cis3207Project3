
#include <stdio.h>
#include <stdlib.h>
#include "logQ.h"
#include <string.h>

int workersActive = 0;

Node* newNode(char* logEntry){
    Node* node = malloc(sizeof(Node));
    node->logEntry = strdup(logEntry);
    return node;
}

/***************************************************************
 *newQueue() allocates memory for empty Queue, inits the queue,
 * inits size to 0, and returns a pointer
 ***************************************************************/
LogQ* newLogQ(){
    LogQ* newQ = malloc(sizeof(LogQ));
    newQ->size =  0;
    int rc = pthread_mutex_init(&newQ->lock, NULL);
    if (rc != 0){
        printf("Lock not initialized!\n");
    }
    rc = pthread_cond_init(&newQ->fill, NULL);
    if (rc != 0){
        printf("CV not initialized!\n");
    }
    workersActive = 1;
    return newQ;
}

/***************************************************************
 *push(Queue*, Event*) is a push method if implementing FIFO
 *Queue (cpuQue, disk_i queue). Push creates a new node to hold
 *event,  then inserts the node in last place (last place is
 *tracked by queue). If queue is empty, then node becomes first
 *element in queue.
 ***************************************************************/
void push(LogQ* targetQ, char* entryToAdd){
    pthread_mutex_lock(&targetQ->lock);
    Node* nodeToAdd = newNode(entryToAdd);
    if(isEmpty(targetQ)){
        targetQ->firstNode = nodeToAdd;
    }
    else{
        targetQ->lastNode->nextNode = nodeToAdd;
    }
    targetQ->lastNode = nodeToAdd;
    targetQ->size += 1;
    pthread_cond_signal(&targetQ->fill);
    pthread_mutex_unlock(&targetQ->lock);
}
/**********************************************************************
 Is empty returns yes if target Queue is empty
 ***********************************************************************/
int isEmpty(LogQ* logQ){
    return(logQ->size == 0);
}

/***************************************************************
 printQ iterates the queue, printing each element in order
 ***************************************************************/
void printQ(LogQ* targetQ){
    if(!isEmpty(targetQ)){
        Node *currentNode = targetQ->firstNode;
        for(int i = 0; i < targetQ->size; i++){
            printf("Entry %d : %s", i, currentNode->logEntry);
            currentNode = currentNode->nextNode;
        }
    }
}

/***************************************************************
 pop(targetQueue) removes first element from queue, and returns it to user.
 **removeNode is called on the first node, and memory is deallocated for node **(but not event)**
 **returns NULL if targetQueue is empty.
 *****************************************************************/
char* pop(LogQ* targetQ){
    //TODO: add condition variable for remove from empty Q!!
    pthread_mutex_lock(&targetQ->lock);
    while(isEmpty(targetQ) && workersActive){
       // printf("writer going to sleep on empty \n");
        pthread_cond_wait(&targetQ->fill, &targetQ->lock);
       // printf("writer awake\n");
    }
    char* event = NULL;
    Node* nodeToBeRemoved;
    if (!isEmpty(targetQ)){
        nodeToBeRemoved = targetQ->firstNode;
        event = nodeToBeRemoved->logEntry;
        if (targetQ->size > 1){
            targetQ->firstNode = nodeToBeRemoved->nextNode;
        }
        free(nodeToBeRemoved);
        targetQ->size -= 1;
    }
    pthread_mutex_unlock(&targetQ->lock);
    return event;
}

/***************************************************************
 ***************************************************************/
void workersFinished(LogQ * logQ){
    pthread_mutex_lock(&logQ->lock);
    workersActive = 0;
    pthread_cond_signal(&logQ->fill);
    pthread_mutex_unlock(&logQ->lock);
}

/***************************************************************
 destroyQueue(Queue*) removes all nodes and events in queue.
 * functions by popping and freeing memory for elements of the list until the list is empty.
 ***************************************************************/
void destroyLogQ(LogQ * targetQueue){
    while(!isEmpty(targetQueue)){
        pop(targetQueue);
    }
    free(targetQueue);
}
