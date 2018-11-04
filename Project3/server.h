//
//  server.h
//  
//
//  Created by Leo Vergnetti on 11/2/18.
//

#ifndef server_h
#define server_h

#include <stdio.h>

void *workerThr(void *arg);
void *writerThr(void *arg);
int runServer();
int stopServer();
#endif /* server_h */
