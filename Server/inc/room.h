#ifndef __ROOM_H__
#define __ROOM_H__

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
#include "accountSystem.h"

#define MAX_ROOM 5
#define MAX_ROOM_PLAYER 5
#define ROOM_NAME_MAX_LENGTH 20

// ROOM ------------------------------------------------------------
typedef struct _room
{
    pthread_t room_thread_id;
    pthread_cond_t playerReady_cond;
    pthread_mutex_t roomLock;

    int roomID;
    char roomName[ROOM_NAME_MAX_LENGTH];

    int playerNumPreSet;
    int currentPlayerNum;

    accountNode* roomHost;
    int hostConnfd;
    int hostRealdy;

    accountNode* player[MAX_ROOM_PLAYER - 1];
    int playerConnfd[MAX_ROOM_PLAYER - 1];
    int playerRealdy[MAX_ROOM_PLAYER - 1];
    int numOfRealdyPlayer;
} room;

void roomListInit();
void printRoom ();
void* newRoom(void* args);
void* playerEnterRoom (void* args);
void* roomChat (void* args);
room* getRoomByID(int ID);
void* roomPlay (void* args);
void* quitRoom (void* args);

#endif