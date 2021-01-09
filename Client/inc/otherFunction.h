#ifndef __OTHERFUNCTION_H__
#define __OTHERFUNCTION_H__

#define TIMEOUT -1;

typedef struct
{
    char* str1;
    char* str2;
    int* int1;
    int* int2;
    struct sockaddr_in* addr1;
    struct sockaddr_in* addr2;
}Args;

void holdScreen ();
void clearScreen ();
int fgets_timeout (char* buff, int sizeOfBuff, int seconds);
void tostring(char str[], int num);
int checkCharacter(char* c);
void delay(int number_of_seconds);

#endif
