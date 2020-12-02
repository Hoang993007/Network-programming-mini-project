#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//
#include <sys/types.h>
#include <sys/socket.h>
//
#include <netinet/in.h>
//
#include <arpa/inet.h>
//
#include <sys/select.h>
#include <sys/time.h>

#include "./inc/errorCode.h"

#define TERMINATE 101
#define RECV_BUFF_SIZE 4096

#define MAX_USERNAME_LENGTH 50
#define MAX_PASS_LENGTH 20

typedef char userNameType[MAX_USERNAME_LENGTH] ;
typedef char passwordType[MAX_PASS_LENGTH] ;

// supose that client will comunicate with only one
int sockfd, rcvBytes;
struct sockaddr_in servaddr;

int SERV_PORT;
char SERV_ADDR[255];

socklen_t len;
char recvBuff[RECV_BUFF_SIZE + 1];

int maxfd;

//set of socket descriptors
fd_set writefds;

void play ()
{
}

void login ()
{
    int loginFlag = 0;
    do
    {
        char service[5];
        // service 3: login
        strcpy(service, "3");
        send(sockfd, service, sizeof(service), 0);

        userNameType userName;
        passwordType password;

        printf("User name: ");
        scanf("%s", userName);
        getchar();
        send(sockfd, userName, sizeof(userName), 0);

        rcvBytes = recv(sockfd, recvBuff, sizeof(recvBuff), 0);
        recvBuff[rcvBytes] = '\0';
        if(strcmp(recvBuff, "X") == 0)
        {
            printf("Wrong account\n");
            continue;
        }

        printf("Insert password: ");
        scanf("%s", password);
        getchar();
        send(sockfd, password, sizeof(password), 0);

        int res;
        rcvBytes = recv(sockfd, recvBuff, sizeof(recvBuff), 0);
        recvBuff[rcvBytes] = '\0';

        res = atoi(recvBuff);
        //printf("resCode: %d\n", res);

        if(res == LOGIN_SUCCESS)
        {
            printf("OK\n");
            loginFlag = 1;
        }
        else if(res == ACCOUNT_JUST_BLOCKED)
        {
            printf("Account is blocked\n");
        }
        else if(res == ACCOUNT_IDLE || res == ACCOUNT_BLOCKED)
        {
            printf("Account not ready\n");
        }
        else  // wrong password
        {
            printf("NOT OK\n");
        }
    }
    while (loginFlag != 1);



//            strcpy(service, "6");
//
//            int terminateNewPass = 0;
//            do
//            {
//                passwordType newPassword;
//                printf("New password: ");
//                scanf("%s", newPassword);
//                getchar();
//
//                if(strcmp(newPassword, "bye") == 0)
//                {
//                    terminateNewPass = 1;
//                    terminate = 1; // end program
//                    strcpy(service, "7");
//                    send(sockfd, service, sizeof(service), 0);
//                    printf("Goodbye %s\n", userName);
//                    continue;
//                }
//
//                send(sockfd, service, sizeof(service), 0);
//                printf("Demand for changing password...\n");
//                send(sockfd, newPassword, sizeof(newPassword), 0);
//                printf("Receive encoded password...\n");
//                rcvBytes = recv(sockfd, recvBuff, sizeof(recvBuff), 0);
//                puts(recvBuff);
//            }
//            while(terminateNewPass == 0);
//        }
//
//        /*
//          if((rcvBytes = recv(sockfd, recvBuff, sizeof(recvBuff), 0)) == 0) {
//          perror("The server terminated prematurely");
//          exit(4);
//          } else {
//          recvBuff[rcvBytes] = '\0';
//          printf("%s\n", recvBuff);
//          }*/

}

int main(int argc, char *argv[])
{
    SERV_PORT = atoi(argv[2]);
    strcpy(SERV_ADDR, argv[1]);

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
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("\n Error : Connecting to the server failed \n");
        exit(3);
    }

    int askForSending;
    FD_SET(sockfd, &writefds);
    maxfd = sockfd;
    askForSending = select(maxfd + 1, NULL, &writefds, NULL, NULL);
    if(askForSending == -1)
    {
        perror("\Error: ");
        // error occurred in select()
    }

    //Step 3: Communicate with server
    if(FD_ISSET(sockfd, &writefds))
    {
        rcvBytes = recv(sockfd, recvBuff, sizeof(recvBuff), 0);
        recvBuff[rcvBytes] = '\0';
        printf("From server: %s\n", recvBuff);

        printf("Successfully connected to the server\n");

        rcvBytes = recv(sockfd, recvBuff, sizeof(recvBuff), 0);
        recvBuff[rcvBytes] = '\0';
        if(strcmp(recvBuff, "NEED_LOGIN") == 0)
        {
            login ();
        }
        else
        {
            printf("Welcome back! %s\n", recvBuff);
        }

        int choice;
        do
        {
            printf("1. New room\n");
            printf("2. Join room\n");
            printf("3. Playing history\n");
            printf("Enter your choice: ");

            scanf("%d", &choice);

            switch(choice)
            {
            case 1:
                printf("New room\n");
                char service[5];
                // service 7: Sign out
                strcpy(service, "7");
                send(sockfd, service, sizeof(service), 0);
                break;
            case 2:
                break;
            case 3:
                break;
            default:
                break;
            }
        }
        while(choice <= 3 && choice > 0);
    }

// close the descriptor
    close(sockfd);
}
