#ifndef __ACCOUNTSYSTEM_H__
#define __ACCOUNTSYSTEM_H__

#include <stdio.h>
#include "errorCode.h"

#define PRINT_LOGIN_INFO_DB 1 // for testing

#define MAX_USERNAME_LENGTH 50
#define MAX_PASS_LENGTH 20
#define ACTIVE_CODE_LENGTH 20

#define MAX_WRONG_ACTIVE_CODE 3
#define MAX_WRONG_PASS 3

#define MAX_LOGINED_IP 5

extern char ACTIVE_CODE[ACTIVE_CODE_LENGTH];

/*status: 1: active  - 2: blocked -  0: idle*/
typedef enum 
  {    IDLE,
       ACTIVE,
       BLOCKED
  }accountStatus;

typedef char userNameType[MAX_USERNAME_LENGTH];
typedef char passwordType[MAX_PASS_LENGTH];

typedef struct _accountNode {
  userNameType userName;
  passwordType password;
  accountStatus status;
  int wrongActiveCodeCount;
  int wrongPassCount;
  
  int loginedCount;
  char loginedIP[255][5];
  struct _accountNode* next;
}accountNode;

extern accountNode* accountNode_front;
extern accountNode* accountNode_rear;

int openFileStream(FILE* file, char* path, char* opt);
void freeAccountNode();
void accountRegister (userNameType newUserName, passwordType password);

// add new account -----------------------------------------
int addAccountNode (char* userName, char* password, accountStatus status);
// ============================================
accountNode* getAccountNodeByUserName (char* userName);
int isExistUserName(char* userName);
// ===================================
int checkPassword(accountNode* accountCheck, char* password);
accountNode* accessToAccount (userNameType userName, passwordType password, int *res);
// ===================================
void accountChangePass (accountNode* accountModify, char* newPass);
// ============================================
void deleteAccountNodeByUserName (char* userName);
// ===========================================================
void loadUsername_passData ();
int storeUsername_passData ();
// ==============================================================
/*status: 1: active  - 0: blocked -  2: idle*/
int activateAccount (userNameType userName, passwordType password, char* code);
void print_username_pass();

#endif
