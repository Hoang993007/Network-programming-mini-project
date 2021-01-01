#ifndef __QUESTION_H__
#define __QUESTION_H__

#include <stdio.h>
#include "errorCode.h"

#define QUES_MAXLEN 255
#define ANS_MAXLEN 255

typedef struct _ques_ans {
 int id;

 char ques[QUES_MAXLEN];
 char ans[ANS_MAXLEN];
 int ansLen;
 struct _ques_ans* next;
}ques_ans;

extern char quesFile[255];
extern ques_ans* ques_ans_front;
extern ques_ans* ques_ans_rear;

#endif
