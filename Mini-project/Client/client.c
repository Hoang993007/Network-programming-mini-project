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
int maxfd_read;

//set of socket descriptors
fd_set writefds;
fd_set readfds;

void recv2 ()
{
    FD_ZERO(&readfds);

    FD_SET(sockfd, &readfds);
    maxfd = sockfd;
    int askForSending;
    //printf("Selecting...\n");
    askForSending = select(maxfd + 1, &readfds, NULL, NULL, NULL);

    if(askForSending == -1)
    {
        perror("\Error: ");
        // error occurred in select()
    }

    rcvBytes = recv(sockfd, recvBuff, RECV_BUFF_SIZE + 1, 0);

    //puts(recvBuff);
    //printf("%d\n",rcvBytes);
    recvBuff[rcvBytes] = '\0';

    if(rcvBytes > 0)
    {
        send(sockfd, "RECEIVE_SUCCESS", strlen("RECEIVE_SUCCESS") + 1, 0);
    }
    //printf("From server: %s\n",recvBuff);

    if(strcmp(recvBuff, "NOTIFICATION") == 0)
    {
        rcvBytes = recv(sockfd, recvBuff, sizeof(recvBuff), 0);
        recvBuff[rcvBytes] = '\0';
        printf("Notification from server: %s\n", recvBuff);
        recv2();
    }
}

void tostring(char str[], int num)
{
    int i, rem, len = 0, n;

    n = num;
    while (n != 0)
    {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++)
    {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[len] = '\0';
}

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

        recv2();
        if(strcmp(recvBuff, "X") == 0)
        {
            printf("-----------------------------\n\n");
            printf("Wrong account\n");
            printf("-----------------------------\n");
            continue;
        }

        printf("Insert password: ");
        scanf("%s", password);
        getchar();
        send(sockfd, password, sizeof(password), 0);

        int res;
        recv2();

        res = atoi(recvBuff);
        //printf("resCode: %d\n", res);

        if(res == LOGIN_SUCCESS)
        {
            printf("Log in sucessfuly\n\n");
            printf("-----------------------------\n");
            loginFlag = 1;
        }
        else if(res == ACCOUNT_JUST_BLOCKED)
        {
            printf("Account is blocked\n");
            printf("-----------------------------\n");
        }
        else if(res == ACCOUNT_IDLE || res == ACCOUNT_BLOCKED)
        {
            printf("Account not ready\n");
            printf("-----------------------------\n");
        }
        else  // wrong password
        {
            printf("Wrong password\n");
            printf("-----------------------------\n");
        }
    }
    while (loginFlag != 1);
}

int main(int argc, char *argv[])
{
    int recvBytes;

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
        recv2();
        printf("From server: %s\n", recvBuff);

        printf("Successfully connected to the server\n");
        printf("-----------------------------\n\n");

        recv2();
        if(strcmp(recvBuff, "NEED_LOGIN") == 0)
        {
            printf("You did not loged in yet! Please login before playing!\n*****----***\n\n");
            login ();
        }
        else
        {
            printf("Welcome back! %s\n", recvBuff);
        }

        int choice;
        do
        {
            printf("\n\n-----------------------------\n");
            printf("OPTION\n\n");
            printf("0. Change pass\n");
            printf("1. \n");
            printf("2. Join room\n");
            printf("3. New room\n");
            printf("Enter your choice: ");

            scanf("%d", &choice);
            printf("-----------------------------\n");

            switch(choice)
            {
            case 0:
                printf("Change pass\n");
                char service[5];

                strcpy(service, "4");

                send(sockfd, service, sizeof(service), 0);

                passwordType newPassword;
                printf("New password: ");
                scanf("%s", newPassword);
                getchar();

                send(sockfd, newPassword, sizeof(newPassword), 0);

                /*
                if(strcmp(newPassword, "bye") == 0)
                {
                    terminateNewPass = 1;
                    terminate = 1; // end program
                    strcpy(service, "7");
                    send(sockfd, service, sizeof(service), 0);
                    printf("Goodbye %s\n", userName);
                    continue;
                }
                */


                // CHECK IF SUCCESS

                /*
                  if((rcvBytes = recv(sockfd, recvBuff, sizeof(recvBuff), 0)) == 0) {
                  perror("The server terminated prematurely");
                  exit(4);
                  } else {
                  recvBuff[rcvBytes] = '\0';
                  printf("%s\n", recvBuff);
                  }*/

                break;

            case 1:
                // service 7: Sign out
                strcpy(service, "7");
                send(sockfd, service, sizeof(service), 0);
                break;
            case 2:
                printf("Enter room\n\n");
                strcpy(service, "6");
                send(sockfd, service, sizeof(service), 0);

                recv2();
                if(strcmp(recvBuff, "NO_ROOM") == 0)
                {
                    puts(recvBuff);
                    continue;
                }

                printf("Room num: %s\n\n", recvBuff);
                printf("=============\n\n");
                printf("Room list:\n\n");

                while(1)
                {
                    recv2();
                    if(strcmp(recvBuff, "PRINT_ROOM_END") == 0)
                        break;
                    printf("Room ID: %s\n", recvBuff);
                    recv2();
                    printf("Room name: %s\n", recvBuff);
                    recv2();
                    printf("Num of player preset: %s\n", recvBuff);
                    recv2();
                    printf("Num of player: %s\n", recvBuff);
                    printf("\n");
                }

                int chosenRoom;
                printf("Enter ID of room: ");
                scanf("%d", &chosenRoom);
                printf("-----------------------------\n\n\n");
                getchar();
                char chosenRoom_char[5];

                tostring(chosenRoom_char, chosenRoom);
                send(sockfd, chosenRoom_char, sizeof(chosenRoom_char), 0);

                recv2();
                puts(recvBuff);

                printf("Your are in room: What do you want (show oftion)");
                break;
            case 3:
                printf("New room\n");
                // service 7: Sign out
                strcpy(service, "5");
                recvBytes = send(sockfd, service, sizeof(service), 0);



                char roomName[255];
                printf("Room name: ");
                scanf("%s", roomName);
                getchar();

                recvBytes = send(sockfd, roomName, sizeof(roomName), 0);

                int playerNumPreSet;
                printf("playerNumPreSet: ");
                scanf("%d", &playerNumPreSet);

                char playerNumPreSet_char[2];
                playerNumPreSet_char[0] = playerNumPreSet + '0';
                playerNumPreSet_char[1] = '\0';

                recvBytes = send(sockfd, playerNumPreSet_char, sizeof(playerNumPreSet_char), 0);
                break;
            default:
                break;
            }
        }
        while(choice <= 3 && choice >= 0);
    }

// close the descriptor
    close(sockfd);
}
