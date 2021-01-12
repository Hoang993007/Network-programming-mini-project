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
#include "../inc/recvMessage.h"
#include "../inc/sendMessage.h"
#include "../inc/otherFunction.h"
#include "../inc/room.h"

int testMode = 1;

int clientNum;
int clientConnfd[MAX_CLIENT];
accountNode* client_account[MAX_CLIENT];
char clientName[MAX_CLIENT][50];

struct in_addr clientIP[MAX_CLIENT];

pthread_mutex_t clientFDDataLock = PTHREAD_MUTEX_INITIALIZER;;
pthread_mutex_t clientConnfdLock[MAX_CLIENT];
//TODO: Initialize those

char messageRecv_fromClient[MAX_CLIENT][RECV_BUFF_SIZE];
int client_messageReady[MAX_CLIENT];

int recvThreadPipe[2];

void* recv_message(void *args)
{
    fd_set readfds;
    int max_readfd;

    int listenfd;
    int connfd;
    int recvBytes;

    socklen_t clientSocketLen; // size of client socket address
    struct sockaddr_in servaddr, cliaddr; // address structure
    char recvBuff[RECV_BUFF_SIZE];

    //Step 1: Construct socket
    printf("Constructing socket IPv4 - TCP/IP...\n");

    // listenfd is a fd
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error: Error in creating socket");
        exit(2);
    }

    bzero(&servaddr, sizeof(servaddr)); // clear srevaddr

    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    /* INADDR_ANY: kernel chooses any IP address avalable for server
       If run just local host: 172.0.0.1 - local host address */

    //Step 2: Bind address to socket
    printf("Binding servaddr address to the socket...\n");
    int SERV_PORT = 7999;
    int res;
    do
    {
        SERV_PORT++;
        servaddr.sin_port = htons(SERV_PORT); // specifies port
        res = bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

        perror("Error: ");
        //exit(EXIT_FAILURE);
    }
    while(res);

    printf("Server started.\n\n");
    printf("----------------------------------------\n");
    printf("Server IP: %s\n", inet_ntoa(servaddr.sin_addr));
    printf("Server Using Port: %d\n", SERV_PORT);
    printf("----------------------------------------\n");

    // Establish a socket to LISTENING for incoming connection
    // Listen to the socket by creating a connection queue, then wait for clients
    printf("Establishing a socket to LISTENING for incoming connection\n");
    if(listen(listenfd, LISTENQ) < 0)
    {
        perror("Error: Error in establising socket to listen for incoming connection");
        exit(EXIT_FAILURE);
    }

    // Step 3: Select
    clientNum = 0;

    for(int i = 0; i < MAX_CLIENT; i++)
    {
        clientConnfd[i] = -1;
        client_account[i] = NULL;
        client_messageReady[i] = -1;
    }

    FD_ZERO(&readfds);
    // fd_sets used to specify the descriptors that we want the kernel to test for reading, writing, and exception conditions.

    roomListInit();

    printf("%s\n", "Receiving message...");

    int pipeRes = pipe(recvThreadPipe);
    if(pipeRes < 0)
    {
        perror("pipe ");
        exit(1);
    }

    while(1)
    {
        clientSocketLen = sizeof(cliaddr);

        // The sets of file descriptors passed to select() are modified by select(),
        // so the set need to be re-initialized before for select() again.
        // (The programming error would only be notable if from more than one socket data sall be received.)

        //printf("\n#Server are selecting for service request...\n\n");
        // add fd to set and turn on the bit for fd in set
        FD_ZERO(&readfds);
        // IMPORTANT: Nếu add trước khi select  khởi động thfi select  sẽ không nghe nó(vì nó chưa được thêm vào readfds đốivới select
        // vì thế thì nghe cứ nghe hếtrồi sau chọn cai nào thì chọn

        FD_SET(listenfd, &readfds);
        if(max_readfd < listenfd)
        {
            max_readfd = listenfd;
        }
        FD_SET(recvThreadPipe[0], &readfds);
        if(recvThreadPipe[0] > max_readfd)
        {
            max_readfd = recvThreadPipe[0]; // REALLY IMPORTANT
        }
        for(int k = 0; k < MAX_CLIENT; k++)
            if(clientConnfd[k] != -1)
            {
                FD_SET(clientConnfd[k], &readfds);
                if(max_readfd < clientConnfd[k])
                {
                    max_readfd = clientConnfd[k];
                }
            }

        int commingMessage;
        commingMessage = select(max_readfd + 1, &readfds, NULL, NULL, NULL);
        //Nếu thêm cai time vào sẽ sai, không hiểu tại sao
        // Dự đoán: gây độ trễ

        if(commingMessage == -1)
        {
            perror("\Error: ");
            exit(0);
        }

        // NOTE: the listenfd or connfd are both descriptor
        // select() check if a descriptor have an activity to exec or not. Not only use for listenfd or connfd

        // check the status of listenfd
        // if there is a client want to connect through listenfd socket the select() function will mark listenfd in readfds as 1 - have an activity

        int newClientConnfd = -1;
        if (FD_ISSET (listenfd, &readfds))
        {
            if(clientNum >= MAX_CLIENT)
            {
                printf("Have reached max clients!\n");
                continue;
            }

            connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clientSocketLen);
            // Accept a connection request -> return a File Descriptor (FD)
            if((connfd) < 0)
            {
                perror("Error: ");
                exit(EXIT_FAILURE);
            }
            else
            {
                printf("###New connection...\n");

                Args args;
                args.int1 = &connfd;
                args.addr1 = &cliaddr;

                pthread_t tmp_threadId;
                pthread_create(&tmp_threadId, NULL, &newConnection, (void*)&args);
            }
        }

        if(FD_ISSET(recvThreadPipe[0], &readfds))
        {
            recvBytes = read(recvThreadPipe[0], recvBuff, sizeof(recvBuff));
            recvBuff[recvBytes] = '\0';
            printf("### New connection created successfully\n");
        }

        // check the status of clientConnfd(s)
        for(int k = 0; k < MAX_CLIENT; k++)
        {
            //after add to set, it mark as have something to read while actual not
            if(clientConnfd[k] > -1 && FD_ISSET(clientConnfd[k], &readfds))
                if(clientConnfd[k] != newClientConnfd)
                {
                    printf("#### Receiving message fromm lient fd: %d\n", clientConnfd[k]);

                    while(client_messageReady[k] == 1);

                    recvBytes = recv(clientConnfd[k], recvBuff, sizeof(recvBuff), 0);
                    if(recvBytes < 0)
                    {
                        perror("Error: \n");
                        exit(0);
                    }
                    else if(recvBytes == 0)
                    {
                        FD_CLR(clientConnfd[k], &readfds);
                        clientConnfdUnconnect(k);
                        continue;
                    }

                    recvBuff[recvBytes] = '\0';

                    if(testMode == 1) printf("### recvBuff: %s\n", recvBuff);
                    strcpy(messageRecv_fromClient[k], recvBuff);
                    client_messageReady[k] = 1;

                    checkFirstMessage_type(k, SERVICE);
                }
        }
    }
}

void* newConnection (void *args)
{
    Args* actual_args = args;

    int connfd = *(actual_args->int1);
    struct sockaddr_in* cliaddr = actual_args->addr1;

    printf("\n\nNew connection:\n");
    printf("\t1.Socket fd: %d\n", connfd);
    printf("\t2.Ip: %s\n", inet_ntoa(cliaddr->sin_addr));
    printf("\t3.Prot: %d\n", ntohs(cliaddr->sin_port));

    int i;
    for(i = 0; i < MAX_CLIENT; i++)
    {
        if(clientConnfd[i] == -1)
        {
            clientConnfd[i] = connfd;
            clientNum++;
            break;
        }
    }
    printf("\t4.Index: %d\n", i);
    printf("\t5.Connfd: %d\n\n", connfd);

    char recvBuff[RECV_BUFF_SIZE];
    int sendBytes, recvBytes;

    sendBytes = send_message(connfd, MESSAGE, "CONNECT_SUCCESSFULY");
    if(sendBytes == -1)
    {
        clientConnfdUnconnect(i);
        pthread_cancel(pthread_self());
    }

    int resWrite = write(recvThreadPipe[1], "NEW_CONNECTION", sizeof("NEW_CONNECTION"));
    if(resWrite < 0)
    {
        perror ("write");
        exit (2);
    }

    //PART2: check loged in
    printf("### check loged in\n");
    recvBytes = getMessage(connfd, CLIENT_MESSAGE, recvBuff, sizeof(recvBuff));
    if(recvBytes == 0)
    {
        clientConnfdUnconnect(i);
        pthread_cancel(pthread_self());
    }

    int isLogedIn = -1;
    if(strcmp(recvBuff, "NOT_LOGEDIN") != 0)
    {
        char* userName = strtok(recvBuff, ";");
        char* password = strtok(NULL, ";");

        int res = logIn (&(cliaddr->sin_addr), connfd, userName, password);
        if(res == LOGIN_SUCCESS)
        {
            isLogedIn = 1;
            printf("Address: %s alrealdy loged in! Username: %s", inet_ntoa(cliaddr->sin_addr), userName);

            sendBytes = send_message(connfd, MESSAGE, userName);
            if(sendBytes == -1)
            {
                clientConnfdUnconnect(i);
                pthread_cancel(pthread_self());
            }

            accountNode* logInAccount = accessToAccount (userName, password, &res);
            client_account[i] = getAccountNodeByUserName(userName);
        }
    }

    if (isLogedIn == -1)
    {
        printf("Address: %s did not log in!", inet_ntoa(cliaddr->sin_addr));
        sendBytes = send_message(connfd, MESSAGE, "NEED_LOGIN");
        if(sendBytes == -1)
        {
            clientConnfdUnconnect(i);
            pthread_cancel(pthread_self());
        }
    }
}

int checkFirstMessage_type(int clientMessageIndex, recvFromClientMessageType type) {
    char firstMessageType[20];

    char* message = messageRecv_fromClient[clientMessageIndex];
    int i = 0;
    while(message[i] != '-')
    {
        firstMessageType[i] = message[i];
        i++;

        if(i > 20)
        {
            printf("Error: Cannot find message type\n");
            exit(0);
        }
    }

    if(type == SERVICE)
    {
        if(strcmp(firstMessageType, "SERVICE") == 0)
        {
            printf("Service come...\n");
            getMessage(clientConnfd[clientMessageIndex], SERVICE, NULL, -1);
        }
        return -1;
    }

        switch(type)
        {
            case CLIENT_MESSAGE:
                if(strcmp(firstMessageType, "CLIENT_MESSAGE") == 0)
                    return 1;
                else return -1;
                break;
            default:
                printf("Incorrect recv from client message type\n");
                break;
        }
}

int getMessage(int connfd, recvFromClientMessageType type, char* buff, int sizeOfBuff)
{
    // REASON: why not connfd_INDEX? because we need to know if this connfd are still exits

    int connfd_index;
    connfd_index = getConnfdIndex(connfd);
    if(connfd_index == -1)
        return 0; // MEANING: client is no longer connected

    if(type != SERVICE)
    {
        while(client_messageReady[connfd_index] != 1
        || checkFirstMessage_type(connfd_index, type) != 1);// wait...
    }

    char firstMessage[RECV_BUFF_SIZE];

    int i = 0;
    while(messageRecv_fromClient[connfd_index][i] != '-')
        i++;

    i++;
    int firstMessageCharCount = 0;
    while(messageRecv_fromClient[connfd_index][i] != messageEndMarker[0])
    {
        firstMessage[firstMessageCharCount] = messageRecv_fromClient[connfd_index][i];
        firstMessageCharCount++;
        i++;
    }

    firstMessage[firstMessageCharCount] = '\0';

    i = i + strlen(messageEndMarker);
    if(messageRecv_fromClient[connfd_index][i] == '\0')
    {
        client_messageReady[connfd_index] = -1;
    }
    else
    {
        char afterGetMessage_clientMessage[RECV_BUFF_SIZE];
        int newClientMessageCharCount = 0;
        while(messageRecv_fromClient[connfd_index][i] != '\0')
        {
            afterGetMessage_clientMessage[newClientMessageCharCount] = messageRecv_fromClient[connfd_index][i];
            newClientMessageCharCount++;
            i++;
        }

        afterGetMessage_clientMessage[newClientMessageCharCount] = '\0';
        strcmp(messageRecv_fromClient[connfd_index], afterGetMessage_clientMessage);
    }

    if(type == SERVICE)
    {
        int service = atoi(firstMessage);
        serviceOrder(service, connfd_index);
        return 10;
    }

    //TODO: process sizeBuff
    strcpy(buff, firstMessage);
    client_messageReady[type] = 0;

    checkFirstMessage_type(connfd_index, SERVICE);
    return 10;
}
