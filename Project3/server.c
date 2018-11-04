#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "server.h"
#include <stdio.h>
#include "jobQ.h"
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "logQ.h"
#include "open_listenfd.h"
#define BUF_LEN 512

char** dictionary;
int length;
JobQ *jobQ;
LogQ *logQ;
pthread_t writerThread;
pthread_t workerThread[10];
int acceptingConnections;
int workersActive;
int connectionSocket,clientSocket, connectionPort;


/***********************************************************
 *METHOD : lookupWord uses binary search to locate the word
 *INPUT: string
 *OUTPUT: int: 1 if word is in dictionary, 0 else
 ***********************************************************/
int lookupWord(char* word){
    int result = 0;
    int i;
    for(i = 0; i < length; i++){
        if(strcmp(word, dictionary[i]) == 0){
            result = 1;
            break;
        }
    }
    return result;
}
/***********************************************************
 METHOD: workerThr: method contains logic for each worker thread.
 INPUT: threadID: integer used to identify thread
 *workerThr is the logic for each worker thread.  First they pop
 *a socket from the jobQ (jobQ handles mutex/sync).  The threads
 *run until a null value is popped from the Q, which indicates
 *the Q is empty,
 ***********************************************************/
void *workerThr(void *arg){
    while(1){
        int bytesReturned;
        char recvBuffer[BUF_LEN];
        recvBuffer[0] = '\0';
        //
        int clientSocket;
        free(arg);
        char* msgRequest = "Please spell your word\n";
        char* msgResponse;
        char* msgOk = " OK";
        char* msgWrong = " MISSPELLED";
        char* msgHandledBy = "-by worker_id: ";
        char* msgPrompt = ">>>";
        char* msgError = "Word not recieved. ):\n";
        //OUTER LOOP
        //printf("Value popped from list : %s\n", pop(logQ));
        int *i = jobPop(jobQ);
        if(i == NULL){
            break;
        }
        else{
            clientSocket = *i;
            send(clientSocket, msgRequest, strlen(msgRequest), 0);
            //recv() will store the message from the user in the buffer, returning
            //how many bytes we received.
            bytesReturned = recv(clientSocket, recvBuffer, BUF_LEN, 0);
            
            int s;
            for(s = 0; s < bytesReturned; s++){
                if(recvBuffer[s] == '\r' || recvBuffer[s] == '\n'){
                    recvBuffer[s] = '\0';
                }
            }
            
            //Make sure the message was recieved, if not report error
            if(bytesReturned == -1){
                send(clientSocket, msgError, strlen(msgError), 0);
            } else{
                
                msgResponse = strdup(recvBuffer);
                char logEntry[75] = {'\0'};
                strcat(logEntry, msgResponse);
                
                if(lookupWord(logEntry)){
                    strcat(logEntry, msgOk);
                } else{
                    strcat(logEntry, msgWrong);
                }
                send(clientSocket, logEntry, strlen(logEntry), 0);
                int length = snprintf(NULL, 0,"%d", *i);
                char string[length*sizeof(char) + 1];
                sprintf(string, "%d", *i);
                strcat(logEntry, msgHandledBy);
                strcat(logEntry ,string);
                free(msgResponse);
                push(logQ, logEntry);
            }
            close(clientSocket);
            free(i);
        }
    }
    return 0;
}

/*******************************************************
 *METHOD writerThr:
 *******************************************************/
void *writerThr(void *arg){
    FILE * logfile = fopen("logfile.txt", "w+");
    while(1){
        //printf("writer about to call pop\n");
        char* val= pop(logQ);
        //printf("Value popped from list : %s\n", pop(logQ));
        if(val == NULL){
            fclose(logfile);
            break;
        }
        else{
            printf("Popped %s: writing to logQ\n", val);
            fprintf(logfile,"%s\n", val);
            printf("logwritten FREEING mem for string\n");
            free(val);
        }
    }
    return 0;
}


/*******************************************************
 METHOD: initDictionary: inits dictionary values .
 *if unable to load dictionary, loads from standard
 *******************************************************/

void initDictionary(){
    FILE *dictFile = fopen("words.txt", "r");
    char word[50];
    int counter = 0;
    
    while(fgets(word, 50, dictFile)!= NULL){
        counter++;
    }
    length = counter;
    dictionary = malloc(length*sizeof(char*));
    fclose(dictFile);
    dictFile = fopen("words.txt", "r");
    
    int i;
    for(i = 0; i < length; i++){
        if(fgets(word, 50, dictFile)){
            int j;
            for(j = strlen(word); j >0; j--){
                if(word[j] == '\n'){
                    word[j] = '\0';
                    break;
                }
            }
            dictionary[i] = strdup(word);
        }
    }
    fclose(dictFile);
}

/*******************************************************
 METHOD: initServer: inits server values creates threads
 *to handle server logic.  IF initialized correctly, we
 *return 0, else, we return -1.
 *******************************************************/
int initServer(int argc, char** argv){
    initDictionary();
    acceptingConnections = 1;
    jobQ = newJobQ();
    logQ = newLogQ();
    int i, rc;
    
    //VERIFY we have a port number IF not use default 1024
    //TODO: setup default 1024 port
    if(argc == 1){
        printf("No port number entered.\n");
        return -1;
    }
    connectionPort = atoi(argv[1]);
    //VERIFY user port was a valid port, IF NOT use default
    if(connectionPort < 1024 || connectionPort > 65535){
        printf("Port number is either too low(below 1024), or too high(above 65535).\n");
        return -1;
    }
    
    //CREATE NUM_WORKERS threads to work on the server
    //TODO: CREATE constant to specify number of workers
    for(i = 0; i < 10; i++){
        //create user id arg to send to thread
        //create threads set to run workers method
        if((rc = pthread_create(&workerThread[i], NULL, workerThr, NULL))!=0){
            printf("Unable to create worker threads\n");
            return -1;
        }
    }
    //CREATE writer thread to write to the logFILE
    if((rc = pthread_create(&writerThread, NULL, writerThr, NULL))!=0){
        printf("Unable to create writer thread\n");
        return -1;
    }
    
    
    //Does all the hard work for us.
    connectionSocket = open_listenfd(connectionPort);
    if(connectionSocket == -1){
        printf("Could not connect to %s, maybe try another port number?\n", argv[1]);
        return -1;
    }
    return 0;
}


/*******************************************************
 METHOD: runServer:
 *******************************************************/
int runServer(int argc, char** argv){
    //sockaddr_in holds information about the user connection.
    //We don't need it, but it needs to be passed into accept().
    struct sockaddr_in client;
    int clientLen = sizeof(client);
    //Here we attempt to initialize server.  If unable, we exit the program with error.
    if(initServer(argc, argv)){
        printf("Unable to start server, EXITING\n");
        exit(-1);
    }
    
    //If server was properly initialized, we begin accepting connections,
    //adding accepted sockets to jobQ.
    else{
        while(acceptingConnections){
            if((clientSocket = accept(connectionSocket, (struct sockaddr*)&client, &clientLen)) == -1){
                printf("Error connecting to client.\n");
                return -1;
            }
            printf("Connection success!\n");
            int *arg = malloc(sizeof(*arg));
            *arg = clientSocket;
            jobPush(jobQ, arg);
        }
        printf("server exited while loop \n");
        return 0;
    }
}

/*******************************************************
 *METHOD destroyDictionary
 *******************************************************/
void destroyDictionary(){
    int i;
    for(i = 0; i < length; i++){
        free(dictionary[i]);
    }
    free(dictionary);
}

/*******************************************************
 *METHOD stopServer:
 *******************************************************/
int stopServer(){
    ///////KILL THE SERVER DAWG
    acceptingConnections = 0;
    printf("closing serverSocket\n");
    shutdown(connectionSocket, SHUT_RDWR);
    printf("signalling jobQ\n");
    serverFinished(jobQ);
    int i;
    for(i = 0; i < 10; i++){
        pthread_join(workerThread[i], NULL);
    }
    printf("Worker threads finished \n");
    
    if(!jobQIsEmpty(jobQ)){
        printf("ERROR: UNWRITTEN VALUES: JobQ not empty\n");
    }
    destroyJobQ(jobQ);
    workersFinished(logQ);
    pthread_join(writerThread, NULL);
    printf("writer thread finished\n");
    if(!isEmpty(logQ)){
        printf("ERROR: UNWRITTEN VALUES: logQ not empty\n");
        //break;
    }
    destroyLogQ(logQ);
    close(connectionSocket);
    destroyDictionary();
    printf("socket closed!\n");
    printf("Exiting normally!\n");
    return 0;
}
