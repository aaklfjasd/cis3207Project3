
#include "jobQ.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int acceptingConnections = 0;


JobNode* newJobNode(int* socket){
    JobNode* node = malloc(sizeof(JobNode));
    node->socket = socket;
    return node;
}

/***************************************************************
 *newQueue() allocates memory for empty Queue, inits the queue,
 * inits size to 0, and returns a pointer
 ***************************************************************/
JobQ* newJobQ(){
    JobQ* newQ = malloc(sizeof(JobQ));
    newQ->size =  0;
    int rc = pthread_mutex_init(&newQ->lock, NULL);
    if (rc != 0){
        printf("Lock not initialized!\n");
    }
    rc = pthread_cond_init(&newQ->fill, NULL);
    if (rc != 0){
        printf("CV not initialized!\n");
    }
    acceptingConnections = 1;
    return newQ;
}

/***************************************************************
 *push(Queue*, Event*) is a push method if implementing FIFO
 *Queue (cpuQue, disk_i queue). Push creates a new node to hold
 *event,  then inserts the node in last place (last place is
 *tracked by queue). If queue is empty, then node becomes first
 *element in queue.
 ***************************************************************/
void jobPush(JobQ* targetQ, int* socketToAdd){
    pthread_mutex_lock(&targetQ->lock);
    JobNode* nodeToAdd = newJobNode(socketToAdd);
    if(jobQIsEmpty(targetQ)){
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
int jobQIsEmpty(JobQ* jobQ){
    return(jobQ->size == 0);
}

/***************************************************************
 printQ iterates the queue, printing each element in order
 * NOT THREADSAFE at the moment (may not fix)
 ***************************************************************/
void printJobQ(JobQ* targetQ){
    if(!jobQIsEmpty(targetQ)){
        JobNode *currentNode = targetQ->firstNode;
        for(int i = 0; i < targetQ->size; i++){
            printf("Entry %d : %d", i, *currentNode->socket);
            currentNode = currentNode->nextNode;
        }
    }
}

/***************************************************************
 pop(targetQueue) removes first element from queue, and returns it to user.
 **removeNode is called on the first node, and memory is deallocated for node **(but not event)**
 **returns NULL if targetQueue is empty.
 *****************************************************************/
int* jobPop(JobQ* targetQ){
    //TODO: add condition variable for remove from empty Q!!
    pthread_mutex_lock(&targetQ->lock);
    while(jobQIsEmpty(targetQ) && acceptingConnections){
        //printf("worker thread going to sleep on empty \n");
        pthread_cond_wait(&targetQ->fill, &targetQ->lock);
        //printf("worker thread awake\n");
    }
    int* event = NULL;
    JobNode* nodeToBeRemoved;
    if (!jobQIsEmpty(targetQ)){
        nodeToBeRemoved = targetQ->firstNode;
        event = nodeToBeRemoved->socket;
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
void serverFinished(JobQ * jobQ){
    pthread_mutex_lock(&jobQ->lock);
    acceptingConnections = 0;
    pthread_cond_broadcast(&jobQ->fill);
    pthread_mutex_unlock(&jobQ->lock);
}

/***************************************************************
 destroyQueue(Queue*) removes all nodes and events in queue.
 * functions by popping and freeing memory for elements of the list until the list is empty.
 ***************************************************************/
void destroyJobQ(JobQ * targetQueue){
    while(!jobQIsEmpty(targetQueue)){
        jobPop(targetQueue);
    }
    free(targetQueue);
}
