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

// My librarys
#include "../inc/server.h"
#include "../inc/network.h"
#include "../inc/recvMessage.h"
#include "../inc/sendMessage.h"

int send_message(int connfd, sendToClientMessageType type, char* message)
{
    char sendMessage[SEND_BUFF_SIZE];
    strcpy(sendMessage, message);
    sendMessage_add_messageType(type, sendMessage);
    sendMessage_markEnd(sendMessage);

    int sendMessageSize;
    sendMessageSize = strlen(sendMessage); // in c each chracter have the size of 1 byte

    int sendBytes;

    printf("\n\nSend message to file discriptor %d\n", connfd);
    printf("Message type: %d\n", type);
    printf("Message: %s\n", sendMessage);
    printf("Size: %d\n", sendMessageSize);

    sendBytes = send(connfd, sendMessage,  strlen(sendMessage), 0);
    if(sendBytes == -1)
    {
        return CONNFD_CANNOT_CONNECT;
    }
    printf("Sent bytes: %d\n\n", sendBytes);

    return SEND_SUCCESS;
};

int sendMessage_markEnd(char* message)
{
    strcat(message, messageEndMarker);
}

int sendMessage_add_messageType(sendToClientMessageType type, char* message)
{
    char type_message[SEND_BUFF_SIZE];

    if(type == NOTIFICATION)
    {
        strcpy(type_message, "NOTIFICATION-");
    }
    else if(type == MESSAGE)
    {
        strcpy(type_message, "MESSAGE-");
    }
    else if(type == CHAT_MESSAGE)
    {
        strcpy(type_message, "CHAT_MESSAGE-");
    }
    else if(type == GAME_CONTROL_MESSAGE)
    {
        strcpy(type_message, "GAME_CONTROL_MESSAGE-");
    }
    else if(type == GAME_CONTROL_DATA)
    {
        strcpy(type_message, "GAME_CONTROL_DATA-");
    }

    strcat(type_message, message);
    strcpy(message, type_message);
}
