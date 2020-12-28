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

#include "../inc/room.h"
#include "../inc/accountSystem.h"
#include "../inc/convertNumAndStr.h"
#include "../inc/network.h"
#include "../inc/server.h"

#define RECV_BUFF_SIZE 4096

pthread_mutex_t room_data_lock = PTHREAD_MUTEX_INITIALIZER;

int roomIDGenerate = 0;
int roomNum = 0;
room* roomList[MAX_ROOM];

pthread_t room_thread_id[MAX_ROOM];


void roomListInit()
{
    for(int i = 0; i < MAX_ROOM; i++)
    {
        roomList[i] = NULL;
    }
}

void* newRoom(void* args)
{
    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    int connfd_index;

    user_thread_args *actual_args = args;
    connfd_index = actual_args->clientConnfd_index;
    free(args);


    int recvBytes;
    char recvBuff[RECV_BUFF_SIZE];

    char roomName[ROOM_NAME_MAX_LENGTH];
    recvBytes = recv(clientConnfd[connfd_index], recvBuff, sizeof(recvBuff), 0);
    recvBuff[recvBytes] = '\0';
    strcpy(roomName, recvBuff);
    printf("[%s]: Room name: %s\n", inet_ntoa(clientIP[connfd_index]), roomName);

    int playerNumPreSet;
    recvBytes = recv(clientConnfd[connfd_index], recvBuff, sizeof(recvBuff), 0);
    recvBuff[recvBytes] = '\0';
    playerNumPreSet = atoi(recvBuff);
    printf("[%s]: Max player: %d\n", inet_ntoa(clientIP[connfd_index]), playerNumPreSet);

    if(roomNum < MAX_ROOM)
    {
        pthread_mutex_lock(&room_data_lock);

        room* newRoom = (room*)malloc(sizeof(room));

        roomIDGenerate++;
        roomNum++;

        newRoom->roomID = roomIDGenerate;
        strcpy(newRoom->roomName, roomName);
        newRoom->room_thread_id = pthread_self();

        newRoom->playerNumPreSet = playerNumPreSet;
        newRoom->currentPlayerNum = 1;
        pthread_cond_init(&(newRoom->playerReady_cond), NULL);
        pthread_mutex_init(&(newRoom->roomLock), NULL);

        newRoom->roomHost = client_account[connfd_index];
        newRoom->hostConnfd = clientConnfd[connfd_index];
        newRoom->hostRealdy = 0;
        newRoom->numOfRealdyPlayer = 0;

        for(int i = 1; i < playerNumPreSet; i++)
        {
            newRoom->player[i] = NULL;

        }

        int room_index;
        for(room_index = 0; room_index < MAX_ROOM; room_index++)
        {
            if(roomList[room_index] == NULL)
            {
                roomList[room_index] = newRoom;
                break;
            }
        }

        printf("Sucessfully created new room!\n");
        printRoom();

        pthread_mutex_unlock(&room_data_lock);

        char roomIDStr[5];
        tostring(roomIDStr, newRoom->roomID);
        send_message(clientConnfd[connfd_index], MESSAGE, roomIDStr);

        pthread_t tmp;
        pthread_create(&tmp, NULL, &roomChat, (void*)(&(newRoom->roomID)));

//        //delete room
//        room* tmp = newRoom;
//        roomList[room_index] = NULL;
//        free(tmp);
//        roomNum--;
    }
    else
    {
        printf("No more room can created!\n");
        connfdNoServiceRunning[connfd_index] = 1;
    }
}

void* playerEnterRoom (void* args)
{
    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    int connfd_index;

    user_thread_args *actual_args = args;

    connfd_index = actual_args->clientConnfd_index;

    free(args);


    int recvBytes;
    char recvBuff[RECV_BUFF_SIZE + 1];
    char sendBuff[SEND_BUFF_SIZE + 1];

    if(roomNum == 0)
    {
        send_message(clientConnfd[connfd_index], MESSAGE, "NO_ROOM");

        pthread_mutex_unlock(&room_data_lock);
        connfdNoServiceRunning[connfd_index] = 1;
        return NULL;
    }

    int curRoomNum = roomNum;

    tostring(sendBuff, curRoomNum);
    send_message(clientConnfd[connfd_index], MESSAGE, sendBuff);

    for(int i = 0; i < MAX_ROOM; i++)
    {
        if(roomList[i] != NULL)
        {
            char sendNum[5];

            tostring(sendNum, roomList[i]->roomID);
            //printf("roomID: *%s*\n", sendNum);
            send_message(clientConnfd[connfd_index], MESSAGE, sendNum);
//printf("Cheking0...!\n");

            //printf("room name: *%s*\n", roomList[i]->roomName);
            send_message(clientConnfd[connfd_index], MESSAGE, roomList[i]->roomName);
//printf("Cheking1...!\n");

            tostring(sendNum, roomList[i]->playerNumPreSet);
            //printf("room num preset: *%s*\n", sendNum);
            send_message(clientConnfd[connfd_index], MESSAGE, sendNum);
//printf("Cheking2...!\n");

            tostring(sendNum, roomList[i]->currentPlayerNum);
            //printf("player num: *%s*\n", sendNum);
            send_message(clientConnfd[connfd_index], MESSAGE, sendNum);

            //printf("Cheking3...!\n");
        }
    }

    send_message(clientConnfd[connfd_index], MESSAGE, "PRINT_ROOM_END");

    recvBytes = recv(clientConnfd[connfd_index], recvBuff, sizeof(recvBuff), 0);
    recvBuff[recvBytes] = '\0';
    int chosenRoomID = atoi(recvBuff);
    printf("[%s]: Chosen room ID: %d\n", inet_ntoa(clientIP[connfd_index]), chosenRoomID);

    pthread_mutex_lock(&room_data_lock);
    printf("start entering...\n");

    int i;
    for(i = 0; i < MAX_ROOM; i++)
    {
        if(roomList[i] != NULL && roomList[i]->roomID == chosenRoomID)
        {
            if(roomList[i]->currentPlayerNum < roomList[i]->playerNumPreSet)
            {
                for(int j = 0; j < roomList[i]->playerNumPreSet; j++)
                {
                    if(roomList[i]->player[j] == NULL)
                    {
                        roomList[i]->player[j] = client_account[connfd_index];
                        roomList[i]->playerConnfd[j] = clientConnfd[connfd_index];
                        roomList[i]->playerRealdy[j] = 0;
                        roomList[i]->currentPlayerNum++;
                        send_message(clientConnfd[connfd_index], MESSAGE, "ENTER_ROOM_SUCCESSFULY");
                        printRoom();
                        break;
                    };
                }
            }
            else
            {
                printf("Room full\n");
                send_message(clientConnfd[connfd_index], MESSAGE, "ROOM_FULL");
            }
            break;
        }
    }

    if(i == MAX_ROOM)
    {
        send_message(clientConnfd[connfd_index], MESSAGE, "ROOM_NOT_FOUND");
        connfdNoServiceRunning[connfd_index] = 1;
    }

    pthread_mutex_unlock(&room_data_lock);

}

room* getRoomByID(int ID)
{
    for(int i = 0; i < MAX_ROOM; i++)
    {
        if(roomList[i]->roomID == ID)
            return roomList[i];
    }
}

void* roomChat (void* args)
{
    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    int roomID = *((int*)args);

    printf("Starting room...\nRoom chat...\n\n");
    room* currentRoom = getRoomByID(roomID);

    char recvBuff[RECV_BUFF_SIZE + 1];
    char sendBuff[SEND_BUFF_SIZE + 1];

    printf("Room ID: %d\n", currentRoom->roomID);
    printf("Room thread ID: %ld\n", currentRoom->room_thread_id);
    printf("Room name: %s\n", currentRoom->roomName);
    printf("Num of player preset: %d\n", currentRoom->playerNumPreSet);
    printf("Num of player: %d\n", currentRoom->currentPlayerNum);

    for(int j = 0; j < currentRoom->playerNumPreSet - 1; j++)
    {
        if(currentRoom->player[j] != NULL)
        {
            printf("Player name: %s\n", currentRoom->player[j]->userName);
        }
        else printf("Player name: waiting...\n");
    }

    int recvBytes;
    fd_set readfds;
    int max_readfd;

    while(currentRoom->currentPlayerNum != currentRoom->playerNumPreSet
            || currentRoom->hostRealdy == 0
            || currentRoom->numOfRealdyPlayer != currentRoom->playerNumPreSet - 1)
    {
        FD_ZERO(&readfds);
        FD_SET(currentRoom->hostConnfd, &readfds);
        max_readfd = currentRoom->hostConnfd;
        for(int i = 0; i < currentRoom->playerNumPreSet - 1; i++)
        {
            if(currentRoom->player[i] != NULL)
                FD_SET(currentRoom->playerConnfd[i], &readfds);
            if(currentRoom->playerConnfd[i] > max_readfd)
                max_readfd = currentRoom->playerConnfd[i];
        }

        int askForRecv = select(max_readfd + 1, NULL, &readfds, NULL, NULL);

        if(askForRecv == -1)
        {
            perror("Error");
            exit(0);
        }

        if(FD_ISSET(currentRoom->hostConnfd, &readfds))
        {
            recvBytes = recv(currentRoom->hostConnfd, recvBuff, sizeof(recvBuff), 0);
            recvBuff[recvBytes] = '\0';
            if(recvBuff[0] != '$')
            {
                char clientName_message[500];
                strcpy(clientName_message, currentRoom->roomHost->userName);
                clientName_message[strlen(clientName_message) + 1] = '\0';
                clientName_message[strlen(clientName_message)] = ':';
                // nếu dể thằng duwois lên trước có nghĩa đã xóa đi '\0'sau strlen khong phai cai truoc do nua
                strcat(clientName_message, recvBuff);

                for(int i = 0; i < currentRoom->playerNumPreSet - 1; i++)
                {
                    if(currentRoom->player[i] != NULL)
                        send_message(currentRoom->playerConnfd[i], CHAT_MESSAGE, clientName_message);
                }
            }

        }

        for(int i = 0; i < currentRoom->playerNumPreSet - 1; i++)
        {
            if(currentRoom->player[i] != NULL)
                if(FD_ISSET(currentRoom->playerConnfd[i], &readfds))
                {









                }
        }
    }

    //TODO: Room play
}

void* roomPlay (void* args)
{
}

void* quitRoom (void* args)
{
}

void printRoom ()
{
    printf("-----------------------------\n");
    printf("Room info\n\n");
    printf("Room num: %d\n", roomNum);
    for(int i = 0; i < MAX_ROOM; i++)
    {
        if(roomList[i] != NULL)
        {
            printf("Room ID: %d\n", roomList[i]->roomID);
            printf("Room thread ID: %ld\n", roomList[i]->room_thread_id);
            printf("Room name: %s\n", roomList[i]->roomName);
            printf("Num of player preset: %d\n", roomList[i]->playerNumPreSet);
            printf("Num of player: %d\n", roomList[i]->currentPlayerNum);

            for(int j = 0; j < roomList[i]->playerNumPreSet - 1; j++)
            {
                if(roomList[i]->player[j] != NULL)
                {
                    printf("Player name: %s\n", roomList[i]->player[j]->userName);
                }
                else printf("Player name: waiting...\n");
            }
        }
    }
    printf("-----------------------------\n");
}
