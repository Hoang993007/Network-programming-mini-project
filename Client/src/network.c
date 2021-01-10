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

#include "../inc/network.h"
#include "../inc/client.h"
#include "../inc/otherFunction.h"

char recvMessage[messageTypeNum][RECV_BUFF_SIZE];
int messageReady[messageTypeNum];
// 0: notification - 1:...

void* recv_message(void *args)
{
    int rcvBytes;

    fd_set readfds;
    int max_readfd;

    char recvBuff[RECV_BUFF_SIZE];

    for(int i = 0; i < 5; i++)
        messageReady[i] = 0;

    printf("\n#Start listent to message from server...\n\n");
    while(1)
    {
        FD_ZERO(&readfds);
        FD_SET(client.sockfd, &readfds);
        max_readfd = client.sockfd;
        int askForSending;
        //printf("Selecting...\n");
        askForSending = select(max_readfd + 1, &readfds, NULL, NULL, NULL);

        if(askForSending == -1)
        {
            perror("\Error: ");
            // error occurred in select()
        }

        rcvBytes = recv(client.sockfd, recvBuff, RECV_BUFF_SIZE, 0);

        if(rcvBytes < 0)
        {
            perror("Error: ");
        }
        else if(rcvBytes == 0)
        {
            printf("\n\n-------------------------------------\n");
            printf("Server is no longer connected\n");
            exit(0);
        }

        recvBuff[rcvBytes] = '\0';
        //printf("%s\n\n", recvBuff);

        char* receiveType = strtok(recvBuff, "-");
        char* message = strtok(NULL, ";");

        //printf("\n\n\t(TYPE: %s -- MESSAGE: %s)\n\n", receiveType, message);

        if(strcmp(receiveType, "NOTIFICATION") == 0)
        {
            printf("\n---- Notification ----\n%s\n\n", message);
        }
        else if(strcmp(receiveType, "MESSAGE") == 0)
        {
            while(messageReady[MESSAGE] == 1);
            strcpy(recvMessage[MESSAGE], message);
            messageReady[MESSAGE] = 1;
        }
        else if(strcmp(receiveType, "CHAT_MESSAGE") == 0)
        {
            printf("> %s\n", message);
        }
        else if(strcmp(receiveType, "GAME_CONTROL_MESSAGE") == 0)
        {
            printf("\n---- Game notification ----\n%s\n\n", message);
        }
        else if(strcmp(receiveType, "GAME_CONTROL_DATA") == 0)
        {
            while(messageReady[GAME_CONTROL_DATA] == 1);
            //printf("Add to message queue\n");
            strcpy(recvMessage[GAME_CONTROL_DATA], message);
            messageReady[GAME_CONTROL_DATA] = 1;

            if(strcmp(recvMessage[GAME_CONTROL_DATA], "NEW_HOST") == 0)
            {
                //printf("#### NEW_HOST\n");
                messageReady[GAME_CONTROL_DATA] = 0;
                client.isHost = 1;
            }

            if(strcmp(recvMessage[GAME_CONTROL_DATA], "GAME_BREAK") == 0)
            {
                //printf("#### GAME_BREAK\n");
                messageReady[GAME_CONTROL_DATA] = 0;
                //printf("client game thread ID: %ld\n", client.gamePlayThreadId);
                pthread_cancel(client.gamePlayThreadId);
            }
        }

        send(client.sockfd, "SEND_SUCCESS",  sizeof("SEND_SUCCESS"), 0);
    }
}

void getMessage(messageType type, char* buff)
{
    while(messageReady[type] == 0);
    strcpy(buff, recvMessage[type]);
    messageReady[type] = 0;
}

void waitMessage(messageType type, char* buff)
{
    while(messageReady[type] == 0 && (strcmp(recvMessage[type], buff) != 0));
    messageReady[type] = 0;
}
