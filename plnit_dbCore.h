// KNU CSE 2021114026 Jeongwoo Kim
// recent update: 240530 v1.0.1 build 32
// Basic structure of ADT:
/*
 * Years [Year Linked List w/index]
 * Year  [Array of Months w/index, check if it's leap year]
 * Month [Array of Days w/index]
 * Week  [No need to implement; week doesn't matter]
 * Day   [Priority Queue of ToDos]
 * ToDo  [Currently has 6 data areas]
 *      - hash no.
 *      - YYYYMMDDHHMM (unsigned long long)
 *      - Priority no.
 *      - Title   [MAXLEN := 25 w/ null terminated string]
 *      - Online share feature(Maybe later for Socket)
 *      - Details [MAXLEN := 60 w/ null terminated string]
*/

/* 2024-05-04 Important update
     Priority Number Means [BookMark]
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

typedef struct reminder {
    int isSet;
    int repeatCounter;
    time_t start;
    time_t end;
    time_t intervals;
    char info[31];
} reminder;
typedef reminder* reminderPtr;

typedef struct toDo {
    char userName[16];
    int isShared; /* 1: shared 2: being shared */
    char code[9]; /* if it's being shared */

    unsigned long long hashNum;
    unsigned long long dateData;
    int priority;
    char title[26];
    char details[61];
} toDo;
typedef toDo* toDoPtr;

typedef struct dDay {
    toDo dDayArr[2]; /* [0]: D+, [1]: D- */
    int maxIndex;
    int isPlusExist;
    int isMinusExist;
} dDay;
typedef dDay* dDayPtr;

typedef struct day {
    int isBookMarkExists;
    int sharedToDoExists;
    int maxIndex;
    int isHoliday;
    toDoPtr* toDoArr;
} day;
typedef day* dayPtr;

typedef struct month {
    /*
     * [0] is used for 28/29/30/31 det.
     *      
    */
    dayPtr dates[32];
} month;
typedef month* monthPtr;

typedef struct year {
    /*
     * [0] is used for leap year chk:
     *      
    */
    monthPtr months[13];
} year;
typedef year* yearPtr;

typedef struct yearsLL {
    int year;
    yearPtr target;
    struct yearsLL* prev;
    struct yearsLL* next;
} yearsLL;
typedef yearsLL* yearGrp;

typedef struct saveData {
    /* acts like a Linux file system */
    toDoPtr* toDoData;
    int maxIndex;
} saveData;
typedef saveData* savePtr;

/* saving features start */
void resizeSaveMem(void);
//int load(void);
//int save(void);
//int save_hr(FILE* fp);
void freeAllMem(void);
void initSaveMem(void);
void putSaveData(toDoPtr target);
toDoPtr getSaveData(void);
/* saving features ended */

//void errOcc(const char* str);
void myscanf(FILE* fd, char* target); /* to allow spaces */

toDoPtr create_Node(unsigned long long date, int priority_num, const char* title, const char* info);
void resizeArr_longer(dayPtr target);
void insert_toDo(dayPtr when, toDoPtr target);
dayPtr create_day(void);
dayPtr findDay(int target, monthPtr db);
monthPtr create_month(void);
monthPtr findMonth(int target, yearPtr db);
yearGrp create_yearGrp(int num);
yearPtr findYear(int target, yearGrp* db);
int insert(yearGrp* db, toDoPtr targetData);
void printAll(yearGrp db);
dayPtr search_byDate(unsigned long long target);
int get_toDo_byForm(toDoPtr target, char** buf);
int deleteRecord(dayPtr when, int index);

void sortGivenDateToDos(dayPtr when, int sortType);
void quickSort_byPriNum(toDoPtr* arr, int from, int to);
void quickSort_byDate(toDoPtr* arr, int from, int to);
void quickSort_byHashNum(toDoPtr* arr, int from, int to);

void printToday(dayPtr when);

/* For Bookmark Features - 0.0.3 added */
toDoPtr getBookMarked(unsigned long long src, int distance);
/* For Deletion Features - 0.0.3 added */
//void deleteWhileIterate(unsigned long long src, int pageNum);
//int editWhileIterate(unsigned long long src, int pageNum);
/* For Reminder Features - 0.0.5 added */
void reminderHandler(int signum);
void setReminderHandler(int status, unsigned long init, unsigned long repeat);
//int setReminder(time_t current, time_t delta, int repeatCnter);
void restoreReminder(void);
void allocReminder(void);
/* 0511 added: D-day settings, holiday */
//void setDday(unsigned long long target, int adjust); /* YYYYMMDD, adjust := 1 to D+, adjust := 2 to D- */
//int getDday(int adjust); /* adjust := 1 to D+, adjust := 2 to D- */
//int isHoliday(unsigned long long target);
void getHolidayInfos(int* fd); /* time: 9999 fixed */
void saveDday(int* fd);
void loadDday(int* fd);
void initDday(void);
int isBothDdayTheSame(toDoPtr s, toDo o);
void delDdayStack(int whatto);
void setDdayStack(toDoPtr target, int addto);
/* 0518 added for safe exit */
void clearAll(void);
/* 0519 added for network features */
//void setUserName(char* myName);
int getFromServer(toDoPtr target, char* shareCode);
//int getFromServer_Highlevel(char* shareCode);
int pushToServer(toDoPtr o, char* shareCode);
//int shareWhileIterate(unsigned long long src, int pageNum, char* shareCode);
//int isSharedToDoExisting(unsigned long long targetDate);
void deBookMarkInDate(unsigned long long src);
/*-------------------------------------------------------*/

/*--UX Layer interactive API Methods---------------------*/

//void coreInit(void);
//int getNumOfSchedule(unsigned long long targetDate);
//void getUpcomingSchedule(unsigned long long today, char* strbuf, int scrSize);
//int setSchedule(unsigned long long today, char* title, char* details, int priority);
//void getTodaySchedule(unsigned long long today, int sortType, char* strbuf, int scrSize); /* debugging only feature */

//int getTodaySchedule_Summarized(unsigned long long today, char* strbuf);
//int getTodaySchedule_withDetails(unsigned long long today, char* strbuf, int direction);
//void getTodaySchedule_withDetails_iterEnd(void);
//void getBookMarkedInDate(unsigned long long today, int counter, char* str);

//int deleteWhileIterate(unsigned long long src, int pageNum);
//int editWhileIterate(unsigned long long src, int pageNum, unsigned long long t_day, char* t_title, char* t_details, int t_priority);
//int setReminder(time_t current, time_t delta, int repeatCnter, char* what, int intervals);
//int isReminderSetAlready(char* str);
//void turnOffReminder(void);
void reminder_extends_popup(int signum); /* <--- UX Layer interactions */

void printUsage(void);
//void __launchOptions(int argc, char* argv[]);
/* 0511 added */
//int isBookMarked(unsigned long long targetDate); /* YYYYMMDD */
/* 0511 added, 0518 fixed: D-day settings, holiday */
//void getDday(int* slot1, char* title1, int* slot2, char* title2);
//void setDdayWhileIterate(unsigned long long src, int pageNum, int mode);
//int checkDdayWhileIterate(unsigned long long src, int pageNum, int mode);
//int isHoliday(unsigned long long target);
/* 0524 added */
//char* getUserName(void);
/*-------------------------------------------------------*/

/*--Debug-only features----------------------------------*/

void printMarkUP(char* str, int lineLimit);

/*-------------------------------------------------------*/