#include "dictionary.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DEFAULT_DICTIONARY "words.txt"

char **dictionary;
int length;

/*******************************************************
 METHOD: initDictionary: inits dictionary values .
 *if unable to load dictionary, loads from DEFAULT_DICTIONARY.
 *******************************************************/
void initDictionary(int argc, char** argv){
    char* dictString;
    int dictArg = 0;
    FILE * dictFile;
    if(argc == 3){
         dictFile = fopen(argv[2], "r");
        if(dictFile != NULL){
            dictArg = 2;
        }else {
            dictFile = fopen(argv[1], "r");
            if(dictFile != NULL){
                dictArg = 1;
            }else{
                dictFile = fopen(DEFAULT_DICTIONARY, "r");
                printf("Using DEFAULT dictionary\n");
                dictArg = 0;
            }
        }
    }else if(argc == 2){
         dictFile = fopen(argv[1], "r");
        if(dictFile != NULL){
            dictArg = 1;
        }else{
            dictFile = fopen(DEFAULT_DICTIONARY, "r");
            printf("Using DEFAULT dictionary\n");
            dictArg = 0;
        }
    }else{
        dictFile = fopen(DEFAULT_DICTIONARY, "r");
        printf("Using DEFAULT dictionary\n");
        dictArg = 0;
    }
    char word[50];
    int counter = 0;
    
    while(fgets(word, 50, dictFile)!= NULL){
        counter++;
    }
    length = counter;
    dictionary = malloc(length*sizeof(char*));
    fclose(dictFile);
    if(dictArg){
        dictFile = fopen(argv[dictArg], "r");
        printf("opened %s\n",argv[dictArg]);
    }else{
        dictFile = fopen(DEFAULT_DICTIONARY, "r");
        printf("loaded default dictionary\n");
    }
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
    qsort(dictionary, length, sizeof(char*), cmpfunc);
    for(i = 0; i < length; i++){
        printf("%d: %s\n", i, dictionary[i]);
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
    printf("dictionary memory deallocated\n");
}


/***********************************************************
 *METHOD : lookupWord uses an iterative binary search to locate
 the word in the dictionary
 *INPUT: string
 *OUTPUT: int: 1 if word is in dictionary, 0 else
 ***********************************************************/
int lookupWord(char* word){
    int left = 0;
    int right = length -1;
    while(left <= right){
        int middle = left + (right-left)/2;
        if(strcmp(word, dictionary[middle])==0){
            return 1;
        }
        if (strcmp(dictionary[middle], word) < 0){
            left= middle + 1;
        }
        else{
            right = middle - 1;
        }
    }
    return 0;
}

//cmpfunc is a wrapper function for qsort.
int cmpfunc(const void * a, const void * b){
    return strcmp(*((char**) a), *((char**) b));
}
