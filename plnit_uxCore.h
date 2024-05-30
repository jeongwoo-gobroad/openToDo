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
void printColorStrip(char *c, int colorNum);
void print_date_BookMark(unsigned long long targetDate);
void printReminderControl(int how);
void print_userName();
void getSharedTodo();
void change_userName();
void print_below(void);
/*-----Display control--------------------------------------------------------------*/
void clearGivenCalendarArea(/*index of pos_sc_date*/int row, int col);
void clearGivenRowCols(int fromRow, int fromCol, int toRow, int toCol);
void clearGivenNonCalendarArea(/*pre-defined Macros*/int area);
void save_UXPart(void);
void load_UXPart(void);
void reminder_extends_popup(int signum);
char getCommandScreen(const char* context, char availToken[]);
/*-----Signal handling--------------------------------------------------------------*/
void setInputModeSigHandler(int status); /* Global Variable */
void inputMode_sigHndl(int signum);         int inputModeForceQuit;
/*-----Event control--------------------------------------------------------------*/
void popup(char* title, char* str1, char* str2, int delay); /* delay in secs */
/*-------------------------------------------------------------------*/
void get_todo(); /* global */ unsigned long long chosenDate = 0;
void print_Dday(void);
void Set_Dday(unsigned long long targetDate, int pageNum);