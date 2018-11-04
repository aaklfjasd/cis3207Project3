#include <stdio.h>
#include "logQ.h"
#include <string.h>
#include <pthread.h>
#include <unistd.h>

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
        //printf("Value popped from list : %s\n", pop(logQ));
        if(pop(logQ) == NULL){
            break;
        }
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
    
    /* push(logQ, s1);
     printQ(logQ);
     printf("added entry logQ! Current size: %d\n", logQ->size);
     s1 = strdup("oh what a underful world\n");
     push(logQ, s1);
     printQ(logQ);
     printf("Printing popped string: %s\n", pop(logQ));
     destroyLogQ(logQ);
     printf("successfully destroyed Q\n");
     */
}
