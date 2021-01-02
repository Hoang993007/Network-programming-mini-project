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
#include "./../inc/convertNumAndStr.h"
#include "./../inc/room.h"
#include "./../inc/network.h"
#include "./../inc/server.h"
#include "./../inc/question.h"

pthread_mutex_t user_passwordFileLock;

pthread_t service_thread_id[MAX_SERVICE_THREAD];
int service_thread_index[MAX_SERVICE_THREAD];

// clientConnfd hold ID of client connection
int clientNum;
struct in_addr clientIP[MAX_CLIENT];
accountNode* client_account[MAX_CLIENT];
int clientConnfd[MAX_CLIENT];
int connfdNoServiceRunning[MAX_CLIENT];

//set of socket descriptors
fd_set readfds;

// maxfd is the highest socket ID in set of socket descriptors
int max_readfd;

int main(int argc, char *argv[])
{
    loadUsername_passData();
    loadQues_ansData();

    printf("Server's data is now ready!\n\n");

    pthread_mutex_init(&user_passwordFileLock, NULL);


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

    //Step 1: Construct socket
    printf("Constructing socket IPv4 - TCP...\n");

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        // listenfd is the socketID
        perror("Error: Error in creating socket");
        exit(2);
    }

    bzero(&servaddr, sizeof(servaddr));
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

    printf("Server started.\n\n");

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
        connfdNoServiceRunning[i] = 0;
    }

    /* clear all bits in fdset */
    FD_ZERO(&readfds);
    // fd_sets used to specify the descriptors that we want the kernel to test for reading, writing, and exception conditions.

    for (int i = 0; i < MAX_SERVICE_THREAD; i++)
    {
        service_thread_index[i] = -1;
    }

    roomListInit();

    printf("%s\n", "Server are now waiting for connections...");
    while(1)
    {
        clientSocketLen = sizeof(cliaddr);

        // The sets of file descriptors passed to select() are modified by select(),
        // so the set need to be re-initialized before for select() again.
        // (The programming error would only be notable if from more than one socket data sall be received.)

        printf("\n#Server are selecting for service request...\n\n");
        // add fd to set and turn on the bit for fd in set
        FD_ZERO(&readfds);
        // Nếu add trước khi select  khởi động thfi select  sẽ không nghe nó(vì nó chưa được thêm vào readfds đốivới select
        // vì thế thì nghe cứ nghe hếtrồi sau chọn cai nào thì chọn
        FD_SET(listenfd, &readfds);
        if(max_readfd < listenfd)
        {
            max_readfd = listenfd;
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

        int canReadFdIndex;
        canReadFdIndex = select(max_readfd + 1, &readfds, NULL, NULL, NULL);
        //Nếu thêm cai time vào sẽ sai, không hiểu tại sao
        // Dự đoán: gây độ trễ

        //  On success, returns the total number of bits that are set(that is the number of ready file descriptors)
        // • On time-out, returns 0
        // • On error, return -1

        if(canReadFdIndex == -1)
        {
            perror("\Error: ");
            // error occurred in select()
        }
        else printf ("The index of ready file descriptors (in the FD array) : %d\n", canReadFdIndex);

        printf("\n#Start processing service\n\n");

        // NOTE: the listenfd or connfd are both descriptor
        // select() check if a descriptor have an activity to exec or not. Not only use for listenfd or connfd

        // check the status of listenfd
        // if there is a client want to connect through listenfd socket the select() function will mark listenfd in readfds as 1 - have an activity

        // Neu sau vai lan select ma khong nhan duoc tin hieu => client da ngat ket noi

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
                printf("\n\nNew connection:\n");
                printf("\t1.Socket fd: %d\n", connfd);
                printf("\t2.Ip: %s\n", inet_ntoa(cliaddr.sin_addr));
                printf("\t3.Prot: %d\n\n", ntohs(cliaddr.sin_port));

                int i;
                for(i = 0; i < MAX_CLIENT; i++)
                {
                    if(clientConnfd[i] == -1)
                    {
                        clientConnfd[i] = connfd;
                        connfdNoServiceRunning[i] = 1;
                        memcpy(&(clientIP[i]), &(cliaddr.sin_addr), sizeof(cliaddr.sin_addr) + 1);
                        clientNum++;
                        break;
                    }
                }

                // confirm that connection are generated
                send_message(connfd, MESSAGE, "CONNECT_SUCCESSFULY");

                recvBytes = recv(connfd, recvBuff, sizeof(recvBuff), 0);

                if(recvBuff[0] != '\0')
                {
                    printf("%s\n", recvBuff);
                    char* userName = strtok(recvBuff, ";");
                    char* password = strtok(NULL, ";");

                    int res = logIn (&(cliaddr.sin_addr), connfd, userName, password);
                    if(res == LOGIN_SUCCESS)
                    {
                        printf("Address: %s alrealdy loged in!", inet_ntoa(cliaddr.sin_addr));
                        printf("Hello %s\n", userName);
                        send_message(connfd, MESSAGE, userName);
                        accountNode* logInAccount = accessToAccount (userName, password, &res);
                        client_account[i] = getAccountNodeByUserName(userName);
                    }
                    else
                    {
                        printf("Address: %s did not log in!", inet_ntoa(cliaddr.sin_addr));
                        send_message(connfd, MESSAGE, "NEED_LOGIN");
                    }
                }
                else
                {
                    printf("Address: %s did not log in!", inet_ntoa(cliaddr.sin_addr));
                    send_message(connfd, MESSAGE, "NEED_LOGIN");
                }
            }
        }

        // check the status of clientConnfd(s)
        for(int k = 0; k < MAX_CLIENT; k++)
        {
            //after add to set, it mark as have something to read while actual not
            if(clientConnfd[k] != -1 && connfdNoServiceRunning[k] == 1 && clientConnfd[k] != newClientConnfd)
            {
                if(FD_ISSET(clientConnfd[k], &readfds))
                {
                    printf("Client connection descriptor: %d\n", clientConnfd[k]);

                    recvBytes = recv(clientConnfd[k], recvBuff, sizeof(recvBuff), 0);

                    if(recvBytes < 0)
                    {
                        perror("Client exited\n");
                        continue;
                    }

                    recvBuff[recvBytes] = '\0';

                    if(recvBytes == 0)
                    {
                        printf("Client have disconnected with server\n");
                        printf("Closing the file descriptor of the client connection...\n");

                        FD_CLR(clientConnfd[k], &readfds);
                        close(clientConnfd[k]);
                        clientConnfd[k] = -1;

                        clientNum--;
                        connfdNoServiceRunning[k] = 1;
                        continue;
                    }

                    printf("[%s:%d]: Service num: %s\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), recvBuff);

                    int service;
                    service = atoi(recvBuff);

                    // Prepare user args for thread
                    user_thread_args* args = (user_thread_args*)malloc(sizeof(user_thread_args));
                    args->clientConnfd_index = k;

                    int freeThread_index = -1;
                    for(int i = 0; i < MAX_SERVICE_THREAD; i++)
                        if(service_thread_index[i] == -1)
                        {
                            freeThread_index = i;
                            break;
                        }
                    //TODO: check if too many thread


                    args->thread_index = freeThread_index;

                    // Create a thread to process the service
                    switch(service)
                    {
                    case 1:
                        printf ("%s\n", "Service 1: Register");
                        connfdNoServiceRunning[k] = 0;
                        //service_register(cliaddr, clientConnfd[k]);
                        break;

                    case 2:
                        printf ("%s\n", "Service 2: Activate account");
                        connfdNoServiceRunning[k] = 0;
                        //service_activate(cliaddr, clientConnfd[k]);
                        break;

                    case 3:
                        printf ("%s\n", "Service 3: Sign in");
                        connfdNoServiceRunning[k] = 0;
                        pthread_create(&(service_thread_id[freeThread_index]), NULL, &service_signin, (void*)args);
                        break;

                    case 4:
                        printf ("%s\n", "Service 4: Change password");

                        if(client_account[k]->isLogined == 1)
                        {
                            connfdNoServiceRunning[k] = 0;
                            pthread_create(&(service_thread_id[freeThread_index]), NULL, &service_changePass, (void*)args);
                        }
                        else printf("Not loged in\n");
                        break;

                    case 5:
                        printf ("%s\n", "Service 5: New room");
                        if(client_account[k]->isLogined == 1)
                        {
                            connfdNoServiceRunning[k] = 0;
                            pthread_create(&(service_thread_id[freeThread_index]), NULL, &newRoom, (void*)args);
                        }
                        else printf("Not loged in\n");
                        break;

                    case 6:
                        printf ("%s\n", "Service 6: Player enter room");

                        if(client_account[k]->isLogined == 1)
                        {
                            connfdNoServiceRunning[k] = 0;
                            pthread_create(&(service_thread_id[freeThread_index]), NULL, &playerEnterRoom, (void*)args);

                        }
                        else printf("Not loged in\n");
                        break;

                    case 7:
                        connfdNoServiceRunning[k] = 0;

                        if(client_account[k]->isLogined == 1)
                        {
                            printf ("%s\n", "Service 7: Sign out");
                            pthread_create(&(service_thread_id[freeThread_index]), NULL, &service_signout, (void*)args);
                            void *val;
                            pthread_join(service_thread_id[freeThread_index], &val);
                        }


                        printf("Closing the file descriptor of the client connection...\n");
                        FD_CLR(clientConnfd[k], &readfds);
                        close(clientConnfd[k]);
                        clientConnfd[k] = -1;

                        clientNum--;
                        connfdNoServiceRunning[k] = 1;

                        break;
                    }
                }
            }
        }
    }

    freeAccountNode();
    freeQues_ansNode();

    return 0;
}

void* service_register(void *args)
{

}

void* service_signin(void *args)
{
    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    int thread_index;
    int connfd_index;

    user_thread_args *actual_args = args;

    thread_index = actual_args->thread_index;
    connfd_index = actual_args->clientConnfd_index;

    free(args);

    int recvBytes;
    char recvBuff[RECV_BUFF_SIZE + 1];

    userNameType userName;
    passwordType password;

    recvBytes = recv(clientConnfd[connfd_index], recvBuff, sizeof(recvBuff), 0);
    userName[recvBytes] = '\0';
    strcpy(userName, recvBuff);
    printf("[%s]: User name: %s\n", inet_ntoa(clientIP[connfd_index]), userName);

    if(isExistUserName(userName) == ACCOUNT_EXIST)
    {
        printf("Account exist\n");
        send_message(clientConnfd[connfd_index],MESSAGE, "O");
    }
    else
    {
        printf("Account doesn't exist\n");
        send_message(clientConnfd[connfd_index], MESSAGE, "X");

        connfdNoServiceRunning[connfd_index] = 1;
        service_thread_index[thread_index] = -1;
        return NULL;
    }

    recvBytes = recv(clientConnfd[connfd_index], recvBuff, sizeof(recvBuff), 0);
    recvBuff[recvBytes] = '\0';
    strcpy(password, recvBuff);
    printf("[%s]: password: %s\n", inet_ntoa(clientIP[connfd_index]), password);

    int res = logIn (&(clientIP[connfd_index]), clientConnfd[connfd_index], userName, password);

    if(res == LOGIN_SUCCESS)
        client_account[connfd_index] = getAccountNodeByUserName(userName);


    char sres[10];
    tostring(sres, res);
    send_message(clientConnfd[connfd_index], MESSAGE, sres);

    connfdNoServiceRunning[connfd_index] = 1;
    service_thread_index[thread_index] = -1;
}

void* service_changePass(void *args)
{
    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    int thread_index;
    int connfd_index;

    user_thread_args *actual_args = args;

    thread_index = actual_args->thread_index;
    connfd_index = actual_args->clientConnfd_index;

    free(args);

    int recvBytes;
    char recvBuff[RECV_BUFF_SIZE + 1];

    passwordType newPassword;

    recvBytes = recv(clientConnfd[connfd_index], recvBuff, sizeof(recvBuff), 0);
    recvBuff[recvBytes] = '\0';
    strcpy(newPassword, recvBuff);
    printf("New password: %s\n", newPassword);

    printf("Changing password...\n");

    pthread_mutex_lock(&user_passwordFileLock);
    changePass(client_account[connfd_index], newPassword);
    pthread_mutex_unlock(&user_passwordFileLock);

    connfdNoServiceRunning[connfd_index] = 1;

    storeUsername_passData();
    service_thread_index[thread_index] = -1;
}

void* service_newGame(void *args)
{

}

void* service_gamePlayingHistory(void *args)
{

}

void* service_signout(void *args)
{
    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    int thread_index;
    int connfd_index;

    user_thread_args *actual_args = args;

    thread_index = actual_args->thread_index;
    connfd_index = actual_args->clientConnfd_index;

    free(args);

    signOut(client_account[connfd_index]);
    printf("Client exited\n");

    client_account[connfd_index] = NULL;
    service_thread_index[thread_index] = -1;
}
