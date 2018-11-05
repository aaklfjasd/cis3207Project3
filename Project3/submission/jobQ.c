
#include "jobQ.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/***************************************************************
 *jobQ is a threadsafe linked list to hold ints containing
 *the sockets of the spellchecked requests. Code adapted
 *from project1 Queue.
 *NOTE NOT ALL methods are threadsafe.  Only those required for
 *this assignment were adapted.
 ***************************************************************/
int acceptingConnections = 0;

/***************************************************************
 METHOD: newJobNode allocates heap memory for a new node.  The
 *node is returned to the caller, used only by push method in
 *current adaptation.
 ***************************************************************/
JobNode* newJobNode(int* socket){
    JobNode* node = malloc(sizeof(JobNode));
    node->socket = socket;
    return node;
}

/***************************************************************
 *newQueue() allocates memory for empty JobQ, inits the queue,
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
 *push(Queue*, Event*) is a push method for the linked list.
 *It is thread safe, allowing the server to access it without
 *being disturbed by worker threads.  When adding, the server
 *signals the workers.
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
 *WARNING jobQIsEmpty is not threadsafe, and must only be accessed from
 *INSIDE JobQ while holding mutex.
 ***********************************************************************/
int jobQIsEmpty(JobQ* jobQ){
    return(jobQ->size == 0);
}

/***************************************************************
 printQ iterates the queue, printing each element in order
 * NOT THREADSAFE, must only be accessed from INSIDE
 * JobQ while holding mutex.
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
 **pop is THREADSAFE.  if the Q is empty, and the server is still
 *accepting connections, the calling thread will sleep until
 *signalled (either by server closing or adding to the Q).
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
 METHOD: serverFinished obtains the lock, sets the value of the
 *accepting connections flag to 0, and broadcasts to all sleeping
 *threads it has done so.  It then releases the lock.
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
 *NOT THREAD SAFE, and should only be called after jobQ is
 *inactive.
 ***************************************************************/
void destroyJobQ(JobQ * targetQueue){
    while(!jobQIsEmpty(targetQueue)){
        jobPop(targetQueue);
    }
    int rc = pthread_mutex_destroy(&targetQueue->lock);
    if (rc != 0 ){
        printf("ERROR : jobQ lock not destroyed!\n");
    }
    free(targetQueue);
}
