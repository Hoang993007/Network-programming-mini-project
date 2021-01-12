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

#define LISTENQ 1

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("Parameter incorrect\n");
        exit(0);
    }

    int SERV_PORT;
    int listenfd, connfd, recvBytes, sendBytes;
    socklen_t clientSocketLen;
    struct sockaddr_in servaddr, cliaddr;

    SERV_PORT = atoi(argv[1]);

    //Step 1: Construct socket
    printf("Constructing socket IPv4 - TCP...\n");
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error: Error in creating socket");
        exit(2);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    //Step 2: Bind address to socket
    printf("Binding servaddr address to the socket...\n");
    if(bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)))
    {
        perror("Error: ");
        exit(EXIT_FAILURE);
    }

    printf("Server started.\n\n");

    printf("Establishing a socket to LISTENING for incoming connection\n");
    if(listen(listenfd, LISTENQ) < 0)
    {
        perror("Error: Error in establising socket to listen for incoming connection");
        exit(EXIT_FAILURE);
    }

    // Step 3: Select
    fd_set readfds;
    int max_readfd;

    printf("%s\n", "Server are now waiting for connections...");

    clientSocketLen = sizeof(cliaddr);

    FD_ZERO(&readfds);
    FD_SET(listenfd, &readfds);
    if(max_readfd < listenfd)
    {
        max_readfd = listenfd;
    }

    int canReadFdIndex;
    canReadFdIndex = select(max_readfd + 1, &readfds, NULL, NULL, NULL);
    if(canReadFdIndex == -1)
    {
        perror("\Error: ");
    }

    if (FD_ISSET (listenfd, &readfds))
    {
        connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clientSocketLen);
        if((connfd) < 0)
        {
            perror("Error: ");
            exit(EXIT_FAILURE);
        }

        printf("###New connection...\n");
    }

    char message[] = "0_a_really_really_long_message";
 
    message[0]++;
    printf("Message: %s\n", message);
    send(connfd, message, strlen(message), 0);

    message[0]++;
    printf("Message: %s\n", message);
    send(connfd, message, strlen(message), 0);

    message[0]++;
    printf("Message: %s\n", message);
    send(connfd, message, strlen(message), 0);

    message[0]++;
    printf("Message: %s\n", message);
    send(connfd, message, strlen(message), 0);

    message[0]++;
    printf("Message: %s\n", message);
    send(connfd, message, strlen(message), 0);

    message[0]++;
    printf("Message: %s\n", message);
    send(connfd, message, strlen(message), 0);

    message[0]++;
    printf("Message: %s\n", message);
    send(connfd, message, strlen(message), 0);

    message[0]++;
    printf("Message: %s\n", message);
    send(connfd, message, strlen(message), 0);

    message[0]++;
    printf("Message: %s\n", message);
    send(connfd, message, strlen(message), 0);

    return 0;
}
