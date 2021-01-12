#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char test_str[] = "1-2-3-4-5-6";
    test_str[5] = '\0';

    char* token;
    token = strtok(test_str, "-");
    while(token != NULL)
    {
        printf("token: %s\n", token);
        token = strtok (NULL, "-");
    }
}

/*
Result:

token: 1
token: 2
token: 3

conclusiton:
    strtok will stop when meet '\0' character

    But should see test: origin_string
    To know more
*/
