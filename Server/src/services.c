#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../inc/services.h"
#include "../inc/accountSystem.h"

int logIn (struct in_addr* IP, int connfd, userNameType userName, passwordType password)
{
    //TODO: check if username doesn't exist

    int res;

    accountNode* logInAccount = accessToAccount (userName, password, &res);

    // TEST:
    // printf("Res code: %d\n", res);
    if(logInAccount == NULL)
    {
        printf("Loging in failed\n");
        return res;
    }

    memcpy(&(logInAccount->loginedClientIP), IP, sizeof(IP)+1);

        logInAccount->isLogined = 1;
    logInAccount->loginedClientConnfd = connfd;

    printf("Hello %s\n", logInAccount->userName);

    if(PRINT_LOGEDIN == 1)
        printLogedInAccount();

    res = LOGIN_SUCCESS;
    return res;
}

void changePass (accountNode* account, passwordType newPassword)
{
    accountChangePass(account, newPassword);
}

void signOut (accountNode* account)
{
    printf ("Goodbye %s\n", account->userName);

    account->isLogined = 0;

    if(PRINT_LOGEDIN == 1) printLogedInAccount();
}

void printLogedInAccount()
{
    printf ("---------------------------------");
    printf("\n");
    printf ("ACCOUNT LOGED IN:");
    printf("\n");
    printf("\n");
    accountNode* tmp = accountNode_front;
    while(tmp != NULL)
    {
        accountNode* getNode = tmp;
        tmp = tmp->next;

        if(getNode->isLogined == 1)
        {
            printf ("UserName: %s\n", getNode->userName);
            printf ("\t%s\n", inet_ntoa(getNode->loginedClientIP));
            printf ("\tConnfd: %d\n", getNode->loginedClientConnfd);
        }
    }

    printf ("---------------------------------");
    printf("\n");
}
