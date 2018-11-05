// Client side C/C++ program to demonstrate Server


#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 1024
/******************************************************
 *clientdemo is simply a test driver for the server.
 *clientdemo runs, first initializing the dictionary
 *with the default dictionary.  The client then creates
 *20 threads, each of which establishes a new connection
 *with the server, and sends a random word (USES ARC4RANDOM)
 * in the dictionary.  The server responds with (OK) the
 *socket is closed, and the thread repeats this process
 *20 times.  We verify proper operation from the server
 *end.  For each running of clientdemo, the server should
 *add a line in the logfile.  Wordcount on the lines of
 *logfile verifies correctness.
 ******************************************************/

char** dictionary;
int length;

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
            dictionary[i] = strdup(word);
        }
    }
    fclose(dictFile);
}
/*******************************************************
 *METHOD destroyDictionary
 *ITERATES dictionary, freeing allocated memory
 *******************************************************/
void destroyDictionary(){
    int i;
    for(i = 0; i < length; i++){
        free(dictionary[i]);
    }
    free(dictionary);
}

/*******************************************************
 *METHOD clientThread LOOPS 10 times doing the following:
 
 *1. Establishes connected siocket with the server
 *2. chooses a random word in the dictionary
 *3. Sends word to the socket
 *4. Recieves response, printing to teh screen
 *5. closes socket.
 *******************************************************/
void *clientThr(void *arg){
    //CLIENTDEMO
    
    //ThreadLogic
    //connect to the server
    //randomly choose to spell or misspell a word
    //send the word to the server
    //close connection
    int i;
    for(i = 0; i < 10; i++){
        //printf("starting client socket!\n");
        sleep(1);
        
        struct sockaddr_in address;
        int sock = 0, valread;
        struct sockaddr_in serv_addr;
        int randomIndex = arc4random_uniform(length);
        char *hello;
        hello = dictionary[randomIndex];
        //printf("word: %s\n", dictionary[randomIndex]);
        char buffer[1024] = {0};
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error \n");
            return 0;
        }
        
        memset(&serv_addr, '0', sizeof(serv_addr));
        
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);
        
        // Convert IPv4 and IPv6 addresses from text to binary form
        if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
        {
            printf("\nInvalid address/ Address not supported \n");
            return 0;
        }
        
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("\nConnection Failed \n");
            return 0;
        }
        valread = read(sock, buffer, 1024);
        if(valread < 0){
            printf("ERROR reading from socket\n");
            
        } else if(valread == 0){
            printf("socket closed by server\n");
        }else{
            buffer[valread] = '\0';
            //printf("readFromSocket: %s\n", buffer);
            send(sock , hello , strlen(hello) , 0 );
            valread = read(sock, buffer, 1024);
            if(valread < 0){
                printf("ERROR reading from socket\n");
            }
            else if(valread == 0){
                printf("socket closed by server\n");
            }else{
                buffer[valread] = '\0';
                printf("readFromSocket: %s\n", buffer);
            }
        }
        close(sock);
    }
    return 0;
}

/*******************************************************
 *METHOD main
 *First initialzies dictionary, and then creates 20 threads.
 *main waits for all threads to complete, and destroys
 *dictionary
 *******************************************************/
int main(int argc, char const *argv[])
{
    initDictionary();
    printf("INITED DICTIONARY\n");
    pthread_t clientThread[10];
    int j;
    
    for(j = 0; j < 10; j++){
        pthread_create(&clientThread[j], NULL, clientThr, NULL);
        
    }
    
    for(j = 0; j < 10; j++){
        pthread_join(clientThread[j], NULL);
    }
    
    destroyDictionary();
}

