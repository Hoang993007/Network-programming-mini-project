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
#define ROOM_QUEST_NUM 3

#define ROOM_ERROR_PLAYER_OUTROOM 101

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

    //The first player is roomhost
    accountNode* player[MAX_ROOM_PLAYER];
    int playerConnfd[MAX_ROOM_PLAYER];
        int playerConnfdRSBusy[MAX_ROOM_PLAYER];
    int playerReady[MAX_ROOM_PLAYER];
    int playerPoint[MAX_ROOM_PLAYER];

    fd_set readfds;
    int fd[2];
} room;

void roomListInit();
void* newRoom(void* args);
void* playerEnterRoom (void* args);
void* roomChat (void* args);
void invitePlayer(room* room, int playerIndex, char* invitedUserName);
void kickPlayer(room* room, char* kickedUserName);
void newRoomHost(room* room, int newHostIndex);
room* getRoomByID(int ID);
int roomPlay (int roomID);
int countLetter(char* src, char letter);
int nextPlayerTurn (room* currentRoom, int startRoundPlayer, int* inTurnPlayer);
void quitRoom (int playerIndex, room* room);
int numOfReadyPlayer(room* room);
void deleteRoom (room* room);
void printRoom ();
void printOneRoom (int roomID);

#endif
