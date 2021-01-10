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

#include <../inc/otherFunction.h>

#define RECV_BUFF_SIZE 4096

void holdScreen ()
{
    printf("(Hit enter to continue...)");
    while(getchar() != '\n');
    clearScreen ();
}

void clearScreen ()
{
#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    system("clear");
#endif

#if defined(_WIN32) || defined(_WIN64)
    system("cls");
#endif
}

int fgets_timeout (char* buff, int sizeOfBuff, int seconds)
{
    fd_set          input_set;
    struct timeval  timeout;
    int             ready_for_reading = 0;
    int             read_bytes = 0;
    char            recvBuff[RECV_BUFF_SIZE];

    FD_ZERO(&input_set );
    FD_SET(0, &input_set);

    timeout.tv_sec = seconds;
    //tv.tv_usec = 500000;

    ready_for_reading = select(1, &input_set, NULL, NULL, &timeout);

    if (ready_for_reading == -1)
    {
        //
    }
    else if (ready_for_reading == 0)
    {
        return TIMEOUT;
    }

    read_bytes = read(0, buff, sizeOfBuff);
    if(recvBuff[read_bytes-1]=='\n')
    {
        --read_bytes;
        buff[read_bytes]='\0';
    }

    if(read_bytes==0)
    {
        // Just hit enter
    }
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

void delay(int secs)
{
        unsigned int retTime = time(0) + secs;   // Get finishing time.
    while (time(0) < retTime);               // Loop until it arrives.
}
