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
//
#include <fcntl.h>
//
#include "../inc/room.h"
#include "../inc/accountSystem.h"
#include "../inc/convertNumAndStr.h"
#include "../inc/network.h"
#include "../inc/server.h"
#include "../inc/question.h"

#define RECV_BUFF_SIZE 4096

#define ROUND_END 1
#define ROUND_CONTINUE 0

pthread_mutex_t room_data_lock = PTHREAD_MUTEX_INITIALIZER;

int roomIDGenerate = 0;
int roomNum = 0;
room* roomList[MAX_ROOM];

pthread_t room_thread_id[MAX_ROOM];

void roomListInit()
{
    for(int i = 0; i < MAX_ROOM; i++)
    {
        roomList[i] = NULL;
    }
}

void* newRoom(void* args)
{
    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    int connfd_index;
    user_thread_args *actual_args = args;
    connfd_index = actual_args->clientConnfd_index;
    free(args);

    int recvBytes, sendBytes;
    char recvBuff[RECV_BUFF_SIZE];

    char roomName[ROOM_NAME_MAX_LENGTH];
    recvBytes = recv(clientConnfd[connfd_index], recvBuff, sizeof(recvBuff), 0);
    if(recvBytes == 0)
    {
        clientConnfdUnconnect(connfd_index);
        pthread_cancel(pthread_self());
    }

    recvBuff[recvBytes] = '\0';
    strcpy(roomName, recvBuff);
    printf("[%s]: Room name: %s\n", inet_ntoa(clientIP[connfd_index]), roomName);

    int playerNumPreSet;
    recvBytes = recv(clientConnfd[connfd_index], recvBuff, sizeof(recvBuff), 0);
    if(recvBytes == 0)
    {
        clientConnfdUnconnect(connfd_index);
        pthread_cancel(pthread_self());
    }

    recvBuff[recvBytes] = '\0';
    playerNumPreSet = atoi(recvBuff);
    printf("[%s]: Max player: %d\n", inet_ntoa(clientIP[connfd_index]), playerNumPreSet);

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
        newRoom->currentPlayerNum = 1;
        pthread_cond_init(&(newRoom->playerReady_cond), NULL);
        pthread_mutex_init(&(newRoom->roomLock), NULL);

        newRoom->player[0] = client_account[connfd_index];
        newRoom->playerConnfd[0] = clientConnfd[connfd_index];
        newRoom->playerReady[0] = 0;

        for(int i = 0; i < newRoom->playerNumPreSet; i++)
        {
            newRoom->playerConnfdRSBusy[i] = 0;
        }

        int pipeRes = pipe(newRoom->fd);
        if(pipeRes < 0)
        {
            perror("pipe ");
            exit(1);
        }
        for(int i = 1; i < playerNumPreSet; i++)
        {
            newRoom->player[i] = NULL;
            newRoom->playerConnfd[i] = -1;
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

        char roomIDStr[5];
        tostring(roomIDStr, newRoom->roomID);
        sendBytes = send_message(clientConnfd[connfd_index], MESSAGE, roomIDStr);
        if(sendBytes == 0)
        {
            clientConnfdUnconnect(connfd_index);
            pthread_cancel(pthread_self());
        }

        pthread_t tmp;
        pthread_create(&tmp, NULL, &roomChat, (void*)(&(newRoom->roomID)));
    }
    else
    {
        printf("No more room can created!\n");
        connfdNoServiceRunning[connfd_index] = 1;
    }
}

void* playerEnterRoom (void* args)
{
    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    int connfd_index;

    user_thread_args *actual_args = args;

    connfd_index = actual_args->clientConnfd_index;

    free(args);

    int recvBytes, sendBytes;
    char recvBuff[RECV_BUFF_SIZE + 1];
    char sendBuff[SEND_BUFF_SIZE + 1];

    if(roomNum == 0)
    {
        sendBytes = send_message(clientConnfd[connfd_index], MESSAGE, "NO_ROOM");
        if(sendBytes == 0)
        {
            clientConnfdUnconnect(connfd_index);
            pthread_cancel(pthread_self());
        }

        connfdNoServiceRunning[connfd_index] = 1;
        return NULL;
    }

    int curRoomNum = roomNum;

    tostring(sendBuff, curRoomNum);
    sendBytes = send_message(clientConnfd[connfd_index], MESSAGE, sendBuff);
    if(sendBytes == 0)
    {
        clientConnfdUnconnect(connfd_index);
        pthread_cancel(pthread_self());
    }

    for(int i = 0; i < MAX_ROOM; i++)
    {
        if(roomList[i] != NULL)
        {
            char sendNum[5];

            tostring(sendNum, roomList[i]->roomID);
            sendBytes = send_message(clientConnfd[connfd_index], MESSAGE, sendNum);
            if(sendBytes == 0)
            {
                clientConnfdUnconnect(connfd_index);
                pthread_cancel(pthread_self());
            }
            sendBytes = send_message(clientConnfd[connfd_index], MESSAGE, roomList[i]->roomName);
            if(sendBytes == 0)
            {
                clientConnfdUnconnect(connfd_index);
                pthread_cancel(pthread_self());
            }
            tostring(sendNum, roomList[i]->playerNumPreSet);
            sendBytes = send_message(clientConnfd[connfd_index], MESSAGE, sendNum);
            if(sendBytes == 0)
            {
                clientConnfdUnconnect(connfd_index);
                pthread_cancel(pthread_self());
            }
            tostring(sendNum, roomList[i]->currentPlayerNum);
            sendBytes = send_message(clientConnfd[connfd_index], MESSAGE, sendNum);
            if(sendBytes == 0)
            {
                clientConnfdUnconnect(connfd_index);
                pthread_cancel(pthread_self());
            }
        }
    }

    sendBytes = send_message(clientConnfd[connfd_index], MESSAGE, "PRINT_ROOM_END");
    if(sendBytes == 0)
    {
        clientConnfdUnconnect(connfd_index);
        pthread_cancel(pthread_self());
    }

    recvBytes = recv(clientConnfd[connfd_index], recvBuff, sizeof(recvBuff), 0);
    if(recvBytes == 0)
    {
        clientConnfdUnconnect(connfd_index);
        pthread_cancel(pthread_self());
    }

    recvBuff[recvBytes] = '\0';
    int chosenRoomID = atoi(recvBuff);
    printf("[%s]: Chosen room ID: %d\n", inet_ntoa(clientIP[connfd_index]), chosenRoomID);

    pthread_mutex_lock(&room_data_lock);
    printf("start entering...\n");

    int i;
    int enterSuccess = 0;
    for(i = 0; i < MAX_ROOM; i++)
    {
        if(roomList[i] != NULL && roomList[i]->roomID == chosenRoomID)
        {
            if(roomList[i]->currentPlayerNum < roomList[i]->playerNumPreSet)
            {
                for(int j = 0; j < roomList[i]->playerNumPreSet; j++)
                {
                    if(roomList[i]->player[j] == NULL)
                    {
                        roomList[i]->player[j] = client_account[connfd_index];
                        roomList[i]->playerConnfd[j] = clientConnfd[connfd_index];
                        roomList[i]->playerReady[j] = 0;
                        roomList[i]->currentPlayerNum++;

                        char index_str[5];
                        char newMemberMessage[100];
                        strcpy(newMemberMessage, "NEW_MEMBER-");
                        tostring(index_str, j);
                        strcat(newMemberMessage, index_str);
                        int resWrite = write(roomList[i]->fd[1], newMemberMessage, sizeof(newMemberMessage));
                        if(resWrite < 0)
                        {
                            perror ("write");
                            exit (2);
                        }

                        sendBytes = send_message(clientConnfd[connfd_index], MESSAGE, "ENTER_ROOM_SUCCESSFULY");
                        if(sendBytes == 0)
                        {
                            clientConnfdUnconnect(connfd_index);
                            pthread_cancel(pthread_self());
                        }
                        enterSuccess = 1;
                        printRoom();
                        break;
                    };
                }
                if(enterSuccess == 1) break;
            }
            else
            {
                printf("Room full\n");
                sendBytes = send_message(clientConnfd[connfd_index], MESSAGE, "ROOM_FULL");
                if(sendBytes == 0)
                {
                    clientConnfdUnconnect(connfd_index);
                    pthread_cancel(pthread_self());
                }
            }
            break;
        }
    }

    if(i == MAX_ROOM)
    {
        sendBytes = send_message(clientConnfd[connfd_index], MESSAGE, "ROOM_NOT_FOUND");
        if(sendBytes == 0)
        {
            clientConnfdUnconnect(connfd_index);
            pthread_cancel(pthread_self());
        }
        connfdNoServiceRunning[connfd_index] = 1;
    }

    pthread_mutex_unlock(&room_data_lock);

}

room* getRoomByID(int ID)
{
    for(int i = 0; i < MAX_ROOM; i++)
    {
        if(roomList[i]->roomID == ID)
            return roomList[i];
    }
}

void* roomChat (void* args)
{
    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    int roomID = *((int*)args);

    printf("Starting room...\nRoom chat...\n\n");
    room* currentRoom = getRoomByID(roomID);
    currentRoom->room_thread_id = pthread_self();
    printf("%ld\n", pthread_self());

    char recvBuff[RECV_BUFF_SIZE + 1];
    char sendBuff[SEND_BUFF_SIZE + 1];

    printOneRoom(roomID);

    for(int j = 0; j < currentRoom->playerNumPreSet - 1; j++)
    {
        if(currentRoom->player[j] != NULL)
        {
            printf("Player name: %s\n", currentRoom->player[j]->userName);
        }
        else printf("Player name: waiting...\n");
    }

    int recvBytes, sendBytes;
    int max_readfd;
    fd_set readfds;

    int check = 0;
    do
    {
        while(currentRoom->currentPlayerNum != currentRoom->playerNumPreSet
                || numOfReadyPlayer(currentRoom) != currentRoom->playerNumPreSet)
        {
            // PREPARE FOR SELECTING
            FD_ZERO(&(currentRoom->readfds));
            for(int i = 0; i < currentRoom->playerNumPreSet; i++)
            {
                if(currentRoom->playerConnfd[i] != -1 && currentRoom->playerConnfdRSBusy[i] == 0)
                {
                    FD_SET(currentRoom->playerConnfd[i], &(currentRoom->readfds));
                    if(currentRoom->playerConnfd[i] > max_readfd)
                        max_readfd = currentRoom->playerConnfd[i];
                }
            }
            FD_SET(currentRoom->fd[0], &(currentRoom->readfds));
            if(currentRoom->fd[0] > max_readfd)
            {
                max_readfd = currentRoom->fd[0]; // REALLY IMPORTANT
            }

            int askForRecv = select(max_readfd + 1, &(currentRoom->readfds), NULL, NULL, NULL);

            if(askForRecv == -1)
            {
                perror("Error");
                exit(0);
            }

            // START TO PROCESS
            int newConnfd = -1;
            if(FD_ISSET(currentRoom->fd[0], &(currentRoom->readfds)))
            {
                recvBytes = read(currentRoom->fd[0], recvBuff, sizeof(recvBuff));
                recvBuff[recvBytes] = '\0';
                char* orderType = strtok(recvBuff, "-");

                if(strcmp(orderType, "NEW_MEMBER") == 0)
                {
                    char* indexStr = strtok(NULL, "-");
                    int newMemberIndex = atoi(indexStr);
                    newConnfd = currentRoom->playerConnfd[newMemberIndex];
                    char newMemberMessage[100];
                    strcpy(newMemberMessage, "Room have a new member name ");
                    strcat(newMemberMessage, currentRoom->player[newMemberIndex]->userName);
                    for(int i = 0; i < currentRoom->playerNumPreSet; i++)
                    {
                        if(currentRoom->player[i] != NULL && i != newMemberIndex)
                        {
                            currentRoom->playerConnfdRSBusy[i] = 1;
                            send_message(currentRoom->playerConnfd[i], NOTIFICATION, newMemberMessage);
                            currentRoom->playerConnfdRSBusy[i] = 0;
                        }
                    }
                }
            }

            //printf("#####%d\n", newConnfd);
            for(int i = 0; i < currentRoom->playerNumPreSet; i++)
            {
                if(currentRoom->playerConnfd[i] != -1 && currentRoom->playerConnfd[i] != newConnfd)
                    if(FD_ISSET(currentRoom->playerConnfd[i], &(currentRoom->readfds)))
                    {
                        printf("#####Processing client connfd %d\n", currentRoom->playerConnfd[i]);
                        int connfd_index = getConnfdIndex(currentRoom->playerConnfd[i]);
                        recvBytes = recv(currentRoom->playerConnfd[i], recvBuff, sizeof(recvBuff), 0);
                        if(recvBytes == 0)
                        {
                            for(int j = 0; j < currentRoom->playerNumPreSet; j++)
                                if(j != i && currentRoom->playerConnfd[j] != -1)
                                {
                                    char messageTmp[100];
                                    strcpy(messageTmp, "Cannot connect to ");
                                    strcat(messageTmp, currentRoom->player[i]->userName);
                                    send_message(currentRoom->playerConnfd[j], NOTIFICATION, messageTmp);
                                }
                            clientConnfdUnconnect(connfd_index);
                            quitRoom(i, currentRoom);
                            if(i == 0)
                                newRoomHost(currentRoom, -1);
                            printOneRoom(roomID);
                            continue;
                        }

                        recvBuff[recvBytes] = '\0';

                        printf("####Recv mess: %s\n", recvBuff);
                        if(strcmp(recvBuff, "SEND_SUCCESS") == 0)
                        {
                            printf("error break\n");
                        }
                        else if(recvBuff[0] != '$')
                        {
                            char clientName_message[500];
                            strcpy(clientName_message, currentRoom->player[i]->userName);
                            clientName_message[strlen(clientName_message) + 1] = '\0';
                            clientName_message[strlen(clientName_message)] = ':';
                            // nếu dể thằng duwois lên trước có nghĩa đã xóa đi '\0'sau strlen khong phai cai truoc do nua
                            strcat(clientName_message, recvBuff);

                            for(int j = 0; j < currentRoom->playerNumPreSet; j++)
                            {
                                if(j != i && currentRoom->player[j] != NULL)
                                    send_message(currentRoom->playerConnfd[j], CHAT_MESSAGE, clientName_message);
                            }
                        }
                        else
                        {
                            char* keyWord = strtok(recvBuff, "\t");
                            if(strcmp(keyWord, "$INVITE") == 0)
                            {
                                char* userName = strtok(NULL, "\t");
                                invitePlayer(currentRoom, i, userName);
                            }
                            else if(strcmp(keyWord, "$KICK") == 0)
                            {
                                if(i == 0)
                                {
                                    char* userName = strtok(NULL, "\t");
                                    kickPlayer(currentRoom, userName);
                                }
                            }
                            else if(strcmp(keyWord, "$READY") == 0)
                            {
                                currentRoom->playerReady[i] = 1;
                                send_message(currentRoom->playerConnfd[i], NOTIFICATION, "You've hit ready");
                            }
                            else if(strcmp(keyWord, "$QUIT") == 0)
                            {
                                send_message(currentRoom->playerConnfd[i], GAME_CONTROL_DATA, "OUT_ROOM");
                                send_message(currentRoom->playerConnfd[i], NOTIFICATION, "You've hit quit");
                                quitRoom(i, currentRoom);

                                if(i == 0)
                                {
                                    char* userName = strtok(NULL, "\t");
                                    int j;
                                    if(userName == NULL)
                                        j = -1;
                                    else
                                        for(j = 0; j < currentRoom->playerNumPreSet; j++)
                                            if(currentRoom->player[j] != NULL
                                                    && strcmp(currentRoom->player[j]->userName, userName) == 0)
                                                break;

                                    newRoomHost(currentRoom, j);
                                }
                                printOneRoom(currentRoom->roomID);
                            }
                        }
                    }
            }
        }
        int res = roomPlay (currentRoom->roomID);
        if(res == ROOM_ERROR_PLAYER_OUTROOM)
        {
            for(int j = 0; j < currentRoom->playerNumPreSet; j++)
                if(currentRoom->playerConnfd[j] > -1)
                    if(fcntl(currentRoom->playerConnfd[j], F_GETFD) != -1)
                    {
                        sendBytes = send_message(currentRoom->playerConnfd[j], GAME_CONTROL_DATA, "GAME_BREAK");
                    }
        }

        // Un ready everyone
        for(int j = 0; j < currentRoom->playerNumPreSet; j++)
        {
            currentRoom->playerReady[j] = 0;
        }
    }
    while(1);
}

void kickPlayer(room* room, char* kickedUserName)
{
    int i;
    for(i = 0; i < room->playerNumPreSet; i++)
        if(room->player[i] != NULL
                && strcmp(room->player[i]->userName, kickedUserName) == 0)
            break;

    if (i == room->playerNumPreSet - 1)
    {
        send_message(room->playerConnfd[0], NOTIFICATION, "Kick fail");
    }

    send_message(room->playerConnfd[i], NOTIFICATION, "Host have kicked you out of room");
    send_message(room->playerConnfd[i], GAME_CONTROL_DATA, "KICKED");

    quitRoom(i, room);

    send_message(room->playerConnfd[0], NOTIFICATION, "Kick successfuly");
    printOneRoom(room->roomID);
}

void newRoomHost(room* room, int newHostIndex)
{
    if(newHostIndex == -1)
        for(int i = 0; i < room->playerNumPreSet; i++)
            if(room->playerConnfd[i] != -1)
                newHostIndex =  i;

    room->player[0] = room->player[newHostIndex];
    room->playerConnfd[0] = room->playerConnfd[newHostIndex];
    room->playerReady[0] = room->playerReady[newHostIndex];

    room->player[newHostIndex] = NULL;
    room->playerConnfd[newHostIndex] = -1;
    room->playerReady[newHostIndex] = 0;

    send_message(room->playerConnfd[0], GAME_CONTROL_MESSAGE, "You're now the room host");
    send_message(room->playerConnfd[0], GAME_CONTROL_DATA, "NEW_HOST");
}

void invitePlayer(room* room, int playerIndex, char* invitedUserName)
{
    accountNode* invAccount = getAccountNodeByUserName(invitedUserName);
    if(invAccount->isLogined == 0)
    {
        send_message(room->playerConnfd[playerIndex], NOTIFICATION, "Player is not online now");
    }
    else
    {
        char invMessage[100];
        strcpy(invMessage, "You have an invitation from ");
        strcat(invMessage, room->player[playerIndex]->userName);
        strcat(invMessage, " to enter room with ID is ");
        char roomIDStr[5];
        tostring(roomIDStr, room->roomID);
        strcat(invMessage, roomIDStr);
        send_message(invAccount->loginedClientConnfd, NOTIFICATION, invMessage);
    }
}

int roomPlay (int roomID)
{
    //Detached thread is cleaned up automatically when it terminates
    pthread_detach(pthread_self());

    printRoom(roomID);
    room* currentRoom = getRoomByID(roomID);

    char recvBuff[RECV_BUFF_SIZE + 1];
    char sendBuff[SEND_BUFF_SIZE + 1];

    int recvBytes, sendBytes;
    int max_readfd;
    fd_set readfds;

    for(int j = 0; j < currentRoom->playerNumPreSet; j++)
    {
        sendBytes = send_message(currentRoom->playerConnfd[j], MESSAGE, "GAME_START");
        if(sendBytes == 0)
        {
            return ROOM_ERROR_PLAYER_OUTROOM;
        }
    }

    for(int j = 0; j < currentRoom->playerNumPreSet; j++)
    {
        currentRoom->playerPoint[j] = 0;
    }

    printf("\n\n---------------------------------------------\n");
    printf("Game start!\n");
    printf("---------------------------------------------\n\n");

    int avoidID[ROOM_QUEST_NUM];
    for(int i = 0; i < ROOM_QUEST_NUM; i++)
    {
        avoidID[i] =  -1;
    }

    // TODO: u xinh xem thang nao di truoc
    int inTurnPlayer;

    //presum that the host go first

    inTurnPlayer = 0;


    sendBytes = send_message(currentRoom->player[inTurnPlayer]->loginedClientConnfd, GAME_CONTROL_MESSAGE, "You go first\n");
    if(sendBytes == 0)
    {
        return ROOM_ERROR_PLAYER_OUTROOM;
    }

    int numQuesPass = 0;
    char curQuesAns[ANS_MAXLEN];
    int ques_solved;

    while(numQuesPass != ROOM_QUEST_NUM)
    {
        ques_ansNode* cur_ques_ans = getRandomQues_ans(avoidID);
        for(int i = 0; i < ROOM_QUEST_NUM; i++)
        {
            if(avoidID[i] ==  -1)
            {
                avoidID[i] = cur_ques_ans->id;
                break;
            }
        }
        ques_solved = 0;

        for(int i = 0; i <= cur_ques_ans->ansLen; i++)
        {
            curQuesAns[i] = '_';
            if(i == cur_ques_ans->ansLen)curQuesAns[i] = '\0';
        }


        char ansLenStr[5];
        tostring(ansLenStr, cur_ques_ans->ansLen);

        char curAns[100];
        strcpy(curAns, "Current answer: ");
        strcat(curAns, curQuesAns);

        for(int j = 0; j < currentRoom->playerNumPreSet; j++)
        {
            sendBytes = send_message(currentRoom->playerConnfd[j], GAME_CONTROL_DATA, cur_ques_ans->ques);
            if(sendBytes == 0)
            {
                return ROOM_ERROR_PLAYER_OUTROOM;
            }
            sendBytes = send_message(currentRoom->playerConnfd[j], GAME_CONTROL_DATA, ansLenStr);
            if(sendBytes == 0)
            {
                return ROOM_ERROR_PLAYER_OUTROOM;
            }
        }

        int ansSolvedLetterNum = 0;

        while(ques_solved != 1)
        {
            //TODO: roll the wheel to start new round

            int roundEnd = 0;
            int startRoundPlayer = inTurnPlayer;
            fd_set readfd_inturnPlayer;
            do
            {
                char turnMessage[100];
                int inTurnPlayerConnfd = currentRoom->player[inTurnPlayer]->loginedClientConnfd;
                sendBytes = send_message(inTurnPlayerConnfd, GAME_CONTROL_DATA, "YOUR_TURN");
                if(sendBytes == 0)
                {
                    return ROOM_ERROR_PLAYER_OUTROOM;
                }
                strcpy(turnMessage, currentRoom->player[inTurnPlayer]->userName);
                strcat(turnMessage, "'s turn");
                for(int j = 0; j < currentRoom->playerNumPreSet; j++)
                {
                    if(j != inTurnPlayer)
                        sendBytes = send_message(currentRoom->playerConnfd[j], GAME_CONTROL_MESSAGE, turnMessage);
                    if(sendBytes == 0)
                    {
                        return ROOM_ERROR_PLAYER_OUTROOM;
                    }
                }

                sendBytes = send_message(inTurnPlayerConnfd, GAME_CONTROL_DATA, "10");
                if(sendBytes == 0)
                {
                    return ROOM_ERROR_PLAYER_OUTROOM;
                }

                FD_ZERO(&readfd_inturnPlayer);
                FD_SET(inTurnPlayerConnfd, &readfd_inturnPlayer);
                int n = inTurnPlayerConnfd;
                struct timeval tv;
                tv.tv_sec = 10;
                tv.tv_usec = 500000;
                int rv = select(n + 1, &readfd_inturnPlayer, NULL, NULL, &tv);
                if (rv == -1)
                {
                    perror("Error: ");
                }
                else if (rv == 0)
                {
                    sendBytes = send_message(inTurnPlayerConnfd, GAME_CONTROL_MESSAGE, "Time out");
                    if(sendBytes == 0)
                    {
                        return ROOM_ERROR_PLAYER_OUTROOM;
                    }
                    roundEnd = nextPlayerTurn(currentRoom, startRoundPlayer, &inTurnPlayer);
                    continue;
                }

                recvBytes = recv(inTurnPlayerConnfd, recvBuff, sizeof(recvBuff), 0);
                if(recvBytes == 0)
                {
                    return ROOM_ERROR_PLAYER_OUTROOM;
                }

                recvBuff[recvBytes] = '\0';
                char inTurnPlayerAns[200];
                strcat(inTurnPlayerAns, currentRoom->player[inTurnPlayer]->userName);
                char* stringCut;
                stringCut = strtok(recvBuff, "-");

                strcpy(inTurnPlayerAns, currentRoom->player[inTurnPlayer]->userName);
                strcat(inTurnPlayerAns, " chose to ");

                if(stringCut[0] == '1')
                {
                    char playerAnswer[20];
                    stringCut = strtok(NULL, "-");
                    strcpy(playerAnswer, stringCut);

                    strcat(inTurnPlayerAns, " chose to solve the question\nThe answer they gave is: ");
                    strcat(inTurnPlayerAns, playerAnswer);
                    for(int j = 0; j < currentRoom->playerNumPreSet; j++)
                    {
                        if(j != inTurnPlayer)
                            sendBytes = send_message(currentRoom->playerConnfd[j], GAME_CONTROL_MESSAGE, inTurnPlayerAns);
                    }

                    if(strcmp(playerAnswer, cur_ques_ans->ans) == 0)
                    {
                        sendBytes = send_message(inTurnPlayerConnfd, GAME_CONTROL_MESSAGE, "Congratulation!");
                        if(sendBytes == 0)
                        {
                            return ROOM_ERROR_PLAYER_OUTROOM;
                        }
                        ques_solved = 1;

                        strcpy(inTurnPlayerAns, "The answer is correct");
                        for(int j = 0; j < currentRoom->playerNumPreSet; j++)
                        {
                            if(j != inTurnPlayer)
                                sendBytes = send_message(currentRoom->playerConnfd[j], GAME_CONTROL_MESSAGE, inTurnPlayerAns);
                            if(sendBytes == 0)
                            {
                                return ROOM_ERROR_PLAYER_OUTROOM;
                            }
                        }

                        //TODO: Tinh diem cho nguoi choi
                        roundEnd = 1;
                        continue;
                    }
                    else
                    {
                        sendBytes = send_message(inTurnPlayerConnfd, GAME_CONTROL_MESSAGE, "Wrong!");
                        if(sendBytes == 0)
                        {
                            return ROOM_ERROR_PLAYER_OUTROOM;
                        }
                        strcpy(inTurnPlayerAns, "The answer is wrong");
                        for(int j = 0; j < currentRoom->playerNumPreSet; j++)
                        {
                            if(j != inTurnPlayer)
                                sendBytes = send_message(currentRoom->playerConnfd[j], GAME_CONTROL_MESSAGE, inTurnPlayerAns);
                            if(sendBytes == 0)
                            {
                                return ROOM_ERROR_PLAYER_OUTROOM;
                            }
                        }
                    }
                }
                else if(stringCut[0] == '2')
                {
                    strcat(turnMessage, "guess letter");
                    for(int j = 0; j < currentRoom->playerNumPreSet; j++)
                    {
                        if(j != inTurnPlayer)
                            sendBytes = send_message(currentRoom->playerConnfd[j], GAME_CONTROL_MESSAGE, turnMessage);
                        if(sendBytes == 0)
                        {
                            return ROOM_ERROR_PLAYER_OUTROOM;
                        }
                    }

                    char character[2];
                    stringCut = strtok(NULL, "-");
                    strcpy(character, stringCut);


                    strcat(inTurnPlayerAns, " chose to guess letter\nThe answer they gave is: ");
                    strcat(inTurnPlayerAns, character);
                    for(int j = 0; j < currentRoom->playerNumPreSet; j++)
                    {
                        if(j != inTurnPlayer)
                            sendBytes = send_message(currentRoom->playerConnfd[j], GAME_CONTROL_MESSAGE, inTurnPlayerAns);
                        if(sendBytes == 0)
                        {
                            return ROOM_ERROR_PLAYER_OUTROOM;
                        }
                    }

                    int numOfChar = 0;
                    int checkIfGuessed = 0;
                    for(int i = 0; i < cur_ques_ans->ansLen; i++)
                    {
                        if(curQuesAns[i] == character[0])
                        {
                            checkIfGuessed = 1;
                            break;
                        }
                    }
                    if(checkIfGuessed == 1)
                    {
                        sendBytes = send_message(inTurnPlayerConnfd, GAME_CONTROL_MESSAGE, "You've guessed the same character which is guessed before");
                        if(sendBytes == 0)
                        {
                            return ROOM_ERROR_PLAYER_OUTROOM;
                        }
                    }
                    else
                    {
                        for(int i = 0; i < cur_ques_ans->ansLen; i++)
                        {
                            if(cur_ques_ans->ans[i] == character[0])
                            {
                                curQuesAns[i] = character[0];
                                numOfChar++;
                            }
                        }

                        if(numOfChar == 0)
                        {
                            sendBytes = send_message(inTurnPlayerConnfd, GAME_CONTROL_MESSAGE, "Try again later");
                            if(sendBytes == 0)
                            {
                                return ROOM_ERROR_PLAYER_OUTROOM;
                            }
                            strcpy(inTurnPlayerAns, "We have no letter");
                            for(int j = 0; j < currentRoom->playerNumPreSet; j++)
                            {
                                if(j != inTurnPlayer)
                                    sendBytes = send_message(currentRoom->playerConnfd[j], GAME_CONTROL_MESSAGE, inTurnPlayerAns);
                                if(sendBytes == 0)
                                {
                                    return ROOM_ERROR_PLAYER_OUTROOM;
                                }
                            }
                        }
                        else
                        {
                            char guessCharRes[100];
                            sendBytes = send_message(inTurnPlayerConnfd, GAME_CONTROL_MESSAGE, "Congratulation!");
                            if(sendBytes == 0)
                            {
                                return ROOM_ERROR_PLAYER_OUTROOM;
                            }
                            char numOfCharStr[5];
                            tostring(numOfCharStr, numOfChar);
                            strcpy(guessCharRes, "We have ");
                            strcat(guessCharRes, numOfCharStr);
                            strcat(guessCharRes, " letter ");
                            strcat(guessCharRes, character);
                            sendBytes = send_message(inTurnPlayerConnfd, GAME_CONTROL_MESSAGE, guessCharRes);
                            if(sendBytes == 0)
                            {
                                return ROOM_ERROR_PLAYER_OUTROOM;
                            }
                            // TODO: tinh diem cho nguoi choi

                            strcpy(inTurnPlayerAns, "We have ");
                            strcat(inTurnPlayerAns, numOfCharStr);
                            strcat(guessCharRes, " letters");
                            for(int j = 0; j < currentRoom->playerNumPreSet; j++)
                            {
                                if(j != inTurnPlayer)
                                    sendBytes = send_message(currentRoom->playerConnfd[j], GAME_CONTROL_MESSAGE, inTurnPlayerAns);
                                if(sendBytes == 0)
                                {
                                    return ROOM_ERROR_PLAYER_OUTROOM;
                                }
                            }

                            ansSolvedLetterNum += numOfChar;

                            if(ansSolvedLetterNum == cur_ques_ans->ansLen)
                                ques_solved = 1;

                            roundEnd = 1;
                        }
                    }
                }

                if(roundEnd == 0)  //round not end => next player's turn
                {
                    roundEnd = nextPlayerTurn(currentRoom, startRoundPlayer, &inTurnPlayer);
                }
            }
            while(roundEnd == ROUND_CONTINUE);
            if(ques_solved != 1)
            {
                for(int j = 0; j < currentRoom->playerNumPreSet; j++)
                {
                    sendBytes = send_message(currentRoom->playerConnfd[j], GAME_CONTROL_MESSAGE, curQuesAns);
                    if(sendBytes == 0)
                    {
                        return ROOM_ERROR_PLAYER_OUTROOM;
                    }
                    sendBytes = send_message(currentRoom->playerConnfd[j], GAME_CONTROL_DATA, "NEXT_ROUND");
                    if(sendBytes == 0)
                    {
                        return ROOM_ERROR_PLAYER_OUTROOM;
                    }
                }
            }
        }
        numQuesPass++;

        for(int j = 0; j < currentRoom->playerNumPreSet; j++)
        {
            sendBytes = send_message(currentRoom->playerConnfd[j], GAME_CONTROL_DATA, "QUES_SOLVED");
            if(sendBytes == 0)
            {
                return ROOM_ERROR_PLAYER_OUTROOM;
            }
        }

        //TODO: thong bao diem cua nguoi choi
    }

    for(int j = 0; j < currentRoom->playerNumPreSet; j++)
    {
        sendBytes = send_message(currentRoom->playerConnfd[j], GAME_CONTROL_DATA, "END_GAME");
        if(sendBytes == 0)
        {
            return ROOM_ERROR_PLAYER_OUTROOM;
        }
    }
    return 0;
}

int nextPlayerTurn (room* currentRoom, int startRoundPlayer, int* inTurnPlayer)
{
    if(*inTurnPlayer != currentRoom->playerNumPreSet - 1)
    {
        *inTurnPlayer = *inTurnPlayer + 1;
    }
    else *inTurnPlayer = 0;
    if(startRoundPlayer == *inTurnPlayer)
    {
        return ROUND_END;
    }
    return ROUND_CONTINUE;
}

void quitRoom (int playerIndex, room* room)
{
    // tra tu do cho cong
    for(int i = 0; i < MAX_CLIENT; i++)
        if(clientConnfd[i] == room->playerConnfd[playerIndex])
            connfdNoServiceRunning[i] = 1;

    if(room->currentPlayerNum > 1)
    {
        room->currentPlayerNum--;

        room->player[playerIndex] = NULL;
        room->playerConnfd[playerIndex] = -1;
        room->playerReady[playerIndex] = 0;
        room->playerPoint[MAX_ROOM_PLAYER] = 0;
    }
    else
    {
        deleteRoom(room);
    }
}

int numOfReadyPlayer(room* room)
{
    int ready;
    ready = 0;
    for(int i = 0; i < room->playerNumPreSet; i++)
        if(room->playerConnfd[i] != -1 && room->playerReady[i] == 1)
            ready++;
    return ready;
}

void deleteRoom (room* room)
{
    printf("Delete room with ID: %d\n", room->roomID);
    for(int i = 0; i < MAX_ROOM; i++)
        if(roomList[i] != NULL && roomList[i]->roomID == room->roomID)
            roomList[i] = NULL;
    roomNum--;
    pthread_cancel(room->room_thread_id);
}

void printRoom ()
{
    printf("-----------------------------\n");
    printf("Room info\n\n");
    printf("Room num: %d\n\n", roomNum);
    for(int i = 0; i < MAX_ROOM; i++)
    {
        if(roomList[i] != NULL)
        {
            printf("Room ID: %d\n", roomList[i]->roomID);
            printf("Room thread ID: %ld\n", roomList[i]->room_thread_id);
            printf("Room name: %s\n", roomList[i]->roomName);
            printf("Num of player preset: %d\n", roomList[i]->playerNumPreSet);
            printf("Num of player: %d\n", roomList[i]->currentPlayerNum);
            printf("Host name: %s\n", roomList[i]->player[0]->userName);
            for(int j = 1; j < roomList[i]->playerNumPreSet; j++)
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

void printOneRoom (int roomID)
{
    printf("-----------------------------\n");
    for(int i = 0; i < MAX_ROOM; i++)
        if(roomList[i]->roomID == roomID)
        {
            printf("Room ID: %d\n", roomList[i]->roomID);
            printf("Room thread ID: %ld\n", roomList[i]->room_thread_id);
            printf("Room name: %s\n", roomList[i]->roomName);
            printf("Num of player preset: %d\n", roomList[i]->playerNumPreSet);
            printf("Num of player: %d\n", roomList[i]->currentPlayerNum);
            printf("Host name: %s\n", roomList[i]->player[0]->userName);
            for(int j = 1; j < roomList[i]->playerNumPreSet; j++)
            {
                if(roomList[i]->player[j] != NULL)
                {
                    printf("Player name: %s\n", roomList[i]->player[j]->userName);
                }
                else printf("Player name: waiting...\n");
            }
            for(int j = 0; j < roomList[i]->playerNumPreSet; j++)
            {
                printf("%d|", roomList[i]->playerConnfd[j]);
            }
            printf("\n");
            break;
        }
    printf("-----------------------------\n");
}
