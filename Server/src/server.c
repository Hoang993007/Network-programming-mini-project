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
#include "./inc/services.h"
#include "./inc/accountSystem.h"
#include "./inc/errorCode.h"
#include "./inc/convertNumAndStr.h"
#include "./inc/room.h"
#include "./inc/network.h"

#define RECV_BUFF_SIZE 4096
#define SEND_BUFF_SIZE 100
#define LISTENQ 3 /* the number of pending connections that can be queued for a server socket. (call waiting allowance */

#define MAX_CLIENT 3
#define MAX_SERVICE_THREAD 10

#define MAX_ROOM 2
#define MAX_ROOM_PLAYER 5
#define ROOM_NAME 20

pthread_t service_thread_id[MAX_SERVICE_THREAD];
int service_thread_index[MAX_SERVICE_THREAD];

pthread_mutex_t client_connfd_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// clientConnfd hold ID of client connection
int clientNum = 0;
int clientConnfd[MAX_CLIENT];
int wantSelect[MAX_CLIENT];
struct in_addr connfd_IP[MAX_CLIENT];

//set of socket descriptors
fd_set readfds;
// maxfd is the highest socket ID in readfds
int maxfd = 0;

void *service_register(void *arg);
void *service_activate(void *arg);
void *service_signin(void *arg);
void *service_changePass(void *arg);
void *service_newGame(void *arg);
void *service_gamePlayingHistory(void *arg);
void *service_signout(void *arg);

void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;

    // Storing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}

int send2(int connfd, char* send_message)
{
    pthread_mutex_lock(&lock);

    fd_set writefds;

    int flag = 0;
    int sendBytes;

    do
    {
        FD_ZERO(&writefds);
        FD_SET(connfd, &writefds);
        maxfd = connfd;

        //printf("Selecting...\n");
        int askForSending = select(maxfd + 1, NULL, &writefds, NULL, NULL);

//    if(askForSending != connfd)
//    {
//    continue;//sai vi askForSending khong phai so hieu
//        //perror("\Error: ");
//        // error occurred in select()
//    }

        if(!FD_ISSET(connfd, &writefds))
        {
            continue;
        }
        //printf("read to send\n");
        flag = 1;

        int message_size = strlen(send_message) + 5;
        //puts(send_message);
        //printf("%d\n",message_size);
        sendBytes = send(connfd, send_message,  message_size, 0);

        //check
        int recvBytes;
        char recvBuff[RECV_BUFF_SIZE + 1];
        recvBytes = recv(connfd, recvBuff, sizeof(recvBuff), 0);
        recvBuff[recvBytes] = '\0';
        //puts(recvBuff);
        if(strcmp(recvBuff, "RECEIVE_SUCCESS") != 0)
            printf("ERROR: send_error\n");
    }
    while(flag == 0);

    pthread_mutex_unlock(&lock);
    return sendBytes;
};



// ROOM ------------------------------------------------------------
typedef struct _room
{
    pthread_cond_t enoughPlayer_cond;
    pthread_mutex_t room_lock;
    int roomID;
    char roomName[ROOM_NAME];
    pthread_t room_thread_id;

    int playerNumPreSet;
    int playerNum;
    accountNode* player[MAX_ROOM_PLAYER];
    //struct _room* next;
} room;


int roomIDGenerate = 0;
int roomNum = 0;
room* roomList[MAX_ROOM];

pthread_t room_thread_id[MAX_ROOM];
pthread_mutex_t room_data_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    struct sockaddr_in cliaddr;
    int clientConnfd;
    int thread_index;
} user_thread_args;

void roomListInit();
void printRoom ();
void* newRoom(void* arg);
void* playerEnterRoom (void* arg);


void roomListInit()
{
    for(int i = 0; i < MAX_ROOM; i++)
    {
        roomList[i] = NULL;
    }
}

void* newRoom(void* arg)
{
    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    int connfd;
    struct sockaddr_in cliaddr;

    user_thread_args *actual_args = arg;
    cliaddr = actual_args->cliaddr;
    connfd = actual_args->clientConnfd;
    free(arg);

    // Have to stop select with this connfd because even receive in here, process in here but still selected by the main thread
    // meaning: now this connfd are belong to this thread
    int connfd_index;

    for(int k = 0; k < MAX_CLIENT; k++)
        if(clientConnfd[k] == connfd)
        {
            connfd_index = k;
            wantSelect[connfd_index] = 0;
            break;
        }


    int recvBytes;
    char recvBuff[RECV_BUFF_SIZE + 1];

    char roomName[ROOM_NAME];
    recvBytes = recv(connfd, recvBuff, sizeof(recvBuff), 0);
    recvBuff[recvBytes] = '\0';
    strcpy(roomName, recvBuff);
    printf("[%s:%d]: Room name: %s\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), roomName);

    int playerNumPreSet;
    recvBytes = recv(connfd, recvBuff, sizeof(recvBuff), 0);
    recvBuff[recvBytes] = '\0';
    playerNumPreSet = atoi(recvBuff);
    printf("[%s:%d]: Max player: %d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), playerNumPreSet);


    wantSelect[connfd_index] = 1;



    if(roomNum < MAX_ROOM)
    {
        pthread_mutex_lock(&room_data_lock);

        room* newRoom = (room*)malloc(sizeof(room));

        roomIDGenerate++;
        roomNum++;

        newRoom->roomID = roomIDGenerate;
        strcpy(newRoom->roomName, roomName);
        newRoom->room_thread_id = pthread_self();

        newRoom->playerNumPreSet = playerNumPreSet;
        newRoom->playerNum = 1;
        pthread_cond_init(&(newRoom->enoughPlayer_cond), NULL);
        pthread_mutex_init(&(newRoom->room_lock), NULL);
        newRoom->player[0] = getAccountNodeByLoginedIP(inet_ntoa(cliaddr.sin_addr));

        for(int i = 1; i < playerNumPreSet; i++)
        {
            newRoom->player[i] = NULL;

        }

        int room_index;
        for(room_index = 0; room_index < MAX_ROOM; room_index++)
        {
            if(roomList[room_index] == NULL)
            {
                roomList[room_index] = newRoom;
                break;
            }
        }

        printf("Sucessfully created new room!\n");
        printRoom();

        pthread_mutex_unlock(&room_data_lock);

        //start playing
        //wait untill there are enough player
        pthread_mutex_lock(&(newRoom->room_lock));
        if (newRoom->playerNum != newRoom->playerNumPreSet)
        {


            // Have to use is own mutex_lock or have to init mutex_lock??????????????????????????
            printf("Waiting for player\n");
            pthread_cond_wait(&(newRoom->enoughPlayer_cond), &(newRoom->room_lock));

            //while(newRoom->playerNum != newRoom->playerNumPreSet);
            //lặp quá nhiều và liên tục sẽ làm rối loạn luồng
        }

        printf("-----------------------------\n");
        printf("Enough player! GAME START!\n");
        printf("-----------------------------\n");


        //Khóa tất cả các connfd của player để dành riêng cho trò chơi




        // release lock
        pthread_mutex_unlock(&(newRoom->room_lock));


        room* tmp = newRoom;
        roomList[room_index] = NULL;
        free(tmp);
        roomNum--;
    }
}

void* playerEnterRoom (void* arg)
{
    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    int connfd;
    struct sockaddr_in cliaddr;

    user_thread_args *actual_args = arg;
    cliaddr = actual_args->cliaddr;
    connfd = actual_args->clientConnfd;
    free(arg);


    // Have to stop select with this connfd because even receive in here, process in here but still selected by the main thread
    // meaning: now this connfd are belong to this thread
    int connfd_index;

    for(int k = 0; k < MAX_CLIENT; k++)
        if(clientConnfd[k] == connfd)
        {
            connfd_index = k;
            wantSelect[connfd_index] = 0;
            break;
        }


    int recvBytes;
    char recvBuff[RECV_BUFF_SIZE + 1];
    char sendBuff[SEND_BUFF_SIZE + 1];

    if(roomNum == 0)
    {
        send2(connfd, "NO_ROOM");

        pthread_mutex_unlock(&room_data_lock);
        wantSelect[connfd_index] = 1;
        return NULL;
    }

    int curRoomNum = roomNum;

    int check;
    tostring(sendBuff, curRoomNum);
    send2(connfd, sendBuff);

    for(int i = 0; i < MAX_ROOM; i++)
    {
        if(roomList[i] != NULL)
        {
            char sendNum[5];

            tostring(sendNum, roomList[i]->roomID);
            //printf("roomID: *%s*\n", sendNum);
            send2(connfd, sendNum);
//printf("Cheking0...!\n");

            //printf("room name: *%s*\n", roomList[i]->roomName);
            send2(connfd, roomList[i]->roomName);
//printf("Cheking1...!\n");

            tostring(sendNum, roomList[i]->playerNumPreSet);
            //printf("room num preset: *%s*\n", sendNum);
            send2(connfd, sendNum);
//printf("Cheking2...!\n");

            tostring(sendNum, roomList[i]->playerNum);
            //printf("player num: *%s*\n", sendNum);
            send2(connfd, sendNum);

            //printf("Cheking3...!\n");
        }
    }

    send2(connfd, "PRINT_ROOM_END");

    recvBytes = recv(connfd, recvBuff, sizeof(recvBuff), 0);
    recvBuff[recvBytes] = '\0';
    int chosenRoomID = atoi(recvBuff);
    printf("[%s:%d]: Chosen room ID: %d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), chosenRoomID);

    pthread_mutex_lock(&room_data_lock);
    printf("start entering...\n");

    int i;
    for(i = 0; i < MAX_ROOM; i++)
    {
        if(roomList[i] != NULL && roomList[i]->roomID == chosenRoomID)
        {
            if(roomList[i]->playerNum < roomList[i]->playerNumPreSet)
            {
                for(int j = 0; j < roomList[i]->playerNumPreSet; j++)
                {
                    if(roomList[i]->player[j] == NULL)
                    {
                        roomList[i]->player[j] = getAccountNodeByLoginedIP(inet_ntoa(cliaddr.sin_addr));
                        roomList[i]->playerNum++;
                        send2(connfd, "ENTER_ROOM_SUCCESSFULY");
                        printRoom();
                        break;
                    };
                }

                if (roomList[i]->playerNum == roomList[i]->playerNumPreSet)
                {
                    printf("enough player!\n");
                    pthread_cond_signal(&(roomList[i]->enoughPlayer_cond));
                }
            }
            else
            {
                printf("Room full\n");
                send2(connfd, "ROOM_FULL");
            }
            break;
        }
    }

    if(i == MAX_ROOM)
        send2(connfd, "ROOM_NOT_FOUND");

    pthread_mutex_unlock(&room_data_lock);
    wantSelect[connfd_index] = 1;
}

void printRoom ()
{
    printf("-----------------------------\n");
    printf("Room info\n\n");
    printf("Room num: %d\n", roomNum);
    for(int i = 0; i < MAX_ROOM; i++)
    {
        if(roomList[i] != NULL)
        {
            printf("Room ID: %d\n", roomList[i]->roomID);
            printf("Room thread ID: %ld\n", roomList[i]->room_thread_id);
            printf("Room name: %s\n", roomList[i]->roomName);
            printf("Num of player preset: %d\n", roomList[i]->playerNumPreSet);
            printf("Num of player: %d\n", roomList[i]->playerNum);

            for(int j = 0; j < roomList[i]->playerNumPreSet; j++)
            {
                if(roomList[i]->player[j] != NULL)
                {
                    printf("Player name: %s\n", roomList[i]->player[j]->userName);
                }
                else printf("Player name: waiting...\n");
            }
        }
    }
    printf("-----------------------------\n");
}

// add check free room thread
//-------------------------------------------------------------------



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
    int listenfd, connfd, recvBytes, sendBytes;
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

    // Step 3: Select
    for(int i = 0; i < MAX_CLIENT; i++)
    {
        clientConnfd[i] = -1;
        wantSelect[i] = 0;
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

        //(timeout 10.5 secs)
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 500000;

        // The select() function asks kernel to simultaneously check multiple sockets (saved in readfs)...
        //...to see if they have data waiting to be recv(), or if you can send() data to them without blocking, or if some exception has occurred.

        // The sets of file descriptors passed to select() are modified by select(),
        // so the set need to be re-initialized before for select() again.
        // (The programming error would only be notable if from more than one socket data sall be received.)

        printf("\n#Selecting request...\n\n");
        // add fd to set and turn on the bit for fd in set
        FD_ZERO(&readfds);
        // Nếu add trước khi select  khởi động thfi select  sẽ không nghe nó(vì nó chưa được thêm vào readfds đốivới select
        // vì thế thì nghe cứ nghe hếtrồi sau chọn cai nào thì chọn
        FD_SET(listenfd, &readfds);
        if(maxfd < listenfd)
        {
            maxfd = listenfd;
        }
        for(int k = 0; k < MAX_CLIENT; k++)
            if(clientConnfd[k] != -1)
            {
                FD_SET(clientConnfd[k], &readfds);
                if(maxfd < clientConnfd[k])
                {
                    maxfd = clientConnfd[k];
                }
            }

        int someThingToRead;
        //someThingToRead = select(maxfd + 1, &readfds, NULL, NULL, &tv);
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        someThingToRead = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        //Nếu thêm cai time vào sẽ sai, không hiểu tại sao
        // Dự đoán: gây độ trễ
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!111

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
            printf("\n#Timeout occurred! No data... \n\n");
        }
        else printf ("The number of ready file descriptors (in the FD array) : %d\n", someThingToRead);

        printf("\n#Start processing\n\n");

        //after add to set, it mark as have something to read while actual not
        connfd = -1;

        // NOTE: the listenfd or connfd are both descriptor
        // select() check if a descriptor have an activity to exec or not. Not only use for listenfd or connfd

        // check the status of listenfd
        // if there is a client want to connect through listenfd socket the select() function will mark listenfd in readfds as 1 - have an activity



        // Neu sau vai lan select ma khong nhan duoc tin hieu => client da ngat ket noi






        if (FD_ISSET (listenfd, &readfds))
        {
            if(clientNum >= MAX_CLIENT)
            {
                printf("Have reached max client!\n");
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
                printf("New connection:\n");
                printf("\t1.Socket fd: %d\n", connfd);
                printf("\t2.Ip: %s\n", inet_ntoa(cliaddr.sin_addr));
                printf("\t3.Prot: %d\n", ntohs(cliaddr.sin_port));

                for(int i = 0; i < MAX_CLIENT; i++)
                {
                    if(clientConnfd[i] == -1)
                    {
                        clientConnfd[i] = connfd;
                        wantSelect[i] = 1;
                        connfd_IP[i].s_addr=cliaddr.sin_addr.s_addr;
                        clientNum++;
                        break;
                    }
                }

                // confirm that connection are generated
                send2(connfd, "OK");

                if(isLogedIn(inet_ntoa(cliaddr.sin_addr)) == LOGED_IN)
                {
                    printf("%s alrealdy logined - user name: %s\n", inet_ntoa(cliaddr.sin_addr), getAccountNodeByLoginedIP(inet_ntoa(cliaddr.sin_addr))->userName);
                    send2(connfd, getAccountNodeByLoginedIP(inet_ntoa(cliaddr.sin_addr))->userName);
                }
                else
                {
                    printf("IP not logined\n");
                    send2(connfd, "NEED_LOGIN");
                }
            }
        }

        // check the status of clientConnfd(s)
        for(int k = 0; k < MAX_CLIENT; k++)
        {
            //after add to set, it mark as have something to read while actual not
            if(clientConnfd[k] != -1 && wantSelect[k] == 1 && clientConnfd[k] != connfd)
            {
                if(FD_ISSET(clientConnfd[k], &readfds)) // => have something to read (select have found something to read)
                {

                    //check if printf("Too many thread...");

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
                        clientNum--;
                        perror("Nothing receive from client\n");
                        continue;
                    }

                    recvBuff[recvBytes] = '\0';
                    printf("[%s:%d]: Service num: %s\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), recvBuff);

                    int service;
                    service = atoi(recvBuff);

                    // Prepare user args for thread
                    user_thread_args* args = (user_thread_args*)malloc(sizeof(user_thread_args));
                    args->cliaddr = cliaddr;
                    args->clientConnfd = clientConnfd[k];

                    int freeThread_index = -1;
                    for(int i = 0; i < MAX_SERVICE_THREAD; i++)
                        if(service_thread_index[i] == -1)
                        {
                            freeThread_index = i;
                            break;
                        }

                    args->thread_index = freeThread_index;

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
                        wantSelect[k] = 0;
                        pthread_create(&(service_thread_id[freeThread_index]), NULL, &service_signin, (void*)args);
                        break;

                    case 4:
                        printf ("%s\n", "Service 4: Change password");

                        if(isLogedIn(inet_ntoa(cliaddr.sin_addr)) == LOGED_IN)
                        {
                            wantSelect[k] = 0;
                            pthread_create(&(service_thread_id[freeThread_index]), NULL, &service_changePass, (void*)args);
                            printf("cheking...\n");
                        }
                        else printf("Not loged in\n");
                        break;

                    case 5:
                        printf ("%s\n", "Service 5: New room");
                        if(isLogedIn(inet_ntoa(cliaddr.sin_addr)) == LOGED_IN)
                        {
                            wantSelect[k] = 0;
                            pthread_create(&(service_thread_id[freeThread_index]), NULL, &newRoom, (void*)args);
                        }
                        else printf("Not loged in\n");
                        break;

                    case 6:
                        printf ("%s\n", "Service 6: Player enter room");

                        if(isLogedIn(inet_ntoa(cliaddr.sin_addr)) == LOGED_IN)
                        {
                            wantSelect[k] = 0;
                            pthread_create(&(service_thread_id[freeThread_index]), NULL, &playerEnterRoom, (void*)args);

                        }
                        else printf("Not loged in\n");
                        break;

                    case 7:
                        printf ("%s\n", "Service 7: Sign out");

                        if(isLogedIn(inet_ntoa(cliaddr.sin_addr)) == LOGED_IN)
                        {
                            wantSelect[k] = 0;
                            pthread_create(&(service_thread_id[freeThread_index]), NULL, &service_signout, (void*)args);
                            void *val;
                            pthread_join(service_thread_id[freeThread_index], &val);

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
    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    int connfd;
    struct sockaddr_in cliaddr;
    int thread_index;

    user_thread_args *actual_args = arg;

    cliaddr = actual_args->cliaddr;
    connfd = actual_args->clientConnfd;
    thread_index = actual_args->thread_index;

    free(arg);

    // Have to stop select with this connfd because even receive in here, process in here but still selected by the main thread
    // meaning: now this connfd are belong to this thread
    int connfd_index;

    for(int k = 0; k < MAX_CLIENT; k++)
        if(clientConnfd[k] == connfd)
        {
            connfd_index = k;
            wantSelect[connfd_index] = 0;
            break;
        }

    int recvBytes;
    char recvBuff[RECV_BUFF_SIZE + 1];

    userNameType userName;
    passwordType password;

    recvBytes = recv(connfd, recvBuff, sizeof(recvBuff), 0);
    userName[recvBytes] = '\0';
    strcpy(userName, recvBuff);
    printf("[%s:%d]: User name: %s\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), userName);

    if(isExistUserName(userName) == ACCOUNT_EXIST)
    {
        printf("Account exist\n");
        send2(connfd, "O");
    }
    else
    {
        printf("Account doesn't exist\n");
        send2(connfd, "X");

        wantSelect[connfd_index] = 1;
        service_thread_index[thread_index] = -1;
        return NULL;
    }

    recvBytes = recv(connfd, recvBuff, sizeof(recvBuff), 0);
    recvBuff[recvBytes] = '\0';
    strcpy(password, recvBuff);
    printf("[%s:%d]: password: %s\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), password);

    int res = logIn (inet_ntoa(cliaddr.sin_addr), userName, password);
    char sres[10];
    tostring(sres, res);
    send2(connfd, sres);

    wantSelect[connfd_index] = 1;
    service_thread_index[thread_index] = -1;
}

void* service_changePass(void *arg)
{
    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    int connfd;
    struct sockaddr_in cliaddr;
    int thread_index;

    user_thread_args *actual_args = arg;

    cliaddr = actual_args->cliaddr;
    connfd = actual_args->clientConnfd;
    thread_index = actual_args->thread_index;

    free(arg);

    // Have to stop select with this connfd because even receive in here, process in here but still selected by the main thread
    // meaning: now this connfd are belong to this thread
    int connfd_index;

    for(int k = 0; k < MAX_CLIENT; k++)
        if(clientConnfd[k] == connfd)
        {
            connfd_index = k;
            wantSelect[connfd_index] = 0;
            break;
        }

    int recvBytes;
    char recvBuff[RECV_BUFF_SIZE + 1];

    passwordType newPassword;

    recvBytes = recv(connfd, recvBuff, sizeof(recvBuff), 0);
    recvBuff[recvBytes] = '\0';
    strcpy(newPassword, recvBuff);
    printf("New password: %s\n", newPassword);

    printf("Changing password...\n");

    pthread_mutex_lock(&client_connfd_lock);
    changePass(inet_ntoa(cliaddr.sin_addr), newPassword);
    pthread_mutex_unlock(&client_connfd_lock);

    for(int i = 0; i< MAX_CLIENT; i++)
    {
        //printf("%d\n", clientConnfd[i]);
        //printf("%d\n", connfd_IP[i].s_addr);
        if(clientConnfd[i] != -1 && clientConnfd[i] != connfd && connfd_IP[i].s_addr == cliaddr.sin_addr.s_addr)
        {
            //printf("***%d\n", clientConnfd[i]);
            send2(clientConnfd[i], "NOTIFICATION");
            send2(clientConnfd[i], "Your password have been changed!");
        }
    }

    wantSelect[connfd_index] = 1;

    storeUsername_passData();
    service_thread_index[thread_index] = -1;
}

void* service_newGame(void *arg)
{

}

void* service_gamePlayingHistory(void *arg)
{

}

void* service_signout(void *arg)
{
    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    int connfd;
    struct sockaddr_in cliaddr;
    int thread_index;

    user_thread_args *actual_args = arg;

    cliaddr = actual_args->cliaddr;
    connfd = actual_args->clientConnfd;
    thread_index = actual_args->thread_index;

    free(arg);

    // Have to stop select with this connfd because even receive in here, process in here but still selected by the main thread
    // meaning: now this connfd are belong to this thread
    int connfd_index;

    for(int k = 0; k < MAX_CLIENT; k++)
        if(clientConnfd[k] == connfd)
        {
            connfd_index = k;
            wantSelect[connfd_index] = 0;
            break;
        }

    signOut(inet_ntoa(cliaddr.sin_addr));
    printf("Client exited\n");

    wantSelect[connfd_index] = 1;
    service_thread_index[thread_index] = -1;
}
