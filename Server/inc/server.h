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

#define LISTENQ 3 /* the number of pending connections that can be queued for a server socket. (call waiting allowance */

#define MAX_CLIENT 3
#define MAX_SERVICE_THREAD 10

extern int clientNum;
extern struct in_addr clientIP[MAX_CLIENT];
extern accountNode* client_account[MAX_CLIENT];
extern int clientConnfd[MAX_CLIENT];
extern int connfdNoServiceRunning[MAX_CLIENT];

typedef struct
{
    int thread_index;
    int clientConnfd_index;
}user_thread_args;

void *service_register(void *args);
void *service_activate(void *args);
void *service_signin(void *args);
void *service_changePass(void *args);
void *service_newGame(void *args);
void *service_gamePlayingHistory(void *args);
void* service_signout(void *args);

#endif
