#ifndef __SERVICES_H__
#define __SERVICES_H__

#include <stdio.h>
#include "accountSystem.h"
#include "errorCode.h"

#define PRINT_LOGEDIN 1

#define LOGED_IN 60
#define NOT_LOGED_IN 61

int logIn (struct in_addr* IP, int connfd, userNameType userName, passwordType password);
void changePass (accountNode* account, passwordType newPassword);
void signOut (accountNode* account);
void printLogedInAccount();

#endif
