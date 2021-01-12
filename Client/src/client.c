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

Client client;

int main(int argc, char *argv[])
{
    clearScreen();

    client.SERV_PORT = atoi(argv[2]);
    strcpy(client.SERV_ADDR, argv[1]);

    char recvMess[RECV_MESS_SIZE];

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

    //Check connection
    printf("Check connection...\n");
    getMessage(MESSAGE, recvMess);
    printf("From server: %s\n", recvMess);
    printf("-----------------------------\n\n");
    holdScreen();

    //TODO: read form file
    send_message(CLIENT_MESSAGE, "hust;d123");
    getMessage(MESSAGE, recvMess);
    if(strcmp(recvMess, "NEED_LOGIN") == 0)
    {
        printf("You did not loged in!\nPlease login or create a new account before playing!\n\n\n");
        client.logedIn = 0;
    }
    else
    {
        printf("Welcome back! %s\n", recvMess);
        client.logedIn = 1;
    }
    holdScreen();

    client.isHost = -1;

    int programTerminate = -1;
    char choice;
    char service[5];
    do
    {
        Args args;
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
            if(client.logedIn != 1)
            {
                printf("You have not logged in yet\n");
                holdScreen();
                break;
            }

            changePass();
            break;

        case '4':
            printf("NEW ROOM\n\n");
            if(client.logedIn != 1)
            {
                printf("You have not logged in yet\n");
                holdScreen();
                break;
            }

            newRoom();
            break;

        case '5':
            printf("JOIN ROOM\n\n");
            if(client.logedIn != 1)
            {
                printf("You have not logged in yet\n");
                holdScreen();
                break;
            }

            joinRoom();
            break;

        case '6':
            printf("PLAYING HISTORY\n\n");
            if(client.logedIn != 1)
            {
                printf("You have not logged in yet\n");
                holdScreen();
                break;
            }
            break;

        //TODO: playing history

        case '7':
            printf("SIGN OUT\n\n");
            if(client.logedIn != 1)
            {
                printf("You have not logged in yet\n");
                holdScreen();
                break;
            }

            strcpy(service, "7");

            waitMessage(MESSAGE, "SIGN_OUT_SUCCESSFULLY");
            printf("Signed out successfully\n");

            client.logedIn = -1;
            holdScreen();
            break;

        case '8':
            programTerminate = 1;
            break;
        }
    }
    while(programTerminate != 1);

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
    send_message(SERVICE, "1");

    userNameType userName;
    passwordType password;

    printf("User name: ");
    scanf("%s", userName);
    getchar();
    send_message(CLIENT_MESSAGE, userName);

    char message[RECV_MESS_SIZE];
    getMessage(MESSAGE, message);
    if(strcmp(message, "X") == 0)
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
    send_message(CLIENT_MESSAGE, password);

    waitMessage(MESSAGE, "SIGNUP_SUCCESSFULLY");
    printf("-----------------------------\n\n");
    printf("Signed up successfully\n");
    printf("-----------------------------\n");
    holdScreen();
}

void login ()
{
    send_message(SERVICE, "3");

    char message[RECV_MESS_SIZE];

    userNameType userName;
    passwordType password;

    printf("User name: ");
    scanf("%s", userName);
    getchar();
    send_message(CLIENT_MESSAGE, userName);

    getMessage(MESSAGE, message);
    if(strcmp(message, "X") == 0)
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
    send_message(CLIENT_MESSAGE, password);

    int res;

    getMessage(MESSAGE, message);
    res = atoi(message);

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
    send_message(CLIENT_MESSAGE, "4");

    passwordType newPassword;
    printf("New password: ");
    scanf("%s", newPassword);
    getchar();

    send_message(CLIENT_MESSAGE, newPassword);
    holdScreen();
}

void newRoom()
{
    send_message(SERVICE, "5");

    char message[RECV_MESS_SIZE];

    char roomName[255];
    printf("Room name: ");
    scanf("%s", roomName);
    getchar();

    send_message(CLIENT_MESSAGE, roomName);

    char playerNumPreSet[3];

    do
    {
        printf("playerNumPreSet (min: 2 - max: 3): ");
        fgets(playerNumPreSet, sizeof(playerNumPreSet), stdin);
        playerNumPreSet[strlen(playerNumPreSet) - 1] = '\0';// delete '\n'
    }
    while(strlen(playerNumPreSet) != 1
            || playerNumPreSet[0] > '3'
            || playerNumPreSet[0] < '2');

    send_message(CLIENT_MESSAGE, playerNumPreSet);

    getMessage(MESSAGE, message);
    int roomID = atoi(message);
    printf("Room ID: %d\n", roomID);
    holdScreen();

    client.isHost = 1;
    Args args;
    args.int1 = &roomID;

    void* val;
    pthread_t tmp_threadID;
    pthread_create(&tmp_threadID, NULL, &gameRoom, (void*)&args);
    pthread_join(tmp_threadID, &val);

    holdScreen();
}

void joinRoom()
{
    send_message(SERVICE, "6");

    char message[RECV_MESS_SIZE];

    getMessage(MESSAGE, message);
    if(strcmp(message, "NO_ROOM") == 0)
    {
        puts(message);
        holdScreen();
        return;
    }

    printf("Room num: %s\n\n", message);
    printf("=============\n\n");
    printf("Room list:\n\n");

    do
    {
        getMessage(MESSAGE, message);
        printf("Room ID: %s\n", message);
        getMessage(MESSAGE, message);
        printf("Room name: %s\n", message);
        getMessage(MESSAGE, message);
        printf("Num of player preset: %s\n", message);
        getMessage(MESSAGE, message);
        printf("Num of player: %s\n", message);
        printf("\n");
    }
    while(checkMessage_waitRecv(MESSAGE, "PRINT_ROOM_END") != 1);

    char chosenRoom_char[5];
    do
    {
        printf("Enter room ID: ");
        fgets(chosenRoom_char, sizeof(chosenRoom_char), stdin);
        chosenRoom_char[strlen(chosenRoom_char) - 1] = '\0';

        send_message(CLIENT_MESSAGE, chosenRoom_char);

        getMessage(MESSAGE, message);

        if(strcmp(message, "ROOM_FULL") == 0)
        {
            printf("Room full!\nPlease choose another room\n");
            continue;
        }

        if(strcmp(message, "ROOM_NOT_FOUND") == 0)
        {
            printf("Room not found!\nPlease choose another room\n");
            continue;
        }
    }
    while(strcmp(message, "ENTER_ROOM_SUCCESSFULY") != 0);

    clearScreen();

    Args args;
    int chosenRoom = atoi(chosenRoom_char);
    args.int1 = &chosenRoom;

    void* val;
    pthread_t tmp_threadID;
    pthread_create(&tmp_threadID, NULL, &gameRoom, (void*)&args);
    pthread_join(tmp_threadID, &val);

    holdScreen();
}

void* gameRoom(void* args)
{
    pthread_detach(pthread_self());

    int oldstate;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
    int oldtype;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);

    Args* actual_args = args;

    int roomID;
    roomID = *(actual_args->int1);

    char message[RECV_MESS_SIZE];

    int initialState = client.isHost;
    printRoomChatOpt();
    while(1)
    {
        char chatMessage[100];
        int ready = -1;

        printf("Write something to chat...\n\n");

        while(ready != 1)
        {
            if(initialState != 1 && client.isHost == 1)
            {
                initialState = 1;
                printRoomChatOpt();
            }
            do{
            fgets(chatMessage, 500, stdin);
            }while(chatMessage[0] == '\n');
            chatMessage[strlen(chatMessage) - 1] = '\0';

            if(strcmp(chatMessage, "$READY") == 0)
            {
            send_message(CLIENT_MESSAGE, chatMessage);
            waitMessage(GAME_CONTROL_DATA, "READY_SUCCESSFULLY");
                ready = 1;
                continue;
            }

            if(checkMessage(GAME_CONTROL_DATA, "KICKED") == 1
                    || checkMessage(GAME_CONTROL_DATA, "OUT_ROOM") == 1)
            {
                printf("Out room\n");
                pthread_cancel(pthread_self());
            } else {
                printf("> you: %s\n", chatMessage);
                send_message(CLIENT_MESSAGE, chatMessage);
            };
        }

        pthread_t tmp_threadID;
        pthread_create(&tmp_threadID, NULL, &gamePlay, (void*)args);
        pthread_join(tmp_threadID, NULL);

        holdScreen();
    }
}

void printRoomChatOpt()
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

    if(client.isHost != 1)
    {
        printf("-----------------------------\n");
        printf("Your are in room\nKEYWORD\n\n");
        printf("+) Invite:  $INVITE<tab><userName>\n");
        printf("+) Ready:    $READY\n");
        printf("+) Out room: $QUIT\n"); // new host
        printf("-----------------------------\n\n");
    }
}

void* gamePlay(void* args)
{
    pthread_detach(pthread_self());

    int oldstate;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
    int oldtype;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);

    client.gamePlayThreadId = pthread_self();

    Args* actual_args = args;
    int roomID;
    roomID = *(actual_args->int1);

    char message[RECV_MESS_SIZE];

    waitMessage(MESSAGE, "GAME_START");

    clearScreen();
    printf("\n\n---------------------------------------------\n");
    printf("Game start!\n");
    printf("---------------------------------------------\n\n");

    int quesNum = 0;
    char question[100];
    char ans[20];
    int ansLen;

    do
    {
        getMessage(GAME_CONTROL_DATA, question);
        quesNum++;

        getMessage(GAME_CONTROL_DATA, message);
        ansLen = atoi(message);
        int i;
        for(i = 0; i < ansLen; i++)
        {
            ans[i] = '_';
        }
        ans[i] = '\0';

        printf("Question %d: %s\n\n", quesNum, question);
        printf("Number of character of the answer: %d\n", ansLen);
        printf("%s\n\n", ans);

        soldQuestion(quesNum, question, ans, ansLen);
    }
    while(checkMessage_waitRecv(GAME_CONTROL_DATA, "END_GAME") != 1);

    printf("\n\n---------------------------------------------\n");
    printf("End game!\n");
    printf("---------------------------------------------\n\n");
    holdScreen();
}

void soldQuestion(int quesNum, char* question, char* ans, int ansLen)
{
    char message[RECV_MESS_SIZE];

    int quesSolved = -1;
    do
    {
        if(checkMessage_waitRecv(GAME_CONTROL_DATA, "GAME_BREAK") ==1 )
        {
            printf("game break...\n");
        }

        if(checkMessage(GAME_CONTROL_DATA, "QUES_SOLVED") == 1 )
        {
            quesSolved = 1;
            continue;
        }

        if(checkMessage(GAME_CONTROL_DATA, "NEXT_ROUND") == 1 )
        {
            // get current answer state
            getMessage(GAME_CONTROL_DATA, message);
            strcpy(ans, message);
            printf("Next round!\n\n");
            delay(3);
            clearScreen();

            printf("Question %d: %s\n\n", quesNum, question);
            printf("Number of character of the answer: %d\n", ansLen);
            printf("%s\n\n", ans);
            continue;
        }

        waitMessage(GAME_CONTROL_DATA, "YOUR_TURN");
        printf("It's your turn\n");

        int timeOut;
        timeOut = 0;

        // TODO: show the wheel's result

        getMessage(GAME_CONTROL_DATA, message);
        int timeOutS = atoi(message);

        printf("You have %d s to give your answer\n\n", timeOutS);

        char choice_answer[100];
        int res = getAnswer(choice_answer, timeOutS);
        if(res != 1)
        {
            printf("Time out\n");
        }
        else if(res == 1)
        {
            send_message(CLIENT_MESSAGE, choice_answer);
        }
    }
    while(quesSolved != 1);

    printf("Question is solved!\n\n");
    printf("---------------------------------------------------\n");
    delay(3);
    clearScreen();
}

int getAnswer(char* choice_answer, int timeOutS)
{
    char choice[2];
    char playerAnswer[20];
    char fromKeyBoard[30];

    printf("OPTION:\n");
    printf("1. Solve the question\n");
    printf("2. Guess character\n");

    int tabCount = 0;
    int timeOut;
    do
    {
        printf("Enter your choice (choice<tab>answer): \n");
        timeOut = fgets_timeout (fromKeyBoard, sizeof(fromKeyBoard), timeOutS);

        if(timeOut == -1)
        {
            break;
        }

        fromKeyBoard[strlen(fromKeyBoard) - 1] = '\0';
        for(int i = 0; i < strlen(fromKeyBoard); i++)
            if(fromKeyBoard[i] == '\t')
            {
                tabCount = 1;
            }

        if(tabCount == 0) continue;

        char* savePtr;
        char* strTmp;
        strTmp = strtok_r(fromKeyBoard, "\t", &savePtr);
        strcpy(choice, strTmp);
        strTmp = strtok_r(NULL, "\t", &savePtr);
        strcpy(playerAnswer, strTmp);
    }
    while(tabCount == 0
            || strlen(choice) != 1
            || (choice[0] != '1' && choice[0] != '2')
            || (choice[0] == '2' && strlen(playerAnswer) != 1 ));

    strcpy(choice_answer, choice);
    strcat(choice_answer, "-");
    strcat(choice_answer, playerAnswer);
}
