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
#include <netinet/in.h>
#include <netdb.h>
 
#define MAXLINE 1024

typedef unsigned long long ull;

typedef struct toDo {
    char userName[16];
    int isShared; /* 1: shared 2: being shared */
    char code[9]; /* if it's being shared */

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
//static const char* default_ip = "175.201.149.127";
static char* default_addr = "jeongwookimpage.iptime.org";
static const int port = 7227;

/* client level function declared here */
int cli_init(void);
void cli_close(void);
int cli_serverConnect(char* ipaddr);
int cli_pushToDoDataToServer(toDoPtr target, char* code);
int cli_getToDoDataFromServer(toDoPtr target, const char* code);
void cli_serverClose(void);
void errOcc(char* str);
/* ----------------------------------- */

int __clDebug(void) {
    char buf[MAXLINE];
    /*char command[12];*/ int input;
    ull dt; int pr; char tt[26]; char dtl[61]; char un[16];
 
    cli_init();
    cli_serverConnect(NULL);

    while (1) {
        memset(buf, 0x00, MAXLINE);
        printf("input mode(input: 1, receive: 2): ");
        scanf("%d", &input);

        if (input == 1) {
            if (write(conn->server_sockfd, "input", 6) <= 0){
                errOcc("write");
            }
            printf("input username: ");
            scanf("%s", un);
            printf("input YYYYMMDDHHMM: ");
            scanf("%llu", &dt);
            printf("input PNUM: ");
            scanf("%d", &pr);
            printf("input title: ");
            scanf(" %[^\n]s\n", tt);
            printf("input details: ");
            scanf(" %[^\n]s\n", dtl);

            memset(buf, 0x00, MAXLINE);
            sprintf(buf, "@%-15s[*%llu[[%d]*%-25s]]%s", un, dt, pr, tt, dtl);

            /* just for R/W safety */
            if (read(conn->server_sockfd, buf, MAXLINE) < 0) {
                return 2;
            }

            if (write(conn->server_sockfd, buf, MAXLINE) <= 0){
                errOcc("write");
                return 1;
            }

            memset(buf, 0x00, MAXLINE);
            if (read(conn->server_sockfd, buf, MAXLINE) <= 0){
                errOcc("read");
                return 1;
            }

            printf("Code: %s\n", buf);
        }
        else if (input == 2) {
            if (write(conn->server_sockfd, "receive", 8) <= 0){
                errOcc("write");
            }
            printf("input code: ");
            scanf("%s", buf);

            /* just for R/W safety */
            if (read(conn->server_sockfd, buf, MAXLINE) < 0) {
                return 2;
            }

            if (write(conn->server_sockfd, buf, MAXLINE) <= 0){
                errOcc("write");
                return 1;
            }

             memset(buf, 0x00, MAXLINE);
            if (read(conn->server_sockfd, buf, MAXLINE) <= 0){
                errOcc("read");
                return 1;
            }

            printf("String: %s\n", buf);
        }
    
    }
    
    cli_close();

    return 0;
}
int cli_init(void) {
    conn = (connPtr)malloc(sizeof(connection));
    if (conn == NULL) {
        errOcc("cliinit");
    }
    conn->isOkay = 0;

    return 0;
}
void cli_close(void) {
    close(conn->server_sockfd);
    free(conn);

    return;
}
int cli_serverConnect(char* addr) {
    struct hostent* hostPtr;

    hostPtr = gethostbyname(addr);
    if (hostPtr == NULL) errOcc("gethostbyname");

    memset(&(conn->serveraddr), 0x00, sizeof(conn->serveraddr));
    //bcopy(hostPtr->h_addr_list[0], (struct sockaddr*)&conn->serveraddr, );
    memcpy(&hostPtr->h_addr_list[0], (struct sockaddr*)&conn->serveraddr, sizeof(hostPtr->h_addr_list[0]));

    if ((conn->server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        errOcc("socket");
        return 1;
    }
 
    conn->serveraddr.sin_family = AF_INET;
    conn->serveraddr.sin_port = htons(port); /* Port: 7227 fixed */
 
    conn->client_len = sizeof(conn->serveraddr);
 
    if (connect(conn->server_sockfd, (struct sockaddr *)&(conn->serveraddr), sizeof(conn->serveraddr)) == -1){
        //errOcc("connect");
        return 1; /* failed */
    }

    conn->isOkay = 1;

    return 0; /* succeeded */
}
int cli_pushToDoDataToServer(toDoPtr target, char* code) {
    int rtn;
    char buf[MAXLINE] = {'\0', };

    cli_init();
    rtn = cli_serverConnect(default_addr);
    if (rtn) return 2; /* cannot connect */

    /* sending 'input' request to the server */
    if (write(conn->server_sockfd, "input", 6) <= 0){
        return 2;
    }

    /* just for R/W safety */
    if (read(conn->server_sockfd, buf, MAXLINE) < 0) {
        return 2;
    }

    /* tokenizing */
    memset(buf, 0x00, MAXLINE);
    //printf("@%-15s[*%llu[[%d]*%-25s]]%s", target->userName, target->dateData, target->priority, target->title, target->details);
    sprintf(buf, "@%-15s[*%llu[[%d]*%-25s]]%s", target->userName, target->dateData, target->priority, target->title, target->details);
    //printf("%s\n", buf);
    /* send to the server */
    (rtn = write(conn->server_sockfd, buf, strlen(buf)));
    if (rtn < 0) return 2;
    

    /* receiving sharing code */
    memset(buf, 0x00, MAXLINE);
    if (read(conn->server_sockfd, buf, MAXLINE) <= 0){
        return 2;
    }

    strcpy(code, buf);

    cli_close();

    return 0;
}
int cli_getToDoDataFromServer(toDoPtr target, const char* code) {
    char* ptr; int rtn;
    char buf[MAXLINE] = {'\0', };
    char tempDate[13] = {'\0', };
    int i = 0;

    cli_init();
    rtn = cli_serverConnect(default_addr);
    if (rtn) return 2; /* cannot connect */

    /* sending get request to the server */
    if (write(conn->server_sockfd, "receive", strlen("receive")) < 0){
        return 2;
    }

    /* just for R/W safety */
    if (read(conn->server_sockfd, buf, MAXLINE) < 0) {
        return 2;
    }

    /* sending code */
    if (write(conn->server_sockfd, code, strlen(code)) < 0){
        return 2;
    }

    /* getting code */
    memset(buf, 0x00, MAXLINE);
    if (read(conn->server_sockfd, buf, MAXLINE) < 0){
        return 2;
    }

    /* anything received? */
    if (strcmp(buf, "NOSUCHDATA") == 0) {
        return 1; /* no such data */
    }

    //printf("buffer: %s\n", buf);

    ptr = buf;
    /* then split markup string */
    while (*ptr) {
        if (*ptr == '@') {
            ptr++;
            strncpy(target->userName, ptr, 15);
            ptr += 15;
        }
        else if (*ptr == '[') {
            ptr++;
            if (*ptr == '*') {
                ptr++;
                // YYYYMMDDHHMM 
                strncpy(tempDate, ptr, 12);
                //printf("%s\n", tempDate);
                target->dateData = atoll(tempDate);
                ptr += 12;
            }
            else if (*ptr == '[') {
                ptr++;
                target->priority = (int)*ptr - 48;
                ptr++;
                //target->priority = 0;
                //indexing++;
            }
        }
        else if (*ptr == ']') {
            ptr++;
            if (*ptr == '*') {
                ptr++;
                strncpy(target->title, ptr, 25);
                ptr += 25;
            }
            else if (*ptr == ']') {
                ptr++;
                break;
            }
        }
        //ptr++;
    }
    while (*ptr) {
        (target->details)[i++] = *ptr;
        ptr++;
    }
    (target->details)[i] = '\0';

    target->isShared = 1;

    cli_close();
    //printf("@%-15s[*%llu[[%d]*%-25s]]%s", target->userName, target->dateData, target->priority, target->title, target->details);

    return 0;
}
void cli_serverClose(void) {
    close(conn->server_sockfd);

    return;
}