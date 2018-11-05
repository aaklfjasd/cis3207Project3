#include <stdio.h>
#include "jobQ.h"
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "logQ.h"
#include <string.h>
JobQ *jobQ;
LogQ *logQ;
int acceptingConnections;
int workersActive;

/***************************************************************
 **********************WORKER THREAD TEST***********************
 *This method tests the worker thread compatability with both
 *lists simultaneously (simulating server conditions).
 *The main thread allocates integers on the heap and adds them
 *to the jobQ.  The 10 worker threads then proceed to empty the
 *jobQ, converting each integer to a string, and adding that string
 *to the logQ.  Meanwhile, the writer thread is removing elements
 *from the logQ and printing them to the screen.  After some period
 *of time (in this case, 1000 server requests) the server stops
 *accepting connections, and lets the workers know, while waiting
 *for them to finish.  When the jobQ is empty, and the worker threads
 *are finished, the server informs the writer , and the writer
 *completes its print.  Finally cleanup is performed, whiel validating
 *the logQ is in fact empty.
 ***************************************************************/
void *workerThr(void *arg){
    while(1){
        //printf("Value popped from list : %s\n", pop(logQ));
        int *i = jobPop(jobQ);
        if(i == NULL){
            break;
        }
        else{
            //printf("Popped pointer: %p ::: value : %d\n", i, *i);
            int length = snprintf(NULL, 0,"%d\n", *i);
            //char* string = malloc(length*sizeof(char)+1);
            char string[length*sizeof(char) + 1];
            sprintf(string, "%d\n", *i);
            push(logQ, string);
            free(i);
        }
    }
    return 0;
}

void *writerThr(void *arg){
    while(1){
        //printf("writer about to call pop\n");
        char* val= pop(logQ);
        //printf("Value popped from list : %s\n", pop(logQ));
        if(val == NULL){
            break;
        }
        else{
            printf("Popped %s", val);
        }
    }
    return 0;
}

int main(int argc, char** argv){
    int j;
    for(j = 0; j < 100; j++){
        acceptingConnections = 1;
        jobQ = newJobQ();
        logQ = newLogQ();
        pthread_t writerThread;
        pthread_t workerThread[10];
        
        int i;
        for(i = 0; i < 10; i++){
            //create threads set to run workers method
            pthread_create(&workerThread[i], NULL, workerThr, NULL);
        }
        pthread_create(&writerThread, NULL, writerThr, NULL);
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
        workersFinished(logQ);
        pthread_join(writerThread, NULL);
        if(!isEmpty(logQ)){
            printf("UNWRITTEN VALUES: logq not empty\n");
            break;
        }
        destroyLogQ(logQ);
    }
}
