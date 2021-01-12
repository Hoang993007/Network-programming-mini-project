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

char messageEndMarker[5] = "|";

int getConnfdIndex(int connfd)
{
    for(int i = 0; i < MAX_CLIENT; i++)
    {
        if(clientConnfd[i] == connfd)
            return i;
    }
    return -1;
}

void clientConnfdUnconnect(int connfdIndex)
{
    // Sign out
    if(client_account[connfdIndex] != NULL)
    {
        signOut(client_account[connfdIndex]);
        client_account[connfdIndex] = NULL;
        printf("Client exited\n");
    }

    //-----------------------------------------------------------

    printf("### Client've disconnected to server\n");
    printf("###Closing the file descriptor of the client connection...\n");

    pthread_mutex_lock(&clientFDDataLock);

    close(clientConnfd[connfdIndex]);

    clientConnfd[connfdIndex] = -1;

    clientNum--;

    pthread_mutex_unlock(&clientFDDataLock);

    printf("###Closing completed\n");
}
