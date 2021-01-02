#ifndef __QUESTION_H__
#define __QUESTION_H__

#include <stdio.h>
#include <stdlib.h>

#include "errorCode.h"

#define PRINT_QUES_ANS_DB 1 // for testing

#define QUES_MAXLEN 255
#define ANS_MAXLEN 255

typedef struct _ques_ans {
 int id;

 char ques[QUES_MAXLEN];
 char ans[ANS_MAXLEN];
 int ansLen;
 struct _ques_ans* next;
}ques_ansNode;

extern char ques_ans_filePath[255];
extern ques_ansNode* ques_ans_front;
extern ques_ansNode* ques_ans_rear;

extern int ques_ansNum;

void freeQues_ansNode();
int addQues_ansNode (int id, char* ques, char* ans);
ques_ansNode* getQues_ansById (int id);
ques_ansNode* getRandomQues_ans(int avoidID[5]);
void loadQues_ansData ();
int storeQues_ansData ();
void print_ques_ans();

#endif
