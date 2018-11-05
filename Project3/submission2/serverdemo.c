
#include "server.h"
#include <stdio.h>
#include "jobQ.h"
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "logQ.h"
/*******************************************************
 *SERVERDEMO demonstrates the server functionality (and use).
 *First, a user thread is created, which will get exit the
 *server when it reads the ent
 *******************************************************/

/*******************************************************
 *METHOD userThr: simply blockreads until the newline char
 *is read.  Then calls stopServer(), and returns upon
 *completion.
 *******************************************************/
void *userThr(void *arg){
    printf("Press return to quit\n");
    while(fgetc(stdin)!= '\n');
    stopServer();
    printf("returning from userThr()\n");
    return 0;
}

/*******************************************************
*MAIN method - creates a user thread to close upon return
*being pressed.  Calls runserver.  When runServer returns
*main waits for userThread to complete, signalling the
*server has closed.  Then exits.
 *******************************************************/
int main(int argc, char** argv){
    pthread_t userThread;
    pthread_create(&userThread, NULL, userThr, NULL);
    runServer(argc, argv);
    printf("calling join on userthread\n");
    pthread_join(userThread, NULL);
    printf("EXITING DEMO NORMALLY\n");
    return 0;
}
