#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../inc/services.h"
#include "../inc/accountSystem.h"

accessPermit* logedIn = NULL;
int logedInCount = 0;

int logIn (char* IP, userNameType userName, passwordType password) {
  int res;
  accountNode* logInAccount = accessToAccount (userName, password, &res);
  printf("Res code: %d\n", res);
  if(logInAccount == NULL)
    {
      printf("Error: Login error\n");
      return res;
    }

  if(logedInCount == 0) {
    logedIn = (accessPermit*)malloc(sizeof(accessPermit));
    logedInCount++;

  } else {
    logedIn = realloc(logedIn, sizeof(accessPermit)*(logedInCount+1));
    logedInCount++;
  }

  logedIn[logedInCount-1].accessAccount = logInAccount;
  strcpy(logedIn[logedInCount-1].IP, IP);

  logInAccount->loginedCount++;
  for(int i = 0; i < MAX_LOGINED_IP; i++) {
    if(logedIn->accessAccount->loginedIP[i][0] = '\0') {
      strcpy(logInAccount->loginedIP[i], IP);
    }
  }
   
  printf("Hello %s\n", logedIn[logedInCount-1].accessAccount->userName);

  if(PRINT_LOGEDIN == 1) printLogedInAccount();

  res = LOGIN_SUCCESS;
  return res;
}

int isLogedIn (char* IP) {
  if(logInInfoGet(IP) == NULL)
    return NOT_LOGED_IN;
  else return LOGED_IN;
}

void freeLogIn() {
  free(logedIn);
}

accessPermit* logInInfoGet(char* IP) {
  if(logedIn == NULL) return NULL;

  for(int i = 0; i < logedInCount; i++) {
    if(strcmp(logedIn[i].IP, IP) == 0)
      return  &logedIn[i];
  }
  return NULL;
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
  logedIn = logInInfoGet(IP);

  accountNode* accountAccess = getAccountNodeByUserName(logedIn->accessAccount->userName);

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

void signOut (char* IP) {
  accessPermit* logedIn = logInInfoGet(IP);

  printf ("Goodbye %s", logedIn->accessAccount->userName);
  printf("\n");

  logedIn->accessAccount->loginedCount--;
  for(int i = 0; i < MAX_LOGINED_IP; i++) {
    if(strcmp(logedIn->accessAccount->loginedIP[i], IP) == 0) {
      logedIn->accessAccount->loginedIP[i][0] = '\0';
    }
  }

  for(int i=0; i < logedInCount; i++) {
    if(strcmp(logedIn[i].IP, IP) == 0) {
      for(int j = i+1; j < logedInCount; j++)
	logedIn[j-1] = logedIn[j];
      logedInCount--;
      logedIn = realloc(logedIn, sizeof(accessPermit)*logedInCount);

      if(PRINT_LOGEDIN == 1) printLogedInAccount();
      break;
    }
  }
}

void printLogedInAccount() {
  printf ("---------------------------------");
  printf("\n");
  printf ("ACCOUNT LOGED IN:");
  printf("\n");
  printf("\n");
  for(int i=0; i < logedInCount; i++) {
    printf ("%s\n", logedIn[i].IP);
    printf ("\t%s\n", logedIn[i].accessAccount->userName);
  }
  printf ("---------------------------------");
  printf("\n");
}
