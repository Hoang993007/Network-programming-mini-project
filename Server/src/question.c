#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../inc/services.h"
#include "../inc/accountSystem.h"
#include "../inc/question.h"

char ques_ans_filePath[255] = "./data/questionData/question.txt";

ques_ansNode* ques_ansNode_front = NULL;
ques_ansNode* ques_ansNode_rear = NULL;
int ques_ansNum = 0;

void freeQues_ansNode()
{
    ques_ansNode* tmp = ques_ansNode_front;

    while(tmp != NULL)
    {
        ques_ansNode* trash = tmp;
        tmp = trash->next;
        free(trash);
    }
}

int addQues_ansNode (int id, char* ques, char* ans)
{
    ques[strlen(ques)] = '\0';
    ans[strlen(ans)] = '\0';

    ques_ansNode* newNode = (ques_ansNode*)malloc(sizeof(ques_ansNode));

    strcpy(newNode->ques, ques);
    strcpy(newNode->ans, ans);
    if(id == -1) {
        newNode->id = ques_ansNum + 1;
        ques_ansNum++;
        }
    else newNode->id = id;
    newNode->ansLen = strlen(ans);
    newNode->next = NULL;

    if(ques_ansNode_front == NULL)
    {
        ques_ansNode_front = newNode;
        ques_ansNode_rear = ques_ansNode_front;
    }
    else if (ques_ansNode_front != NULL)
    {
        ques_ansNode_rear->next = newNode;
        ques_ansNode_rear = newNode;
    }
}

ques_ansNode* getQues_ansById (int id)
{
    ques_ansNode* tmp = ques_ansNode_front;

    while(tmp != NULL)
    {
        ques_ansNode* getNode = tmp;
        tmp = tmp->next;
        if (getNode->id == id)
        {
            return getNode;
        }
    }

    return NULL;
}

ques_ansNode* getRandomQues_ans(int avoidID[5])
{
    int ranNum;
    while(1)
    {
        time_t t;

        /* Intializes random number generator */
        srand((unsigned) time(&t));

        ranNum = rand() % ques_ansNum;

        ques_ansNode* tmp = ques_ansNode_front;

        int index = 0;
        while(index != ranNum)
        {
            tmp = tmp->next;
            index++;
        }

        for(int i = 0; i < 5; i++) {
            if(avoidID[i] >= 0 && tmp->id == avoidID[i]) {
            continue;
            }
        }
        return tmp;
    }
}

//// ============================================
//void deleteAccountNodeByUserName (char* userName) {
//  accountNode* tmp = accountNode_front;
//
//  if(strcmp(accountNode_front->userName, userName) == 0)
//    if(accountNode_front == accountNode_rear) {
//      accountNode_front = NULL;
//      accountNode_rear = NULL;
//      return;
//    } else {
//      accountNode_front = tmp->next;
//      free(tmp);
//      return;
//    }
//
//  accountNode* previousNode = tmp;
//  tmp = tmp->next;
//
//  do{
//    if((tmp->userName, userName) == 0) {
//      previousNode->next = tmp->next;
//      if(tmp == accountNode_rear) {
//	accountNode_rear = previousNode;
//      }
//      free(tmp);
//      break;
//    }
//
//    previousNode = tmp;
//    tmp = tmp->next;
//  }while(tmp != NULL);
//}

// ===========================================================
void loadQues_ansData ()
{
    if(ques_ansNode_front != NULL)
    {
        freeQues_ansNode();
        ques_ansNode_front = NULL;
        ques_ansNode_rear = NULL;
    }
    char buf[1024];

    int line = 0;

    FILE* ques_ans_file = NULL;
    ques_ans_file = fopen(ques_ans_filePath, "r+");
    if(ques_ans_file == NULL)
    {
        printf("Can\'t open file \"%s\" !\n", ques_ans_filePath);
        exit(0);
    }

    fgets(buf, sizeof(buf), ques_ans_file);
    char* ques_ansNumStr;
    ques_ansNumStr = strtok(buf, "\t");
    ques_ansNumStr = strtok(NULL, "\t");
    ques_ansNum = atoi(ques_ansNumStr);
line++;

    while((fgets(buf, sizeof(buf), ques_ans_file)) != NULL)
    {
        line++;

        char * idStr;
        idStr = strtok(buf, ".");
        idStr = strtok(NULL, ".");
        int id = atoi(idStr);

        char * get;
        char ques[QUES_MAXLEN];
        char ans[ANS_MAXLEN];

        fgets(buf, sizeof(buf), ques_ans_file);
        get = strtok(buf, "\t");
        get = strtok(NULL, "\t");
        strcpy(ques, get);
        ques[strlen(ques) - 1] = '\0';

        fgets(buf, sizeof(buf), ques_ans_file);
        get = strtok(buf, "\t");
        get = strtok(NULL, "\t");
        strcpy(ans, get);
        ans[strlen(ans) - 1] = '\0';

//NOTE: chỗ này sau khi đọc xong thì thấy qué bị lỗi, chuyển thành như ans
        addQues_ansNode(id, ques, ans);
    }

    if(PRINT_QUES_ANS_DB == 1) print_ques_ans();

    fclose(ques_ans_file);
}

int storeQues_ansData ()
{
    FILE* ques_ans_file = NULL;
    ques_ans_file = fopen(ques_ans_filePath, "r+");
    if(ques_ans_file == NULL)
    {
        printf("Can\'t open file \"%s\" !\n", ques_ans_filePath);
        exit(0);
    }

    fprintf(ques_ans_file,"Total\t%d", ques_ansNum);

    ques_ansNode* tmp = ques_ansNode_front;

    while(tmp != NULL)
    {
        fprintf(ques_ans_file,"No.%d\nQues\t%s\nAns%s\n", tmp->id, tmp->ques, tmp->ans);
        tmp = tmp->next;
    }

    fclose(ques_ans_file);
    return IO_SUCCESS;
}


void print_ques_ans()
{
    printf("-----------------------------\n");
    printf("Ques-ans\n\n");
    ques_ansNode* tmp = ques_ansNode_front;
    printf("Total\t%d\n\n", ques_ansNum);
    while(tmp != NULL)
    {
        printf("No.%d\nQues\t%s\nAns\t%s\n", tmp->id, tmp->ques, tmp->ans);
        tmp = tmp->next;
    }

    printf("-----------------------------\n");
}
