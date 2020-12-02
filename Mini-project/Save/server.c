#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> 
#include <errno.h>
#include "inc/services.h"

#define BUFF_SIZE 255

int main(int argc, char *argv[]) {
  init("./bin/nguoidung.txt");

  int SERV_PORT = atoi(argv[1]);

  //cùng một phiên truyền nhiều lần
  // Gửi liên tục 1 dãy từ 1 đến 100 xem sao


  int sockfd, rcvBytes, sendBytes;
  socklen_t len;
  char buff[BUFF_SIZE+1];
  struct sockaddr_in servaddr, cliaddr;

    // The bzero() function erases the data in the n bytes of the memory starting at the location pointed to by s,
  // by writing zeros (bytes containing '\0') to that area.
  bzero(&servaddr, sizeof(servaddr));

  //Step 1: Construct socket
  // int s = socket(domain, type, protocol)
  /*
     For UDP/IP sockets,
     we want to specify the IP address family (AF_INET)
     and datagram service (SOCK_DGRAM).

     Since there's only one form of datagram service,
     there are no variations of the protocol,
     so the last argument, protocol, is zero.
  */


  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
    perror("Error: "); 
    return 0;
  }

  //Step 2: Bind address to socket
  /* bind to an arbitrary return address */
  /* because this is the client side, we don't care about the address */
  /* since no application will initiate communication here - it will */
  /* just send responses */
  /* INADDR_ANY is the IP address and 0 is the socket */
  /* htonl converts a long integer (e.g. address) to a network representation */
  /* htons converts a short integer (e.g. port) to a network representation */

  /*
    struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
    };

    struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
    };
  */
  servaddr.sin_family = AF_INET;

  //The in_addr structure represents an IPv4 Internet address.
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  //bind() of INADDR_ANY does NOT "generate a random IP".
  //It binds the socket to all available interfaces.
  /*
    bind() of INADDR_ANY does NOT "generate a random IP". It binds the socket to all available interfaces.

    For a server, you typically want to bind to all interfaces - not just "localhost".

    If you wish to bind your socket to localhost only,
    the syntax would be my_sockaddress.sin_addr.s_addr = inet_addr("127.0.0.1");, then call bind(my_socket, (SOCKADDR *) &my_sockaddr, ...).
  */
  servaddr.sin_port = htons(SERV_PORT);

  //#include <sys/socket.h>
  // int bind(int socket, const struct sockaddr *address, socklen_t address_len);
  if(bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))){
    perror("Error: ");
    return 0;
  }

  printf("Server started.\n");

  //Step 3: Communicate with client
  /* now loop, receiving data and printing what we received */

  for ( ; ; ) {
    len = sizeof(cliaddr);

    //int
    //recvfrom(int socket, void *restrict buffer, size_t length, int flags, struct sockaddr *restrict src_addr, socklen_t *restrict *src_len)
    rcvBytes = recvfrom(sockfd, buff, BUFF_SIZE, 0, (struct sockaddr *) &cliaddr, &len);

    if(rcvBytes < 0){
      perror("Error: ");
      return 0;
    }

    char mesg[] = "Test";
    buff[rcvBytes] = '\0';
puts(buff);
    printf("[%s:%d]: %s", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), mesg);

    sendBytes = sendto(sockfd, buff, rcvBytes, 0, (struct sockaddr *) &cliaddr, len);
    if(sendBytes < 0){
      perror("Error: "); return 0;
    }
  }

  close (sockfd);
  // không đóng => mất socket

  rubbishCollection();
  //another rubbish collection
  return 0;
}
