#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../inc/services.h"
#include "../inc/accountSystem.h"

char quesFile[255] = "./../data/questionData/question.txt";

ques_ans* ques_ans_front = NULL;
ques_ans* ques_ans_rear = NULL;


//void freeAccountNode() {
//  accountNode* tmp = accountNode_front;
//
//  while(tmp != NULL) {
//    accountNode* trash = tmp;
//    tmp = trash->next;
//    free(trash);
//  }
//}
//
//void accountRegister (userNameType newUserName, passwordType password) {
//  addAccountNode(newUserName, password, IDLE);
//  printf("Successful registration. Activation required.\n");
//
//  if(PRINT_LOGIN_INFO_DB == 1) print_username_pass();
//}
//
//// add new account -----------------------------------------
//int addAccountNode (char* userName, char* password, accountStatus status) {
//  userName[strlen(userName)] = '\0';
//  password[strlen(password)] = '\0';
//
//  if(isExistUserName(userName) == ACCOUNT_EXIST) {
//    return ACCOUNT_EXIST;
//  }
//
//  accountNode* newNode = (accountNode*)malloc(sizeof(accountNode));
//
//  strcpy(newNode->userName, userName);
//  strcpy(newNode->password, password);
//  newNode->status = status;
//  newNode->wrongActiveCodeCount = 0;
//  newNode->wrongPassCount = 0;
//
//newNode->isLogined = 0;
//
//  newNode->next = NULL;
//
//  if(accountNode_front == NULL) {
//    accountNode_front = newNode;
//    accountNode_rear = accountNode_front;
//  } else if (accountNode_front != NULL) {
//    accountNode_rear->next = newNode;
//    accountNode_rear = newNode;
//  }
//}
//
//// ============================================
//accountNode* getAccountNodeByUserName (char* userName) {
//  accountNode* tmp = accountNode_front;
//
//  while(tmp != NULL) {
//    accountNode* getNode = tmp;
//    tmp = tmp->next;
//    if (strcmp(getNode->userName, userName) == 0) {
//      return getNode;
//    }
//  }
//
//  return NULL;
//}
//
//int isExistUserName(char* userName) {
//  if(getAccountNodeByUserName(userName) != NULL) {
//    return ACCOUNT_EXIST;
//  }
//  return ACCOUNT_NOT_EXIST;
//}
//
//// ===================================
//int checkPassword(accountNode* accountCheck, char* password) {
//  if(accountCheck->status == IDLE){
//    printf ("Account is not activated\n");
//    return ACCOUNT_IDLE;
//  }
//  else if(accountCheck->status == BLOCKED) {
//    printf ("Account is blocked\n");
//    return ACCOUNT_BLOCKED;
//  }
//  else if(strcmp(password, accountCheck->password) == 0) {
//
//    return PASS_CORRECT;
//  }
//  else {
//    printf ("Password is incorrect\n");
//    accountCheck->wrongPassCount++;
//
//    if(accountCheck->wrongPassCount == MAX_WRONG_PASS) {
//      accountCheck->status = BLOCKED;
//      return ACCOUNT_JUST_BLOCKED;
//    }
//
//    return PASS_INCORRECT;
//  }
//}
//
//accountNode* accessToAccount (userNameType userName, passwordType password, int *res) {
//  accountNode* accountAccess = getAccountNodeByUserName(userName);
//
//  *res = checkPassword(accountAccess, password);
//
//  if(*res == PASS_CORRECT){
//    printf("Access to account %s sucessfully\n", accountAccess->userName);
//    return accountAccess;
//  }
//  else {
//    return NULL;
//  }
//}
//
//// ===================================
//void accountChangePass (accountNode* accountModify, char* newPass) {
//  strcpy(accountModify->password, newPass);
//  if(PRINT_LOGIN_INFO_DB == 1) print_username_pass();
//};
//
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
//
//// ===========================================================
//void loadUsername_passData () {
//  if(accountNode_front != NULL) {
//    freeAccountNode();
//    accountNode_front = NULL;
//    accountNode_rear = NULL;
//  }
//  char buf[1024];
//
//  int line = 0;
//
//  FILE* username_pass_file = NULL;
//  username_pass_file = fopen(username_pass_filePath, "r+");
//  if(username_pass_file == NULL) {
//    printf("Can\'t open file \"%s\" !\n", username_pass_filePath);
//    exit(0);
//  }
//
//  while((fgets(buf, sizeof(buf), username_pass_file)) != NULL) {
//    line++;
//
//    char * userName;
//    char * password;
//    accountStatus status;
//
//    userName = strtok(buf, ";");
//    password = strtok(NULL, ";");
//    char * tmp = strtok(NULL, ";");
//    tmp[strlen(tmp)-1] = '\0';
//
//    if(strlen(tmp) > 1 || (tmp[0] > '2' || tmp[0] < '0')) {
//      printf ("Line %d: There's an error in account status!", line);
//      printf("\n");
//      continue;
//    }
//
//    status = atoi(tmp);
//
//    addAccountNode (userName, password, status);
//  }
//
//  if(PRINT_LOGIN_INFO_DB == 1) print_username_pass();
//
//  fclose(username_pass_file);
//}
//
//int storeUsername_passData () {
//  FILE* username_pass_file = NULL;
//  username_pass_file = fopen(username_pass_filePath, "r+");
//  if(username_pass_file == NULL) {
//    printf("Can\'t open file \"%s\" !\n", username_pass_filePath);
//    exit(0);
//  }
//
//  accountNode* tmp = accountNode_front;
//
//  while(tmp != NULL) {
//    fprintf(username_pass_file,"%s;%s;%d\n", tmp->userName, tmp->password, tmp->status);
//    tmp = tmp->next;
//  }
//
//  fclose(username_pass_file);
//  return IO_SUCCESS;
//}
//
//// ==============================================================
///*status: 1: active  - 0: blocked -  2: idle*/
//int activateAccount (userNameType userName, passwordType password, char* code) {
//  int res;
//  accountNode* accountActivate = accessToAccount(userName, password, &res);
//
//  if(accountActivate == NULL) {
//    return 0;
//  }
//
//  switch (accountActivate->status) {
//  case 0:
//    return ACCOUNT_BLOCKED;
//    break;
//  case 1:
//    return ACCOUNT_ACTIVE;
//    break;
//  case 2:
//    if(strcmp(code, ACTIVE_CODE) == 0) {
//      accountActivate->status = 1;
//      printf ("Account is activated\n");
//      if(PRINT_LOGIN_INFO_DB == 1) print_username_pass();
//      return ACTIVE_SUCCESS;
//    } else {
//      accountActivate->wrongActiveCodeCount ++;
//
//      if(accountActivate->wrongActiveCodeCount < MAX_WRONG_ACTIVE_CODE) {
//	printf ("Account is not activated\n");
//        return ACTIVE_ERROR;
//
//      } else if (accountActivate->wrongActiveCodeCount == MAX_WRONG_ACTIVE_CODE) {
//	accountActivate->status = 0;
//	printf ("Activation code is incorrect!\n");
//	printf("Account is blocked\n");
//	if(PRINT_LOGIN_INFO_DB == 1) print_username_pass();
//        return ACCOUNT_BLOCKED;
//      }
//    }
//    break;
//  }
//}
//
//void print_username_pass() {
//  printf("-----------------------------\n");
//  printf("Username-password\n\n");
//  accountNode* tmp = accountNode_front;
//
//  while(tmp != NULL) {
//    printf("%s %s %d\n", tmp->userName, tmp->password, tmp->status);
//    tmp = tmp->next;
//  }
//
//  printf("-----------------------------\n");
//}
//
//
//
//
//struct node *makeNode(char ques[200], char ans[20], int stt)
//{
//    struct node *p = (struct node *)malloc(sizeof(struct node));
//
//    strcpy(p->ques, ques);
//    strcpy(p->ans, ans);
//    p->stt = stt;
//    p->next = NULL;
//    return p;
//}
//struct node *insertNode(struct node *head, char ques[200], char ans[20], int stt)
//{
//    struct node *cur = makeNode(ques, ans, stt);
//    struct node *p = (struct node *)malloc(sizeof(struct node));
//    if (head == NULL)
//    {
//        head = makeNode(ques, ans, stt);
//        return head;
//    }
//    else
//    {
//        p = head;
//        while (p->next != NULL)
//        {
//            p = p->next;
//        }
//        p->next = cur;
//        return head;
//    }
//}
//struct node *addNode(struct node *head, struct node *add)
//{
//    struct node *p = (struct node *)malloc(sizeof(struct node));
//    if (head == NULL)
//    {
//        head = add;
//        return head;
//    }
//    else
//    {
//        p = head;
//        while (p->next != NULL)
//            p = p->next;
//        p->next = add;
//        return head;
//    }
//}
//void printNode(struct node *p)
//{
//    if (p == NULL)
//        printf("node khong ton tai\n\n");
//    else
//        printf("node %d\n\n-> %s\n\n->%s\n\n___________\n\n", p->stt, p->ques, p->ans);
//}
//void printList(struct node *head)
//{
//    if (head == NULL)
//        printf("List khong ton tai\n\n");
//    else
//    {
//        printf("-------------------begin---------------------\n\n");
//        int i = 1;
//        struct node *cur = head;
//        while (cur != NULL)
//        {
//            printNode(cur);
//            cur = cur->next;
//        }
//        printf("----------------------end---------------------\n\n");
//    }
//}
//struct node *findNode(struct node *head, int stt)
//{
//    struct node *p = head;
//    int check;
//
//    while (p != NULL)
//    {
//
//        if (p->stt == stt)
//            return p;
//        p = p->next;
//    }
//    return NULL;
//}
//int getNumOfNode(struct node *head)
//{
//    struct node *p = head;
//    int dem = 0;
//    if (p == NULL)
//        return dem;
//    else
//        dem++;
//    while (p->next != NULL)
//    {
//        dem++;
//        p = p->next;
//    }
//    return dem;
//}
//
//struct node *deleteNode(struct node *h, int stt)
//{
//    struct node *head = h;
//    struct node *p = head;
//    int i = 1;
//    struct node *find = findNode(h, stt);
//    if (find == NULL)
//    {
//        printf("khong co node nay!\n\n");
//        return h;
//    }
//    if (p->stt == stt)
//    {
//        p = p->next;
//        head = p;
//        return head;
//    }
//    else
//    {
//        while ((p->stt != stt) && (p->next != NULL))
//        {
//            p = p->next;
//            // printf("khong phai %d \n\n", i);
//            // i++;
//        }
//        if (p->next != NULL)
//        {
//            struct node *buff = p->next;
//            free(buff);
//            p->next = p->next->next;
//            return h;
//        }
//        else if (p->next == NULL)
//            printf("sai\n\n");
//        return h;
//    }
//}
//void deleteList(struct node *head)
//{
//    struct node *p = head;
//    while (head != NULL)
//    {
//        root = deleteNode(head, head->stt);
//        head = head->next;
//    }
//}
//int check(char c, char xau[20]) // kiem tra ki tu nhap vao co ton tai trong xau khong
//{
//    int dem = 0;
//    for (int i = 0; i < strlen(xau); i++)
//    {
//        if (xau[i] == c)
//            dem++;
//    }
//
//    return dem;
//}
//void resetFile(FILE *f)
//{
//    f = fopen("account.txt", "w");
//    fprintf(f, "%s", "");
//    fclose(f);
//}
//
//void reWrite(char c, char str[20])
//{
//    FILE *f2 = fopen("dapan.txt", "w+");
//    resetFile(f2);
//    // fprintf(f2,"adsfdsafdsa");
//    int dem = check(c, str);
//    int len = strlen(str);
//    if (dem != 0)
//    {
//        for (int i = 0; i < len * 2; i++)
//        {
//            if ((i % 2) == 1)
//                fprintf(f2, " ");
//            else
//            {
//                if (str[i / 2] == c)
//                    fprintf(f2, "%c", c);
//                else
//                    fprintf(f2, "_");
//            }
//        }
//    }
//    fclose(f2);
//}
//void capnhat(char str[20]) // ghi ket qua hien ra vao file "dapan.txt"
//{
//
//    FILE *f2 = fopen("dapan.txt", "w+");
//    resetFile(f2);
//    fprintf(f2, "%s", str);
//    fclose(f2);
//}
//void capnhat2(char str[20])
//{
//    FILE *f2 = fopen("ghifile.txt", "w+");
//    resetFile(f2);
//    fprintf(f2, "%s", str);
//    fclose(f2);
//}
//int checkDA(char str[20]) // kiem tra trong xau co ton tai ki tu "_" khong
//{
//    if (check('_', str) > 0)
//        return 0;
//    else
//        return 1;
//}
//void getIn(char *get) // doc xau trong file "dapan.txt"
//{
//    //char get[40];
//
//    FILE *f1 = fopen("dapan.txt", "r");
//    fgets(get, 100, f1);
//    fclose(f1);
//    //return get;
//}
//void getIn2(char *get)
//{
//    FILE *f1 = fopen("ghifile.txt", "r");
//    fgets(get, 100, f1);
//    fclose(f1);
//}
//void khoitao(char *da, char dapan[20])
//{ // khoi tao da
//    strcpy(da, "");
//    int i;
//    for (i = 0; i < strlen(dapan) * 2; i++)
//    {
//
//        if (i % 2 == 1)
//        {
//            strcat(da, " ");
//        }
//        else
//        {
//            da[i] = dapan[i / 2];
//            da[i + 1] = '\0';
//        }
//    }
//}
//int checkCharacter(char c[20])
//{
//    if (strlen(c) == 1 && ((c[0] >= 'a' && c[0] <= 'z') || (c[0] >= 'A' && c[0] <= 'Z')))
//        return 1;
//    else
//        return 0;
//}
//int checkArr(int arr[30], int k)
//{
//    for (int i = 0; i < 30; i++)
//        if (arr[i] == k)
//            return 1;
//    return 0;
//}
//void docFile(FILE *f)
//{
//    f = fopen("question.txt", "r");
//    char c;
//    char ch[70], tl[20];
//    char buf[10];
//    int status;
//    int count = 0;
//    c = fgetc(f);
//    while (c != EOF)
//    {
//        if (c == '#')
//            count++;
//        c = fgetc(f);
//    }
//    fclose(f);
//    f = fopen("question.txt", "r");
//    for (int i = 0; i < count; i++)
//    {
//
//        c = fgetc(f);
//        fscanf(f, "%d", &status);
//        fgets(buf, 10, f);
//        fgets(ch, 255, f);
//        fgets(tl, 255, f);
//        tl[strlen(tl) - 1] = '\0';
//        struct node *t = makeNode(ch, tl, status);
//        root = addNode(root, t);
//        //      struct node *n7 = makeNode("may biet cau nay khong?", "bochiuu", 7);
//
//        // root = addNode(root, n1);
//    }
//    fclose(f);
//}
//int main(int argc, char *argv[])
//{
//    FILE *f;
//    docFile(f);
//    // readFile(f);
//
//
//    char thongbao[MAX];
//    char gui[MAX];
//    char guiYeuCau[MAX];
//    char guiyc[MAX];
//    char nhan[MAX];
//    char nhanyc[MAX];
//    char nhanYeuCau[MAX];
//    char nhanKiTu[MAX];
//    char guiThongBao[MAX];
//    char choice[MAX];
//    char in[40];
//    char da[40];
//    char dapan[20];
//    char cauhoi[200];
//    char crossWord[40];
//    char s[40];
//    char ch;
//    int i;
//    int j = 0;
//    int list[30];
//    time_t t;
//    int numberQuestion = 0;
//    printList(root);
//
//    int NumNode = getNumOfNode(root);
//
//    FILE *f1;
//    char st[20];
//
//
//
//    int k = 1;
//    int countCharacter;
//    int qe;
//    for (k = 0; k < 30; k++)
//        list[k] = -1;
//    while (1)                               // vòng lặp thực hiện cho đến khi hết câu hỏi hoặc client quit
//    {
//
//        numberQuestion++;                       // numberQuestion : số thứ tự của câu hỏi trong list
//        if (numberQuestion > NumNode){          // nếu hết câu hỏi -> thông báo cho client và thoát
//            strcpy(thongbao,"exit");
//            write(connfd, thongbao, BUFFSIZE);
//            break;
//        }
//         else{
//            strcpy(thongbao,"continue");
//            write(connfd, thongbao, BUFFSIZE);
//        }
//
//
//
//        printf(" numberQuestion =  %d\n\n", numberQuestion);
//        struct node *tk = findNode(root, numberQuestion);
//        strcpy(cauhoi, tk->ques);
//        strcpy(dapan, tk->ans);
//        int lengthOfCrossWord = strlen(dapan);
//
//        strcpy(crossWord, "");
//        for (i = 0; i < lengthOfCrossWord; i++)
//        {
//            strcat(crossWord, "_ ");
//        }
//        capnhat(crossWord);                                // cập nhật file "dapan.txt"
//
//
//
//        khoitao(da, dapan);                         // biến đáp án thành dạng có dấu " "
//                                                    // ví dụ đáp án là  dapan = "bachkhoa" (viết liền )
//                                                    // => da = " b a c h k h o a"
//        printf("\n\n");
//        printf("da = %s \n\nlength(da) = %zu\n\n", da, strlen(da));
//        printf("\n\n");
//
//        write(connfd, cauhoi, BUFFSIZE);           // gửi câu hỏi tới client
//
//        write(connfd, crossWord, BUFFSIZE);               // gửi các ô trống tới client
//        do
//        {
//
//            bzero(choice, sizeof(choice));
//            read(connfd,choice, BUFFSIZE);       // đọc vào lựa chọn (1 hoặc 2)
//                                                  // lựa chọn 1 : client chọn trả lời toàn bộ đáp án
//                                                  // lựa chọn 2 : client chọn trả lời từng chữ cái
//            choice[strlen(choice)] = '\0';
//
//
//            if (choice[0] == '1')                // client chọn trả lời toàn bộ đáp án
//            {
//                read(connfd, nhan, BUFFSIZE);     // nhận vào đáp án từ client
//                nhan[strlen(nhan)] = '\0';
//                if (strcmp(nhan, dapan) == 0)     // trả lời đúng -> thông báo tới client ->kết thúc câu hỏi hiện tại
//
//                {
//                    strcpy(thongbao, "\nBạn đã trả lời đúng !");
//                    write(connfd, thongbao, BUFFSIZE);
//                    break;
//                }
//                else
//                {
//                    strcpy(thongbao, "\nBạn đã trả lời sai!");
//                    write(connfd, thongbao, BUFFSIZE);
//                    break;
//                }
//            }
//            else                                          // chọn trả lời từng kí tự
//            {
//                bzero(nhanKiTu, sizeof(nhanKiTu));
//                qe = read(connfd, nhanKiTu, BUFFSIZE);    // Đọc vào kí tự nhập vào từ client
//
//                nhanKiTu[strlen(nhanKiTu)] = '\0';
//                printf("from client: %s\n\n", nhanKiTu);
//                //strcpy(st, nhanKiTu);
//                countCharacter = check(nhanKiTu[0], da);
//                printf("count = %d \n\n", countCharacter);
//                for (i = 0; i < strlen(da); i++)
//                {
//                    if (da[i] == nhanKiTu[0])
//                        crossWord[i] = nhanKiTu[0];
//                }
//                capnhat(crossWord);                              // cập nhật "dapan".txt"
//
//                f1 = fopen("ghifile.txt", "w+");          // cập nhật file "ghifile.txt"
//                fprintf(f1, "có %d chữ cái %s\n\n", countCharacter,nhanKiTu);
//                fclose(f1);
//
//                getIn2(guiThongBao);                      // đọc nội dung file "ghifile.txt" ( kí tự và nhập xuất hiên bn lần)
//                write(connfd, guiThongBao, BUFFSIZE);     // gửi nội dung file "ghifile.txt" tới client
//
//
//                getIn(crossWord);                                // đọc nội dung file "dapan.txt"
//                write(connfd, crossWord, BUFFSIZE);              // gửi nội dung file "dapan.txt" tới client
//
//                if (strcmp(da, crossWord) == 0)                  // kiểm tra đáp án đúng và đáp án in ra cho các client
//                                                          // đã giống nhau chưa
//                {
//                    strcpy(send1, "done");                // nếu đã giống nhau gửi tới client là "done"
//                    write(connfd, send1, BUFFSIZE);
//                }
//                else                                      // nếu chưa giống nhau gửi tới client là "notdone"
//                {
//                    strcpy(send1, "notdone");
//                    write(connfd, send1, BUFFSIZE);
//                }
//            }
//
//        } while (checkDA(crossWord) != 1);                       // kiểm tra xem các ô chữ đã được mở hết chưa
//
//        // Sau khi toàn bộ ô chữ đã được mở
//        strcpy(guiYeuCau, "enter (y) to continue (n) to quit");
//        write(connfd, guiYeuCau, BUFFSIZE);               // server hỏi client xem có muốn tiếp tục chơi không
//
//        bzero(nhanYeuCau, sizeof(nhanYeuCau));
//        qe = read(connfd, nhanYeuCau, BUFFSIZE);          // nhận yêu cầu từ client
//        nhanYeuCau[strlen(nhanYeuCau)] = '\0';            // kí tự đọc vào là "n" thì kết thúc
//                                                          // kí tự đọc vào là "y" tiếp tục chơi với câu hỏi mới
//        // if (nhanYeuCau[0] == 'n')
//        // {
//
//        //     break;
//        // }
//    }                           // kết thúc vòng lặp while
//    deleteList(root);                                     // xóa đi Linkedlist
//    close(sockfd);
//
//    printf("\n\n");
//    resetFile(f1);                                        // xóa nội dung file f1
//    return 0;
//}
