#include <stdio.h>
#include <pthread.h>

FILE *logFile;
pthread_mutex_t logLock;
int threadFactory;
void *mythread(void  *arg){
  int val = (int) arg;
  pthread_mutex_lock(&logLock);
  int i;
  for(i = 0; i < 1000; i++){
    fprintf(logFile,"Value: %d\n", val);
  }
  pthread_mutex_unlock(&logLock);
  return NULL;
}

int main(int argc, char *argv[]){
  pthread_t p[5];
  int rc;
  int threadVal = 5;
  int i;
  threadFactory = 0;
  rc = pthread_mutex_init(&logLock, NULL);
  if (rc != 0){
    printf("Lock not initialized\n");
  }
  logFile = fopen("testFile.txt", "w+");
  fprintf(logFile, "%s\n", "here we are!");
  for (i = 0; i < 5; i++){
    rc = pthread_create(&p[i], NULL, mythread, (void *) 100);
    if(rc != 0 ){
      printf("Unable to create thread\n");
    }
  }
  for(i = 0; i < 5; i++){
    rc = pthread_join(p[i], NULL);
  }

  fclose(logFile);
  pthread_mutex_destroy(&logLock);
  printf("Quitting!\n");
}

