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

int testMode = 0;

char messageEndMarker[] = "|";
char type_mess_separateChar[] = "-";

char recvMessage[RECV_MESSAGE_TYPE_NUM][RECV_MESS_SIZE];
int messageReady[RECV_MESSAGE_TYPE_NUM];

void* recv_message(void *args)
{
    for(int i = 0; i < RECV_MESSAGE_TYPE_NUM; i++)
        messageReady[i] = -1;

    printf("\n### NOTICE: ONLY LISTEN TO 1 SERVER\n");
    printf("###  Start listent to message from server...\n\n");

    fd_set readfds;
    int max_readfd;
    int askForSending;

    int rcvBytes;
    char recvBuff[RECV_BUFF_SIZE];

    while(1)
    {
        FD_ZERO(&readfds);
        FD_SET(client.sockfd, &readfds);
        max_readfd = client.sockfd;

        askForSending = select(max_readfd + 1, &readfds, NULL, NULL, NULL);
        if(askForSending == -1)
        {
            perror("\Error: ");
            exit(3);
        }

        rcvBytes = recv(client.sockfd, recvBuff, sizeof(recvBuff), 0);
        if(testMode == 1)
        {
            printf("\n### recvBuff: %s\n", recvBuff);
            printf("### Recvbytes = %d\n", rcvBytes);
        }
        if(rcvBytes == -1)
        {
            perror("Error: ");
            exit(3);
        }
        else if(rcvBytes == 0)
        {
            printf("\n\n-------------------------------------\n");
            printf("No longer connect with Server\n");
            printf("\n\n-------------------------------------\n");
            exit(0);
        }

        recvBuff[rcvBytes] = '\0';

        char* savePtr;
        char* message;

        message = strtok_r(recvBuff, messageEndMarker, &savePtr);
        while(message != NULL)
        {
            storeMessage(message);
            message = strtok_r(NULL, messageEndMarker, &savePtr);
        }
    }
}

void storeMessage (char* newMessage)
{
    char* save_ptr;

    char* receiveType = strtok_r(newMessage, type_mess_separateChar, &save_ptr);
    char* message = strtok_r(NULL, type_mess_separateChar, &save_ptr);

    if(strcmp(receiveType, "NOTIFICATION") == 0)
    {
        printf("\n---- Notification ----\n%s\n\n", message);
        return;
    }

    if(strcmp(receiveType, "MESSAGE") == 0)
    {
        while(messageReady[MESSAGE] == 1);

        strcpy(recvMessage[MESSAGE], message);
        messageReady[MESSAGE] = 1;
        return;
    }

    if(strcmp(receiveType, "CHAT_MESSAGE") == 0)
    {
        printf("> %s\n", message);
        return;
    }

    if(strcmp(receiveType, "GAME_CONTROL_MESSAGE") == 0)
    {
        printf("\n---- Game notification ----\n%s\n\n", message);
        return;
    }

    if(strcmp(receiveType, "GAME_CONTROL_DATA") == 0)
    {
        if(strcmp(message, "NEW_HOST") == 0)
        {
            client.isHost = 1;
            return;
        }

        if(strcmp(message, "GAME_BREAK") == 0)
        {
            pthread_cancel(client.gamePlayThreadId);
            return;
        }

        while(messageReady[GAME_CONTROL_DATA] == 1);

        strcpy(recvMessage[GAME_CONTROL_DATA], message);
        messageReady[GAME_CONTROL_DATA] = 1;
        return;
    }

}

void getMessage(recvFromServer_MessageType type, char* buff)
{
    while(messageReady[type] != 1);

    if(buff != NULL)
    {
        strcpy(buff, recvMessage[type]);
    }

    messageReady[type] = -1;
    return;
}

int waitMessage(recvFromServer_MessageType type, char* message)
{
    if(testMode == 1)
    printf("### wait message: %s\n", message);

    while(messageReady[type] != 1 || strcmp(recvMessage[type], message) != 0);
//    if(testMode == 1)
//    {
//        printf("### messageReady[type]: %d\n", messageReady[type]);
//        printf("### recvMessage[type]: %s\n", recvMessage[type]);
//    }

    getMessage(type, NULL);
}

/*
Des:
    No wait if message buff have no message
*/
int checkMessage(recvFromServer_MessageType type, char* message)
{
    if(messageReady[type] == 1)
        if(strcmp(recvMessage[type], message) == 0)
        {
            getMessage(type, NULL);
            return 1;
        }

    return -1;
}

/*
Des:
    Wait if message buff have no message
*/
int checkMessage_waitRecv(recvFromServer_MessageType type, char* message)
{
    while(messageReady[type] != 1);

    if(strcmp(recvMessage[type], message) == 0)
    {
        getMessage(type, NULL);
        return 1;
    }

    return -1;
}

int send_message(sendToServer_MessageType type, char* message)
{
    char sendMessage[SEND_MESS_SIZE];
    strcpy(sendMessage, message);

    sendMessage_add_messageType(type, sendMessage);
    sendMessage_markEnd(sendMessage);

    int sendBytes;

    // in c each chracter have the size of 1 byte
    if(testMode == 1) printf("### sendMessage: %s\n", sendMessage);

    sendBytes = send(client.sockfd, sendMessage,  strlen(sendMessage), 0);
    if(sendBytes == -1)
    {
        printf("\n\n-------------------------------------\n");
        printf("No longer connect with Server\n");
        printf("\n\n-------------------------------------\n");
        exit(0);
    }

    //printf("Sent bytes: %d\n\n", sendBytes);

    return SEND_SUCCESS;
};

int sendMessage_markEnd(char* message)
{
    strcat(message, messageEndMarker);
}

int sendMessage_add_messageType(sendToServer_MessageType type, char* message)
{
    char type_sendMessage[SEND_BUFF_SIZE];

    switch(type)
    {
    case CLIENT_MESSAGE:
        strcpy(type_sendMessage, "CLIENT_MESSAGE-");
        break;

    case SERVICE:
        strcpy(type_sendMessage, "SERVICE-");
        break;
    default:
        printf("In correct message type\n");
        break;
    }

    strcat(type_sendMessage, message);
    strcpy(message, type_sendMessage);
    if(testMode == 1) printf("### type_sendMessage: %s\n", type_sendMessage);
}
