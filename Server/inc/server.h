#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
//
#include <sys/types.h>
#include <sys/socket.h>
//
#include <netinet/in.h>
//
#include <unistd.h>
//
#include <arpa/inet.h>
#include <errno.h>
//
#include <sys/select.h>
#include <sys/time.h>
//
#include <pthread.h>
//
#include <time.h>

typedef struct
{
    int clientConnfd_index;
}serviceThread_args;

void* newConnection (void *args);
int serviceOrder(int service, int clientConnfd_index);
void *service_register(void *args);
void *service_activate(void *args);
void *service_signin(void *args);
void *service_changePass(void *args);
void *service_newGame(void *args);
void *service_gamePlayingHistory(void *args);
void* service_signout(void *args);

#endif
