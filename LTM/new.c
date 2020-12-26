#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>  //Thư viện chứa các cấu trúc cần thiết cho socket
#include <netinet/in.h> //Thư viện chứa các hằng số, cấu trúc khi sử dụng địa chỉ trên internet
#include <errno.h>
#include <stdio_ext.h>
#include <math.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(),connect(),send() and recv() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#define MAX 50
#define BUFFSIZE 256
#define PORT 8080
//#define IPadd "172.0.0.1"
#define SA struct sockaddr
struct node
{
    char ques[200];
    char ans[20];
    int stt;
    struct node *next;
};
struct node *root = NULL;
struct node *makeNode(char ques[200], char ans[20], int stt)
{
    struct node *p = (struct node *)malloc(sizeof(struct node));

    strcpy(p->ques, ques);
    strcpy(p->ans, ans);
    p->stt = stt;
    p->next = NULL;
    return p;
}
struct node *insertNode(struct node *head, char ques[200], char ans[20], int stt)
{
    struct node *cur = makeNode(ques, ans, stt);
    struct node *p = (struct node *)malloc(sizeof(struct node));
    if (head == NULL)
    {
        head = makeNode(ques, ans, stt);
        return head;
    }
    else
    {
        p = head;
        while (p->next != NULL)
        {
            p = p->next;
        }
        p->next = cur;
        return head;
    }
}
struct node *addNode(struct node *head, struct node *add)
{
    struct node *p = (struct node *)malloc(sizeof(struct node));
    if (head == NULL)
    {
        head = add;
        return head;
    }
    else
    {
        p = head;
        while (p->next != NULL)
            p = p->next;
        p->next = add;
        return head;
    }
}
void printNode(struct node *p)
{
    if (p == NULL)
        printf("node khong ton tai\n\n");
    else
        printf("node %d\n\n-> %s\n\n->%s\n\n___________\n\n", p->stt, p->ques, p->ans);
}
void printList(struct node *head)
{
    if (head == NULL)
        printf("List khong ton tai\n\n");
    else
    {
        printf("-------------------begin---------------------\n\n");
        int i = 1;
        struct node *cur = head;
        while (cur != NULL)
        {
            printNode(cur);
            cur = cur->next;
        }
        printf("----------------------end---------------------\n\n");
    }
}
struct node *findNode(struct node *head, int stt)
{
    struct node *p = head;
    int check;

    while (p != NULL)
    {

        if (p->stt == stt)
            return p;
        p = p->next;
    }
    return NULL;
}
int getNumOfNode(struct node *head)
{
    struct node *p = head;
    int dem = 0;
    if (p == NULL)
        return dem;
    else
        dem++;
    while (p->next != NULL)
    {
        dem++;
        p = p->next;
    }
    return dem;
}

struct node *deleteNode(struct node *h, int stt)
{
    struct node *head = h;
    struct node *p = head;
    int i = 1;
    struct node *find = findNode(h, stt);
    if (find == NULL)
    {
        printf("khong co node nay!\n\n");
        return h;
    }
    if (p->stt == stt)
    {
        p = p->next;
        head = p;
        return head;
    }
    else
    {
        while ((p->stt != stt) && (p->next != NULL))
        {
            p = p->next;
            // printf("khong phai %d \n\n", i);
            // i++;
        }
        if (p->next != NULL)
        {
            struct node *buff = p->next;
            free(buff);
            p->next = p->next->next;
            return h;
        }
        else if (p->next == NULL)
            printf("sai\n\n");
        return h;
    }
}
void deleteList(struct node *head)
{
    struct node *p = head;
    while (head != NULL)
    {
        root = deleteNode(head, head->stt);
        head = head->next;
    }
}
int check(char c, char xau[20]) // kiem tra ki tu nhap vao co ton tai trong xau khong
{
    int dem = 0;
    for (int i = 0; i < strlen(xau); i++)
    {
        if (xau[i] == c)
            dem++;
    }

    return dem;
}
void resetFile(FILE *f)
{
    f = fopen("account.txt", "w");
    fprintf(f, "%s", "");
    fclose(f);
}

void reWrite(char c, char str[20])
{
    FILE *f2 = fopen("dapan.txt", "w+");
    resetFile(f2);
    // fprintf(f2,"adsfdsafdsa");
    int dem = check(c, str);
    int len = strlen(str);
    if (dem != 0)
    {
        for (int i = 0; i < len * 2; i++)
        {
            if ((i % 2) == 1)
                fprintf(f2, " ");
            else
            {
                if (str[i / 2] == c)
                    fprintf(f2, "%c", c);
                else
                    fprintf(f2, "_");
            }
        }
    }
    fclose(f2);
}
void capnhat(char str[20]) // ghi ket qua hien ra vao file "dapan.txt"
{

    FILE *f2 = fopen("dapan.txt", "w+");
    resetFile(f2);
    fprintf(f2, "%s", str);
    fclose(f2);
}
void capnhat2(char str[20])
{
    FILE *f2 = fopen("ghifile.txt", "w+");
    resetFile(f2);
    fprintf(f2, "%s", str);
    fclose(f2);
}
int checkDA(char str[20]) // kiem tra trong xau co ton tai ki tu "_" khong
{
    if (check('_', str) > 0)
        return 0;
    else
        return 1;
}
void getIn(char *get) // doc xau trong file "dapan.txt"
{
    //char get[40];

    FILE *f1 = fopen("dapan.txt", "r");
    fgets(get, 100, f1);
    fclose(f1);
    //return get;
}
void getIn2(char *get)
{
    FILE *f1 = fopen("ghifile.txt", "r");
    fgets(get, 100, f1);
    fclose(f1);
}
void khoitao(char *da, char dapan[20])
{ // khoi tao da
    strcpy(da, "");
    int i;
    for (i = 0; i < strlen(dapan) * 2; i++)
    {

        if (i % 2 == 1)
        {
            strcat(da, " ");
        }
        else
        {
            da[i] = dapan[i / 2];
            da[i + 1] = '\0';
        }
    }
}
int checkCharacter(char c[20])
{
    if (strlen(c) == 1 && ((c[0] >= 'a' && c[0] <= 'z') || (c[0] >= 'A' && c[0] <= 'Z')))
        return 1;
    else
        return 0;
}
int checkArr(int arr[30], int k)
{
    for (int i = 0; i < 30; i++)
        if (arr[i] == k)
            return 1;
    return 0;
}
void docFile(FILE *f)
{
    f = fopen("question.txt", "r");
    char c;
    char ch[70], tl[20];
    char buf[10];
    int status;
    int count = 0;
    c = fgetc(f);
    while (c != EOF)
    {
        if (c == '#')
            count++;
        c = fgetc(f);
    }
    fclose(f);
    f = fopen("question.txt", "r");
    for (int i = 0; i < count; i++)
    {

        c = fgetc(f);
        fscanf(f, "%d", &status);
        fgets(buf, 10, f);
        fgets(ch, 255, f);
        fgets(tl, 255, f);
        tl[strlen(tl) - 1] = '\0';
        struct node *t = makeNode(ch, tl, status);
        root = addNode(root, t);
        //      struct node *n7 = makeNode("may biet cau nay khong?", "bochiuu", 7);

        // root = addNode(root, n1);
    }
    fclose(f);
}
int main(int argc, char *argv[])
{
    FILE *f;
    docFile(f);
    // readFile(f);
    char *port_char = argv[1];
    int port_number = atoi(port_char);
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;
    char buff[MAX];
    char send2[MAX];
    char send1[MAX];
    char recv[MAX];
    char thongbao[MAX];
    char gui[MAX];
    char guiyc[MAX];
    char nhan[MAX];
    char nhanyc[MAX];
    char nhankt[MAX];
    char in[40];
    char da[40];
    char dapan[20];
    char cauhoi[200];
    char s[40];
    char ch;
    int i;
    int j = 0;
    int list[30];
    time_t t;
    int r = 0;
    printList(root);

    int NumNode = getNumOfNode(root);

    FILE *f1;
    char st[20];

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // tạo socket
    if (sockfd == -1)
    {
        printf("socket creation failed...\n\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n\n");
    bzero(&servaddr, sizeof(servaddr)); // giống hàm memset

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port_number);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("socket bind failed...\n\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0)
    {
        printf("Listen failed...\n\n");
        exit(0);
    }
    else
        printf("Server listening..\n\n");
    len = sizeof(cli);

    // Accept the data packet from client and verification
    connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
    if (connfd < 0)
    {
        printf("server acccept failed...\n\n");
        exit(0);
    }
    else
        printf("server accept the client...\n\n");

    int k = 1;
    int count;
    int qe;
    for (k = 0; k < 30; k++)
        list[k] = -1;
    while (1)
    {

        r++;
        if (r > NumNode)
            break;

        printf(" r=  %d\n\n", r);

        struct node *tk = findNode(root, r);
        strcpy(cauhoi, tk->ques);
        strcpy(dapan, tk->ans);
        int chieudai = strlen(dapan);
        // in ra cac o trong
        strcpy(in, "");
        for (i = 0; i < chieudai; i++)
        {
            strcat(in, "_ ");
        }
        capnhat(in);

        khoitao(da, dapan);
        printf("\n\n");
        printf("da = %s \n\nlength(da) = %zu\n\n", da, strlen(da));
        printf("\n\n");

        write(connfd, cauhoi, BUFFSIZE);     // ghi ra cau hoi
      
        write(connfd, in, BUFFSIZE);         // ghi ra các ô trống 
        do
        {
              
            bzero(nhan, sizeof(nhan));               
            read(connfd, nhan, BUFFSIZE);       // đọc vào lựa chọn (1 hoặc 2)
            nhan[strlen(nhan)] = '\0';
            if (nhan[0] == '1')
            {
                read(connfd, nhan, BUFFSIZE);
                nhan[strlen(nhan)] = '\0';
                if (strcmp(nhan, dapan) == 0)
                {
                    strcpy(thongbao, "\nBạn đã trả lời đúng !");
                    write(connfd, thongbao, BUFFSIZE);
                    break;
                }
                else
                {
                    strcpy(thongbao, "\nBạn đã trả lời sai!");
                    write(connfd, thongbao, BUFFSIZE);
                    break;
                }
            }

            //write(connfd, in, sizeof(in));
            else
            {
                bzero(nhankt, sizeof(nhankt));
                qe = read(connfd, nhankt, BUFFSIZE); // doc vao ki tu

                nhankt[strlen(nhankt)] = '\0';
                printf("from client: %s\n\n", nhankt);
                strcpy(st, nhankt);
                count = check(st[0], da);
                printf("count = %d \n\n", count);
                for (i = 0; i < strlen(da); i++)
                {
                    if (da[i] == st[0])
                        in[i] = st[0];
                }
                capnhat(in);

                f1 = fopen("ghifile.txt", "w+");
                fprintf(f1, "co %d chu cai %s\n\n", count, st);
                fclose(f1);

                getIn2(gui);
                write(connfd, gui, BUFFSIZE); // in ra thong bao xem co bao nhieu ki tu vua nhap trong dap an
                getIn(in);
                // printf("in = %s\n\n", in);

                write(connfd, in, BUFFSIZE); // in ra ket qua

                strcpy(send2, "notdone");
                // printf("send2 = %s\n\n", send2);

                if (strcmp(da, in) == 0)
                { // in ra thong bao xem tra loi xong chua
                    strcpy(send1, "done");
                    write(connfd, send1, BUFFSIZE);
                }
                else
                {
                    strcpy(send1, "notdone");
                    write(connfd, send1, BUFFSIZE);
                }
            }

        } while (checkDA(in) != 1);

        strcpy(guiyc, "enter (y) to continue (n) to quit");
        write(connfd, guiyc, BUFFSIZE); // in ra cau hoi xem co muon tiep tuc choi khong
        bzero(nhanyc, sizeof(nhanyc));
        qe = read(connfd, nhanyc, BUFFSIZE);

        nhanyc[strlen(nhanyc)] = '\0';
        // printf("recv = %s strlen(recv) = %zu\n\n", nhanyc, strlen(nhanyc));

        if (nhanyc[0] == 'n')
        {
            break;
        }
    }
    deleteList(root);
    close(sockfd);

    printf("\n\n");
    resetFile(f1);
    return 0;
}