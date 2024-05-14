// KNU CSE 2021114026 Jeongwoo Kim
// recent update: 240509 v0.0.7
#include <stdio.h>     
#include <string.h>
#include <stdlib.h>    
#include <unistd.h>  
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <sys/stat.h>
 
#define MAXLINE 1024

typedef unsigned long long ull;

void errOcc(char* str);

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

        /*
        if (write(server_sockfd, command, 12) <= 0){
            errOcc("write");
        }

        printf("input text: ");
        scanf("%s", text);
        buf[0] = '*';
        buf[1] = '\0';
        strcat(buf, id);
        buf[5] = '[';
        buf[6] = '\0';
        strcat(buf, text);
        if (write(server_sockfd, buf, MAXLINE) <= 0){
            errOcc("write");
            return 1;
        }
        */
    }

    /*
    memset(buf, 0x00, MAXLINE);
    if (read(server_sockfd, buf, MAXLINE) <= 0){
        errOcc("read");
        return 1;
    }
    */
 
    close(server_sockfd);
    printf("read : %s", buf);
    return 0;
}

void errOcc(char* str) {
    perror(str);

    exit(1);
}