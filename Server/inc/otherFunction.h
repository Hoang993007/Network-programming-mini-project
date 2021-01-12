#ifndef __OTHERFUNCTION_H__
#define __OTHERFUNCTION_H__

#define TIMEOUT -1;
#define NOT_CHARACTER 0
#define IS_CHARACTER 1

typedef struct
{
    char* str1;
    char* str2;
    int* int1;
    int* int2;
    struct sockaddr_in* addr1;
    struct sockaddr_in* addr2;

    int* resInt;
}Args;

void tostring(char str[], int num);
int checkCharacter(char* c);
void delay(int secs);

#endif
