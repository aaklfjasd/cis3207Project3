// Client side C/C++ program to demonstrate Server


#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 1024

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
 *******************************************************/
void destroyDictionary(){
    int i;
    for(i = 0; i < length; i++){
        free(dictionary[i]);
    }
    free(dictionary);
}

void *clientThr(void *arg){
    //CLIENTDEMO
    
    //ThreadLogic
    //connect to the server
    //randomly choose to spell or misspell a word
    //send the word to the server
    //close connection
    int i;
    for(i = 0; i < 10; i++){
        printf("starting client socket!\n");
        sleep(1);
        
        struct sockaddr_in address;
        int sock = 0, valread;
        struct sockaddr_in serv_addr;
        int randomIndex = arc4random_uniform(length);
        char *hello;
        hello = dictionary[randomIndex];
        printf("word: %s\n", dictionary[randomIndex]);
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
        buffer[valread] = '\0';
        printf("readFromSocket: %s\n", buffer);
        
        send(sock , hello , strlen(hello) , 0 );
        valread = read(sock, buffer, 1024);
        buffer[valread] = '\0';
        printf("readFromSocket: %s\n", buffer);
        printf("closing client Socket !\n");
        close(sock);
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    initDictionary();
    printf("INITED DICTIONARY\n");
    pthread_t clientThread[20];
    int j;
    
    for(j = 0; j < 20; j++){
        pthread_create(&clientThread[j], NULL, clientThr, NULL);
        
    }
    
    for(j = 0; j < 20; j++){
        pthread_join(clientThread[j], NULL);
    }
    
    destroyDictionary();
}

