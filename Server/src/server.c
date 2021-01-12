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

//My library
#include "./../inc/services.h"
#include "./../inc/accountSystem.h"
#include "./../inc/errorCode.h"
#include "../inc/recvMessage.h"
#include "../inc/otherFunction.h"
#include "./../inc/room.h"
#include "./../inc/network.h"
#include "./../inc/server.h"
#include "./../inc/question.h"
#include "../inc/sendMessage.h"

pthread_mutex_t user_passwordFileLock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{
    loadUsername_passData();
    loadQues_ansData();

    printf("Server's data is now ready!\n\n");

    pthread_t tmp_threadID;
    pthread_create(&tmp_threadID, NULL, &recv_message, NULL);
    void* val;
    pthread_join(tmp_threadID, &val);


    freeAccountNode();
    freeQues_ansNode();

    return 0;
}

int serviceOrder(int service, int clientConnfd_index)
{
    // Prepare user args for thread
    serviceThread_args args;
    args.clientConnfd_index = clientConnfd_index;

    pthread_t tmp_threadID;

    switch(service)
    {
    case 1:
        printf ("%s\n", "Service 1: Register");
        pthread_create(&tmp_threadID, NULL, &service_register, (void*)&args);
        break;

    case 2:
        printf ("%s\n", "Service 2: Activate account");
        break;

    case 3:
        printf ("%s\n", "Service 3: Sign in");
        pthread_create(&tmp_threadID, NULL, &service_signin, (void*)&args);
        break;

    case 4:
        printf ("%s\n", "Service 4: Change password");
        if(client_account[clientConnfd_index]->isLogined == 1)
        {
            pthread_create(&tmp_threadID, NULL, &service_changePass, (void*)&args);
        }
        else printf("Not loged in\n");
        break;

    case 5:
        printf ("%s\n", "Service 5: New room");
        if(client_account[clientConnfd_index]->isLogined == 1)
        {
            pthread_create(&tmp_threadID, NULL, &newRoom, (void*)&args);
        }
        else printf("Not loged in\n");
        break;

    case 6:
        printf ("%s\n", "Service 6: Player enter room");
        if(client_account[clientConnfd_index]->isLogined == 1)
        {
            pthread_create(&tmp_threadID, NULL, &playerEnterRoom, (void*)&args);

        }
        else printf("Not loged in\n");
        break;

    case 7:
        if(client_account[clientConnfd_index]->isLogined == 1)
        {
            printf ("%s\n", "Service 7: Sign out");
            pthread_create(&tmp_threadID, NULL, &service_signout, (void*)&args);
        }
        break;
    }
}

void* service_register(void *args)
{
//Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    int oldstate;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
    int oldtype;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);

    int connfd_index;
    connfd_index = ((serviceThread_args*)args)->clientConnfd_index;

    char recvBuff[RECV_BUFF_SIZE];
    int sendBytes, recvBytes;

    userNameType userName;
    passwordType password;

    //FIX
    recvBytes = getMessage(clientConnfd[connfd_index], CLIENT_MESSAGE, recvBuff, sizeof(recvBuff));
    if(recvBytes == 0)
    {
        clientConnfdUnconnect(connfd_index);
        pthread_cancel(pthread_self());
    }

    userName[recvBytes] = '\0';
    strcpy(userName, recvBuff);
    printf("[%s]: User name: %s\n", inet_ntoa(clientIP[connfd_index]), userName);

    if(isExistUserName(userName) == ACCOUNT_EXIST)
    {
        printf("Account exist\n");
        send_message(clientConnfd[connfd_index],MESSAGE, "X");
        if(sendBytes == -1)
        {
            clientConnfdUnconnect(connfd_index);
            pthread_cancel(pthread_self());
        }
        return NULL;
    }
    else
    {
        printf("Account doesn't exist\n");
        send_message(clientConnfd[connfd_index], MESSAGE, "O");
        if(sendBytes == -1)
        {
            clientConnfdUnconnect(connfd_index);
            pthread_cancel(pthread_self());
        }
    }

    recvBytes = getMessage(clientConnfd[connfd_index], CLIENT_MESSAGE, recvBuff, sizeof(recvBuff));
    if(recvBytes == 0)
    {
        clientConnfdUnconnect(connfd_index);
        pthread_cancel(pthread_self());
    }

    recvBuff[recvBytes] = '\0';
    strcpy(password, recvBuff);
    printf("[%s]: password: %s\n", inet_ntoa(clientIP[connfd_index]), password);

    signUp (userName, password);
}

void* service_signin(void *args)
{
    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    int oldstate;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
    int oldtype;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);

    int connfd_index;
    connfd_index = ((serviceThread_args*)args)->clientConnfd_index;

    char recvBuff[RECV_BUFF_SIZE];
    int sendBytes, recvBytes;

    userNameType userName;
    passwordType password;

    recvBytes = getMessage(clientConnfd[connfd_index], CLIENT_MESSAGE, recvBuff, sizeof(recvBuff));
    if(recvBytes == 0)
    {
        clientConnfdUnconnect(connfd_index);
        pthread_cancel(pthread_self());
    }

    userName[recvBytes] = '\0';
    strcpy(userName, recvBuff);
    printf("[%s]: User name: %s\n", inet_ntoa(clientIP[connfd_index]), userName);

    if(isExistUserName(userName) == ACCOUNT_EXIST)
    {
        printf("Account exist\n");
        sendBytes = send_message(clientConnfd[connfd_index],MESSAGE, "O");
        if(sendBytes == -1)
        {
            clientConnfdUnconnect(connfd_index);
            pthread_cancel(pthread_self());
        }
    }
    else
    {
        printf("Account doesn't exist\n");
        sendBytes = send_message(clientConnfd[connfd_index], MESSAGE, "X");
        if(sendBytes == -1)
        {
            clientConnfdUnconnect(connfd_index);
            pthread_cancel(pthread_self());
        }
        return NULL;
    }

    recvBytes = getMessage(clientConnfd[connfd_index], CLIENT_MESSAGE, recvBuff, sizeof(recvBuff));
    if(recvBytes == 0)
    {
        clientConnfdUnconnect(connfd_index);
        pthread_cancel(pthread_self());
    }

    recvBuff[recvBytes] = '\0';
    strcpy(password, recvBuff);
    printf("[%s]: password: %s\n", inet_ntoa(clientIP[connfd_index]), password);

    int res = logIn (&(clientIP[connfd_index]), clientConnfd[connfd_index], userName, password);

    if(res == LOGIN_SUCCESS)
        {
        client_account[connfd_index] = getAccountNodeByUserName(userName);
    strcpy(clientName[connfd_index], client_account[connfd_index]->userName);
}

    char sres[10];
    tostring(sres, res);
    sendBytes = send_message(clientConnfd[connfd_index], MESSAGE, sres);
    if(sendBytes == -1)
    {
        clientConnfdUnconnect(connfd_index);
        pthread_cancel(pthread_self());
    }
}

void* service_changePass(void *args)
{
    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    int oldstate;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
    int oldtype;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);

    int connfd_index;
    connfd_index = ((serviceThread_args*)args)->clientConnfd_index;

    int recvBytes;
    char recvBuff[RECV_BUFF_SIZE + 1];

    passwordType newPassword;

    recvBytes = getMessage(clientConnfd[connfd_index], CLIENT_MESSAGE, recvBuff, sizeof(recvBuff));
    if(recvBytes == 0)
    {
        clientConnfdUnconnect(connfd_index);
        pthread_cancel(pthread_self());
    }

    recvBuff[recvBytes] = '\0';
    strcpy(newPassword, recvBuff);
    printf("## New password: %s\n", newPassword);

    printf("## Changing password...\n");

    pthread_mutex_lock(&user_passwordFileLock);
    changePass(client_account[connfd_index], newPassword);
    pthread_mutex_unlock(&user_passwordFileLock);

    storeUsername_passData();
}

void* service_gamePlayingHistory(void *args)
{

}

void* service_signout(void *args)
{
    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    int oldstate;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
    int oldtype;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);

    int connfd_index;
    connfd_index = ((serviceThread_args*)args)->clientConnfd_index;

    signOut(client_account[connfd_index]);
    printf("Client exited\n");

    client_account[connfd_index] = NULL;
}
