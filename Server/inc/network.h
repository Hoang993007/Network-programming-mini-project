#ifndef __NETWORK_H__
#define __NETWORK_H__

#define RECV_BUFF_SIZE 4096
#define SEND_BUFF_SIZE 100

typedef enum
{
    NOTIFICATION,
    MESSAGE,
    CHAT_MESSAGE
} messageType;

int send_message(int connfd, messageType type, char* message);
int recv_message(int connfd, char* message, struct timeval* tv);
void delay(int number_of_seconds);

#endif
