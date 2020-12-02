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
#include "./inc/services.h"
#include "./inc/accountSystem.h"
#include "./inc/errorCode.h"
#include "./inc/convertNumAndStr.h"
//
#include <sys/select.h>
#include <sys/time.h>
//
#include <pthread.h>

#define RECV_BUFF_SIZE 4096
#define LISTENQ 3 /* the number of pending connections that can be queued for a server socket. (call waiting allowance */

#define MAX_CLIENT 3
#define MAX_THREAD 10

pthread_t tid[MAX_THREAD];
int t_index[MAX_THREAD];

pthread_mutex_t lock;

typedef struct
{
    struct sockaddr_in cliaddr;
    int clientConnfd;
    int thread_index;
} user_thread_args;

typedef struct
{
    struct sockaddr_in servaddr, cliaddr;
} room;

void *service_register(void *arg);
void *service_activate(void *arg);
void *service_signin(void *arg);
void *service_changePass(void *arg);
void *service_newGame(void *arg);
void *service_gamePlayingHistory(void *arg);
void *service_signout(void *arg);

int main(int argc, char *argv[])
{
    loadUsername_passData();
    printf("Server is now ready for logining\n");

    if(argc != 2)
    {
        printf("Parameter incorrect\n");
        exit(0);
    }

    int SERV_PORT;
    int listenfd, connfd, recvBytes;

    socklen_t clientSocketLen; // size of client socket address

    struct sockaddr_in servaddr, cliaddr; // address structure

    char recvBuff[RECV_BUFF_SIZE + 1];


    SERV_PORT = atoi(argv[1]);

    bzero(&servaddr, sizeof(servaddr));

    //Step 1: Construct socket
    printf("Constructing socket IPv4 - TCP...\n");

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        // listenfd is the socketID
        perror("Error: Error in creating socket");
        exit(2);
    }

    // Preparation of the server address
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    /* INADDR_ANY: kernel chooses IP address for server
       This time is 172.0.0.1 - local host address */
    servaddr.sin_port = htons(SERV_PORT); // specifies port

    //Step 2: Bind address to socket
    printf("Binding servaddr address to the socket...\n");

    if(bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)))
    {
        perror("Error: ");
        exit(EXIT_FAILURE);
    }

    printf("Server started.\n");

    // Establish a socket to LISTENING for incoming connection
    // Listen to the socket by creating a connection queue, then wait for clients
    printf("Establishing a socket to LISTENING for incoming connection\n");
    if(listen(listenfd, LISTENQ) < 0)
    {
        perror("Error: Error in establising socket to listen for incoming connection");
        exit(EXIT_FAILURE);
    }

    // clientConnfd hold ID of client connection
    int clientConnfd[FD_SETSIZE], someThingToRead;
    int maxfd;

    //set of socket descriptors
    fd_set readfds;

    struct timeval tv;
    // Assign initial value for the array of connection socket
    for(int i = 0; i < MAX_CLIENT; i++)
    {
        clientConnfd[i] = -1;
    }

    /* clear all bits in fdset */
    FD_ZERO(&readfds);

    // fd_sets use to specify the descriptors that we want the kernel to test for reading, writing, and exception conditions.

    // add listenfd to set and turn on the bit for fd in set
    FD_SET(listenfd, &readfds);
    maxfd = listenfd;

    // maxfd is the highest socket ID in readfds

    printf("%s\n", "Server are now waiting for connections...");

    //Step 3: Communicate with client
    /* now loop, receiving data and printing what we received */

    for (int i = 0; i < MAX_THREAD; i++)
    {
        t_index[i] = -1;
    }

    while(1)
    {
        clientSocketLen = sizeof(cliaddr);

        //wait for an activity from a descriptor
        //(timeout 10.5 secs)

    tv.tv_sec = 10;
    tv.tv_usec = 500000;
        // ở đây có nghĩa là khởi động việc nghe
        // nghe 1 lượt xem có cổng nào có cái để đọc vào không thì thêm vào hàng đợi.
        // sau đó là khóa các cổng, tiến hành xử lý các cái trong hàng đợi

        // The select() function asks kernel to simultaneously check multiple sockets (saved in readfs)...
        //...to see if they have data waiting to be recv(), or if you can send() data to them without blocking, or if some exception has occurred.
        //askForCheckingActivity = select(maxfd + 1, &readfds, NULL, NULL, &tv);
        printf("Selecting...\n");
        //select() not detecting incoming data
        // The sets of file descriptors passed to select() are modified by select(),
        // so the set need to be re-initialized before for select() again.
        // (The programming error would only be notable if from more than one socket data sall be received.)
        // re-initialized...
        FD_SET(listenfd, &readfds);
        for(int k = 0; k < MAX_CLIENT; k++)
            if(clientConnfd[k] != -1)
                FD_SET(clientConnfd[k], &readfds);
        someThingToRead = select(maxfd + 1, &readfds, NULL, NULL, &tv);
        printf("Start processing\n");
        //  On success, returns the total number of bits that are set(that is the number of ready file descriptors)
        // • On time-out, returns 0
        // • On error, return -1

        if(someThingToRead == -1)
        {
            perror("\Error: ");
            // error occurred in select()
        }
        else if(someThingToRead == 0)
        {
            printf("Timeout occurred! No data after 10.5s... \n");
        }
        else printf ("The number of ready file descriptors (in the FD array) : %d\n", someThingToRead);

        // Xử lý các yêu cầu mà select đã nghe được từ các cổng


        // NOTE: the listenfd or connfd are both descriptor
        // select() check if a descriptor have an activity to exec or not. Not only use for listenfd or connfd

        // NOTE: in this small program we use only listenfd socket to connect with client

        // check the status of listenfd
        // if there is a client want to connect through listenfd socket the select() function will mark listenfd in readfds as 1 - have an activity
        if (FD_ISSET (listenfd, &readfds))
        {
            connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clientSocketLen);
            // Accept a connection request -> return a File Descriptor (FD)
            if((connfd) < 0)
            {
                perror("Error: ");
                exit(EXIT_FAILURE);
            }
            else
            {
                printf("New connection:\n");
                printf("\t1.Socket fd: %d\n", connfd);
                printf("\t2.Ip: %s\n", inet_ntoa(cliaddr.sin_addr));
                printf("\t3.Prot: %d\n", ntohs(cliaddr.sin_port));

                for(int i = 0; i < MAX_CLIENT; i++)
                {
                    if(clientConnfd[i] == -1)
                    {
                        clientConnfd[i] = connfd;

                        break;
                    }
                }

                // confirm that connection are generated
                send(connfd, "OK", sizeof("OK"), 0);

                if(isLogedIn(inet_ntoa(cliaddr.sin_addr)) == LOGED_IN)
                {
                    accessPermit* logInAccount = logInInfoGet(inet_ntoa(cliaddr.sin_addr));
                    printf("%s alrealdy logined - user name: %s\n", inet_ntoa(cliaddr.sin_addr), logInAccount->accessAccount->userName);
                    send(connfd, logInAccount->accessAccount->userName, sizeof(userNameType), 0);
                }
                else
                {
                    send(connfd, "NEED_LOGIN", sizeof("NEED_LOGIN") + 1, 0);
                }

                // Thêm thằng mới kết nối vào dãy FD để bắt đầu lắng nghe và nhận yeu cầu từ nó
                FD_SET(connfd, &readfds);
                if(connfd > maxfd)
                {
                    maxfd = connfd;
                }
            }
        }

        // check the status of clientConnfd(s)
        for(int k = 0; k < MAX_CLIENT; k++)
        {printf("%d\n", k);
            if(clientConnfd[k] != -1)
            {
                if(FD_ISSET(clientConnfd[k], &readfds)) // => have something to read (select have found something to read)
                {
                    printf("Client connection descriptor: %d\n", clientConnfd[k]);
                    recvBytes = recv(clientConnfd[k], recvBuff, sizeof(recvBuff), 0);  // receive service number
                    if(recvBytes < 0)
                    {
                        perror("Client exited\n");
                        continue;
                    }
                    else if(recvBytes == 0)
                    {
                        printf("Closing the file descriptor of the client connection...\n");
                        FD_CLR(clientConnfd[k], &readfds);
                        close(clientConnfd[k]);
                        clientConnfd[k] = -1;
                        perror("Nothing receive from client\n");
                        continue;
                    }

                    recvBuff[recvBytes] = '\0';
                    printf("[%s:%d]: Service num: %s\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), recvBuff);

                    int service;
                    service = atoi(recvBuff);

                    //print_username_pass();

                    // Prepare args for thread
                    user_thread_args* args = (user_thread_args*)malloc(sizeof(user_thread_args));
                    args->cliaddr = cliaddr;
                    args->clientConnfd = connfd;

                    int freeThread = -1;
                    while (freeThread < 0)
                    {
                        for(int i = 0; i < MAX_THREAD; i++)
                            if(t_index[i] == -1)
                                freeThread = i;
                            else printf("Too many thread...");
                    }

                    args->thread_index = freeThread;

                    // Create a thread to process the service
                    switch(service)
                    {
                    case 1:
                        printf ("%s\n", "Service 1: Register");
                        //service_register(cliaddr, clientConnfd[k]);
                        break;

                    case 2:
                        printf ("%s\n", "Service 2: Activate account");
                        //service_activate(cliaddr, clientConnfd[k]);
                        break;

                    case 3:
                        printf ("%s\n", "Service 3: Sign in");
                        pthread_create(&(tid[freeThread]), NULL, &service_signin, (void*)args);
                        break;

                    case 4:
                        printf ("%s\n", "Service 4: Change password");
                        if(isLogedIn(inet_ntoa(cliaddr.sin_addr)) == LOGED_IN)
                        {
                            //service_changePass(cliaddr, clientConnfd[k]);
                        }
                        else printf("Not loged in\n");
                        break;

                    case 5:
                        printf ("%s\n", "Service 5: New game");
                        if(isLogedIn(inet_ntoa(cliaddr.sin_addr)) == LOGED_IN)
                        {
                            //service_newGame(cliaddr, clientConnfd[k]);
                        }
                        else printf("Not loged in\n");
                        break;

                    case 6:
                        printf ("%s\n", "Service 6: Game playing history");
                        if(isLogedIn(inet_ntoa(cliaddr.sin_addr)) == LOGED_IN)
                        {

                            //service_gamePlayingHistory(cliaddr, clientConnfd[k]);
                        }
                        else printf("Not loged in\n");
                        break;

                    case 7:
                        printf ("%s\n", "Service 7: Sign out");
                        if(isLogedIn(inet_ntoa(cliaddr.sin_addr)) == LOGED_IN)
                        {
                            pthread_create(&(tid[freeThread]), NULL, &service_signout, (void*)args);
                            void *val;
                            pthread_join(tid[freeThread], &val);

                            printf("Closing the file descriptor of the client connection...\n");
                            FD_CLR(clientConnfd[k], &readfds);
                            close(clientConnfd[k]);
                            clientConnfd[k] = -1;
                        }
                        else printf("Not loged in\n");
                        break;
                    }
                }
            }
        }
    }

    freeAccountNode();
    freeLogIn();

    return 0;
}

void* service_register(void *arg)
{

}

void* service_activate(void *arg)
{

}

void* service_signin(void *arg)
{
    int clientConnfd;
    struct sockaddr_in cliaddr;
    int thread_index;

    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    user_thread_args *actual_args = arg;
    cliaddr = actual_args->cliaddr;
    clientConnfd = actual_args->clientConnfd;
    thread_index = actual_args->thread_index;
    free(arg);

    int recvBytes;
    char recvBuff[RECV_BUFF_SIZE + 1];

    userNameType userName;
    passwordType password;

    recvBytes = recv(clientConnfd, recvBuff, sizeof(recvBuff), 0);
    userName[recvBytes] = '\0';
    strcpy(userName, recvBuff);
    printf("[%s:%d]: User name: %s\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), userName);

    if(isExistUserName(userName) == ACCOUNT_EXIST)
    {
        send(clientConnfd, "O", sizeof("O"), 0);
    }
    else
    {
        printf("Account doesn't exit\n");
        send(clientConnfd, "X", sizeof("X"), 0);
        return NULL;
    }

    recvBytes = recv(clientConnfd, recvBuff, sizeof(recvBuff), 0);
    recvBuff[recvBytes] = '\0';
    strcpy(password, recvBuff);
    printf("[%s:%d]: password: %s\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), password);

    if(isLogedIn(inet_ntoa(cliaddr.sin_addr)) == NOT_LOGED_IN)
    {
        int res = logIn (inet_ntoa(cliaddr.sin_addr), userName, password);
        // for test
        // printf("%d\n", res);
        char sres[10];
        tostring(sres, res);
        send(clientConnfd, sres, sizeof(sres), 0);
    }
    else
    {
        printf("You're alrealdy logined\n");
    }

    t_index[thread_index] = -1;
}

void* service_changePass(void *arg)
{
    int clientConnfd;
    struct sockaddr_in cliaddr;
    int thread_index;

    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    user_thread_args *actual_args = arg;
    cliaddr = actual_args->cliaddr;
    clientConnfd = actual_args->clientConnfd;
    thread_index = actual_args->thread_index;
    free(arg);

    int recvBytes;
    char recvBuff[RECV_BUFF_SIZE + 1];

    if(isLogedIn(inet_ntoa(cliaddr.sin_addr)) != LOGED_IN)
    {
        printf("Not loged in\n");
    }
    else
    {
        passwordType newPassword;

        recvBytes = recv(clientConnfd, recvBuff, sizeof(recvBuff), 0);
        recvBuff[recvBytes] = '\0';
        strcpy(newPassword, recvBuff);
        printf("New password: %s\n", newPassword);

        int numChar = 0;
        for(int i = 0; i < strlen(newPassword); i++)
        {
            if (newPassword[i] < '0'
                    || (newPassword[i] > '9' && newPassword[i] < 'A')
                    || (newPassword[i] > 'Z' && newPassword[i] < 'a')
                    || newPassword[i] > 'z')
            {
                send(clientConnfd, "Error", sizeof("Error"), 0);
                break;
            }

            if(newPassword[i] <= '9' && newPassword[i] >= '0')
            {
                numChar++;
            }
        }
        printf("Changing password...\n");

        changePass(inet_ntoa(cliaddr.sin_addr), newPassword);

        passwordType encodePass;
        int charCharIndex, numCharIndex;
        encodePass[strlen(newPassword)] = '\0';
        charCharIndex = numChar;
        numCharIndex = 0;
        printf("New password num: %d\n", numChar);
        printf("New password char: %d\n", (int)(strlen(newPassword) - numChar));

        for(int i = 0; i < strlen(newPassword); i++)
        {
            if(newPassword[i] <= '9' && newPassword[i] >= '0')
            {
                encodePass[numCharIndex] = newPassword[i];
                numCharIndex++;
            }
            else
            {
                encodePass[charCharIndex] = newPassword[i];
                charCharIndex++;
            }
        }
        printf("EncodedPass: %s\n", encodePass);
        send(clientConnfd, encodePass, sizeof(encodePass), 0);
    }

    t_index[thread_index] = -1;
}

void* service_newGame(void *arg)
{

}

void* service_gamePlayingHistory(void *arg)
{

}

void* service_signout(void *arg)
{
    int clientConnfd;
    struct sockaddr_in cliaddr;
    int thread_index;

    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    user_thread_args *actual_args = arg;
    cliaddr = actual_args->cliaddr;
    clientConnfd = actual_args->clientConnfd;
    thread_index = actual_args->thread_index;
    free(arg);

    if(isLogedIn(inet_ntoa(cliaddr.sin_addr)) == LOGED_IN)
    {
        signOut(inet_ntoa(cliaddr.sin_addr));
        //print_username_pass();
        printf("Client exited\n");
    }
    else printf("Not loged in\n");

    t_index[thread_index] = -1;
}
