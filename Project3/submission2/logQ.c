
#include <stdio.h>
#include <stdlib.h>
#include "logQ.h"
#include <string.h>


/***************************************************************
 *logQ is a threadsafe linked list to hold strings containing
 *the results of the spellchecked requests. Code adapted
 *from project1 Queue.
 *NOTE NOT ALL methods are threadsafe.  Only those required for
 *this assignment were adapted.
 ***************************************************************/
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
 *push(Queue*, entryToAdd*) is a synchronized push method, allowing
 *multiple threads to write to it.  A lock is declared around the
 *method, and elements are added inside the lock.  Once the element
 *is the writer thread is signalled, and finally the lock is released.
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
 METHOD: isEmpty returns yes if target Queue is empty.
 *WARNING: This method is strictly speaking, not threadsafe and in current
 *implementation, method is private, only to be accessed from inside
 *the lock.
 ***********************************************************************/
int isEmpty(LogQ* logQ){
    return(logQ->size == 0);
}

/***************************************************************
 printQ iterates the queue, printing each element in order
 *WARNING: printQ is not threadsafe, and should only be accessible
 *from inside the logQ
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
 *method first quires lock, and then proceeds to pop the next elemetn.
 * if the targetQ is empty, (and we still have active workers) the
 *thread sleeps on the condition.
 *****************************************************************/
char* pop(LogQ* targetQ){
    pthread_mutex_lock(&targetQ->lock);
    while(isEmpty(targetQ) && workersActive){
        pthread_cond_wait(&targetQ->fill, &targetQ->lock);
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
 METHOD: workersFinished aquires the lock, and sets the workersActive
 *flag to 0.  It then signalls the fillbuffer, letting the writer
 *(if sleeping) know there will be no new additions to logQ.
 *The lock is then released. 
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
    int rc = pthread_mutex_destroy(&targetQueue->lock);
    if (rc != 0 ){
        printf("ERROR : logQ lock not destroyed!\n");
    }
    free(targetQueue);
}
