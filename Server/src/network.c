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
#include "../inc/services.h"
#include "../inc/accountSystem.h"
#include "../inc/network.h"
#include "../inc/server.h"

#define SEND_ERROR 600
#define EXCESS_TIME_LIMIT 601
#define SEND_SUCCESS 602

int send_message(int connfd, messageType type, char* message)
{
    int sendBytes;

    fd_set writefds;
    int max_writefd;

    FD_ZERO(&writefds);
    FD_SET(connfd, &writefds);
    max_writefd = connfd;

    int askForSending = select(max_writefd + 1, NULL, &writefds, NULL, NULL);

    if(askForSending == -1)
    {
        return SEND_ERROR;
    }
    else if(askForSending == 0)
    {
        return EXCESS_TIME_LIMIT;
    }

    printf("\n\nSend message to file discriptor %d\n", connfd);
    printf("Message type: %d\n", type);
    char type_message[500];

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
    printf("Message: %s\n", type_message);
    printf("Size: %lu\n", sizeof(type_message));
    sendBytes = send(connfd, type_message,  sizeof(type_message), 0);

    char recvBuff[RECV_BUFF_SIZE];
    do{
    int recvBytes = recv(connfd, recvBuff, sizeof(recvBuff), 0);
    if(recvBytes == 0)
        return recvBytes;

    recvBuff[recvBytes] = '\0';
    }while(strcmp(recvBuff, "SEND_SUCCESS") != 0);


    printf("Send bytes: %d\n\n", sendBytes);

    return SEND_SUCCESS;
};

void clientConnfdUnconnect(int connfdIndex)
{
    if(client_account[connfdIndex] != NULL)
    {
        signOut(client_account[connfdIndex]);
        client_account[connfdIndex] = NULL;
        printf("Client exited\n");
    }

    //-----------------------------------------------------------

    printf("###Client've disconnected to server\n");
    printf("###Closing the file descriptor of the client connection...\n");

    pthread_mutex_lock(&clientDataLock);

    close(clientConnfd[connfdIndex]);

    clientConnfd[connfdIndex] = -1;

    clientNum--;
    connfdNoServiceRunning[connfdIndex] = 0;

    pthread_mutex_unlock(&clientDataLock);

    printf("###Closing completed\n");
}

void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;

    // Storing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds);
}
