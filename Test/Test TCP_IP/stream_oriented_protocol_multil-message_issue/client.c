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

#define RECV_BUFF_SIZE 200

int main(int argc, char *argv[])
{
    int SERV_PORT = atoi(argv[2]);
    char* SERV_ADDR = argv[1];

    int sockfd;
    socklen_t clientSocketLen;
    struct sockaddr_in servaddr;

    //Step 1: Construct socket
    printf("%s\n", "Constructing soket...");
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error: Problem in creating the socket");
        exit(2);
    }

    //Step 2: Create the remote server socket info structure
    printf("%s\n", "Creating remote server socket info structure...");
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERV_ADDR);
    servaddr.sin_port = htons(SERV_PORT);// convert to big-edian order

    // connect to server socket
    printf("%s\n", "Connecting the the server socket...");
    if(connect(sockfd, (struct sockaddr *)&(servaddr), sizeof(servaddr)) < 0)
    {
        printf("\n Error : Connecting to the server failed \n");
        exit(3);
    }

    int recvBytes;
    char recvBuff[RECV_BUFF_SIZE];

    recvBytes = recv(sockfd, recvBuff, RECV_BUFF_SIZE, 0);
    recvBuff[recvBytes] = '\0';

    while(recvBytes > 0)
    {
        printf("Recv message: %s\n", recvBuff);

        recvBytes = recv(sockfd, recvBuff, RECV_BUFF_SIZE, 0);
        recvBuff[recvBytes] = '\0';
    }
    return 0;
}
