#include <stdio.h>
#include "logQ.h"
#include <string.h>
#include <pthread.h>
#include <unistd.h>
/****************************************************************
 *************************LOG Q TEST ****************************
 ****************************************************************
 *Test for the jobQ.  Spawns 10 worker threads, and begins adding
 *heap-allocated STRINGS to the logQ. The writer thread pops values
 *from the logQ and frees them.  Then the main thread waits for all
 *threads to complete after broadcasting it is no longer accepting
 *connections. Upon completion, we check to make sure there is nothing
 *left in the logQ (indicating error), and finally, we free the logQ.
 ****************************************************************/
char* strings[10] = {"a\n", "b\n", "c\n", "d\n", "e\n", "f\n", "g\n", "h\n", "i\n", "j\n"};
LogQ *logQ;
int workersRunning;
int writerRunning;

void *mythread(void *arg) {
    int i = *((int *) arg);
    free(arg);
    while(workersRunning){
        push(logQ,strings[i]);
    }
    return 0;
}

void *writerThr(void *arg){
    while(1){
        char* val = pop(logQ);
        if(val == NULL){
            break;
        }
        printf("Popped %s\n from the Q\n", val);
    }
    return 0;
}
int main(int argc, char** argv){
    int j;
    for(j = 0; j < 10   ; j++){
        printf("so far so good");
        logQ = newLogQ();
        printf("made logQ! Current size: %d\n", logQ->size);
        char * strings[10];
        pthread_t p[10];
        pthread_t writerThread;
        writerRunning = 1;
        workersRunning = 1;
        int i;
        
        pthread_create(&writerThread, NULL, writerThr, NULL);
        for(i = 0; i < 10; i++){
            int *arg = malloc(sizeof(*arg));
            *arg = i;
            int rc = pthread_create(&p[i], NULL, mythread, arg);
            if(rc != 0){
                printf("Unable to create thread\n");
                break;
            }
        }
        sleep(1);
        workersRunning = 0;
        for(i = 0; i < 10; i++){
            pthread_join(p[i], NULL);
        }
        writerRunning = 0;
        workersFinished(logQ);
        pthread_join(writerThread, NULL);
        if(!isEmpty(logQ)){
            printf("UNWRITTEN VALUES: logq not empty\n");
            break;
        }
        destroyLogQ(logQ);
    }
}
