#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct toDo {
    unsigned long long hashNum;
    unsigned long long dateData;
    int priority;
    char title[26];
    // SOCKET FEATURE {method}();
    char details[61];
} toDo;
typedef toDo* toDoPtr;

int main(void) {
    int cmd; int fd; const char* fn = "public.dsv";
    int idx = 0; int i; int btw;

    toDo arr[366];

    while (1) {
        printf("0: quit with save 1: force quit 2: input mode 3: listing mode\n");
        printf("input: ");
        scanf("%d", &cmd);

        if (cmd == 0 || cmd == 1) {
            break;
        }

        if (cmd == 3) {
            for (i = 0; i < idx; i++) {
                printf("%llu | %d | %s | %s \n", arr[i].dateData, arr[i].priority, arr[i].title, arr[i].details);
            }
        }

        if (cmd == 2) {
            scanf("%llu", &arr[i].dateData);
            arr[i].dateData *= 10000; arr[i].dateData += 9999;
            arr[i].priority = 0;
            scanf(" %[^\n]s\n", &arr[i].title);
            scanf(" %[^\n]s\n", &arr[i].details);
            idx += 1;
        }
    }

    if (cmd == 1) exit(0);

    fd = open(fn, O_CREAT | O_RDWR | O_TRUNC); i = 0;
    while (i < idx && (btw = write(fd, &arr[i], sizeof(arr[i])) != 0)) {
        i++;
    }
    close(fd);

    return 0;
}