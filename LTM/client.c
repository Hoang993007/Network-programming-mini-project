// Client side implementation of UDP client-server model
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>  //Thư viện chứa các cấu trúc cần thiết cho socket
#include <netinet/in.h> //Thư viện chứa các hằng số, cấu trúc khi sử dụng địa chỉ trên internet
#include <errno.h>
#include <stdio_ext.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>
#define MAX 50
#define PORT 8080
#define BUFFSIZE 256
#define SA struct sockaddr
int checkCharacter(char c[20])
{
    if (strlen(c) == 1 && ((c[0] >= 'a' && c[0] <= 'z') || (c[0] >= 'A' && c[0] <= 'Z')))
        return 1;
    else
        return 0;
}

int main(int argc, char *argv[])
{
    char *ipAddress = (char *)argv[2];
    char *port_char = argv[1];
    int port_number = atoi(port_char);
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1)
    {
        printf("socket creation failed...\n\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ipAddress);
    //ví dụ : “220.231.122.114”  Sử dụng hàm API inet_addr
    // để chuyển đổi thành địa chỉ IP dạng số 32 bit
    servaddr.sin_port = htons(port_number);

    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n\n");
        exit(0);
    }
    else
        printf("connected to the server..\n\n");

    // function for chat

    char send[MAX];
    char recv[MAX];
    char recv2[MAX];
    char thongbao[MAX];
    char nhankq[MAX];
    char cauhoi[256];
    char gui[MAX];
    char nhan[MAX];
    char nhanyc[MAX];
    char guiyc[MAX];
    char st[10];
    char choice[10];
    char da[MAX];
    int n;
    int diem;
    for (;;)
    {
        read(sockfd, nhan, BUFFSIZE);  // đọc vào câu hỏi 
        nhan[strlen(nhan)] = '\0';
       printf("                     câu hỏi  : %s\n\n", nhan);
        strcpy(cauhoi,nhan);
        read(sockfd, nhan, BUFFSIZE);  // đọc vào các ô trống 
        nhan[strlen(nhan)] = '\0';
        int sl = strlen(nhan) / 2;    // sl = số  chữ cái có trong đáp án 
       printf("                     %d CHU CAI\n\n", sl);
        printf("                     %s\n\n", nhan);
      
        while (1)
        {
            // read(sockfd, nhan, BUFFSIZE);  // đọc vào các ô trống 
            // nhan[strlen(nhan)] = '\0';
            // printf("                     câu hỏi  : %s\n\n", cauhoi);
            // printf("                     %s\n\n\n\n", nhan);
            printf("\n\n1.Trả lời toàn bộ đáp án.\n\n2.Trả lời từng ô chữ.\n");
            do
            {
                printf("\n Nhập lựa chọn:  ");
                gets(choice);
            } while (strlen(choice) > 1 || (choice[0] != '1' && choice[0] != '2'));

            write(sockfd, choice, BUFFSIZE);

            if (choice[0] == '1')
            {
                printf("\nnhập vào đáp án của bạn : ");
                gets(da);
                write(sockfd, da, BUFFSIZE);
                read(sockfd, thongbao, BUFFSIZE);
                thongbao[strlen(thongbao)] = '\0';
                printf("%s\n\n", thongbao);
                break;
            }
            else
            {

                //int ch = atoi(ch);

                do
                {

                    printf("\nNhập vào một chữ cái : ");
                    __fpurge(stdin);
                    gets(gui);
                    // printf("check = %d\n\n",checkCharacter(send));
                } while (checkCharacter(gui) == 0);
                gui[strlen(gui)] = '\0';

                write(sockfd, gui, BUFFSIZE); // nhap vao 1 ki tu

                bzero(thongbao, BUFFSIZE);
                read(sockfd, thongbao, BUFFSIZE); // doc thong bao : ki tu vua nhap xuat hien bn lan
                thongbao[strlen(thongbao)] = '\0';
                printf("       %s\n\n", thongbao);

                bzero(nhankq, sizeof(nhankq));
                read(sockfd, nhankq, BUFFSIZE); // doc ket qua
                nhankq[strlen(nhankq)] = '\0';
                printf("                     %s\n\n", nhankq);

                bzero(nhan, sizeof(nhan));
                read(sockfd, nhan, BUFFSIZE);
                nhan[strlen(nhan)] = '\0';
                // printf(" %s\n\n", nhan);
                if (strcmp(nhan, "done") == 0)
                {
                    printf("Ô chữ đã được giải !\n\n\n\n");
                    break;
                }
            }
        } // while(..)

        bzero(nhanyc, sizeof(nhanyc));
        read(sockfd, nhanyc, BUFFSIZE); // doc thong bao ( ban co muon tiep tuc choi tiep)
        nhanyc[strlen(nhanyc)] = '\0';
        do
        {

            printf("%s\n\n", nhanyc);
            __fpurge(stdin);
            gets(guiyc);

        } while (checkCharacter(guiyc) == 0 || (checkCharacter(guiyc) == 1 && guiyc[0] != 'y' && guiyc[0] != 'n'));
        write(sockfd, guiyc, BUFFSIZE);
        if (guiyc[0] == 'n')
        {
            printf("kết thúc chương trình\n");
            break;
        }
    } // for(;;)

    // close the socket
    close(sockfd);
}