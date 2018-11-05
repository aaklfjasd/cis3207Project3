//
//  jobQ.h
//  
//
//  Created by Leo Vergnetti on 10/29/18.
//

#ifndef jobQ_h
#define jobQ_h
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct jobnode{
    struct jobnode * nextNode;
    int * socket;
}JobNode;

typedef struct jobqueue{
    //lock
    JobNode* firstNode;
    JobNode* lastNode;
    pthread_mutex_t lock;
    pthread_cond_t fill;
    int size;
}JobQ;

JobNode* newJobNode(int* socket);
void destroyJobQ(JobQ* targetQueue);
int* jobPop(JobQ* targetQ);
JobQ* newJobQ();
void jobPush(JobQ* targetQueue, int * socketToAdd);
int jobQIsEmpty(JobQ* targetQueue);
void printJobQ(JobQ* targetQ);
void serverFinished(JobQ *jobQ);
#endif /* jobQ_h */
