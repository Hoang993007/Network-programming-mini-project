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

#include "./../inc/errorCode.h"
#include "./../inc/client.h"

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

char recvMessage[5][RECV_BUFF_SIZE];
int messageReady[5];
// 0: notification - 1:...

socklen_t len;

int maxfd;

//set of socket descriptors
fd_set writefds;

typedef enum
{
    NOTIFICATION,
    MESSAGE,
    CHAT_MESSAGE
} messageType;

void* recv_message(void *args)
{
    fd_set readfds;
    int max_readfd;

    char recvBuff[RECV_BUFF_SIZE];

    for(int i = 0; i < 5; i++)
        messageReady[i] = 0;

    printf("\n#Start listent to message from server...\n\n");
    while(1)
    {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        max_readfd = sockfd;
        int askForSending;
        //printf("Selecting...\n");
        askForSending = select(max_readfd + 1, &readfds, NULL, NULL, NULL);

        if(askForSending == -1)
        {
            perror("\Error: ");
            // error occurred in select()
        }

        rcvBytes = recv(sockfd, recvBuff, RECV_BUFF_SIZE, 0);

        if(rcvBytes < 0)
        {
            perror("Error: ");
        }
        else if(rcvBytes == 0)
        {
            printf("\n\n-------------------------------------\n");
            printf("Server is no longer connected\n");
            exit(0);
        }

        recvBuff[rcvBytes] = '\0';

//printf("%s\n\n", recvBuff);
        char* receiveType = strtok(recvBuff, "-");
        char* message = strtok(NULL, ";");

        //printf("\n\n\t(TYPE: %s -- MESSAGE: %s)\n\n", receiveType, message);

        if(strcmp(receiveType, "NOTIFICATION") == 0)
        {
            printf("Notification: %s\n", message);
        }
        else if(strcmp(receiveType, "MESSAGE") == 0)
        {
            while(messageReady[1] == 1);
            strcpy(recvMessage[1], message);
            messageReady[1] = 1;
        }
        else if(strcmp(receiveType, "CHAT_MESSAGE") == 0)
        {
            printf("> %s\n", message);
        }

         send(sockfd, "SEND_SUCCESS",  sizeof("SEND_SUCCESS"), 0);

    }
}

void getMessage(messageType type, char* buff)
{
    while(messageReady[type] == 0);
    strcpy(buff, recvMessage[type]);
    messageReady[type] = 0;
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

int checkCharacter(char* c)
{
    if (strlen(c) == 1 && ((c[0] >= 'a' && c[0] <= 'z') || (c[0] >= 'A' && c[0] <= 'Z')))
        return 1;
    else
        return 0;
}

void signin ()
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


        char recvBuff[RECV_BUFF_SIZE];
        getMessage(MESSAGE, recvBuff);
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
        getMessage(MESSAGE, recvBuff);
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

void login ()
{
    int loginFlag = 0;
    char recvBuff[RECV_BUFF_SIZE];
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
        getMessage(MESSAGE, recvBuff);
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

        getMessage(MESSAGE, recvBuff);
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
    SERV_PORT = atoi(argv[2]);
    strcpy(SERV_ADDR, argv[1]);
    char recvBuff[RECV_BUFF_SIZE];

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

    pthread_t threadId;
    pthread_create(&threadId, NULL, &recv_message, NULL);

    //Step 3: Communicate with server
    getMessage(MESSAGE, recvBuff);
    printf("From server: %s\n", recvBuff);

    printf("Successfully connected to the server\n");
    printf("-----------------------------\n\n");

    send(sockfd, "hust;123", sizeof("hust;123"), 0);

    getMessage(MESSAGE, recvBuff);
    if(strcmp(recvBuff, "NEED_LOGIN") == 0)
    {
        printf("You did not loged in!\nPlease login or create a new account before playing!\n*****----***\n");
        int loginOrSignin;
        printf("-----------------------------\n");
        printf("OPTION\n\n");
        printf("0. Login\n");
        printf("1. Sign in\n");
        printf("Enter your choice: ");

        scanf("%d", &loginOrSignin);
        getchar();
        printf("-----------------------------\n");
        switch(loginOrSignin)
        {
        case 0:
            printf("LOGIN\n\n");
            login ();
            break;
        case 1:
            printf("SIGN IN\n\n");
            signin();
            break;
        default:
            break;
        }
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
        printf("1. New room\n");
        printf("2. Join room\n");
        printf("3. Playing history\n");
        printf("4. Sign out\n");
        printf("Enter your choice: ");

        scanf("%d", &choice);
        getchar();
        printf("-----------------------------\n");

        switch(choice)
        {
        case 0:
            printf("Change pass\n");
            char service[5];

            strcpy(service, "4");

            send(sockfd, service, sizeof(service)*5, 0);

            passwordType newPassword;
            printf("New password: ");
            scanf("%s", newPassword);
            getchar();

            send(sockfd, newPassword, sizeof(newPassword), 0);
            break;

        case 1:
            printf("New room\n");
            // service 7: Sign out
            strcpy(service, "5");
            send(sockfd, service, sizeof(service), 0);

            char roomName[255];
            printf("Room name: ");
            scanf("%s", roomName);
            getchar();

            send(sockfd, roomName, sizeof(roomName), 0);

            int playerNumPreSet;
            printf("playerNumPreSet: ");
            scanf("%d", &playerNumPreSet);
            getchar();

            char playerNumPreSet_char[2];
            playerNumPreSet_char[0] = playerNumPreSet + '0';
            playerNumPreSet_char[1] = '\0';

            send(sockfd, playerNumPreSet_char, sizeof(playerNumPreSet_char), 0);

            getMessage(MESSAGE, recvBuff);
            int recvBuffInt = atoi(recvBuff);
            printf("Room ID: %d\n", recvBuffInt);
            gameRoom(recvBuffInt, 1);
            break;

        case 2:
            printf("Enter room\n\n");
            strcpy(service, "6");
            send(sockfd, service, sizeof(service), 0);
            getMessage(MESSAGE, recvBuff);
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
            send(sockfd, chosenRoom_char, sizeof(chosenRoom_char), 0);

            getMessage(MESSAGE, recvBuff);
            puts(recvBuff);

            gameRoom(chosenRoom, 0);
            break;
        case 3:
            // service 7: Sign out
            printf("You're signed out!\n");
            strcpy(service, "7");
            send(sockfd, service, sizeof(service), 0);

            break;
        default:
            break;
        }
    }
    while(choice <= 3 && choice >= 0);

// close the descriptor
    close(sockfd);
}

void gameRoom(int roomID, int isHost)
{

    char recvBuff[RECV_BUFF_SIZE];

    if(isHost == 1)
    {
        printf("-----------------------------\n");
        printf("Your are now the host of the room\nKEYWORD\n\n");
        printf("+) Invite:  $INVITE<tab><userName>\n");
        printf("+) Kick:    $KICK<tab><userName>\n");
        printf("+) Realdy:    $REALDY\n");
        printf("+) Out room: $QUIT<tab><userName>\n"); // new host
        printf("-----------------------------\n\n");
    }

    if(isHost == 0)
    {
        printf("-----------------------------\n");
        printf("Your are in room\nKEYWORD\n\n");
        printf("+) Invite:  $INVITE<tab><userName>\n");
        printf("+) Realdy:    $REALDY\n");
        printf("+) Out room: $QUIT\n"); // new host
        printf("-----------------------------\n\n");
    }

    int realdy = 0;
    char chatMessage[500];

    printf("Write something to chat...\n\n");

    do
    {
        fgets(chatMessage, 500, stdin);
        chatMessage[strlen(chatMessage) - 1] = '\0';
printf("> you: %s\n", chatMessage);
        send(sockfd, chatMessage, sizeof(chatMessage), 0);
        if(strcmp(chatMessage, "$REALDY") == 0) realdy = 1;
    }
    while(realdy == 0);

    while(strcmp(recvMessage[1], "GAME_START") != 0);
    getMessage(MESSAGE, recvBuff);

    printf("\n\n---------------------------------------------\n");
    printf("Game start!\n");
    printf("---------------------------------------------\n\n");

    char thongbao[MAX];
    char choice[10];
    char answer[MAX];
    char request[MAX];
    char crossWord[MAX];
    char question[MAX];
    char character[MAX];
    int n;



    for (;;)                                    // vòng lặp cho tới khi client quit
    {
         read(sockfd,thongbao,BUFFSIZE);        // đọc thông báo từ server, nếu hết câu hỏi sẽ nhận "exit"
        thongbao[strlen(thongbao)] = '\0';      // và kết thúc chương trình
        if(strcmp(thongbao,"exit") == 0) break;

        read(sockfd, question, BUFFSIZE);       // đọc vào câu hỏi
        question[strlen(question)] = '\0';
       printf("                     câu hỏi  : %s\n\n", question);
        read(sockfd, crossWord, BUFFSIZE);      // đọc vào các ô trống
        crossWord[strlen(crossWord)] = '\0';
        int lengthOfCrossWord = strlen(crossWord) / 2;    // số lượng các chữ cái có trong đáp án
       printf("                     %d CHU CAI\n\n", lengthOfCrossWord);
        printf("                     %s\n\n", crossWord);

        while (1)                               // vòng lặp cho tới khi ô chữ được giải hoặc chọn trả lời toàn bộ câu hỏi
        {
            printf("\n\n1.Trả lời toàn bộ đáp án.\n\n2.Trả lời từng ô chữ.\n");
            do
            {
                printf("\n Nhập lựa chọn:  ");
                gets(choice);
            } while (strlen(choice) > 1 || (choice[0] != '1' && choice[0] != '2'));

            write(sockfd, choice, BUFFSIZE);       // gửi lựa chọn tới server

            if (choice[0] == '1')                  // chọn trả lời toàn bộ câu hỏi
            {
                printf("\nnhập vào đáp án của bạn : ");
                gets(answer);
                write(sockfd, answer, BUFFSIZE);   // gửi đáp án tới server

                read(sockfd, thongbao, BUFFSIZE);  // đọc thông báo từ server
                thongbao[strlen(thongbao)] = '\0';
                printf("%s\n\n", thongbao);
                break;
            }
            else                                   // chọn trả lời từng chữ cái
            {

                //int ch = atoi(ch);

                do
                {

                    printf("\nNhập vào một chữ cái : ");
                    __fpurge(stdin);
                    gets(character);
                } while (checkCharacter(character) == 0);
                character[strlen(character)] = '\0';

                write(sockfd, character, BUFFSIZE);      // gửi kí tự vừa nhập đến server

                bzero(thongbao, BUFFSIZE);
                read(sockfd, thongbao, BUFFSIZE);        // đọc thông báo từ client( kí tự vừa nhập xuất hiện bn lần)
                thongbao[strlen(thongbao)] = '\0';
                printf("       %s\n\n", thongbao);

                bzero(crossWord, sizeof(crossWord));
                read(sockfd, crossWord, BUFFSIZE);       // cập nhật crossWord
                crossWord[strlen(crossWord)] = '\0';
                printf("                     %s\n\n", crossWord);

                bzero(thongbao, sizeof(thongbao));               // Nhận thông báo từ server : ô chữ đã được giải chưa
                read(sockfd, thongbao, BUFFSIZE);            // nếu đã được giải thì nhận giá trị "done" thì thoát khỏi vòng lăp
                thongbao[strlen(thongbao)] = '\0';
                if (strcmp(thongbao, "done") == 0)
                {
                    printf("Ô chữ đã được giải !\n\n\n\n");
                    break;
                }
            }
        } // while(..)

        bzero(thongbao, sizeof(thongbao));
        read(sockfd, thongbao, BUFFSIZE); // nhận thông báo từ server: có muốn tiếp tục chơi không
        thongbao[strlen(thongbao)] = '\0';  // nếu có nhập vào "y" , nếu không nhập vào "n"
        do
        {

            printf("%s\n\n", thongbao);
            __fpurge(stdin);
            gets(request);

        } while (checkCharacter(request) == 0 || (checkCharacter(request) == 1 && request[0] != 'y' && request[0] != 'n'));
        write(sockfd, request, BUFFSIZE);
        if (request[0] == 'n')
        {
            printf("kết thúc chương trình\n");
            break;
        }

}
