#include <string.h>
#include <stdio.h>
#include <stdlib.h>
char** dict;
int length;

int main(int argc, char** argv){
  char word[50];
  FILE * dictFile = fopen("words.txt", "r");
  int counter=0;
  while(fgets(word, 50, dictFile)!=NULL){
    counter++;
  }
  length = counter;
  
  dict = malloc(length * sizeof(char*));
  fclose(dictFile);
  dictFile = fopen("words.txt", "r");
  int i;
  for(i = 0; i < length; i++){
    if(fgets(word, 50, dictFile)){
      int j;
      for(j = strlen(word); j > 0; j--){
	if(word[j] == '\n'){
	  word[j] = '\0';
	}
      }
      dict[i] = strdup(word);
    }
  }
  
  for(i=0; i < length; i++){
    printf("%d. %s\n", i, dict[i]);
    free(dict[i]);
  }
  free(dict);
  fclose(dictFile);
}
