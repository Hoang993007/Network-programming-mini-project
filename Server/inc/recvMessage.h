#ifndef __RECVMESSAGE_H__
#define __RECVMESSAGE_H__

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

#include "errorCode.h"
#include "network.h"
#include "accountSystem.h"

#define LISTENQ 3 /* the number of pending connections that can be queued for a server socket. (call waiting allowance */
#define MAX_CLIENT 10

#define RECV_SUCCESS 603

extern int clientNum;
extern struct in_addr clientIP[MAX_CLIENT];
extern accountNode* client_account[MAX_CLIENT];
extern int clientConnfd[MAX_CLIENT];
extern int connfdNoServiceRunning[MAX_CLIENT];
extern pthread_mutex_t clientDataLock;

extern pthread_mutex_t clientFDDataLock;
extern pthread_mutex_t clientConnfdLock[MAX_CLIENT];
//TODO: Initialize those

extern char messageRecv_fromClient[MAX_CLIENT][RECV_BUFF_SIZE];
extern int client_messageReady[MAX_CLIENT];
extern char clientName[MAX_CLIENT][50];

typedef enum{
    SERVICE,
    CLIENT_MESSAGE
} recvFromClientMessageType;

void* recv_message(void *args);
int checkFirstMessage_type(int clientMessageIndex, recvFromClientMessageType type);
int getMessage(int connfd, recvFromClientMessageType type, char* buff, int sizeOfBuff);

#endif
