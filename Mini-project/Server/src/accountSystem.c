#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/accountSystem.h"

char ACTIVE_CODE[ACTIVE_CODE_LENGTH] = "LTM121216";

char username_pass_filePath[255] = "./Server/data/userData/username_pass.txt";
//"./Server/data/userData/username_pass.txt";

accountNode* accountNode_front = NULL;
accountNode* accountNode_rear = NULL;

void freeAccountNode() {
  accountNode* tmp = accountNode_front;

  while(tmp != NULL) {
    accountNode* trash = tmp;
    tmp = trash->next;
    free(trash);
  }
}

void accountRegister (userNameType newUserName, passwordType password) {
  addAccountNode(newUserName, password, IDLE);
  printf("Successful registration. Activation required.\n");

  if(PRINT_LOGIN_INFO_DB == 1) print_username_pass();
}

// add new account -----------------------------------------
int addAccountNode (char* userName, char* password, accountStatus status) {
  userName[strlen(userName)] = '\0';
  password[strlen(password)] = '\0';

  if(isExistUserName(userName) == ACCOUNT_EXIST) {
    return ACCOUNT_EXIST;
  }

  accountNode* newNode = (accountNode*)malloc(sizeof(accountNode));

  strcpy(newNode->userName, userName);
  strcpy(newNode->password, password);
  newNode->status = status;
  newNode->wrongActiveCodeCount = 0;
  newNode->wrongPassCount = 0;
  newNode->loginedCount = 0;
  newNode->next = NULL;

  for(int i = 0; i < MAX_LOGINED_IP; i++) {
    newNode->loginedIP[i][0] = '\0';
  }

  if(accountNode_front == NULL) {
    accountNode_front = newNode;
    accountNode_rear = accountNode_front;
  } else if (accountNode_front != NULL) {
    accountNode_rear->next = newNode;
    accountNode_rear = newNode;
  }
}

// ============================================
accountNode* getAccountNodeByUserName (char* userName) {
  accountNode* tmp = accountNode_front;

  while(tmp != NULL) {
    accountNode* getNode = tmp;
    tmp = tmp->next;
    if (strcmp(getNode->userName, userName) == 0) {
      return getNode;
    }
  }

  return NULL;
}

int isExistUserName(char* userName) {
  if(getAccountNodeByUserName(userName) != NULL) {
    return ACCOUNT_EXIST;
  }
  return ACCOUNT_NOT_EXIST;
}

// ===================================
int checkPassword(accountNode* accountCheck, char* password) {
  if(accountCheck->status == IDLE){
    printf ("Account is not activated\n");
    return ACCOUNT_IDLE;
  }
  else if(accountCheck->status == BLOCKED) {
    printf ("Account is blocked\n");
    return ACCOUNT_BLOCKED;
  }
  else if(strcmp(password, accountCheck->password) == 0) {

    return PASS_CORRECT;
  }
  else {
    printf ("Password is incorrect\n");
    accountCheck->wrongPassCount++;

    if(accountCheck->wrongPassCount == MAX_WRONG_PASS) {
      accountCheck->status = BLOCKED;
      return ACCOUNT_JUST_BLOCKED;
    }

    return PASS_INCORRECT;
  }
}

accountNode* accessToAccount (userNameType userName, passwordType password, int *res) {
  accountNode* accountAccess = getAccountNodeByUserName(userName);

  *res = checkPassword(accountAccess, password);

  if(*res == PASS_CORRECT){
    printf("Access to account %s sucessfully\n", accountAccess->userName);
    return accountAccess;
  }
  else {
    return NULL;
  }
}

// ===================================
void accountChangePass (accountNode* accountModify, char* newPass) {
  strcpy(accountModify->password, newPass);
  if(PRINT_LOGIN_INFO_DB == 1) print_username_pass();
};

// ============================================
void deleteAccountNodeByUserName (char* userName) {
  accountNode* tmp = accountNode_front;

  if(strcmp(accountNode_front->userName, userName) == 0)
    if(accountNode_front == accountNode_rear) {
      accountNode_front = NULL;
      accountNode_rear = NULL;
      return;
    } else {
      accountNode_front = tmp->next;
      free(tmp);
      return;
    }

  accountNode* previousNode = tmp;
  tmp = tmp->next;

  do{
    if((tmp->userName, userName) == 0) {
      previousNode->next = tmp->next;
      if(tmp == accountNode_rear) {
	accountNode_rear = previousNode;
      }
      free(tmp);
      break;
    }

    previousNode = tmp;
    tmp = tmp->next;
  }while(tmp != NULL);
}

// ===========================================================
void loadUsername_passData () {
  if(accountNode_front != NULL) {
    freeAccountNode();
    accountNode_front = NULL;
    accountNode_rear = NULL;
  }
  char buf[1024];

  int line = 0;

  FILE* username_pass_file = NULL;
  username_pass_file = fopen(username_pass_filePath, "r+");
  if(username_pass_file == NULL) {
    printf("Can\'t open file \"%s\" !\n", username_pass_filePath);
    exit(0);
  }

  while((fgets(buf, sizeof(buf), username_pass_file)) != NULL) {
    line++;

    int checkSpace = 0;
    for(int i=0; i < strlen(buf); i++) {
      if(buf[i] == ' ') checkSpace ++;
    }

    if(checkSpace != 2) {
      printf ("Line %d: Invalid account information!", line);
      printf("\n");
      continue;
    }

    char * userName;
    char * password;
    accountStatus status;

    userName = strtok(buf, " ");
    password = strtok(NULL, " ");
    char * tmp = strtok(NULL, " ");
    tmp[strlen(tmp)-1] = '\0';

    if(strlen(tmp) > 1 || (tmp[0] > '2' || tmp[0] < '0')) {
      printf ("Line %d: There's an error in account status!", line);
      printf("\n");
      continue;
    }

    status = atoi(tmp);

    addAccountNode (userName, password, status);
  }

  if(PRINT_LOGIN_INFO_DB == 1) print_username_pass();

  fclose(username_pass_file);
}

int storeUsername_passData () {
  FILE* username_pass_file = NULL;
  username_pass_file = fopen(username_pass_filePath, "r+");
  if(username_pass_file == NULL) {
    printf("Can\'t open file \"%s\" !\n", username_pass_filePath);
    exit(0);
  }

  accountNode* tmp = accountNode_front;

  while(tmp != NULL) {
    fprintf(username_pass_file,"%s %s %d\n", tmp->userName, tmp->password, tmp->status);
    tmp = tmp->next;
  }

  fclose(username_pass_file);
  return IO_SUCCESS;
}

// ==============================================================
/*status: 1: active  - 0: blocked -  2: idle*/
int activateAccount (userNameType userName, passwordType password, char* code) {
  int res;
  accountNode* accountActivate = accessToAccount(userName, password, &res);

  if(accountActivate == NULL) {
    return 0;
  }

  switch (accountActivate->status) {
  case 0:
    return ACCOUNT_BLOCKED;
    break;
  case 1:
    return ACCOUNT_ACTIVE;
    break;
  case 2:
    if(strcmp(code, ACTIVE_CODE) == 0) {
      accountActivate->status = 1;
      printf ("Account is activated\n");
      if(PRINT_LOGIN_INFO_DB == 1) print_username_pass();
      return ACTIVE_SUCCESS;
    } else {
      accountActivate->wrongActiveCodeCount ++;

      if(accountActivate->wrongActiveCodeCount < MAX_WRONG_ACTIVE_CODE) {
	printf ("Account is not activated\n");
        return ACTIVE_ERROR;

      } else if (accountActivate->wrongActiveCodeCount == MAX_WRONG_ACTIVE_CODE) {
	accountActivate->status = 0;
	printf ("Activation code is incorrect!\n");
	printf("Account is blocked\n");
	if(PRINT_LOGIN_INFO_DB == 1) print_username_pass();
        return ACCOUNT_BLOCKED;
      }
    }
    break;
  }
}

void print_username_pass() {
  printf("-----------------------------\n");
  printf("Username-password\n\n");
  accountNode* tmp = accountNode_front;

  while(tmp != NULL) {
    printf("%s %s %d\n", tmp->userName, tmp->password, tmp->status);
    tmp = tmp->next;
  }

  printf("-----------------------------\n");
}

