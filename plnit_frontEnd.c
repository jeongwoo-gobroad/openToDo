// KNU CSE 2021114026 Jeongwoo Kim
// recent update: 240428 v0.0.3
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define DEBUG 1
#define INSERT 2
#define UX_DEBUG 3
#define NO_DEF 4

/* Basic functions */
int options(int argc, char* argv[]) {
    if (argc == 2 && strcmp(argv[1], "-d")) {
        return DEBUG;
    }
    else if (argc == 3 && strcmp(argv[1], "-ux")) {

    }
}

int main(int argc, char* argv[]) {
    int rtn;

}