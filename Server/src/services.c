#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../inc/services.h"
#include "../inc/accountSystem.h"

int logIn (char* IP, userNameType userName, passwordType password)
{
    if(isLogedIn (IP) == LOGED_IN)
        return LOGIN_DENIED;

    int res;

    accountNode* logInAccount = accessToAccount (userName, password, &res);
    printf("Res code: %d\n", res);
    if(logInAccount == NULL)
    {
        printf("Error: Login error\n");
        return res;
    }

    if(logInAccount->loginedIPNum == MAX_LOGIN_IP)
    {
        printf("Strict: Exceed max IP can login to account!\n");
        return LOGIN_DENIED;
    }

    for(int i = 0; i < MAX_LOGIN_IP; i++)
    {
        if(logInAccount->loginedIPMark[i] == 0)
        {
            logInAccount->loginedIPNum++;
            logInAccount->loginedIPMark[i] = 1;
            strcpy(logInAccount->loginedIP[i], IP);
            break;
        }
    }

    printf("Hello %s\n", logInAccount->userName);

    if(PRINT_LOGEDIN == 1) printLogedInAccount();

    res = LOGIN_SUCCESS;
    return res;
}

int isLogedIn (char* IP) {
  if(getAccountNodeByLoginedIP(IP) == NULL)
    return NOT_LOGED_IN;
  else return LOGED_IN;
}

/* find friend

   void search () {
   // 1. read data from keyboard
   userNameType userName;
   passwordType password;

   printf ("Username: ");
   scanf ("%s", userName);
   getchar();

   if(isExistUserName(userName) == ACCOUNT_NOT_EXIST) {
   printf ("Cannot find account\n");
   return;
   }

   account* accountAccess = getAccountByUserName(userName);

   if(accountAccess->status == BLOCKED) {
   printf ("Account is blocked");
   printf("\n");
   } else if(accountAccess->status == IDLE) {
   printf ("Account is not activated");
   printf("\n");
   } else if(accountAccess->status == ACTIVE) {
   printf ("Account is active");
   printf("\n");
   }
   }
*/

void changePass (char* IP, passwordType newPassword) {
  accountNode* accountAccess = getAccountNodeByLoginedIP(IP);

  accountChangePass(accountAccess, newPassword);

  /*
    passwordType password;
    int checkPassRes = checkPassword(accountAccess, password);

    if(checkPassRes == PASS_CORRECT) {
    accountChangePass(accountAccess, newPassword);

    if(PRINT_DB == 1) printDB();

    printf ("Password is changed");
    printf("\n");
    } else {
    if(checkPassRes == PASS_INCORRECT)
    printf ("Current password is incorrect, please try again\n");
    else if(checkPassRes == ACCOUNT_BLOCKED)
    printf ("Account is blocked\n");
    }*/
}

void signOut (char* IP)
{
    accountNode* signOutAccount = getAccountNodeByLoginedIP(IP);

    printf ("Goodbye %s", signOutAccount->userName);
    printf("\n");

    for(int i = 0; i < MAX_LOGIN_IP; i++)
    {
        if(signOutAccount->loginedIPMark[i] == 1 && strcmp(signOutAccount->loginedIP[i], IP) == 0)
            signOutAccount->loginedIPMark[i] = 0;
    }
    signOutAccount->loginedIPNum--;

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
        if(getNode->loginedIPNum > 0)
        {
            printf ("UserName: %s\n", getNode->userName);
            for(int i = 0; i < MAX_LOGIN_IP; i++)
                if(getNode->loginedIPMark[i] == 1)
                {
                    printf ("\t%d. %s\n", i, getNode->loginedIP[i]);
                }

        }
    }

    printf ("---------------------------------");
    printf("\n");
}
