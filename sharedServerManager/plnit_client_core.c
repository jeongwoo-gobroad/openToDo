// KNU CSE 2021114026 Jeongwoo Kim
// recent update: 240509 v0.0.7
#include <stdio.h>     
#include <string.h>
#include <stdlib.h>    
#include <unistd.h>  
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <sys/stat.h>
#include <sys/types.h>
 
#define MAXLINE 1024

typedef unsigned long long ull;

typedef struct toDo {
    unsigned long long hashNum;
    unsigned long long dateData;
    int priority;
    char title[26];
    // SOCKET FEATURE {method}();
    char details[61];
} toDo;
typedef toDo* toDoPtr;

typedef struct connection {
    char ipaddr[16];
    struct sockaddr_in serveraddr;
    int    server_sockfd;
    int    client_len;

    int isOkay;
} connection;
typedef connection* connPtr;

static connPtr conn;

/* client level function declared here */
int cli_init(void);
int cli_serverConnect(char* ipaddr);
int cli_pushToDoDataToServer(toDoPtr target, char* code, char* usrname);
int cli_getToDoDataFromServer(toDoPtr target, const char* code, char* usrname);
void cli_serverClose(void);
void errOcc(char* str);
/* ----------------------------------- */

int main(void){
    struct sockaddr_in serveraddr;
    int server_sockfd;
    int client_len;
    char buf[MAXLINE];
    /*char command[12];*/ int input;
    ull dt; int pr; char tt[26]; char dtl[61];

    
 
    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        errOcc("socket");
        return 1;
    }
 
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("192.168.0.111");
    serveraddr.sin_port = htons(7227);
 
    client_len = sizeof(serveraddr);
 
    if (connect(server_sockfd, (struct sockaddr *)&serveraddr, client_len) == -1){
        errOcc("connect");
        return 1;
    }

    /*int   checkMode(char* str) {
    if (strcmp(str, "input")) {
        return PUT;
    }
    else if (strcmp(str, "receive")) {
        return GET;
    }
    else if (strcmp(str, "delete")) {
        return DEL;
    }
    ull dt; int pr; char tt[26]; char dt[61];
    return 0;
}
    */

    while (1) {
        memset(buf, 0x00, MAXLINE);
        printf("input mode(input: 1, receive: 2): ");
        scanf("%d", &input);

        if (input == 1) {
            if (write(server_sockfd, "input", 6) <= 0){
                errOcc("write");
            }
            printf("input YYYYMMDDHHMM: ");
            scanf("%llu", &dt);
            printf("input PNUM: ");
            scanf("%d", &pr);
            printf("input title: ");
            scanf(" %[^\n]s\n", tt);
            printf("input details: ");
            scanf(" %[^\n]s\n", dtl);

            memset(buf, 0x00, MAXLINE);
            sprintf(buf, "[*%llu[[%d]*%-25s]]%s", dt, pr, tt, dtl);

            if (write(server_sockfd, buf, MAXLINE) <= 0){
                errOcc("write");
                return 1;
            }

            memset(buf, 0x00, MAXLINE);
            if (read(server_sockfd, buf, MAXLINE) <= 0){
                errOcc("read");
                return 1;
            }

            printf("Code: %s\n", buf);
        }
        else if (input == 2) {
            if (write(server_sockfd, "receive", 8) <= 0){
                errOcc("write");
            }
            printf("input code: ");
            scanf("%s", buf);

            if (write(server_sockfd, buf, MAXLINE) <= 0){
                errOcc("write");
                return 1;
            }

             memset(buf, 0x00, MAXLINE);
            if (read(server_sockfd, buf, MAXLINE) <= 0){
                errOcc("read");
                return 1;
            }

            printf("String: %s\n", buf);
        }
 
    close(server_sockfd);
    printf("read : %s", buf);
    return 0;
}

void errOcc(char* str) {
    perror(str);

    exit(1);
}

int cli_init(void) {
    conn = (connPtr)malloc(sizeof(connection));
    if (conn == NULL) {
        errOcc("cliinit");
    }
    conn->isOkay = 0;

    return 0;
}
int cli_serverConnect(char* ipaddr) {
    if (conn->ipaddr == NULL) {
        strcpy(conn->ipaddr, "175.201.149.127"); /* default server ip address if not given */
    }

    if ((conn->server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        errOcc("socket");
        return 1;
    }
 
    conn->serveraddr.sin_family = AF_INET;
    conn->serveraddr.sin_addr.s_addr = inet_addr(conn->ipaddr);
    conn->serveraddr.sin_port = htons(7227); /* Port: 7227 fixed */
 
    conn->client_len = sizeof(serveraddr);
 
    if (connect(conn->server_sockfd, (struct sockaddr *)&(conn->serveraddr), conn->client_len) == -1){
        //errOcc("connect");
        return 1; /* failed */
    }

    conn->isOkay = 1;

    return 0; /* succeeded */
}
int cli_pushToDoDataToServer(toDoPtr target, char* code, char* usrname) {
    /*
    typedef struct toDo {
        unsigned long long hashNum;
        unsigned long long dateData;
        int priority;
        char title[26];
        // SOCKET FEATURE {method}();
        char details[61];
    } toDo;
    */
    char buf[MAXLINE] = {'\0', };

    if (write(conn->server_sockfd, "input", 6) <= 0){ /* sending 'input' request to the server */
        //errOcc("write");
        return 1;
    }

    /* tokenizing */
    sprintf(buf, "[*%llu[[%d]*%-25s]]%s", target->dateData, target->priority, target->title, target->details);

    /* send to the server */
    if (write(conn->server_sockfd, buf, MAXLINE) <= 0){
        //errOcc("write");
        return 1;
    }

    /* receiving sharing code */
    memset(buf, 0x00, MAXLINE);
    if (read(conn->server_sockfd, buf, MAXLINE) <= 0){
        //errOcc("read");
        return 1;
    }

    strcpy(code, buf);

    return 0;
}
int cli_getToDoDataFromServer(toDoPtr target, const char* code, char* usrname) {
    char buf[MAXLINE] = {'\0', };

    /* sending get request to the server */
    if (write(conn->server_sockfd, "receive", 8) <= 0){
        errOcc("write");
        return 1;
    }

    /* sending code */
    if (write(conn->server_sockfd, code, strlen(code)) <= 0){
        errOcc("write");
        return 1;
    }

    /* getting code */
    memset(buf, 0x00, MAXLINE);
    if (read(conn->server_sockfd, buf, MAXLINE) <= 0){
        errOcc("read");
        return 1;
    }

    /* anything received? */
    if (strcmp(buf, "NOSUCHDATA") == 0) {
        return 2; /* no such data */
    }

    /* then split markup string */

}
void cli_serverClose(void) {
    close(conn->server_sockfd);

    return;
}