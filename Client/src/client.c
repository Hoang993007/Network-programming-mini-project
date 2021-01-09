// To candle thread, the thread must be running

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

#include "../inc/errorCode.h"
#include "../inc/client.h"
#include "../inc/network.h"
#include "../inc/otherFunction.h"

#define MAX_USERNAME_LENGTH 50
#define MAX_PASS_LENGTH 20

#define ROOM_QUEST_NUM 2

Client client;

int main(int argc, char *argv[])
{
    clearScreen();
    client.SERV_PORT = atoi(argv[2]);
    strcpy(client.SERV_ADDR, argv[1]);

    char recvBuff[RECV_BUFF_SIZE];

    //Step 1: Construct socket
    printf("%s\n", "Constructing soket...");
    if((client.sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error: Problem in creating the socket");
        exit(2);
    }

    //Step 2: Create the remote server socket info structure
    printf("%s\n", "Creating remote server socket info structure...");
    bzero(&(client.servaddr), sizeof(client.servaddr));
    client.servaddr.sin_family = AF_INET;
    client.servaddr.sin_addr.s_addr = inet_addr(client.SERV_ADDR);
    client.servaddr.sin_port = htons(client.SERV_PORT);// convert to big-edian order

    // connect to server socket
    printf("%s\n", "Connecting the the server socket...");
    if(connect(client.sockfd, (struct sockaddr *)&(client.servaddr), sizeof(client.servaddr)) < 0)
    {
        printf("\n Error : Connecting to the server failed \n");
        exit(3);
    }

    pthread_t recv_messageThreadId;
    pthread_create(&recv_messageThreadId, NULL, &recv_message, NULL);

    //Step 3: Communicate with server
    getMessage(MESSAGE, recvBuff);
    printf("From server: %s\n", recvBuff);
    printf("-----------------------------\n\n");
    holdScreen();

    send(client.sockfd, "hust;d123", sizeof("hust;123"), 0);
    getMessage(MESSAGE, recvBuff);
    if(strcmp(recvBuff, "NEED_LOGIN") == 0)
    {
        printf("You did not loged in!\nPlease login or create a new account before playing!\n\n\n");
        client.logedIn = 0;
    }
    else
    {
        printf("Welcome back! %s\n", recvBuff);
        client.logedIn = 1;
    }
    holdScreen();

    client.isHost = 0;

    int programTerminate = 0;
    char choice;
    char service[5];
    do
    {
        Args args;
        pthread_t gameRoomThreadID;
        void *val;
        choice = getMenuChoice();
        clearScreen();

        switch(choice)
        {
        case '1':
            printf("LOGIN\n\n");
            login ();
            break;

        case '2':
            printf("SIGN UP\n\n");
            signUp();
            break;

        case '3':
            printf("CHANGE PASSWORD\n\n");
            if(client.logedIn == 0)
            {
                printf("You have not logged in yet\n");
                holdScreen();
                break;
            }
            changePass();
            break;

        case '4':
            printf("NEW ROOM\n\n");
            if(client.logedIn == 0)
            {
                printf("You have not logged in yet\n");
                holdScreen();
                break;
            }

            strcpy(service, "5");
            send(client.sockfd, service, sizeof(service), 0);

            char roomName[255];
            printf("Room name: ");
            scanf("%s", roomName);
            getchar();

            send(client.sockfd, roomName, sizeof(roomName), 0);

            char playerNumPreSet[100];

            do
            {
                printf("playerNumPreSet (min: 2 - max: 3): ");
                fgets(playerNumPreSet, sizeof(playerNumPreSet), stdin);
                playerNumPreSet[strlen(playerNumPreSet) - 1] = '\0';// delete '\n'
            }
            while(strlen(playerNumPreSet) != 1
                    || playerNumPreSet[0] > '3'
                    || playerNumPreSet[0] < '2');

            send(client.sockfd, playerNumPreSet, sizeof(playerNumPreSet), 0);

            getMessage(MESSAGE, recvBuff);
            int recvBuffInt = atoi(recvBuff);
            printf("Room ID: %d\n", recvBuffInt);
            holdScreen();

            client.isHost = 0;
            args.int1 = &recvBuffInt;

            pthread_create(&gameRoomThreadID, NULL, &gameRoom, (void*)&args);
            pthread_join(gameRoomThreadID, &val);
            break;

        case '5':
            printf("JOIN ROOM\n\n");
            if(client.logedIn == 0)
            {
                printf("You have not logged in yet\n");
                holdScreen();
                break;
            }

            strcpy(service, "6");
            send(client.sockfd, service, sizeof(service), 0);
            getMessage(MESSAGE, recvBuff);
            if(strcmp(recvBuff, "NO_ROOM") == 0)
            {
                puts(recvBuff);
                holdScreen();
                break;
            }

            printf("Room num: %s\n\n", recvBuff);
            printf("=============\n\n");
            printf("Room list:\n\n");

            while(1)
            {
                getMessage(MESSAGE, recvBuff);
                if(strcmp(recvBuff, "PRINT_ROOM_END") == 0)
                    break;

                printf("Room ID: %s\n", recvBuff);
                getMessage(MESSAGE, recvBuff);
                printf("Room name: %s\n", recvBuff);
                getMessage(MESSAGE, recvBuff);
                printf("Num of player preset: %s\n", recvBuff);
                getMessage(MESSAGE, recvBuff);
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
            send(client.sockfd, chosenRoom_char, sizeof(chosenRoom_char), 0);

            getMessage(MESSAGE, recvBuff);
            puts(recvBuff);

            clearScreen();

            client.isHost = 0;
            args.int1 = &chosenRoom;

            pthread_create(&gameRoomThreadID, NULL, &gameRoom, (void*)&args);
            pthread_join(gameRoomThreadID, &val);

            break;

        case '6':
            printf("PLAYING HISTORY\n\n");
            if(client.logedIn == 0)
            {
                printf("You have not logged in yet\n");
                holdScreen();
                break;
            }
            break;

        case '7':
            printf("SIGN OUT\n\n");
            if(client.logedIn == 0)
            {
                printf("You have not logged in yet\n");
                holdScreen();
                break;
            }
            strcpy(service, "7");
            send(client.sockfd, service, sizeof(service), 0);
            printf("Signed out successfully\n");
            client.logedIn = 0;
            holdScreen();
            break;

        case '8':
            programTerminate = 1;
            break;
        }
    }
    while(programTerminate != 1);

// close the descriptor
    close(client.sockfd);
}

char getMenuChoice()
{
    char choice[100];

    clearScreen();
    printf("-----------------------------\n");
    printf("OPTION\n\n");

//  1. Log in
//  2. Sign up
//  3. Change pass
//  4. New room
//  5. Join room
//  6. Playing history
//  7. Sign out

    if(client.logedIn == 0)
    {
        printf("1. Log in\n");
        printf("2. Sign up\n");
        printf("3. New room\n");
        printf("4. Join room\n");
        printf("5. Playing history\n");
        printf("\n");

        do
        {
            printf("Enter your choice: ");
            fgets(choice, 100, stdin);
            choice[strlen(choice) - 1] = '\0';
        }
        while(strlen(choice) != 1 || (choice[0] > '6' || choice[0] < '1'));
        if(choice[0] >= '3')
            choice[0] = choice[0] + 1;

    }
    else if(client.logedIn == 1)
    {
        printf("1. Change pass\n");
        printf("2. New room\n");
        printf("3. Join room\n");
        printf("4. Playing history\n");
        printf("5. Sign out\n");
        printf("\n");

        do
        {
            printf("Enter your choice: ");
            fgets(choice, 100, stdin);
            choice[strlen(choice) - 1] = '\0';
        }
        while(strlen(choice) != 1 || (choice[0] > '6' || choice[0] < '1'));

        choice[0] = choice[0] + 2;
    }

    return choice[0];
}

void signUp ()
{
    char service[5];
    strcpy(service, "1");
    send(client.sockfd, service, sizeof(service), 0);

    userNameType userName;
    passwordType password;

    printf("User name: ");
    scanf("%s", userName);
    getchar();
    send(client.sockfd, userName, sizeof(userName), 0);


    char recvBuff[RECV_BUFF_SIZE];
    while(messageReady[MESSAGE] == 0);
    getMessage(MESSAGE, recvBuff);
    if(strcmp(recvBuff, "X") == 0)
    {
        printf("-----------------------------\n\n");
        printf("Register failed\n");
        printf("-----------------------------\n");
        holdScreen();
        return;
    }

    printf("Insert password: ");
    scanf("%s", password);
    getchar();
    send(client.sockfd, password, sizeof(password), 0);


    printf("-----------------------------\n\n");
    printf("Signed up successfully\n");
    printf("-----------------------------\n");
    holdScreen();
}

void login ()
{
    char recvBuff[RECV_BUFF_SIZE];
    char service[5];
    // service 3: login
    strcpy(service, "3");
    send(client.sockfd, service, sizeof(service), 0);

    userNameType userName;
    passwordType password;

    printf("User name: ");
    scanf("%s", userName);
    getchar();
    send(client.sockfd, userName, sizeof(userName), 0);
    getMessage(MESSAGE, recvBuff);
    if(strcmp(recvBuff, "X") == 0)
    {
        printf("-----------------------------\n\n");
        printf("Wrong account\n");
        printf("-----------------------------\n");
        holdScreen();
        return;
    }

    printf("Insert password: ");
    scanf("%s", password);
    getchar();
    send(client.sockfd, password, sizeof(password), 0);

    int res;

    getMessage(MESSAGE, recvBuff);
    res = atoi(recvBuff);
    //printf("resCode: %d\n", res);

    if(res == LOGIN_SUCCESS)
    {
        printf("Log in successfully\n\n");
        client.logedIn = 1;
        printf("-----------------------------\n");
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
    holdScreen();
}

void changePass ()
{
    char service[5];
    strcpy(service, "4");
    send(client.sockfd, service, sizeof(service), 0);

    passwordType newPassword;
    printf("New password: ");
    scanf("%s", newPassword);
    getchar();

    send(client.sockfd, newPassword, sizeof(newPassword), 0);
    holdScreen();
}

void* gameRoom(void* args)
{
    Args* actual_args = args;

    int roomID;

    roomID = *(actual_args->int1);

    char recvBuff[RECV_BUFF_SIZE];
    while(1)
    {

        if(client.isHost == 1)
        {
            printf("-----------------------------\n");
            printf("Your are now the host of the room\nKEYWORD\n\n");
            printf("+) Invite:  $INVITE<tab><userName>\n");
            printf("+) Kick:    $KICK<tab><userName>\n");
            printf("+) Ready:    $READY\n");
            printf("+) Out room: $QUIT<tab><userName>\n"); // new host
            printf("-----------------------------\n\n");
        }

        if(client.isHost == 0)
        {
            printf("-----------------------------\n");
            printf("Your are in room\nKEYWORD\n\n");
            printf("+) Invite:  $INVITE<tab><userName>\n");
            printf("+) Ready:    $READY\n");
            printf("+) Out room: $QUIT\n"); // new host
            printf("-----------------------------\n\n");
        }

        char chatMessage[500];
        int ready = 0;
        printf("Write something to chat...\n\n");

        do
        {
            fgets(chatMessage, 500, stdin);

            if((strcmp(recvMessage[GAME_CONTROL_DATA], "KICKED") == 0)
                    || (strcmp(recvMessage[GAME_CONTROL_DATA], "OUT_ROOM") == 0))
            {
                getMessage(GAME_CONTROL_DATA, recvBuff);
                pthread_cancel(pthread_self());
                printf("Out room\n");
            };
            chatMessage[strlen(chatMessage) - 1] = '\0';
            printf("> you: %s\n", chatMessage);
            if(strcmp(chatMessage, "$READY") == 0) ready = 1;
            send(client.sockfd, chatMessage, sizeof(chatMessage), 0);
        }
        while(ready == 0);

        pthread_t gamePlayThreadId;
        pthread_create(&(gamePlayThreadId), NULL, &gamePlay, (void*)args);
        pthread_join(gamePlayThreadId, NULL);

        //printf("Please wait for 5 second to make game play thread completely end...\n");
        //delay(5);

        holdScreen();
    }
}

void* gamePlay(void* args)
{
//printf("Game thread ID: %ld\n", pthread_self());

    client.gamePlayThreadId = pthread_self();

    Args* actual_args = args;

    int roomID;

    roomID = *(actual_args->int1);

    char recvBuff[RECV_BUFF_SIZE];

    while(strcmp(recvMessage[1], "GAME_START") != 0);
    getMessage(MESSAGE, recvBuff);

    clearScreen();
    printf("\n\n---------------------------------------------\n");
    printf("Game start!\n");
    printf("---------------------------------------------\n\n");

    int quesNum = 0;
    char question[100];
    int quitGame = 0;
    do
    {
        getMessage(GAME_CONTROL_DATA, recvBuff);
        quesNum++;

        if(strcmp(recvBuff, "END_GAME") == 0)
            break;

        strcpy(question, recvBuff);

        getMessage(GAME_CONTROL_DATA, recvBuff);
        int ansLen = atoi(recvBuff);

        while(strcmp(recvMessage[GAME_CONTROL_DATA], "QUES_SOLVED") != 0)
        {
            do
            {
                printf("Question %d: %s\n\n", quesNum, question);
                printf("Number of character of the answer: %d\n", ansLen);

                //printf("###waiting turn...\n");
                while(messageReady[GAME_CONTROL_DATA] != 1
                        || (strcmp(recvMessage[GAME_CONTROL_DATA], "YOUR_TURN") != 0
                            && strcmp(recvMessage[GAME_CONTROL_DATA], "GAME_BREAK") != 0
                            && strcmp(recvMessage[GAME_CONTROL_DATA], "QUES_SOLVED") != 0
                            && strcmp(recvMessage[GAME_CONTROL_DATA], "NEXT_ROUND") != 0));

                if(strcmp(recvMessage[GAME_CONTROL_DATA], "QUES_SOLVED") == 0)
                    break;

                if(strcmp(recvMessage[GAME_CONTROL_DATA], "NEXT_ROUND") == 0)
                    break;

                if(strcmp(recvMessage[GAME_CONTROL_DATA], "GAME_BREAK") == 0) {
                    getMessage(GAME_CONTROL_DATA, recvBuff);
                    printf("game break...\n");
                    }

                int timeOut;
                timeOut = 0;

                getMessage(GAME_CONTROL_DATA, recvBuff);
                printf("It's your turn\n");

                // TODO: show the wheel's result

                getMessage(GAME_CONTROL_DATA, recvBuff);
                int timeOutS = atoi(recvBuff);

                printf("You have %d s to give your answer\n\n", timeOutS);

                char choice_answer[100];
                char choice[2];
                char playerAnswer[50];
                printf("OPTION:\n");
                printf("1. Solve the question\n");
                printf("2. Guess character\n");
int tabCount = 0;
                do
                {

                    printf("Enter your choice (choice<tab>answer): \n");

                    timeOut = fgets_timeout (choice_answer, sizeof(choice_answer), timeOutS);
                    if(timeOut == -1)
                        break;
                    choice_answer[strlen(choice_answer) - 1] = '\0';
                    for(int i = 0; i < strlen(choice_answer); i++)
                    if(choice_answer[i] == '\t') tabCount = 1;

                    if(tabCount == 0) continue;

                    char* strTmp;
                    strTmp = strtok(choice_answer, "\t");
                    strcpy(choice, strTmp);
                    strTmp = strtok(NULL, "\t");
                    strcpy(playerAnswer, strTmp);
                }
                while(tabCount == 0
                || strlen(choice) != 1
                || (choice[0] != '1' && choice[0] != '2')
                || (choice[0] == '2' && strlen(playerAnswer) != 1 ));

                if(timeOut == -1)
                {
                    printf("Time out\n");
                }
                else
                {

                    char choiceInTurn[200];
                    strcpy(choiceInTurn, choice);
                    strcat(choiceInTurn, "-");
                    strcat(choiceInTurn, playerAnswer);

                        send(client.sockfd, choiceInTurn, sizeof(choiceInTurn), 0);
                }
                //clearScreen();
//printf("###end turn\n");
                while(messageReady[GAME_CONTROL_DATA] == 0 || (strcmp(recvMessage[GAME_CONTROL_DATA], "NEXT_ROUND") != 0
                        && strcmp(recvMessage[GAME_CONTROL_DATA], "QUES_SOLVED") != 0));

                //printf("###next round\n");
            }
            while(messageReady[GAME_CONTROL_DATA] == 0 || (strcmp(recvMessage[GAME_CONTROL_DATA], "NEXT_ROUND") != 0
                    && strcmp(recvMessage[GAME_CONTROL_DATA], "QUES_SOLVED") != 0));


            if(strcmp(recvMessage[GAME_CONTROL_DATA], "NEXT_ROUND") == 0)
            {
                printf("Next round!\n\n");
                getMessage(GAME_CONTROL_DATA, recvBuff);
            }
        }
        if(strcmp(recvMessage[GAME_CONTROL_DATA], "QUES_SOLVED") == 0)
        {
            printf("Question is solved!\n\n");
            getMessage(GAME_CONTROL_DATA, recvBuff);// Read QUES_SOLVED
            printf("---------------------------------------------------\n");
        }

    }
    while(strcmp(recvMessage[GAME_CONTROL_DATA], "END_GAME") != 0);
}
