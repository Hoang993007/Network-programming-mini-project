#ifndef __THREADMANAGER_H__
#define __THREADMANAGER_H__

#include <stdio.h>
#include "accountSystem.h"
#include "errorCode.h"

#define PRINT_LOGEDIN 1

#define LOGED_IN 60
#define NOT_LOGED_IN 61

int logIn (char* IP, userNameType userName, passwordType password);
int isLogedIn (char* IP);
void changePass (char* IP, passwordType newPassword);
void signOut (char* IP);
void printLogedInAccount();

#endif
