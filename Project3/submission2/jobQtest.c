#include <stdio.h>
#include "jobQ.h"
#include <string.h>
#include <pthread.h>
#include <unistd.h>

/****************************************************************
 *************************JOB Q TEST ****************************
 ****************************************************************
 *Test for the jobQ.  Spawns 10 worker threads, and begins adding
 *heap-allocated integers to the jobQ. The worker threads pop values
 *from the jobQ and free them.  Then the main thread waits for all
 *threads to complete after broadcasting it is no longer accepting
 *connections. Upon completion, we check to make sure there is nothing
 *left in the jobQ (indicating error), and finally, we free the jobQ.
 ****************************************************************/
JobQ *jobQ;
int acceptingConnections;

//Worker threads pop values off of JobQ while its not empty
void *workerThr(void *arg){
    while(1){
        //printf("Value popped from list : %s\n", pop(logQ));
        int *i = jobPop(jobQ);
        if(i == NULL){
            break;
        }
        else{
            //printf("Popped pointer: %p ::: value : %d\n", i, *i);
            free(i);
        }
    }
    return 0;
}

//main method runs everything
int main(int argc, char** argv){
    int j;
    for(j = 0; j < 100; j++){
        acceptingConnections = 1;
        jobQ = newJobQ();
        pthread_t serverThread;
        pthread_t workerThread[10];
        
        int i;
        for(i = 0; i < 10; i++){
            //create threads set to run workers method
            pthread_create(&workerThread[i], NULL, workerThr, NULL);
        }
        for(i =0; i < 1000; i++){
            int *h = malloc(sizeof(int));
            *h = i;
            jobPush(jobQ, h);
        }
        acceptingConnections = 0;
        serverFinished(jobQ);
        for(i = 0; i < 10; i++){
            pthread_join(workerThread[i], NULL);
        }
        if(!jobQIsEmpty(jobQ)){
            printf("UNWRITTEN VALUES: JobQ not empty\n");
        }
        destroyJobQ(jobQ);
    }
}
