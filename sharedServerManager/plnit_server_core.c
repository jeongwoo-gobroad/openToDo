// KNU CSE 2021114026 Jeongwoo Kim
// recent update: 240509 v0.0.7
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>

#define PORT    7227
#define MAXLINE 196

#define PUT 120
#define GET 140
#define DEL 160

typedef unsigned long long ull;

typedef struct personalToDo {
    char accessKey[9];

    ull dateData;
    int priority;
    char title[26];
    char details[61];
} personalToDo;
typedef personalToDo* perToDoPtr;

typedef struct sharedDB {
    perToDoPtr* arr;
    int maxindex;
} sharedDB;

/* Global variables */
sharedDB ssvc;
const char keyPool[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 
/*---size: 22-------*/  'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'j', 'l', };
const char* bin_save = "accesspairs.ssv";
static int storefp = 0;
/*------------------*/

/* serverside db management */
void       initServerDB(void);
void       closeServerDB(void);
char*      genAccessKey(void);
int        isKeyColide(char* key);
perToDoPtr allocData(char* accKey, ull d_data, int p_data, char* t_data, char* dt_data);
int        insert(char* targetString);
perToDoPtr getData(char* key);
void       writeToFile(void);
int        readFromFile(void);
/*--------------------------*/

/* client management functions */
int   checkMode(char* str);
int   markupStringToData(ull* d_data, int* p_data, char* t_data, char* dt_data, char* targetString);
char* dataToMarkUpString(perToDoPtr target);
/*-----------------------------*/

/*----------others----------*/
void errOcc(char* why);
void safeExit(int signum);
void setSigHandler(void);
/*--------------------------*/

int main(void){
    int socket_fd, accepted_fd;
    struct sockaddr_in host_addr, client_addr;
    socklen_t size;
    int recv_length;

    char buffer[MAXLINE];
    perToDoPtr rtn;
    char* temp;

    socket_fd = socket(PF_INET,SOCK_STREAM,0);

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = 0;
    memset(&(host_addr.sin_zero), 0, 8);

    bind(socket_fd, (struct sockaddr *)&host_addr, sizeof(struct sockaddr));

    listen(socket_fd, 3);

    initServerDB();
    setSigHandler();

    while (1){
        size = sizeof(struct sockaddr_in);
        accepted_fd=accept(socket_fd, (struct sockaddr *)&client_addr, &size);

        printf("Client Info : IP %s, Port %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        /* receive */
        recv_length = 1;
        //printf("Received: [%s]\n", buffer);
        while (recv_length > 0) {
            recv_length = recv(accepted_fd, &buffer, MAXLINE, 0);
            printf("Received: [%s]\n", buffer);
            if (checkMode(buffer) == PUT) {
                puts("insert mode...");
                memset(buffer, 0x00, MAXLINE);
                recv(accepted_fd, &buffer, MAXLINE, 0);
                insert(buffer);
                write(accepted_fd, (ssvc.arr)[ssvc.maxindex]->accessKey, 9);
                memset(buffer, 0x00, MAXLINE);
                writeToFile();
            }
            else if (checkMode(buffer) == GET) {
                memset(buffer, 0x00, MAXLINE);
                recv(accepted_fd, &buffer, MAXLINE, 0);
                if ((rtn = getData(buffer)) == NULL) {
                    write(accepted_fd, "NOSUCHDATA", 11);
                }
                else {
                    temp = dataToMarkUpString(rtn);
                    write(accepted_fd, temp, strlen(temp));
                    free(temp);
                }
                memset(buffer, 0x00, MAXLINE);
            }
        } 
        


        recv_length = recv(accepted_fd, &buffer, MAXLINE, 0);
        while (recv_length > 0){
                if (buffer[0] == '*') { /* key input request */
                    printf("Key-Value : %.4s - %s\n", buffer + 1, buffer + 6);
                }
                /* key output request */
                /* data backup request */
                /* data receive request */
                //printf("From Client : %s\n", buffer);
                recv_length = recv(accepted_fd,&buffer, MAXLINE, 0);
        }

        close(accepted_fd);
    }
    return 0;
}

/*
typedef struct personalToDo {
    char accessKey[9];

    ull dateData;
    int priority;
    char title[26];
    char details[61];
} personalToDo;
typedef personalToDo* perToDoPtr;

typedef struct sharedDB {
    perToDoPtr* arr;
    int maxindex;
}
*/

/* serverside db management */
void       initServerDB(void) {
    ssvc.arr = NULL;
    ssvc.maxindex = -1;

    storefp = open(bin_save, O_SYNC | O_APPEND | O_CREAT | O_RDWR);

    readFromFile();

    return;
}
void       closeServerDB(void) {
    free(ssvc.arr);
    close(storefp);

    return;
}
char*      genAccessKey(void) {
    char* key;
    int i;

    key = (char*)malloc(sizeof(char) * 9);

    if (key == NULL) errOcc("gAK");

    srand(time(NULL));

    do {
        for (i = 0; i < 8; i++) {
            key[i] = keyPool[rand() % 22];
        }
        key[8] = '\0';
    } while (isKeyColide(key));

    return key;
}
int        isKeyColide(char* key) {
    int i;

    if (ssvc.arr == NULL) {
        return 0;
    }

    for (i = 0; i < ssvc.maxindex; i++) {
        if (strcmp((ssvc.arr)[i]->accessKey, key) == 0) {
            return 1;
        }
    }

    return 0;
}
perToDoPtr allocData(char* accKey, ull d_data, int p_data, char* t_data, char* dt_data) {
    perToDoPtr temp = (perToDoPtr)malloc(sizeof(personalToDo));

    if (temp == NULL) errOcc("allocData");

    strcpy(temp->accessKey, accKey);
    strcpy(temp->title, t_data);
    strcpy(temp->details, dt_data);

    temp->dateData = d_data;
    temp->priority = p_data;

    return temp;
}
int        insert(char* targetString) {
    ull tmpd; int tmpp; char tmpt[26]; char tmpdt[61];

    if (ssvc.arr == NULL) {
        ssvc.arr = (perToDoPtr*)malloc(sizeof(perToDoPtr));
        if (ssvc.arr == NULL) errOcc("insert");
        ssvc.maxindex = 0;
    }
    else {
        ssvc.maxindex++;
        ssvc.arr = (perToDoPtr*)realloc(ssvc.arr, sizeof(perToDoPtr) * (ssvc.maxindex + 1));
        if (ssvc.arr == NULL) errOcc("insert");
    }
    markupStringToData(&tmpd, &tmpp, tmpt, tmpdt, targetString);
    (ssvc.arr)[ssvc.maxindex] = allocData(genAccessKey(), tmpd, tmpp, tmpt, tmpdt);

    return 0;
}
perToDoPtr getData(char* key) {
    int i;

    if (ssvc.maxindex == -1) {
        return NULL;
    }

    for (i = 0; i <= ssvc.maxindex; i++) {
        if (strcmp((ssvc.arr)[i]->accessKey, key) == 0) {
            return (ssvc.arr)[i];
        }
    }

    return NULL;
}
void       writeToFile(void) { /* happens every insertion */
    //int fd = open(bin_save, O_SYNC | O_APPEND | O_CREAT | O_RDWR);
    int btw; /* bytes written */

    if ((btw = write(storefp, (ssvc.arr)[ssvc.maxindex], sizeof(personalToDo))) != sizeof(personalToDo)) {
        errOcc("wTF");
    }

    return;
}
int        readFromFile(void) {
    int btr; /* bytes read */
    perToDoPtr temp;

    temp = (perToDoPtr)malloc(sizeof(personalToDo));
    while ((btr = read(storefp, temp, sizeof(personalToDo))) > 0) {
        if (ssvc.arr == NULL) {
            ssvc.arr = (perToDoPtr*)malloc(sizeof(perToDoPtr));
            if (ssvc.arr == NULL) errOcc("insert");
            ssvc.maxindex = 0;
        }
        else {
            ssvc.maxindex++;
            ssvc.arr = (perToDoPtr*)realloc(ssvc.arr, sizeof(perToDoPtr) * (ssvc.maxindex + 1));
            if (ssvc.arr == NULL) errOcc("insert");
        }
        (ssvc.arr)[(ssvc.maxindex)] = temp;
        temp = (perToDoPtr)malloc(sizeof(personalToDo));
    } free(temp);

    if (btr == -1) {
        errOcc("rFF");
    }

    return 0;
}
/*--------------------------*/

/* client management functions */
int   checkMode(char* str) {
    if (strcmp(str, "input") == 0) {
        return PUT;
    }
    else if (strcmp(str, "receive") == 0) {
        return GET;
    }
    else if (strcmp(str, "delete") == 0) {
        return DEL;
    }

    return 0;
}
int   markupStringToData(ull* d_data, int* p_data, char* t_data, char* dt_data, char* targetString) {
    /* date: [*, prior: [[, title: ]*, details: ]] */
    int i; char* indexing;
    char tempDate[13] = {'\0', };

    indexing = targetString;
    i = 0;
    
    while (*indexing) {
        //printf("%c\n", *indexing);
        if (*indexing == '[') {
            indexing++;
            if (*indexing == '*') {
                indexing++;
                // YYYYMMDDHHMM 
                strncpy(tempDate, indexing, 12);
                //printf("%s\n", tempDate);
                *d_data = atoll(tempDate);
                indexing += 11;
            }
            else if (*indexing == '[') {
                indexing++;
                *p_data = (int)*indexing - 48;
                //indexing++;
            }
        }
        else if (*indexing == ']') {
            indexing++;
            if (*indexing == '*') {
                indexing++;
                strncpy(t_data, indexing, 25);
                indexing += 24;
            }
            else if (*indexing == ']') {
                //puts("wow");
                indexing++;
                break;
            }
        }
        indexing++;
    }
    while (*indexing) {
        dt_data[i++] = *indexing;
        indexing++;
    }
    dt_data[i] = '\0';
    
    return 0;
}
char* dataToMarkUpString(perToDoPtr target) {
    char* temp;

    temp = (char*)malloc(sizeof(char) * BUFSIZ);

    if (temp == NULL) errOcc("dTMUS");

    sprintf(temp, "[*%llu[[%d]*%-25s]]%s", target->dateData, target->priority, target->title, target->details);

    return temp;
}
/*-----------------------------*/

/*----------others----------*/
void errOcc(char* why) {
    perror(why);

    exit(1);
}
void safeExit(int signum) {
    puts("Terminating plnit-Serverside program...");

    close(storefp);

    exit(0);
}
void setSigHandler(void) {
    static struct sigaction inmode;
    static struct sigaction origin;

    inmode.sa_handler = safeExit;
    inmode.sa_flags &= ~SA_RESETHAND;
    inmode.sa_flags &= ~SA_SIGINFO;

    if (sigaction(SIGINT, &inmode, &origin) == -1) {
        errOcc("setSigHandler");
    }

    return;
}
/*--------------------------*/