// Written by: 2023011393 Nawon Kim, KNU CSE 2021114026 Jeongwoo Kim, 2023013565 Dahye Jeong
// terminal based calendar program ux: GUI-Like program w/curses library
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <time.h>
#include <string.h>

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
 * @                         |@            6 Lines  + 1(worst case: some months can have 6 weeks)
 *                           |              = 6n + 7
 *             SC            |    SLC
 *                           | 
 *                           | 
 *                          @|         @
 * -------------------------------------   1
 * @           SLL           |@   SLR      2 Lines (command key presentation area)
 *                          @|         @    = 2
 * -------------------------------------   1
 * 1           14n + 6       1   7n + 1    1
 *          
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
static pos pos_SC_date[6][7];

unsigned long long selectDate;
int selectDate_row;
int selectDate_col;
int nNum;

void chooseOptimal_nNum(int r, int c);
void initPosVar();
void initScreen();

//--------------------------------------------------
//date형식은 YYYYMMDDHHMM (print_Year_Month, daysInMonth, weeksInMonth, 는 YYYYMM)
unsigned long long get_today_date();
void print_Year_Month(unsigned long long targetDate); //연도와 달 출력
int daysInMonth(unsigned long long targetDate); //해당 달에 포함된 날 수 반환
int stt_day_1(unsigned long long targetDate); //1일이 시작하는 요일을 정수로 반환 (0 : sunday)
int weeksInMonth(unsigned long long targetDate); //해당 달에 포함된 주 수 반환
void print_date(unsigned long long targetdate); //달력에 날짜 출력
void print_date_NumOfSchedule(unsigned long long targetDate); //달력에 날짜에 해당하는 일정 개수 출력
void print_commandLine(int mode); //커맨드라인을 모드에 따라서 출력(mode = 0:normal, 1:select, 2:insert)
void select_today();
void select_date(char c);
void select_highlightOn();
void prev_select_highlightOff();
void get_todo();
int save(void);
int load(void);

/*--UX Layer interactive API Methods---------------------*/

int getNumOfSchedule(unsigned long long targetDate);
void getUpcomingSchedule(unsigned long long today, char* strbuf, int scrSize);
void setSchedule(unsigned long long today, char* title, char* details, int priority);
void getTodaySchedule(unsigned long long today, int sortType, char* strbuf, int scrSize); /* debugging only feature */

void getTodaySchedule_Summarized(unsigned long long today, int sortType, char* strbuf, int maxLines, int width);
int getTodaySchedule_withDetails(unsigned long long today, int sortType, char* strbuf, int maxLines, int width);
void getTodaySchedule_withDetails_iterEnd(void);

int __dbDebug(void);
void __launchOptions(int argc, char* argv[]);

/*-------------------------------------------------------*/

int main(int argc, char* argv[]) {

    /* debuging options */
    if (argc > 1) {
        __launchOptions(argc, argv);
    }

    /* lcurses start */
    load();
    initscr();
    noecho();

    if (LINES < 33 || COLS < 93) { /* Minimum size */
        printf("Not enough space to render a program UI\n");
        endwin();
        exit(1);
    }

    chooseOptimal_nNum(LINES, COLS);

    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_WHITE);

    initPosVar();
    initScreen();
    select_highlightOn(selectDate);
    
    //mvprintw(1, 1, "Current Screen Size: %dx%d, opt.n: %d", LINES, COLS, nNum);

    refresh();

    char c;
    int mode = 0;
    while (1) {
        if (mode == 0) {
            c = getch();
            if (c == 's') mode++;
            if (c == 'i' || c == 'j' || c == 'k' || c == 'l')
                select_date(c);
            if (c=='z') save();
            if (c=='x') load();
        }
        else if (mode == 1) {
            c = getch();
            if (c == 'I') mode++;
            if (c == 'e') mode--;
        }
        else if (mode == 2) {
            if (c == 'e') mode--;
            get_todo();
            c = getch();
        }
        print_commandLine(mode);
        refresh();
    }


    sleep(10000);

    endwin();

    return 0;
}

void chooseOptimal_nNum(int r, int c) {
    int rn, cn;

    rn = ((r - 14) / 12) * 2;
    cn = ((c - 10) / 42) * 2;
    /* n = 2m (let m be an integer) */

    if (rn < cn) {
        nNum = rn;
        return;
    }
    
    nNum = cn;
    return;
}

void initPosVar() {
/**
 * -------------------------------------   1
 * @           SUL          @|@   SUR  @   1 Line = 1
 * -------------------------------------   1
 * @                         |@            6 Lines  + 1(worst case: some months can have 6 weeks)
 *                           |              = 6n + 7
 *             SC            |    SLC
 *                           | 
 *                           | 
 *                          @|         @
 * -------------------------------------   1
 * @           SLL           |@   SLR      2 Lines (command key presentation area)
 *                          @|         @    = 2
 * -------------------------------------   1
 * 1           14n + 6       1   7n + 1    1
 *          
 * n = MIN_of_n(LINES = 6n + 14, COLS = 21n + 10))
*/  /* remember that actual index is different from lib constant COLS, LINES*/
    pos_SUL_stt.row = 1; pos_SUL_stt.col = 1;
    pos_SUL_end.row = 1; pos_SUL_end.col = nNum * 14 * 2 + 6; // *2는 가시성을 높이기 위해서
    pos_SUR_stt.row = 1; pos_SUR_stt.col = pos_SUL_end.col + 2;
    pos_SUR_end.row = 1; pos_SUR_end.col = pos_SUR_stt.col + nNum * 7 * 2; //*2는 가시성을 높이기 위해서
    pos_SC_stt.row = 3; pos_SC_stt.col = 1;
    pos_SC_end.row = 6 * nNum + 9; pos_SC_end.col = pos_SUL_end.col;
    pos_SLC_stt.row = 3; pos_SLC_stt.col = pos_SUR_stt.col;
    pos_SLC_end.row = 6 * nNum + 9; pos_SLC_end.col = pos_SUR_end.col;
    pos_SLL_stt.row = 6 * nNum + 11; pos_SLL_stt.col = 1; 
    pos_SLL_end.row = 6 * nNum + 12; pos_SLL_end.col = pos_SUL_end.col;
    pos_SLR_stt.row = 6 * nNum + 11; pos_SLR_stt.col = pos_SUR_stt.col;
    pos_SLR_end.row = 6 * nNum + 12, pos_SLR_end.col = pos_SUR_end.col;

    for (int i = 0, r = 5; i < 6; i++, r += nNum + 1) {
        for (int j = 0, c = 1; j < 7; j++, c += 4 * nNum + 1) {
            pos_SC_date[i][j].row = r;
            pos_SC_date[i][j].col = c;
        }
    }

    return;
}
void initScreen() {
/**
pos_SUL_stt, pos_SUL_end;
pos_SUR_stt, pos_SUR_end;
pos_SC_stt,  pos_SC_end;
pos_SLC_stt, pos_SLC_end;
pos_SLL_stt, pos_SLL_end;
pos_SLR_stt, pos_SLR_end;
pos_SC_date;
*/  /* remember that actual index is different from lib constant COLS, LINES*/
    int i, j;

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
    for (i = 1; i < 7; i++) {
        for (j = pos_SC_stt.row; j <= pos_SC_end.row; j++) {
            move(j, pos_SC_date[0][i].col - 1);
            addch('|');
        }
    }
    for (i = 0; i < 6; i++) {
        for (j = pos_SUL_stt.col; j <= pos_SUL_end.col; j++) {
            move(pos_SC_date[i][0].row - 1, j);
            addch('-');
        }
    }
    attron(A_BOLD);
    for (i = 0; i < 7; i++) {
        move(pos_SC_date[0][i].row - 2, pos_SC_date[0][i].col);
        switch (i) {
        case 0: addstr("Sun"); break;
        case 1: addstr("Mon"); break;
        case 2: addstr("Tue"); break;
        case 3: addstr("Wed"); break;
        case 4: addstr("Thu"); break; 
        case 5: addstr("Fri"); break;
        case 6: addstr("Sat"); break;
        }
    }
    attroff(A_BOLD);
    select_today();
    print_Year_Month(selectDate / 1000000);

    print_date(selectDate);
    print_date_NumOfSchedule(selectDate);
    print_commandLine(0);
    return;
}

void select_today() {
    time_t t = time(NULL);
    struct tm timeinfo = *localtime(&t);

    unsigned long long year = timeinfo.tm_year + 1900;
    unsigned long long month = timeinfo.tm_mon + 1;
    unsigned long long day = timeinfo.tm_mday;

    selectDate = year * 100000000ULL + month * 1000000ULL + day * 10000ULL;

    int stt_col = stt_day_1(selectDate);
    int date = 1;
 
    for (int i = stt_col; i < 7; i++, date++) {
        if (date == day) {
            selectDate_row = 0;
            selectDate_col = i;
            return;
        }
    }
    for (int i = 1; i < 6; i++) {
        for (int j = 0; j < 7; j++, date++) {
            if (date == day) {
                selectDate_row = i;
                selectDate_col = j;
                return;
            }
            if (date >= daysInMonth(selectDate / 1000000)) return;
        }
    }
}

void print_Year_Month(unsigned long long targetDate) {
    int year = targetDate / 100;
    int month = targetDate % 100;
    char *month_str;
    switch (month) {
    case 1:
        month_str = "Jan";
        break;
    case 2:
        month_str = "Feb";
        break;
    case 3:
        month_str = "Mar";
        break;
    case 4:
        month_str = "Apr";
        break;
    case 5:
        month_str = "May";
        break;
    case 6:
        month_str = "Jun";
        break;
    case 7:
        month_str = "Jul";
        break;
    case 8:
        month_str = "Aug";
        break;
    case 9:
        month_str = "Sep";
        break;
    case 10:
        month_str = "Oct";
        break;
    case 11:
        month_str = "Nov";
        break;
    case 12:
        month_str = "Dec";
        break;
    }
    attron(A_BOLD);
    mvprintw(pos_SUL_stt.row, pos_SUL_stt.col, "%d %s", year, month_str);
    attroff(A_BOLD);
}

int daysInMonth(unsigned long long targetDate) {
    unsigned long long year = targetDate / 100ULL;
    unsigned long long month = targetDate % 100ULL;

    if (month < 1 || month > 12) {
        printf("Invalid month!!!!! %llu-%llu\n", year, month);
        return -1;
    }
    int isLeapYear = ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) ? 1 : 0;
    /* 직관성을 위해 인덱스와 월을 일치시킴 */
    int days[] = {0, 31, 28 + isLeapYear, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    return days[month];
}

int stt_day_1(unsigned long long targetDate) {
    struct tm timeinfo = { 0 };
    timeinfo.tm_year = targetDate / 100000000 - 1900;
    timeinfo.tm_mon = targetDate % 100000000 / 1000000 - 1;
    timeinfo.tm_mday = 1;

    time_t rawtime = mktime(&timeinfo);

    struct tm* local_timeinfo = localtime(&rawtime);

    return local_timeinfo->tm_wday;
}

int weeksInMonth(unsigned long long targetDate) {
    int stt_col = stt_day_1(targetDate * 1000000ULL);

    unsigned long long year = targetDate / 100ULL;
    unsigned long long month = targetDate % 100ULL;
    int date = 22;
    int day = daysInMonth(year * 100ULL + month);
  
    for (int i = stt_col; i < 7; i++, date++) {
        if (date == day) return 4;
    }
    for (int i = 4; i < 6; i++) {
        for (int j = 0; j < 7; j++, date++) {
            if (date == day) return i + 1;
        }
    }
}

void print_date(unsigned long long targetDate) {
    int stt_col = stt_day_1(targetDate);
    int isEndOfMonth = 0;

    int year = targetDate / 100000000;
    int month = targetDate % 100000000 / 1000000;

    int date = 1;
    attron(COLOR_PAIR(1));
    for (int i = 0; i < 7; i++) {
        if (i >= stt_col) {
            mvprintw(pos_SC_date[0][i].row, pos_SC_date[0][i].col, "%-2d", date);
            date++;
        }
        else {
            move(pos_SC_date[0][i].row, pos_SC_date[0][i].col);
            addstr("  ");
        }
    }
    for (int i = 1; i < 6; i++) {
        if (isEndOfMonth) {
            break;
        }
        for (int j = 0; j < 7; j++, date++) {
            if (date > daysInMonth(targetDate / 1000000)) {
                move(pos_SC_date[i][j].row, pos_SC_date[i][j].col);
                addstr("  ");
                isEndOfMonth = 1;
                break;
            }
            mvprintw(pos_SC_date[i][j].row, pos_SC_date[i][j].col, "%-2d", date);
        }
    }
    attroff(COLOR_PAIR(1));
    return;
}

void print_date_NumOfSchedule(unsigned long long targetDate) {
    int stt_col = stt_day_1(targetDate);

    int date = 1; /* * 10000 */
    targetDate /= 10000ULL;
    int num;

    int isEndOfCount = 0;

    for (int i = 0; i < 7; i++) {
        if (i >= stt_col) {
            //mvprintw(pos_SC_date[0][i].row + nNum - 1, pos_SC_date[0][i].col, "%llu", targetDate);
            num = getNumOfSchedule(targetDate);
            mvprintw(pos_SC_date[0][i].row + nNum - 1, pos_SC_date[0][i].col, "%d", num);
            if (num) mvprintw(pos_SC_date[0][i].row + nNum - 1, pos_SC_date[0][i].col + 4 * nNum - 3, "%3d", num);
            date++; targetDate++;
        
        }
        else {
            move(pos_SC_date[0][i].row + nNum - 1, pos_SC_date[0][i].col + 4 * nNum - 3);
            addstr("   ");
        }
    }
    for (int i = 1; i < 6; i++) {
        if (isEndOfCount) {
            break;
        }
        for (int j = 0; j < 7; j++, date++) {
            if (targetDate % 100 > daysInMonth(targetDate / 100)) {
                move(pos_SC_date[i][j].row + nNum - 1, pos_SC_date[i][j].col + 4 * nNum - 3);
                addstr("   ");
                isEndOfCount = 1;
                break;
            }
            targetDate++;
            //printf("%d\n", targetDate);
            //mvprintw(pos_SC_date[0][i].row + nNum - 1, pos_SC_date[0][i].col, "%llu", targetDate);
            num = getNumOfSchedule(targetDate);
            mvprintw(pos_SC_date[i][j].row + nNum - 1, pos_SC_date[i][j].col, "%d", num);
            if (num) mvprintw(pos_SC_date[i][j].row + nNum - 1, pos_SC_date[i][j].col + 4 * nNum - 3, "%3d", num);
        }
    }
}

void print_commandLine(int mode) {
    for (int i = pos_SLL_stt.row; i <= pos_SLL_end.row; i++) {
        for (int j = pos_SLL_stt.col; j <= pos_SLL_end.col; j++) {
            move(i, j);
            addch(' ');
        }
    }
    char commands[150];
    char* command;
    switch(mode) {
    case 0: 
        sprintf(commands, "%c%-10s%c%-10s%c%-10s%c%-10s%c%-10s%c%-10s%c%-10s", 'i', "upwards", 'j', "left", 'k', "downwards", 'l', "right", 's', "select", 'z', "save", 'x', "load");
        break;
    case 1:
        sprintf(commands, "%c%-10s%c%-10s%c%-10s%c%-10s%c%-10s%c%-10s%c%-10s%c%-10s%c%-10s", 'I', "insert", 'i', "upwards", 'k', "downwards", 'd', "delete", 'm', "modify", '+', "D+", '-', "D-", 'b', "BookMark", 'e', "exit");
        break;
    case 2:
        sprintf(commands, "%c%-10s", 'e', "exit");
    }

    int interval = (pos_SLL_end.col - pos_SLL_stt.col + 1) / 5;
    for (int i = pos_SLL_stt.row, index = 0; i <= pos_SLL_end.row; i++) {
        for (int j = pos_SLL_stt.col; j < pos_SLL_end.col - (pos_SLL_end.col - pos_SLL_stt.col + 1) % 5; j += interval, index += 11) {
            command = commands + index;
            if (command[0] == '\0')
                break;
            standout();
            mvprintw(i, j, "%.1s", command);
            standend();
            command++;
            mvprintw(i, j + 2, "%.10s ", command);
        }
        if (command[0] == '\0')
            break;
    }
}

void select_highlightOn() {
    attron(COLOR_PAIR(2));
    mvprintw(pos_SC_date[selectDate_row][selectDate_col].row, pos_SC_date[selectDate_row][selectDate_col].col, "%-2d", selectDate / 10000 % 100);
    attroff(COLOR_PAIR(2));
}

void prev_select_highlightOff() {
    attron(COLOR_PAIR(1));
    mvprintw(pos_SC_date[selectDate_row][selectDate_col].row, pos_SC_date[selectDate_row][selectDate_col].col, "%-2d", selectDate / 10000 % 100);
    attroff(COLOR_PAIR(1));
}

void select_date(char c) {
    prev_select_highlightOff();
    unsigned long long year, month, day;
    year = selectDate / 100000000ULL;
    month = selectDate % 100000000ULL / 1000000ULL;
    day = selectDate % 1000000ULL / 10000ULL;
    switch (c) {
    case 'i': 
        if (day > 7) {
            selectDate_row--;
            selectDate -= 70000;
        }
        else {
            month--;
            if (month == 0) {
                month = 12;
                year--;
            }
            selectDate_row = 0;
            if (selectDate_col >= stt_day_1(selectDate)) selectDate_row--;
            day = daysInMonth(year * 100ULL + month) + (day - 7ULL);
            selectDate = year * 100000000ULL + month * 1000000ULL + day * 10000ULL;
            selectDate_row += weeksInMonth(selectDate / 1000000ULL) - 1;
            print_Year_Month(selectDate / 1000000);
            print_date(selectDate);
            print_date_NumOfSchedule(selectDate);
        }
        break;
    case 'k':
        if (day < daysInMonth(year * 100 + month) - 6) {
            selectDate_row++;
            selectDate += 70000;
        }
        else {
            day -= daysInMonth(year * 100 + month) - 7;
            month++;
            if (month == 13) {
                month = 1;
                year++;
            }
            selectDate = year * 100000000 + month * 1000000 + day * 10000;
            selectDate_row = 0;
            if (selectDate_col < stt_day_1(selectDate)) selectDate_row++;
            print_Year_Month(selectDate / 1000000);
            print_date(selectDate);
            print_date_NumOfSchedule(selectDate);
        }
        break;
    case 'j': 
        if (selectDate_col != 0) {
            selectDate_col--;
            if (day != 1) selectDate -= 10000;
            else {
                month--;
                if (month == 0) {
                    month = 12;
                    year--;
                }
                day = daysInMonth(year * 100ULL + month);
                selectDate_row = weeksInMonth(year * 100ULL + month) - 1;
                selectDate = year * 100000000ULL + month * 1000000ULL + day * 10000ULL;
                print_Year_Month(selectDate / 1000000);
                print_date(selectDate);
                print_date_NumOfSchedule(selectDate);
            }
        }
        else {
            selectDate_col = 6;
            if (day != 1) {
                selectDate_row--;
                selectDate -= 10000;
            }
            else {
                month--;
                if (month == 0) {
                    month = 12;
                    year--;
                }
                day = daysInMonth(year * 100 + month);
                selectDate_row = weeksInMonth(year * 100ULL + month) - 1;
                selectDate = year * 100000000ULL + month * 1000000ULL + day * 10000ULL;
                print_Year_Month(selectDate / 1000000);
                print_date(selectDate);
                print_date_NumOfSchedule(selectDate);
            }
        }
        break;
    case 'l':
        if (selectDate_col != 6) {
            selectDate_col++;
            if (day != daysInMonth(year * 100 + month)) selectDate += 10000;
            else {
                month++;
                if (month == 13) {
                    month = 1;
                    year++;
                }
                day = 1;
                selectDate_row = 0;
                selectDate = year * 100000000 + month * 1000000 + day * 10000;
                print_Year_Month(selectDate / 1000000);
                print_date(selectDate);
                print_date_NumOfSchedule(selectDate);
            }
        }
        else {
            selectDate_col = 0;
            if (day != daysInMonth(year * 100 + month)) {
                selectDate += 10000;
                selectDate_row++;
            }
            else {
                month++;
                if (month == 13) {
                    month = 1;
                    year++;
                }
                day = 1;
                selectDate_row = 0;
                selectDate = year * 100000000ULL + month * 1000000ULL + day * 10000ULL;
                print_Year_Month(selectDate / 1000000);
                print_date(selectDate);
                print_date_NumOfSchedule(selectDate);

            }
        }
        break;
    }
    select_highlightOn();
}

void get_todo() {
    char t[2]; // 문자열을 위한 배열 선언
    char title[31];
    char details[256];
    char p[2];

    nocbreak();  // canonical 모드로 전환
    echo();  // 입력한 키를 화면에 보이도록 설정

    standout();
    mvprintw(pos_SLL_stt.row + 1, pos_SLL_stt.col, "Enter the time ->");
    standend();
    getstr(t);//시각 입력
    if (strcmp(t, "e") == 0) return;

    mvprintw(pos_SLL_stt.row + 1, pos_SLL_stt.col, " ");
    for (int i = 0; i < pos_SUL_end.col - 1; i++)
        printw(" ");
    standout();
    mvprintw(pos_SLL_stt.row + 1, pos_SLL_stt.col, "Enter the title ->");
    standend();
    getstr(title);

    mvprintw(pos_SLL_stt.row + 1, pos_SLL_stt.col, " ");
    for (int i = 0; i < pos_SUL_end.col - 1; i++)
        printw(" ");
    standout();
    mvprintw(pos_SLL_stt.row + 1, pos_SLL_stt.col, "Enter the details ->");
    standend();
    getstr(details);

    mvprintw(pos_SLL_stt.row + 1, pos_SLL_stt.col, " ");
    for (int i = 0; i < pos_SUL_end.col - 1; i++)
        printw(" ");
    standout();
    mvprintw(pos_SLL_stt.row + 1, pos_SLL_stt.col, "Enter the priority ->");
    standend();
    getstr(p);

    cbreak();  // 다시 non-canonical 모드로 전환
    noecho();

    int priority = atoi(p);
    /*
    int time=atoi(t);
    todoDate+=time*100;*/
    //int today=todoDate;// 시간을 0000으로 넘김
    //setSchedule(today, title, details, priority);
}