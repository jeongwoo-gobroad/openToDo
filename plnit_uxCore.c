// Written by: 2023011393 Nawon Kim, KNU CSE 2021114026 Jeongwoo Kim, 2023013565 Dahye Jeong
// Recent update: 240428, v0.0.3
// terminal based calendar program ux: GUI-Like program w/curses library
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>

/**
 * SUL: Screen Upper Left, SUR: Screen Upper Right
 * SC : Screen Calendar,   SLC: Screen Left Calendar
 * SLL: Screen Lower Left, SLR: Screen Lower Right
 * 
 * each variable has a prefix 'pos_'
 * each variable has a suffix '_stt', '_end', respectively.
 * 
 * -------------------------------------   1
 * @           SUL          @|@   SUR  @   1 Line = 1
 * -------------------------------------   1
 * @                         |@            6 Lines (worst case: some months can have 6 weeks)
 *                           |              = 12n | 12n + 5
 *             SC            |    SLC
 *                           | 
 *                           | 
 *                          @|         @
 * -------------------------------------   1
 * @           SLL           |@   SLR      2 Lines (command key presentation area)
 *                          @|         @    = 2
 * -------------------------------------   1
 * 1           14n           1   7n+1   1
 *           14n + 6
 * n = MIN_of_n(LINES = 12n + 9, COLS = 21n + 10))
 * n should be a multiple of 2.
 * !!Important: COLS should be 2 times of LINES
 *      => |  10|  => numbers of a date character can be 2
 *         |0   |
*/

typedef struct pos {
    int row, col;
} pos;

static pos pos_SUL_stt, pos_SUL_end;
static pos pos_SUR_stt, pos_SUR_end;
static pos pos_SC_stt,  pos_SC_end;
static pos pos_SLC_stt, pos_SLC_end;
static pos pos_SLL_stt, pos_SLL_end;
static pos pos_SLR_stt, pos_SLR_end;

int chooseOptimal_nNum(int r, int c);
void initPosVar(int nNum);
void initScreen(int nNum);

int main(int argc, char* argv[]) {
    int nNum;

    /* lcurses start */
    initscr();

    if (LINES < 33 || COLS < 93) { /* Minimum size */
        printf("Not enough space to render a program UI\n");
        endwin();
        exit(1);
    }

    nNum = chooseOptimal_nNum(LINES, COLS);

    initPosVar(nNum);
    initScreen(nNum);
    
    mvprintw(1, 1, "Current Screen Size: %dx%d, opt.n: %d", LINES, COLS, nNum);

    refresh();

    sleep(10000);

    endwin();

    return 0;
}

int chooseOptimal_nNum(int r, int c) {
    int ln, cn;

    ln = ((r - 7) / 12) / 2;
    cn = ((c - 4) / 21) / 2;
    /* n = 2m (let m be an integer) */
    ln *= 2;
    cn *= 2;

    if (ln < cn) return ln;

    return cn;
}

void initPosVar(int nNum) {
/**
 * -------------------------------------   1
 * @           SUL          @|@   SUR  @   1 Line = 1
 * -------------------------------------   1
 * @                         |@            6 Lines (worst case: some months can have 6 weeks)
 *                           |              = 12n | 12n + 5
 *             SC            |    SLC
 *                           | 
 *                           | 
 *                          @|         @
 * -------------------------------------   1
 * @           SLL           |@   SLR      2 Lines (command key presentation area)
 *                          @|         @    = 2
 * -------------------------------------   1
 * 1           14n           1   7n+1   1
 *           14n + 6
 * n = MIN_of_n(LINES = 15n + 9, COLS = 21n + 10))
*/  /* remember that actual index is different from lib constant COLS, LINES*/
    pos_SUL_stt.row = 1; pos_SUL_stt.col = 1;
    pos_SUL_end.row = 1; pos_SUL_end.col = nNum * 14 * 2; // remember?
    pos_SUR_stt.row = 1; pos_SUR_stt.col = nNum * 14 * 2 + 2;
    pos_SUR_end.row = 1; pos_SUR_end.col = nNum * 21 * 2+ 1;
    pos_SC_stt.row = 3; pos_SC_stt.col = 1;
    pos_SC_end.row = 12 * nNum + 3; pos_SC_end.col = 14 * nNum * 2;
    pos_SLC_stt.row = 3; pos_SLC_stt.col = 14 * nNum * 2 + 2;
    pos_SLC_end.row = 12 * nNum + 3; pos_SLC_end.col = 21 * nNum * 2 + 1;
    pos_SLL_stt.row = 12 * nNum + 5; pos_SLL_stt.col = 1; 
    pos_SLL_end.row = 12 * nNum + 6; pos_SLL_end.col = 14 * nNum * 2;
    pos_SLR_stt.row = 12 * nNum + 5; pos_SLR_stt.col = 14 * nNum * 2 + 2;
    pos_SLR_end.row = 12 * nNum + 6, pos_SLR_end.col = 21 * nNum * 2 + 1;

    return;
}
void initScreen(int nNum) {
/**
pos_SUL_stt, pos_SUL_end;
pos_SUR_stt, pos_SUR_end;
pos_SC_stt,  pos_SC_end;
pos_SLC_stt, pos_SLC_end;
pos_SLL_stt, pos_SLL_end;
pos_SLR_stt, pos_SLR_end;
*/  /* remember that actual index is different from lib constant COLS, LINES*/
    int i;

    attron(COLOR_PAIR(1)); attrset(COLOR_PAIR(1));
    standout();
    for (i = 0; i <= pos_SUR_end.col + 1; i++) {
        move(0, i); addch(' ');
    }
    move(1, 0); addch(' ');
    move(1, pos_SUL_end.col + 1); addch(' ');
    move(1, pos_SUR_end.col + 1); addch(' ');
    for (int i = 0; i <= pos_SUR_end.col + 1; i++) {
        move(2, i); addch(' ');
    }

    //attron(COLOR_PAIR(2)); attrset(COLOR_PAIR(2));

    for (i = 3; i <= pos_SC_end.row; i++) {
        move(i, 0); addch(' ');
        move(i, pos_SUL_end.col + 1); addch(' ');
        move(i, pos_SUR_end.col + 1); addch(' ');
    }

    //attron(COLOR_PAIR(3)); attrset(COLOR_PAIR(3));

    for (i = 0; i <= pos_SUR_end.col + 1; i++) {
        move(pos_SLL_stt.row - 1, i); addch(' ');
    }
    for (i = pos_SLL_stt.row; i <= pos_SLL_end.row; i++) {
        move(i, 0); addch(' ');
        move(i, pos_SUL_end.col + 1); addch(' ');
        move(i, pos_SUR_end.col + 1); addch(' ');
    }
    for (i = 0; i <= pos_SUR_end.col + 1; i++) {
        move(pos_SLL_end.row + 1, i); addch(' ');
    }
    standend();
    return;
}