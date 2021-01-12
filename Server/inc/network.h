#ifndef __NETWORK_H__
#define __NETWORK_H__

#define RECV_BUFF_SIZE 4096
#define SEND_BUFF_SIZE 100

#define CONNFD_CANNOT_CONNECT -1
#define EXCESS_TIME_LIMIT 601
#define SEND_SUCCESS 602

extern char messageEndMarker[5];

int getConnfdIndex(int connfd);
void clientConnfdUnconnect(int connfdIndex);

#endif
