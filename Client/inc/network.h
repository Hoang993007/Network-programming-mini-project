#ifndef __NETWORK_H__
#define __NETWORK_H__

#define SEND_SUCCESS 1
#define RECV_BUFF_SIZE 4096
#define RECV_MESS_SIZE 500
#define RECV_MESSAGE_TYPE_NUM 5

#define SEND_BUFF_SIZE 4096
#define SEND_MESS_SIZE 500
#define SEND_MESSAGE_TYPE_NUM 2

typedef enum
{
    NOTIFICATION,
    MESSAGE,
    CHAT_MESSAGE,
    GAME_CONTROL_MESSAGE,
    GAME_CONTROL_DATA
} recvFromServer_MessageType;

typedef enum{
    SERVICE,
    CLIENT_MESSAGE
} sendToServer_MessageType;

extern char recvMessage[RECV_MESSAGE_TYPE_NUM][RECV_MESS_SIZE];
extern int messageReady[RECV_MESSAGE_TYPE_NUM];

void* recv_message(void *args);
void storeMessage (char* message);
void getMessage(recvFromServer_MessageType type, char* buff);
int waitMessage(recvFromServer_MessageType type, char* message);
/*
Des:
    No wait if message buff have no message
*/
int checkMessage(recvFromServer_MessageType type, char* message);
/*
Des:
    Wait if message buff have no message
*/
int checkMessage_waitRecv(recvFromServer_MessageType type, char* message);
int send_message(sendToServer_MessageType type, char* message);
int sendMessage_markEnd(char* message);
int sendMessage_add_messageType(sendToServer_MessageType type, char* message);

#endif
