#ifndef __NETWORK_H__
#define __NETWORK_H__

#define RECV_BUFF_SIZE 4096
#define messageTypeNum 10

typedef enum
{
    NOTIFICATION,
    MESSAGE,
    CHAT_MESSAGE,
    GAME_CONTROL_MESSAGE,
    GAME_CONTROL_DATA
} messageType;

extern char recvMessage[messageTypeNum][RECV_BUFF_SIZE];
extern int messageReady[messageTypeNum];
// 0: notification - 1:...

void* recv_message(void *args);
void getMessage(messageType type, char* buff);

#endif
