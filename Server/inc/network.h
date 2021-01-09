#ifndef __NETWORK_H__
#define __NETWORK_H__

#define RECV_BUFF_SIZE 4096
#define SEND_BUFF_SIZE 100

typedef enum
{
    NOTIFICATION,
    MESSAGE,
    CHAT_MESSAGE,
    GAME_CONTROL_MESSAGE,
    GAME_CONTROL_DATA
} messageType;

int send_message(int connfd, messageType type, char* message);
void clientConnfdUnconnect(int connfdIndex);
void delay(int number_of_seconds);

#endif
