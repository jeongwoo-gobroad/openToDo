// KNU CSE 2021114026 Jeongwoo Kim
// recent update: 240530 v1.0.1 build 32
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

/* client level function declared here */
int cli_init(void);
void cli_close(void);
int cli_serverConnect(char* ipaddr);
//int cli_pushToDoDataToServer(toDoPtr target, char* code);
//int cli_getToDoDataFromServer(toDoPtr target, const char* code);
void cli_serverClose(void);
void errOcc(char* str);
/* ----------------------------------- */

/*--server-client interaction related APIs---------------*/
int __clDebug(void);
int cli_pushToDoDataToServer(toDoPtr target, char* code);
int cli_getToDoDataFromServer(toDoPtr target, const char* code);