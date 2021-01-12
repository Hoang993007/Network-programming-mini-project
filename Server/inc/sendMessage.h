#ifndef __SENDMESSAGE_H__
#define __SENDMESSAGE_H__

#define RECV_BUFF_SIZE 4096
#define SEND_BUFF_SIZE 100

#define EXCESS_TIME_LIMIT 601
#define SEND_SUCCESS 602

typedef enum
{
    NOTIFICATION,
    MESSAGE,
    CHAT_MESSAGE,
    GAME_CONTROL_MESSAGE,
    GAME_CONTROL_DATA
} sendToClientMessageType;

int send_message(int connfd, sendToClientMessageType type, char* message);
int sendMessage_markEnd(char* message);
int sendMessage_add_messageType(sendToClientMessageType type, char* message);

#endif
