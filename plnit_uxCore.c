// Written by: 2023011393 Nawon Kim, KNU CSE 2021114026 Jeongwoo Kim, 2023013565 Dahye Jeong
// terminal based calendar program ux: GUI-Like program w/curses library
// v0.0.5
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SUL 1
#define SUR 2
#define SLL 3
#define SLR 4
#define ON  1
#define OFF 0
#define SET 0
#define DEL 1

/*
 * SUL: Screen Upper Left, SUR: Screen Upper Right
 * SC : Screen Calendar,   SLC: Screen Left Calendar
 * SLL: Screen Lower Left, SLR: Screen Lower Right
 * 
 * each variable has a prefix 'pos_'
 * each variable has a suffix '_stt', '_end', respectively.
 * 
 * 
 * -------------------------------------   1
 * @           SUL          @|@   SUR      1 Line = 1
 * --------------------------|   
 * @                         |             6 Lines  + 1(worst case: some months can have 6 weeks)
 *                           |              = 6n + 7
 *             SC            |   
 *                           |         @
 *                           |----------   1 
 *                          @|@         
 * --------------------------|   
 * @           SLL           |    SLR      2 Lines (command key presentation area)
 *                          @|         @    = 2
 * -------------------------------------   1
 * 1           14n + 6       1   7n + 1    1
 *          
 * n = MIN_of_n(LINES = 6n + 14, COLS = 21n + 10))
 * remember that actual index is different from lib constant COLS, LINES
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

typedef struct reminder {
    int isSet;
    int repeatCounter;
    time_t start;
    time_t end;
    time_t intervals;
    char info[31];
} reminder;
typedef reminder* reminderPtr; /* for extern feature */

static pos pos_SUL_stt, pos_SUL_end;
static pos pos_SUR_stt, pos_SUR_end;
static pos pos_SC_stt,  pos_SC_end;
//static pos pos_SLC_stt, pos_SLC_end;
static pos pos_SLL_stt, pos_SLL_end;
static pos pos_SLR_stt, pos_SLR_end;
static pos pos_SC_date[6][7];

unsigned long long selectDate;
unsigned long long todayDate;
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
void print_date_ToDoSummarized(unsigned long long targetDate);
void print_date_ToDoWithdetails(unsigned long long targetDate, int status, int* page); 
void print_UpcomingBookMark(unsigned long long today);
void edit_plan(unsigned long long targetDate, int* page);
void printColorStrip(int colorNum);
void print_date_BookMark(unsigned long long targetDate);
void printReminderControl(int how);
/*-----Display control--------------------------------------------------------------*/
void clearGivenCalendarArea(/*index of pos_sc_date*/int row, int col);
void clearGivenRowCols(int fromRow, int fromCol, int toRow, int toCol);
void clearGivenNonCalendarArea(/*pre-defined Macros*/int area);
void save_UXPart(void);
void load_UXPart(void);
void reminder_extends_popup(int signum);
char getCommandScreen(char* context, char availToken[]);
/*-----Signal handling--------------------------------------------------------------*/
void setInputModeSigHandler(int status); /* Global Variable */
void inputMode_sigHndl(int signum);         int inputModeForceQuit;
/*-----Event control--------------------------------------------------------------*/
void popup(char* title, char* str1, char* str2, int delay); /* delay in secs */
/*-------------------------------------------------------------------*/
void errOcc(const char* str);
/*--UX Layer interactive API Methods---------------------*/
void get_todo(); /* global */ unsigned long long chosenDate = 0;
int save(void);
int save_hr(FILE* fp);
int load(void);

void coreInit(void);

int getNumOfSchedule(unsigned long long targetDate);
void getUpcomingSchedule(unsigned long long today, char* strbuf, int scrSize);
int setSchedule(unsigned long long today, char* title, char* details, int priority);
void getTodaySchedule(unsigned long long today, int sortType, char* strbuf, int scrSize); /* debugging only feature */

int getTodaySchedule_Summarized(unsigned long long today, char* strbuf);
int getTodaySchedule_withDetails(unsigned long long today, char* strbuf, int direction);
void getTodaySchedule_withDetails_iterEnd(void);
void getBookMarkedInDate(unsigned long long today, int counter, char* str);



extern reminderPtr rmdr; 
int deleteWhileIterate(unsigned long long src, int pageNum);
int editWhileIterate(unsigned long long src, int pageNum, unsigned long long t_day, char* t_title, char* t_details, int t_priority);
int setReminder(time_t current, time_t delta, int repeatCnter, char* what, int intervals);
int isReminderSetAlready(char* str);
void turnOffReminder(void);

int __dbDebug(void);
void __launchOptions(int argc, char* argv[]);
/* 0511 added */
int isBookMarked(unsigned long long targetDate); /* YYYYMMDD */
/* 0511 added: D-day settings, holiday */
void getDday(int* slot1, char* title1, int* slot2, char* title2);
void popDday(void);
int setDdayWhileIterate(unsigned long long src, int pageNum);
int isHoliday(unsigned long long target);

/*-------------------------------------------------------*/

int main(int argc, char* argv[]) {

    /* debuging options */
    if (argc > 1) {
        __launchOptions(argc, argv);
    }

    /* lcurses start */

    //printf("works");

    coreInit();

    initscr();

    noecho();

    if (LINES < 30 || COLS < 95) { /* Minimum size */
        printf("Not enough space to render a program UI\n");
        endwin();
        exit(1);
    }

    chooseOptimal_nNum(LINES, COLS);

    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_WHITE);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_WHITE);
    /* for bookmark strips */
    init_pair(5, COLOR_GREEN, COLOR_GREEN);
    init_pair(6, COLOR_CYAN, COLOR_CYAN);
    init_pair(7, COLOR_MAGENTA, COLOR_MAGENTA);
    init_pair(8, COLOR_RED, COLOR_RED);
    init_pair(9, COLOR_WHITE, COLOR_WHITE);
    /* for bookmark strips ended */

    initPosVar();
    initScreen();
    select_highlightOn(selectDate);
    
    //printf("Works");
    //mvprintw(1, 1, "Current Screen Size: %dx%d, opt.n: %d", LINES, COLS, nNum);

    refresh();

    char c;
    int mode = 0;
    int page = -1;

    /* implement of modes */
    while (1) {
        move(LINES - 1, COLS - 1); /* get rid of cursor */
        if (mode == 0) {
            while (1) {
                c = getch();
                if (c == 's') {
                    mode++;
                    break;
                }
                else if (c == 'i' || c == 'j' || c == 'k' || c == 'l') {
                    select_date(c);
                    break;
                }
                else if (c == 'z') {
                    save_UXPart();
                    break;
                }
                else if (c == 'x') {
                    load_UXPart();
                    break;
                }
                else if (c == 'q') {
                    printReminderControl(SET);
                    break;
                }
                else if (c == 'w') {
                    printReminderControl(DEL);
                    break;
                }
                else continue;
            }
        }
        else if (mode == 1) {
            move(LINES - 1, COLS - 1); /* get rid of cursor */
            print_date_ToDoWithdetails(selectDate, 0, &page);
            while (1) {
                move(LINES - 1, COLS - 1); /* get rid of cursor */
                c = getch();
                if (c == 'I') { /* to insert */
                    get_todo();
                    print_date_NumOfSchedule(selectDate); /* refresh screen */
                    print_date_ToDoWithdetails(selectDate, 0, &page); /* to refresh */
                    print_date_BookMark(selectDate); /* to refresh */
                }
                /* exit details mode */
                else if (c == 'e') { /* details exit */
                    mode--;
                    clearGivenNonCalendarArea(SUR);
                    print_date_ToDoSummarized(selectDate); /* to refresh */
                    break;
                }
                /* prev record */
                else if (c == 'j' && page != 0) { /* prev details record */
                    print_date_ToDoWithdetails(selectDate, 2, &page);
                }
                /* next record */
                else if (c == 'l' && page != 0) { /* next details record */
                    print_date_ToDoWithdetails(selectDate, 1, &page);
                }
                /* edit record */
                else if (c == 'm' && page != 0) {
                    edit_plan(selectDate, &page);
                    print_date_NumOfSchedule(selectDate); /* refresh screen */
                    print_date_ToDoWithdetails(selectDate, 0, &page); /* to refresh */
                    print_date_BookMark(selectDate); /* to refresh */
                    /* don't break */
                }
                /* delete record */
                else if (c == 'd' && page != 0) {
                    if (deleteWhileIterate(selectDate, page) != 0) /* del */ {
                        popup("Cannot Delete!", NULL, "System Default Public Holiday", 5);
                    }
                    else {
                        popup("Deletion Successful", "", NULL, 5);
                    }
                    print_date_NumOfSchedule(selectDate); /* to refresh */
                    print_date_ToDoWithdetails(selectDate, 0, &page); /* to refresh */
                    print_date_BookMark(selectDate); /* to refresh */
                    /* delete related functions */
                    /* don't break */
                    /* maybe some pageIterator refresh needed here */
                }
                /* error handling */
                else if (page == 0) {
                    /* just wait till user adds a record or exit */
                }
                else {
                    continue;
                }
                print_commandLine(mode);
                refresh();
            }
            getTodaySchedule_withDetails_iterEnd(); /* iterator init */
        }
        else if (mode == 2) {
            //if (c == 'e') mode--;
            get_todo();
            //c = getch();
            mode--;
            print_date_NumOfSchedule(selectDate); /* to refresh */
        }
        
        print_UpcomingBookMark(selectDate); /* to refresh */
        print_commandLine(mode);
        refresh();
    }


    sleep(10000);

    endwin();

    return 0;
}

void edit_plan(unsigned long long targetDate, int* page){
    char t[13]; // 문자열을 위한 배열 선언
    char title[31];
    char details[256];
    char p[2]; int priority; 
    unsigned long long date_2;
    int input_2; 
    int input_1 = *page;

    nocbreak();  // canonical 모드로 전환
    echo();  // 입력한 키를 화면에 보이도록 설정
    clearGivenNonCalendarArea(SLL);

    standout();
    mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, "Type edited Date&Time Below (Format: YYYYMMDDHHMM, Date&Time=1234: no edit) :");
    move(pos_SLL_stt.row + 1, pos_SLL_stt.col);
    standend();
    getstr(t);//날짜, 시간 입력
    //if (strcmp(t, "e") == 0) return;

    clearGivenNonCalendarArea(SLL);
    mvprintw(pos_SLL_stt.row + 1, pos_SLL_stt.col, " ");
    for (int i = 0; i < pos_SUL_end.col - 1; i++)
        printw(" ");
    standout();
    mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, "Type edited Title Below (Format: String, Title=Blank: no edit) :");
    move(pos_SLL_stt.row + 1, pos_SLL_stt.col);
    standend();
    getstr(title);

    clearGivenNonCalendarArea(SLL);
    mvprintw(pos_SLL_stt.row + 1, pos_SLL_stt.col, " ");
    for (int i = 0; i < pos_SUL_end.col - 1; i++)
        printw(" ");
    standout();
    mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, "Type edited Details Below (Format: String, Details=Blank: no edit) :");
    move(pos_SLL_stt.row + 1, pos_SLL_stt.col);
    standend();
    getstr(details);


    clearGivenNonCalendarArea(SLL);
    mvprintw(pos_SLL_stt.row + 1, pos_SLL_stt.col, " ");
    for (int i = 0; i < pos_SUL_end.col - 1; i++)
        printw(" ");
    standout();
    mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, "Type edited Priority Number Below: (0/1, Priority Number=-1: no edit) :");
    move(pos_SLL_stt.row + 1, pos_SLL_stt.col);
    standend();
    getstr(p);
    clearGivenNonCalendarArea(SLL);
     
    priority = atoi(p);
    date_2 = strtoull(t, NULL, 10);

    input_2 = editWhileIterate(targetDate, input_1 , date_2, title, details, priority);
        
    //mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, "result value: %d", input_2);
    switch (input_2) {
        case 1:
            popup("Cannot edit!", NULL, "Bookmark Collision!", 5);
            break;
        case 2:
            popup("Cannot edit!", NULL, "No such record", 5);
            break;
        case 0:
            popup("Success", NULL, "Edits applied.", 5);
            break;
        default:
            break;
    }
    mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, "Press ENTER to continue");
    getch();
    
    cbreak();  // 다시 non-canonical 모드로 전환
    noecho();
    return;
}

void chooseOptimal_nNum(int r, int c) {
    int rn, cn;

    rn = (r - 15) / (6);
    cn = (c - 17) / 42;
    /* n = 2m (let m be an integer) */

    if (rn < cn) {
        if (rn < 2) {
            printf("Not enough space to render a program UI\n");
            exit(1);
        }
        nNum = rn;
        return;
    }
    
    if (cn < 2) {
        printf("Not enough space to render a program UI\n");
        exit(1);
    }

    nNum = cn;
    return;
}

void initPosVar() {
/**
 * -------------------------------------   1
 * @           SUL          @|@   SUR      1 Line = 1
 * --------------------------|   
 * @                         |             6 Lines  + 1(worst case: some months can have 6 weeks)
 *                           |              = 6n + 7
 *             SC            |   
 *                           |         @
 *                           |----------   1 
 *                          @|@         
 * --------------------------|   
 * @           SLL           |    SLR      2 Lines (command key presentation area)
 *                          @|         @    = 2
 * -------------------------------------   1
 * 1           14n + 6       1   7n + 1    1
 *          
 * n = MIN_of_n(LINES = 6n + 14, COLS = 21n + 10))
*/  /* remember that actual index is different from lib constant COLS, LINES*/
    pos_SUL_stt.row = 1; pos_SUL_stt.col = 1;
    pos_SUL_end.row = 1; pos_SUL_end.col = nNum * 14 * 2 + 6; // *2는 가시성을 높이기 위해서
    pos_SUR_stt.row = 1; pos_SUR_stt.col = pos_SUL_end.col + 2;
    pos_SUR_end.row = 4 * nNum + 7; pos_SUR_end.col = pos_SUR_stt.col + nNum * 7 * 2; //*2는 가시성을 높이기 위해서
    pos_SC_stt.row = 3; pos_SC_stt.col = 1;
    pos_SC_end.row = 6 * nNum + 9; pos_SC_end.col = pos_SUL_end.col;
    pos_SLL_stt.row = 6 * nNum + 11; pos_SLL_stt.col = 1; 
    pos_SLL_end.row = 6 * nNum + 12; pos_SLL_end.col = pos_SUL_end.col;
    pos_SLR_stt.row = 4 * nNum + 9; pos_SLR_stt.col = pos_SUR_stt.col;
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

    standout();
    for (i = 0; i <= pos_SUR_end.col + 1; i++) {
        move(0, i); addch(' ');
    }
    for (int i = 0; i <= pos_SUL_end.col + 1; i++) {
        move(2, i); addch(' ');
    }
    for (int i = 0; i <= pos_SUL_end.col + 1; i++) {
        move(pos_SLL_stt.row - 1, i); addch(' ');
    }
    for (i = 0; i <= pos_SUR_end.col + 1; i++) {
        move(pos_SLL_end.row + 1, i); addch(' ');
    }
    for (i = 0; i <= pos_SLL_end.row; i++) {
        move(i, 0); addch(' ');
        move(i, pos_SUL_end.col + 1); addch(' ');
        move(i, pos_SUR_end.col + 1); addch(' ');
    }
    standend();
    for (i = 1; i < 7; i++) {
        for (j = pos_SC_stt.row; j <= pos_SC_end.row; j++) {
            move(j, pos_SC_date[0][i].col - 1);
            //attron(A_DIM | COLOR_PAIR(9));
            //addch('|'); // '|'
            //attroff(A_DIM | COLOR_PAIR(9));
            //attron(A_DIM);
            //standout();
            //addch(' ');
            //standend();
            //attroff(A_DIM);
            attron(A_DIM);
            addch('|');
            attroff(A_DIM);
        }
    }
    for (i = 0; i < 6; i++) {
        for (j = pos_SUL_stt.col; j <= pos_SUL_end.col; j++) {
            move(pos_SC_date[i][0].row - 1, j);
            //attron(A_DIM | COLOR_PAIR(9));
            //addch('-'); // '-'
            /*
            if (j % (4 * nNum + 1) != 0) {
                attron(A_DIM);
                standout();
                addch(' ');
                standend();
                attroff(A_DIM);
            }
            else {
                attron(A_DIM);
                standout();
                addch(' ');
                standend();
                attroff(A_DIM);
            }
            */
            attron(A_DIM);
            addch('-');
            attroff(A_DIM);
            //attroff(A_DIM | COLOR_PAIR(9));
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
    todayDate = selectDate;
    print_Year_Month(selectDate / 1000000);
    
    print_date(selectDate);
    print_date_NumOfSchedule(selectDate);
    print_date_BookMark(selectDate);
    print_commandLine(0);
    print_date_ToDoSummarized(selectDate);
    print_UpcomingBookMark(todayDate);
    return;
}

void select_today() {
    time_t t = time(NULL);
    struct tm timeinfo = *localtime(&t);

    unsigned long long year = timeinfo.tm_year + 1900;
    unsigned long long month = timeinfo.tm_mon + 1;
    unsigned long long day = timeinfo.tm_mday;

    selectDate = year * 100000000ULL + month * 1000000ULL + day * 10000ULL;
    //chosenDate = selectDate;

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

    return 0;
}

void print_date(unsigned long long targetDate) {
    int stt_col = stt_day_1(targetDate);
    
    int year = targetDate / 100000000;
    int month = targetDate % 100000000 / 1000000;
    int date = 1;

    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            clearGivenCalendarArea(i, j);
        }
    }

    int days = daysInMonth(year * 100 + month);
    attron(A_DIM);
    for (int i = 0; i < 7; i++) {
        if (i >= stt_col) {
            if (targetDate / 1000000 * 100 + date == todayDate / 10000) {
                attroff(A_DIM);
                attron(A_BOLD);
            }
            if (i == 6) {
                attroff(A_DIM);
                attron(A_BOLD);
                attron(COLOR_PAIR(3));
            }
            if (isHoliday(targetDate / 1000000 * 100 + date) || i == 0) {
                attroff(A_DIM);
                attron(A_BOLD);
                attron(COLOR_PAIR(1));
            }
            mvprintw(pos_SC_date[0][i].row, pos_SC_date[0][i].col, "%-2d", date);
            attrset(A_NORMAL);
            attron(A_DIM);
            date++;
        }
        else {
            clearGivenCalendarArea(0, i);
            //move(pos_SC_date[0][i].row, pos_SC_date[0][i].col);
            //addstr("  ");
        }
    }
    for (int i = 1; i < 6; i++) {
        for (int j = 0; j < 7; j++, date++) {
            if (date > days) {
                clearGivenCalendarArea(i, j);
            }
            else {
                if (targetDate / 1000000 * 100 + date == todayDate / 10000) {
                    attroff(A_DIM);
                    attron(A_BOLD);
                }
                if (j == 6) {
                    attroff(A_DIM);
                    attron(A_BOLD);
                    attron(COLOR_PAIR(3));
                }
                if (isHoliday(targetDate / 1000000 * 100 + date) || j == 0) {
                    attroff(A_DIM);
                    attron(A_BOLD);
                    attron(COLOR_PAIR(1));
                }
                mvprintw(pos_SC_date[i][j].row, pos_SC_date[i][j].col, "%-2d", date);
                attrset(A_NORMAL);
                attron(A_DIM);
            }
        }
    }
    attroff(A_DIM);
    return;
}

void print_date_NumOfSchedule(unsigned long long targetDate) {
    int stt_col = stt_day_1(targetDate);
    targetDate = targetDate / 1000000ULL * 100ULL + 1; /* to make it start from day 1 */
    int days = daysInMonth(targetDate / 100);
    int num;
    //int breakCondition = 0;

    for (int i = 0; i < 7; i++) {
        if (i >= stt_col) {
            num = getNumOfSchedule(targetDate);
            /* erasing only the very last row of the cell */
            for (int k = pos_SC_date[0][i].col; k < pos_SC_date[0][i].col + 4 * nNum; k++) {
                move(pos_SC_date[0][i].row + nNum - 1, k);
                addch(' ');
            }
            if (num) {
                mvprintw(pos_SC_date[0][i].row + nNum - 1, pos_SC_date[0][i].col, "%s", "To-Dos:");
                mvprintw(pos_SC_date[0][i].row + nNum - 1, pos_SC_date[0][i].col + 4 * nNum - 3, "%3d", num);
            }
            else {
                // clearGivenCalendarArea(0, i);
                /* erasing only the very last row of the cell */
                //for (int k = pos_SC_date[0][i].col; k < pos_SC_date[0][i].col + 4 * nNum - 4; k++) {
                //    move(pos_SC_date[0][i].row + nNum - 1, k);
                //    addch(' ');
                //}
            }
            targetDate++;
        
        }
        else {
            // move(pos_SC_date[0][i].row + nNum - 1, pos_SC_date[0][i].col + 4 * nNum - 3);
            clearGivenCalendarArea(0, i);
        }
    }
    for (int i = 1; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            if (targetDate % 100 > days) {
                //move(pos_SC_date[i][j].row + nNum - 1, pos_SC_date[i][j].col + 4 * nNum - 3);
                clearGivenCalendarArea(i, j);
            }
            else {
                /* erasing only the very last row of the cell */
                for (int k = pos_SC_date[i][j].col; k < pos_SC_date[i][j].col + 4 * nNum; k++) {
                    move(pos_SC_date[i][j].row + nNum - 1, k);
                    addch(' ');
                }
                num = getNumOfSchedule(targetDate);
                if (num) {
                    mvprintw(pos_SC_date[i][j].row + nNum - 1, pos_SC_date[i][j].col, "%s", "To-Dos:");
                    mvprintw(pos_SC_date[i][j].row + nNum - 1, pos_SC_date[i][j].col + 4 * nNum - 3, "%3d", num);
                }
                else {
                    //clearGivenCalendarArea(i, j);
                }
            }
            targetDate++;
        }
    }
}

void print_commandLine(int mode) {
    //printf("works");
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
        /*                  i      j      k      l      s      z      x      q      w      e*/
        sprintf(commands, "%c%-10s%c%-10s%c%-10s%c%-10s%c%-10s%c%-10s%c%-10s%c%-10s%c%-10s%c%-10s",
         'i', "upwards", 'j', "left", 'k', "downwards", 'l', "right", 's', "select", 'z', "save", 'x', "load", 'q', "set rmdr", 'w', "del rmdr", 'e', "set D-day");
        break;
    case 1:
        sprintf(commands, "%c%-10s%c%-10s%c%-10s%c%-10s%c%-10s%c%-10s%c%-10s", 'I', "insert", 'd', "delete", 'm', "modify", '+', "D+", '-', "D-", 'b', "BookMark", 'e', "exit");
        break;
    default:
        break;
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
    attron(A_DIM);
    standout();
    if (selectDate == todayDate) {
        attroff(A_DIM);
        attron(A_BOLD);
    }
    if (selectDate_col == 6) {
        standend();
        attroff(A_DIM);
        attron(A_BOLD);
        attron(COLOR_PAIR(4));
    }
    if (isHoliday(selectDate / 10000) || selectDate_col == 0) {
        standend();
        attroff(A_DIM);
        attron(A_BOLD);
        attron(COLOR_PAIR(2));
    }
    mvprintw(pos_SC_date[selectDate_row][selectDate_col].row, pos_SC_date[selectDate_row][selectDate_col].col, "%-2d", selectDate / 10000 % 100);
    attrset(A_NORMAL);
}

void prev_select_highlightOff() {
    attron(A_DIM);
    if (selectDate == todayDate) {
        attroff(A_DIM);
        attron(A_BOLD);
    }
    if (selectDate_col == 6) {
        attroff(A_DIM);
        attron(A_BOLD);
        attron(COLOR_PAIR(3));
    }
    if (isHoliday(selectDate / 10000) || selectDate_col == 0) {
        attroff(A_DIM);
        attron(A_BOLD);
        attron(COLOR_PAIR(1));
    }
    mvprintw(pos_SC_date[selectDate_row][selectDate_col].row, pos_SC_date[selectDate_row][selectDate_col].col, "%-2d", selectDate / 10000 % 100);
    attrset(A_NORMAL);
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
            if (selectDate_col >= stt_day_1(selectDate) && stt_day_1(selectDate)) selectDate_row--;
            day = daysInMonth(year * 100ULL + month) + (day - 7ULL);
            selectDate = year * 100000000ULL + month * 1000000ULL + day * 10000ULL;
            selectDate_row += weeksInMonth(selectDate / 1000000ULL) - 1;
            print_Year_Month(selectDate / 1000000);
            print_date(selectDate);
            print_date_NumOfSchedule(selectDate);
            print_date_BookMark(selectDate);
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
            print_date_BookMark(selectDate);
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
                print_date_BookMark(selectDate);
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
                print_date_BookMark(selectDate);
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
                print_date_BookMark(selectDate);
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
                print_date_BookMark(selectDate);
            }
        }
        break;
    }
    select_highlightOn();
    print_date_ToDoSummarized(selectDate);
}

void get_todo() {
    char t[5]; // 문자열을 위한 배열 선언
    char title[31];
    char details[256];
    char p[2]; int priority; int time;

    nocbreak();  // canonical 모드로 전환
    echo();  // 입력한 키를 화면에 보이도록 설정
    clearGivenNonCalendarArea(SLL);
    setInputModeSigHandler(ON);

    standout();
    mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, "^C to quit insert mode");
    mvprintw(pos_SLL_stt.row + 1, pos_SLL_stt.col, "press enter to continue");
    getch(); /* wait for user input */

    standend();
    if (inputModeForceQuit) return;
    clearGivenNonCalendarArea(SLL);

    standout();
    mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, "Enter the Time Below: (Format: HHMM)");
    move(pos_SLL_stt.row + 1, pos_SLL_stt.col);
    standend();
    
    getstr(t);//시각 입력
    if (inputModeForceQuit) return;
    //if (strcmp(t, "e") == 0) return;

    mvprintw(pos_SLL_stt.row + 1, pos_SLL_stt.col, " ");
    for (int i = 0; i < pos_SUL_end.col - 1; i++)
        printw(" ");
    standout();
    mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, "Enter the Title Below: (Format: String)");
    move(pos_SLL_stt.row + 1, pos_SLL_stt.col);
    standend();
    
    getstr(title);
    if (inputModeForceQuit) return;

    mvprintw(pos_SLL_stt.row + 1, pos_SLL_stt.col, " ");
    for (int i = 0; i < pos_SUL_end.col - 1; i++)
        printw(" ");
    standout();
    mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, "Enter the Details Below: (Format: String)");
    move(pos_SLL_stt.row + 1, pos_SLL_stt.col);
    standend();
    
    getstr(details);
    if (inputModeForceQuit) return;

    mvprintw(pos_SLL_stt.row + 1, pos_SLL_stt.col, " ");
    for (int i = 0; i < pos_SUL_end.col - 1; i++)
        printw(" ");
    standout();
    mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, "Enter the Priority Number Below: (0 ~ 9)");
    move(pos_SLL_stt.row + 1, pos_SLL_stt.col);
    standend();
    
    getstr(p);
    if (inputModeForceQuit) return;

    cbreak();  // 다시 non-canonical 모드로 전환
    noecho();

    priority = atoi(p);
    time = atoi(t);
    if (selectDate == 0) {
        errOcc("get_todo() Error");
    }

    selectDate += (unsigned long long)time;

    /* 에러 메시지에 따른 return value assigning */
    switch (setSchedule(selectDate, title, details, priority)) {
        case 0: /* safe */
            popup("Success", NULL, "Saved.", 5);
            break;
        case 1: /* bc */
            popup("Cannot Save", NULL, "Bookmark Collision!", 5);
            break;
        case 2: /* nsdf */
            popup("Cannot Save", NULL, "No such date format!", 5);
            break;
        case 3:
            popup("Cannot Save", NULL, "invaild time", 5);
        default:
            break;
    }

    selectDate -= (unsigned long long)time;

    setInputModeSigHandler(OFF);

    return;
}

void clearGivenCalendarArea(/*index of pos_sc_date*/int row, int col) {
    int i, j;
    int sttRow = pos_SC_date[row][col].row;
    int endRow = pos_SC_date[row][col].row + nNum - 1;
    int sttCol = pos_SC_date[row][col].col;
    int endCol = pos_SC_date[row][col].col + 4 * nNum - 1;
    
    for (i = sttRow; i <= endRow; i++) {
        for (j = sttCol; j <= endCol; j++) {
            move(i, j);
            addch(' ');
        }
    }

    return;
}

void clearGivenRowCols(int fromRow, int fromCol, int toRow, int toCol) {
    int i, j;

    for (i = fromRow; i <= toRow; i++) {
        for (j = fromCol; j <= toCol; j++) {
            move(i, j);
            addch(' ');
        }
    }

    return;
}

void clearGivenNonCalendarArea(/*pre-defined Macros*/int area) {
    /*
    #define SUL 1
    #define SUR 2
    #define SLL 3
    #define SLR 4
    */
    switch (area) {
        case SUL:
            clearGivenRowCols(pos_SUL_stt.row, pos_SUL_stt.col, pos_SUL_end.row, pos_SUL_end.col);
            break;
        case SUR:
            clearGivenRowCols(pos_SUR_stt.row, pos_SUR_stt.col, pos_SUR_end.row, pos_SUR_end.col);
            break;
        case SLL:
            clearGivenRowCols(pos_SLL_stt.row, pos_SLL_stt.col, pos_SLL_end.row, pos_SLL_end.col);
            break;
        case SLR:
            clearGivenRowCols(pos_SLR_stt.row, pos_SLR_stt.col, pos_SLR_end.row, pos_SLR_end.col);
            break;
        default:
            break;
    }

    return;
}

void setInputModeSigHandler(int status) {
    /* static */
    static struct sigaction inmode;
    static struct sigaction origin;

    if (status == ON) {
        inmode.sa_handler = inputMode_sigHndl;
        inmode.sa_flags &= ~SA_RESETHAND;
        inmode.sa_flags &= ~SA_SIGINFO;

        inputModeForceQuit = 0;

        if (sigaction(SIGINT, &inmode, &origin) == -1) {
            errOcc("sigaction");
        }
    }
    else { /* OFF */
        if (sigaction(SIGINT, &origin, NULL) == -1) {
            errOcc("sigaction");
        }
    }

}
void inputMode_sigHndl(int signum) {
    clearGivenNonCalendarArea(SLL);

    //move(pos_SLL_stt.row, pos_SLL_stt.col);
    //addstr("Terminating input mode...");

    //sleep(3);
    setInputModeSigHandler(OFF);
    inputModeForceQuit = 1;
    cbreak();  // 다시 non-canonical 모드로 전환
    noecho();

    return;
}
void save_UXPart(void) {
    save(); /* need to block signals via sigprocmask; It's a critical zone here. */

    clearGivenNonCalendarArea(SLL);
    move(pos_SLL_stt.row, pos_SLL_stt.col);
    addstr("Saved.");
    move(pos_SLL_stt.row + 1, pos_SLL_stt.col);
    addstr("Press Enter to continue.");
    getch();

    return;
}
void load_UXPart(void) {
    load(); /* need to block signals via sigprocmask; It's a critical zone here. */

    clearGivenNonCalendarArea(SLL);
    move(pos_SLL_stt.row, pos_SLL_stt.col);
    addstr("Loaded.");
    move(pos_SLL_stt.row + 1, pos_SLL_stt.col);
    addstr("Press Enter to continue.");
    getch();

    return;
}

void print_date_ToDoSummarized(unsigned long long targetDate) {
    int year = targetDate / 100000000;
    int month = targetDate % 100000000 / 1000000;
    int day = targetDate % 1000000 / 10000;
    int row = pos_SUR_stt.row, col = pos_SUR_stt.col;
    clearGivenNonCalendarArea(SUR);
    mvprintw(row, col, "ToDos | %d-%02d-%02d", year, month, day);
    int chkColorPair = 0;
    int isHolidayEvent = 0;
    row += 2;
    char str[BUFSIZ] = {'\0', };
    char *strbuf = str;//malloc(1000 * sizeof(char));
    int numofsche = getNumOfSchedule(targetDate / 10000), count = 0;
    if (getTodaySchedule_Summarized((targetDate / 10000), str) == -1) return;
    else {
        strbuf = str;
        while (*strbuf != '\0') {
            col = pos_SUR_stt.col;
            if (row >= pos_SUR_end.row - 1) {
                mvprintw(pos_SUR_end.row, col, "+%d more...", numofsche - count);
                break;
            }
            if (*strbuf == '[') {
                strbuf++;
                if (*strbuf == '^') {
                    strbuf++;
                    if (strncmp(strbuf, "9999", 4) == 0) {
                        attron(A_BOLD);
                        attron(COLOR_PAIR(1));
                        mvprintw(row, col, "All Day Long"); /* 볼드체와 색을 동시에 지정하면, 아스키코드가 이상하게 shift 됩니다. 혹시 왜 그런지 아시나요? */
                        attrset(A_NORMAL);
                        isHolidayEvent = 1;
                        strbuf += 4;
                    }
                    else {
                        mvprintw(row, col, "%.2s:", strbuf);
                        strbuf += 2;
                        printw("%.2s", strbuf);
                        strbuf += 2;
                    }
                    row++;
                }
            }
            else if (*strbuf == '*') {
                strbuf++;
                chkColorPair = (int)(*(strbuf++) - 48);
                if (isHolidayEvent) chkColorPair = 9;
            }
            else if (*strbuf == ']') {
                strbuf++;
                if (*strbuf == '^') {
                    strbuf++;
                    mvprintw(row, col++, " ");
                    /* for bookmark strip */
                    if (isHolidayEvent) chkColorPair = 9;
                    printColorStrip(chkColorPair);
                    /* for bookmark strip */
                    col += 2;
                    if (chkColorPair) attron(A_BOLD); /* for ones bookmarked */
                    mvprintw(row, col, " %.25s", strbuf); /* two blocks of strips */
                    if (chkColorPair) attroff(A_BOLD); /* for ones bookmarked */
                    chkColorPair = 0;
                    isHolidayEvent = 0;
                    row += 2;
                    strbuf += 25;
                    count++;
                }
            }

            /* err hndling */
            if (strbuf > &str[BUFSIZ]) {
                perror("overflow");
            }
        }
    }
}

void print_date_ToDoWithdetails(unsigned long long targetDate, int status, int* page) {
    /* status: 0 -> init load, 1 -> next, 2 -> prev */
    int year = targetDate / 100000000;
    int month = targetDate % 100000000 / 1000000;
    int day = targetDate % 1000000 / 10000;
    int chkColorPair = 0;
    int isHolidayEvent = 0;
    char str[BUFSIZ] = {'\0', };
    char* strbuf = str;
    int numofsche = getNumOfSchedule(targetDate / 10000);

    move(LINES - 1, COLS - 1);

    int row = pos_SUR_stt.row, col = pos_SUR_stt.col;
    clearGivenNonCalendarArea(SUR); 
    mvprintw(pos_SUR_stt.row, pos_SUR_stt.col, "ToDos | %d-%02d-%02d", year, month, day);
    row += 2;

    *page = getTodaySchedule_withDetails(targetDate / 10000, str, status) + 1;

    /* no data 처리 */
    if (*page == 0) { /* warn: when it's 0, there's no record */
        mvprintw((pos_SUR_stt.row + pos_SUR_end.row) / 2, (pos_SUR_stt.col + pos_SUR_end.col) / 2 - 8, "#No details data#");
        return;
    }

    //printf("%d", *page);
    strbuf = str;
    while (*strbuf != '\0') { /* 이게 있어야 뭔가 알 수 없는 프리징이 생기지 않는 것 같습니다. */
        col = pos_SUR_stt.col;
        if (*strbuf == '[') {
            strbuf++;
            if (*strbuf == '^') {
                strbuf++;
                if (strncmp(strbuf, "9999", 4) == 0) {
                    attron(A_BOLD);
                    attron(COLOR_PAIR(1));
                    mvprintw(row, col, "All Day Long"); /* 볼드체와 색을 동시에 지정하면, 아스키코드가 이상하게 shift 됩니다. 혹시 왜 그런지 아시나요? */
                    attrset(A_NORMAL);
                    isHolidayEvent = 1;
                    strbuf += 4;
                }
                else {
                    mvprintw(row, col, "%.2s:", strbuf);
                    strbuf += 2;
                    printw("%.2s", strbuf);
                    strbuf += 2;
                }
                row++;
            }
        }
        else if (*strbuf == '*') {
            strbuf++;
            chkColorPair = (int)(*(strbuf++) - 48);
            if (isHolidayEvent) chkColorPair = 9;
            isHolidayEvent = 0;
        }
        else if (*strbuf == ']') {
            strbuf++;
            if (*strbuf == '^') {
                strbuf++;
                mvprintw(row, col++, " "); /* ok */ 
                /* for bookmark strip */
                if (isHolidayEvent) chkColorPair = 9;
                printColorStrip(chkColorPair);
                /* for bookmark strip */
                col += 2;
                if (chkColorPair) attron(A_BOLD); /* for ones bookmarked */
                mvprintw(row, col, " %.25s", strbuf); /* two blocks of strips */
                if (chkColorPair) attroff(A_BOLD); /* for ones bookmarked */
                chkColorPair = 0;
                row += 2;
                strbuf += 25;
                attroff(COLOR_PAIR(1));
            }
            else if (*strbuf == ']') {
                strbuf++;
                mvprintw(row, col, "Details...");
                row += 2;
                char delimiters[] = "!\"#$%&\'()*+,-./:;<=>?@[\\]^_{|}~ ";
                char* ptr;
                int length;
                if (*strbuf == '\0') 
                    mvprintw(row, col, "empty..");
                while (*strbuf != '\0') {
                    ptr = strpbrk(strbuf, delimiters);
                    if (ptr != NULL) {
                        length = ptr - strbuf; //구분자가 오기 전까지의 길이 저장
                    }
                    else length = strlen(strbuf);
                    if (length != 0 && pos_SUR_end.col - col + 1 < length) {
                        //첫번째로 구분자가 오지 않을 때 남은 공간이 구분자가 오기 전의 단어 길이보다 적을 때
                        row++;
                        col = pos_SUR_stt.col;
                    }
                    if (length != 0) {//첫번째로 구분자가 오지 않으면
                        mvprintw(row, col, "%.*s", length, strbuf);//구분자가 오기 전까지 출력
                        col += length;
                        strbuf += length;
                    }
                    if (col > pos_SUR_end.col) {//현재 커서가 경계를 넘어갈 때
                        row++;
                        col = pos_SUR_stt.col;
                    }
                    if (ptr != NULL) {
                        mvprintw(row, col, "%c", *strbuf); //구분자 출력
                        col++;
                        strbuf++;
                    }
                }
                break;
            }

            /* err hndl */
            if (strbuf > &str[BUFSIZ]) {
                perror("overflow");
            }
        }
    }
    mvprintw(pos_SUR_end.row, (pos_SUR_stt.col + pos_SUR_end.col) / 2 - 5, "Page: %d/%d", *page, numofsche);
    if (numofsche != 1) { /* wow good */
        mvprintw(pos_SUR_end.row - 1, pos_SUR_stt.col, "<-[j]");
        mvprintw(pos_SUR_end.row - 1, pos_SUR_end.col - 4, "[l]->");
    }
    //mvprintw(pos_SUR_end.row, pos_SUR_stt.col, "press [k] to exit");
    //*letter = getch();
    //mvscanw(pos_SUR_end.row, pos_SUR_stt.col + 18, "%d", &input_2);
    //}
    //getTodaySchedule_withDetails_iterEnd();
    /* for GUI-like interaction */
    //mvprintw(pos_SUR_end.row - 2, pos_SUR_stt.col, "           ");
    //mvprintw(pos_SUR_end.row - 1, pos_SUR_stt.col, "             ");
    //mvprintw(pos_SUR_end.row, pos_SUR_stt.col, "                 ");
    //noecho();
    move(LINES - 1, COLS - 1);

    return;
}

void print_UpcomingBookMark(unsigned long long today) {
    char str[BUFSIZ] = {'\0', };
    char *strbuf = str;
    int chkColorPair = 0;
    for (int i = pos_SLR_stt.col; i <= pos_SLR_end.col; i++)
        mvprintw(pos_SLR_stt.row - 1, i, "-");
    mvprintw(pos_SLR_stt.row, pos_SLR_stt.col, "Upcoming Book Marks..");
    int row = pos_SLR_stt.row+2;
    int col = pos_SLR_stt.col;
    int count = (pos_SLR_end.row - (row-1))/2;
    getBookMarkedInDate(today / 10000, count, str);

    strbuf = str;
    while (*strbuf != '\0') {
        col = pos_SLR_stt.col;
        if (*strbuf == '[') {
            strbuf++;
            if (*strbuf == '^') {
                strbuf++;
                mvprintw(row, col, "%.2s:", strbuf);
                strbuf += 2;
                printw("%.2s", strbuf);
                strbuf += 2;
                row++;
            }
        }
        else if (*strbuf == '*') {
            strbuf++;
            chkColorPair = (int)(*(strbuf++) - 48);
        }
        else if (*strbuf == ']') {
            strbuf++;
            if (*strbuf == '^') {
                strbuf++;
                mvprintw(row, col++, " ");                   
                printColorStrip(chkColorPair);
                col += 2;
                if (chkColorPair) attron(A_BOLD); 
                mvprintw(row, col, " %.25s", strbuf);
                if (chkColorPair) attroff(A_BOLD); 
                chkColorPair = 0;
                row += 2;
                strbuf += 25;
            }
        }

        if (strbuf > &str[BUFSIZ]) {
            perror("overflow");
        }

        //strbuf++;
    } 
}

void popup(char* title, char* str1, char* str2, int delay) {
    /*
    ---------------------------------------
    * @           SUL          @|@   SUR      
    * --------------------------|   
    * @                         |             
    *                           |              
    *             SC            |   
    *                           |         @
    *                           |----------  
    *                          @|@         
    * --------------------------|   
    * @           SLL           |    SLR      
    *                          @|         @   
    * -------------------------------------   
    */
    /* implementation of mvinch, which saves current state of the screen */
    /* implementation of mvaddch, which saves current state of the screen */
    chtype** save; int i; int j;
    int lineAxis, colAxis, size; int ftemp; int stemp;

    /* nothing to print */
    if (title == NULL || (str1 == NULL && str2 == NULL)) return;

    lineAxis   = (pos_SUL_stt.row + pos_SLR_end.row) / 2;
    colAxis  = (pos_SUL_stt.col + pos_SLR_end.col) / 2;
    size      = nNum * 16;

    save = (chtype**)malloc(sizeof(chtype*) * LINES);
    if (!save) errOcc("malloc");
    for (i = 0; i < LINES; i++) {
        save[i] = (chtype*)malloc(sizeof(chtype) * COLS);
        if (!save[i]) errOcc("malloc");
    }

    for (i = 0; i < LINES; i++) {
        for (j = 0; j < COLS; j++) {
            save[i][j] = mvinch(i, j); /* actual action */
        }
    }

    /* printing boundaries */
    /* since LINES are longer than COLS, /= 4. */
    for (i = lineAxis - size / 4; i <= lineAxis + size / 4; i++) {\
        move(i, colAxis - size); standout(); addch(' '); standend(); 
        move(i, colAxis + size); standout(); addch(' '); standend();
        for (j = colAxis - size + 1; j <= colAxis + size - 1; j++) {
            if (i == lineAxis - size / 4 || i == lineAxis + size / 4) {
                move(i, j);
                //attron(COLOR_PAIR(3));
                standout(); addch(' '); standend();
                //addch(' ');
                //attroff(COLOR_PAIR(3));
            }
            else {
                move(i, j);
                addch(' ');
            }
        }
    }

    /* printing popup titles */
    move(lineAxis - 1, colAxis - strlen(title) / 2 - 1);
    addch('[');
    addstr(title);
    addch(']');
    move(LINES - 1, COLS - 1);

    /* printing details - str1*/
    if (str1) {
        move(lineAxis, colAxis - strlen(str1) / 2 - 1);
        addch('#');
        addstr(str1);
        addch('#');
    }
    move(LINES - 1, COLS - 1);
    /* printing details - str2*/
    if (str2) {
        move(lineAxis + 1, colAxis - strlen(str2) / 2 - 1);
        addch('#');
        addstr(str2);
        addch('#');
    }
    move(LINES - 1, COLS - 1);

    refresh();

    /* to get EXACT delay (since this process can have its own timer) */
    if ((ftemp = fork()) == -1) {
        errOcc("fork");
    }
    else if (ftemp == 0) {
        sleep(delay);
        exit(0);
    }
    else {
        wait(&stemp);
    }

    /* then restore */
    for (i = 0; i < LINES - 1; i++) {
        for (j = 0; j < COLS - 1; j++) {
            mvaddch(i, j, save[i][j]); /* actual action */
        }
    }

    refresh();

    /* freeing is important */
    for (i = 0; i < LINES; i++) {
        free(save[i]);
    }
    free(save);

    return;
}
void printColorStrip(int colorNum) { 
    attrset(A_NORMAL);
    if (colorNum == 0) /* default */ {
        attron(COLOR_PAIR(9)); /* 9 := off-white */
        addstr("  ");
        attrset(A_NORMAL);
        //attroff(COLOR_PAIR(9));
        return;
    }
    if (colorNum == 9) /* holiday */ {
        attron(COLOR_PAIR(8)); /* 8 := off-red */
        addstr("  ");
        attroff(COLOR_PAIR(8));
        return;
    }
    if (colorNum == -1) { /* if -1: erase */
        addstr("  "); /* -1 := erase */
        return;
    }
    colorNum += 4; /* refer to the definition of bookmark color */
    attron(COLOR_PAIR(colorNum));
    addstr("  ");
    attroff(COLOR_PAIR(colorNum));

    return;
}

void print_date_BookMark(unsigned long long targetDate) {
    int stt_col = stt_day_1(targetDate);

    int year = targetDate / 100000000;
    int month = targetDate % 100000000 / 1000000;
    int date = 1;

    int days = daysInMonth(year * 100 + month);
    int chkColorPair = 0;

    for (int i = 0; i < 7; i++) {
        if (i >= stt_col) {
            move(pos_SC_date[0][i].row, pos_SC_date[0][i].col + 4 * nNum - 2);
            chkColorPair = isBookMarked(targetDate / 1000000 * 100 + date);
            if (chkColorPair) printColorStrip(chkColorPair);
            else {
                printColorStrip(-1);
            }
            date++;
        }
        else {
            clearGivenCalendarArea(0, i);
        }
    }
    for (int i = 1; i < 6; i++) {
        for (int j = 0; j < 7; j++, date++) {
            if (date > days) {
                clearGivenCalendarArea(i, j);
            }
            else {
                move(pos_SC_date[i][j].row, pos_SC_date[i][j].col + 4 * nNum - 2);
                chkColorPair = isBookMarked(targetDate / 1000000 * 100 + date);
                if (chkColorPair) printColorStrip(chkColorPair);
                else {
                    printColorStrip(-1);
                }
            }
        }
    }
    return;
}

char getCommandScreen(char* context, char availToken[]) {
    /*
    ---------------------------------------
    * @           SUL          @|@   SUR      
    * --------------------------|   
    * @                         |             
    *                           |              
    *             SC            |   
    *                           |         @
    *                           |----------  
    *                          @|@         
    * --------------------------|   
    * @           SLL           |    SLR      
    *                          @|         @   
    * -------------------------------------   
    */
    char input;

    clearGivenNonCalendarArea(SLL);

    mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, context);
    mvprintw(pos_SLL_stt.row + 1, pos_SLL_stt.col, "input[");
    addstr(availToken);
    addstr("]: ");

    refresh();
    nocbreak(); // icanonon
    echo();

    while (1) {
        input = getch();

        if (strchr(availToken, input)) {
            break;
        }

        clearGivenNonCalendarArea(SLL);

        mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, context);
        mvprintw(pos_SLL_stt.row + 1, pos_SLL_stt.col, "input[");
        addstr(availToken);
        addstr("]: ");

        refresh();
    }

    return input;
}

void printReminderControl(int how) {
    char buf[BUFSIZ];
    char context[BUFSIZ];
    /* reminder variables */
    char info[31]; time_t delta; int repeat, interval;
    /* ------------------ */
    char temp; char temparr[10];

    memset(buf, 0x00, BUFSIZ);
    memset(context, 0x00, BUFSIZ);

    clearGivenNonCalendarArea(SLL);

    refresh();
    nocbreak(); // icanonon
    echo();

    if (how == SET) {
        if (isReminderSetAlready(buf)) {
            strcpy(context, "Reminder ");
            strcat(context, buf);
            strcat(context, " is already set. Want to override?");
            refresh();
            if ((temp = getCommandScreen(context, "yYnN")) == 'n' || temp == 'N') {
                return; /* don't want to override */
            }
        }

        clearGivenNonCalendarArea(SLL);
        standout();
        mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, "Type reminder title below:");
        move(pos_SLL_stt.row + 1, pos_SLL_stt.col);
        standend();
        refresh();
        getstr(info);

        clearGivenNonCalendarArea(SLL);
        standout();
        mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, "Please enter the time(seconds) to set the reminder:");
        move(pos_SLL_stt.row + 1, pos_SLL_stt.col);
        standend();
        refresh();
        getstr(temparr);
        delta = atol(temparr);

        clearGivenNonCalendarArea(SLL);
        standout();
        mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, "Please enter the number of times to repeat:");
        move(pos_SLL_stt.row + 1, pos_SLL_stt.col);
        standend();
        refresh();
        getstr(temparr);
        repeat = atoi(temparr);

        clearGivenNonCalendarArea(SLL);
        standout();
        mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, "Please enter the interval for the reminder:");
        move(pos_SLL_stt.row + 1, pos_SLL_stt.col);
        standend();
        refresh();
        getstr(temparr);
        interval = atoi(temparr);

        setReminder(time(NULL), delta, repeat, info, interval);
    }
    else if (how == DEL) {
        if (isReminderSetAlready(buf)) {
            refresh();
            if ((temp = getCommandScreen("Would you like to delete the existing reminder?", "yYnN")) == 'n' || temp == 'N') {
                return; /* don't want to delete */
            }
            else {
                turnOffReminder();
                standout();
                mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, "Reminder deleted.");
                mvprintw(pos_SLL_stt.row + 1, pos_SLL_stt.col, "Press ENTER to continue");
                standend();
                refresh();
                getch();
            }
        }
        else {
            standout();
            mvprintw(pos_SLL_stt.row, pos_SLL_stt.col, "No reminder has set yet.");
            mvprintw(pos_SLL_stt.row + 1, pos_SLL_stt.col, "Press ENTER to continue");
            standend();
            refresh();
            getch();
        }
    }

    cbreak();
    noecho();

    return;
}

void reminder_extends_popup(int signum) {
    char str[127];

    memset(str, 0x00, 127);

    if (rmdr->repeatCounter == 0) {
        popup("Reminder Expired!", NULL, "", 5);
        turnOffReminder();

        return;
    }
    sprintf(str, "Timer %d Seconds Left", (int)(rmdr->repeatCounter) * (int)(rmdr->intervals));
    popup("Reminder Alert", rmdr->info, str, 5);
    (rmdr->repeatCounter)--;

    return;
}