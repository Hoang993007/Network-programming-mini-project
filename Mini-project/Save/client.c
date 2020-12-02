#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "inc/services.h"

#define BUFF_SIZE 255

int main(int argc, char *argv[]) {
  int SERV_PORT = atoi(argv[2]);
  char SERV_ADDR[255];
  strcpy(SERV_ADDR, argv[1]);

  int sockfd, rcvBytes, sendBytes;
  socklen_t len;
  char buff[BUFF_SIZE+1];
  struct sockaddr_in servaddr;

  //Step 1: Construct socket
  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
    perror("Error: "); 
    return 0; 
  }

  //Step 2: Define the address of the server
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  // chỗ này server address của nó là gì?
    servaddr.sin_addr.s_addr = inet_addr(SERV_ADDR); 
  servaddr.sin_port = htons(SERV_PORT);


 // connect to server 
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) 
    { 
        printf("\n Error : Connect Failed \n"); 
        exit(0); 
    } 



  //Step 3: Communicate with server
  printf("Send to server: ");
  gets(buff);

  len = sizeof(servaddr);
  sendBytes = sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr *) &servaddr, len);
  if(sendBytes < 0){
    perror("Error: "); 
    return 0; 
  }

  rcvBytes = recvfrom(sockfd, buff, BUFF_SIZE, 0, (struct sockaddr *) &servaddr, &len);

  if(rcvBytes < 0){
    perror("Error: "); 
    return 0; 
  }
    
  buff[rcvBytes] = '\0';

  printf("Reply from server: %s", buff);

// close the descriptor 
    close(sockfd); 

}
