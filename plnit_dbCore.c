// KNU CSE 2021114026 Jeongwoo Kim
// recent update: 240509 v0.0.7
// Basic implementation of ADT:
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
 *      - Title   [MAXLEN := 30 w/ null terminated string]
 *      - Online share feature(Maybe later for Socket)
 *      - Details [MAXLEN := 256 w/ null terminated string]
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
    // SOCKET FEATURE {method}();
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
     *      but how???
    */
    dayPtr dates[32];
} month;
typedef month* monthPtr;

typedef struct year {
    /*
     * [0] is used for leap year chk:
     *      but how???
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

/*--Basic Methods----------------------------------------*/

/* Globals */
static yearGrp key = NULL; // this should be global var. to abstract the processing layer
static savePtr saveLink  = NULL;
int ifAlreadyLoaded = 0;
static int pageIterator = 0; // for GUI interaction; See Codes Below -> void getTodaySchedule_withDetails
static char userName[16] = {'\0', };
/**/
const char* bin_fileName = "todos.sv";
const char*  hr_fileName = "todos.txt"; /* hr stands for human readable */
const char* pub_fileName = "public.dsv"; /* dsv := default save file for public holidays */
const char*   dbDebug = "-d";
const char*   clDebug = "-cl";
const char* cli_input = "-in";
/**/ /* for leap year and month limit check */
monthPtr leapYear = NULL;
dayPtr   twenty_Eight = NULL;
dayPtr   twenty_Nine = NULL;
dayPtr   thirty = NULL;
dayPtr   thirty_one = NULL;
/**/
reminderPtr rmdr = NULL;
/**/
dDayPtr     dStack = NULL;
/**/
int Julian_A[12] = { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int Julian_M[12] = { 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

int julian_day(struct tm *date) {
    int a = Julian_A[date->tm_mon];
    int m = Julian_M[date->tm_mon];
    int y = date->tm_year + 1900 + 4800 - a;

    return date->tm_mday + ((153*m + 2) / 5) + 365*y + y/4 - y/100 + y/400 - 32045;
}
/* julian_day related references:  */
/* https://stackoverflow.com/questions/13932909/difference-between-two-dates-in-c */
/* https://stackoverflow.com/questions/1442116/how-to-get-the-date-and-time-values-in-a-c-program */
/**/

/* saving features start */
void resizeSaveMem(void);
int load(void);
int save(void);
int save_hr(FILE* fp);
void freeAllMem(void);
void initSaveMem(void);
void putSaveData(toDoPtr target);
toDoPtr getSaveData(void);
/* saving features ended */

void errOcc(const char* str);
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
void setUserName(char* myName);
int getFromServer(toDoPtr target, char* shareCode);
int getFromServer_Highlevel(char* shareCode);
int pushToServer(toDoPtr o, char* shareCode);
int shareWhileIterate(unsigned long long src, int pageNum, char* shareCode);
int isSharedToDoExisting(unsigned long long targetDate);
void deBookMarkInDate(unsigned long long src);
/*-------------------------------------------------------*/

/*--server-client interaction related APIs---------------*/
int __clDebug(void);
int cli_pushToDoDataToServer(toDoPtr target, char* code);
int cli_getToDoDataFromServer(toDoPtr target, const char* code);

/*--UX Layer interactive API Methods---------------------*/

void coreInit(void);
int getNumOfSchedule(unsigned long long targetDate);
void getUpcomingSchedule(unsigned long long today, char* strbuf, int scrSize);
int setSchedule(unsigned long long today, char* title, char* details, int priority);
void getTodaySchedule(unsigned long long today, int sortType, char* strbuf, int scrSize); /* debugging only feature */

int getTodaySchedule_Summarized(unsigned long long today, char* strbuf);
int getTodaySchedule_withDetails(unsigned long long today, char* strbuf, int direction);
void getTodaySchedule_withDetails_iterEnd(void);
void getBookMarkedInDate(unsigned long long today, int counter, char* str);

int deleteWhileIterate(unsigned long long src, int pageNum);
int editWhileIterate(unsigned long long src, int pageNum, unsigned long long t_day, char* t_title, char* t_details, int t_priority);
int setReminder(time_t current, time_t delta, int repeatCnter, char* what, int intervals);
int isReminderSetAlready(char* str);
void turnOffReminder(void);
void reminder_extends_popup(int signum); /* <--- UX Layer interactions */

void printUsage(void);
void __launchOptions(int argc, char* argv[]);
/* 0511 added */
int isBookMarked(unsigned long long targetDate); /* YYYYMMDD */
/* 0511 added, 0518 fixed: D-day settings, holiday */
void getDday(int* slot1, char* title1, int* slot2, char* title2);
void setDdayWhileIterate(unsigned long long src, int pageNum, int mode);
int checkDdayWhileIterate(unsigned long long src, int pageNum, int mode);
int isHoliday(unsigned long long target);
/* 0524 added */
char* getUserName(void);
/*-------------------------------------------------------*/

/*--Debug-only features----------------------------------*/

void printMarkUP(char* str, int lineLimit);

/*-------------------------------------------------------*/

int __dbDebug(void) {
    int input; int input_2; int input_3;
    unsigned long long date; int pnum; char title[30]; char details[256];
    unsigned long long date_2;
    int r = 1; 
    int i = 0;
    int temp;
    char testStr[BUFSIZ];
    int dday1, dday2; char dday1_str[30]; char dday2_str[30];
    char tmpname[16] = {'\0', };
    char shareCode[9] = {'\0', };

    allocReminder();
    initDday();
    initSaveMem();
    setUserName("Gildong_Hong");

    /* dummy datas just for leap year / month limit check */
    leapYear     = (monthPtr)malloc(sizeof(month));
    twenty_Eight = (dayPtr)malloc(sizeof(day));
    twenty_Nine  = (dayPtr)malloc(sizeof(day));
    thirty       = (dayPtr)malloc(sizeof(day));
    thirty_one   = (dayPtr)malloc(sizeof(day));

    while (r) {
        puts("Plan_it DB Core Debugger Menu | v 0.1.0 build 12");
        puts("Basic operation: ");
        puts("(0) to set username\n(1) to insert\n(2) to print all");
        puts("(3) to save\n(4) to load\n(-1) to quit");
        puts("  API Test menu: ");
        puts("  (5) to search by YYYYMMDD\n  (6) to get today's infos for given sort type");
        puts("  (7) to print summarized info of the given date in a markup form");
        puts("  (8) to print detailed info of the given date in a markup form, with iterator index");
        puts("  (9) to print a markup form in a human readable form");
        puts("  (10) to print upcoming bookmarked todos, with user input number of it.");
        puts("  (11) to delete a ToDo of the given date with iterator 'page' number");
        puts("  (12) to set a reminder");
        puts("  (13) to turn off the reminder");
        puts("  (14) to show current reminder info");
        puts("  (15) to edit records with YYYYMMDD and iterator 'page' number");
        puts("  (16) to get Bookmark designator with YYYYMMDD");
        puts("  (17) to determine whether a given day(YYYYMMDD) is a public holiday");
        puts("  (18) to set a D-day with YYYYMMDD and page number");
        puts("  (19) to show the D-day list");
        puts("  (20) debug only feature: to clear out D-day lists");
        puts("  (21) to share records with YYYYMMDD and iterator 'page' number");
        puts("  (22) to receive shared records by invitation code");
        puts("  (23) to get does the given date includes shared todos");
        printf("Type: ");
        //getchar();
        scanf("%d", &input);
        switch (input) {
            case 0:
                printf("Type username: ");
                scanf("%s", tmpname); getchar();
                setUserName(tmpname);
                break;
            case 1:
                printf("Type number of records: \n");
                scanf("%d", &input);
                for (i = 0; i < input; i++) {
                    pnum++;
                    printf("Type infos[YYYYMMDDHHMM PRIORITY_NUM TITLE DETAILS]: \n");
                    scanf("%llu", &date); //getchar();//printf("%llu <- \n", date);
                    scanf("%d", &pnum);
                    scanf(" %[^\n]s\n", title); //getchar();//printf("%s <- \n", title);
                    //myscanf(stdin, title);
                    //fgets(title, 30, stdin);
                    scanf(" %[^\n]s\n", details); //getchar();//printf("%s <- \n", details);
                    //myscanf(stdin, details);
                    //fgets(title, 60, stdin);
                    setSchedule(date, title, details, pnum);
                }
                break;
            case 2:
                printAll(key);
                break;
            case 3:
                if (save() == 0) {
                    puts("saved.");
                }
                break;
            case 4:
                if (load() == 0) {
                    puts("loaded.");
                }
                else {
                    puts("already loaded.");
                }
                break;
            case 5:
                printf("Type scan target YYYYMMDD: \n");
                scanf("%llu", &date); //getchar();
                printf("%llu: %d records found.\n", date, getNumOfSchedule(date));
                break;
            case 6:
                printf("Type scan target YYYYMMDD: \n");
                scanf("%llu", &date); //getchar();
                printf("Type sorting type(2 for time first, 1 for priority first): \n");
                scanf("%d", &input);
                getTodaySchedule(date, input, NULL, input);
                break;
            case 7:
                printf("Type scan target YYYYMMDD: \n");
                scanf("%llu", &date); //getchar();
                printf("Type sorting type(2 for time first, 1 for priority first): <- deleted feature, abstracted in API\n");
                scanf("%d", &input);
                getTodaySchedule_Summarized(date, testStr);
                printf("%s\n", testStr);
                break;
            case 8:
                printf("Type scan target YYYYMMDD: \n");
                scanf("%llu", &date); //getchar();
                printf("Type sorting type(2 for time first, 1 for priority first): <- deleted feature, abstracted in API\n");
                scanf("%d", &input);
                printf("starting iteration...\nType 1 to move to the next page, 2 to prev, 0 to cur, 3 to exit: ");
                scanf("%d", &input_2);
                while (input_2 != 3) {
                    input_2 = getTodaySchedule_withDetails(date, testStr, input_2);
                    printf("%s\n", testStr);
                    printf("Page: %d\ncontinuing iteration...\nType 1 to move to the next page, 2 to prev, 0 to cur, 3 to exit: ", input_2 + 1);
                    scanf("%d", &input_2);
                }
                getTodaySchedule_withDetails_iterEnd();
                break;
            case 9:
                printf("Type target string: ");
                scanf("%s", testStr); getchar();
                printf("Type target max width of the screen (input >= 9): ");
                scanf("%d", &input);
                puts("");
                printMarkUP(testStr, input);
                break;
            case 10:
                printf("Type scan target YYYYMMDD: \n");
                scanf("%llu", &date); //getchar();
                printf("Type max bookmark count: \n");
                scanf("%d", &input);
                getBookMarkedInDate(date, input, testStr);
                printf("%s\n", testStr);
                break;
            case 11:
                printf("Type scan target YYYYMMDD: \n");
                scanf("%llu", &date); //getchar();
                printf("Type Page Number [Alert: not an index]: \n");
                scanf("%d", &input);
                deleteWhileIterate(date, input);
                break;
            case 12:
                printf("Type the delta(seconds) value from the current time: \n");
                scanf("%d", &input);
                printf("Type the count of reminder alarm 'before' expiration: \n");
                scanf("%d", &input_2);
                printf("Type the interval(seconds) of reminder alarm 'before' expiration: \n");
                scanf("%d", &input_3);
                printf("Type reminder info string: ");
                scanf("%s", testStr); getchar();
                temp = setReminder(time(NULL), input, input_2, testStr, input_3);
                printf("Set result: %d\n", temp);
                break;
            case 13:
                turnOffReminder();
                puts("Reminder has been cleared");
                break;
            case 14:
                temp = isReminderSetAlready(testStr);
                if (temp) {
                    printf("Reminder Name: %s\n Expiration: %s\n", testStr, ctime(&(rmdr->end)));
                }
                else {
                    puts("No data");
                }
                break;
            case 15:
                printf("Type scan target YYYYMMDD: \n");
                scanf("%llu", &date); //getchar();
                printf("Type Page Number [Alert: not an index]: \n");
                scanf("%d", &input);
                printf("Type edited infos for given record: [YYYYMMDDHHMM PRIORITY_NUM TITLE DETAILS]: \n");
                printf("date:=1234: no edit, pnum:=-1: no edit title||details:=Blank no edit\n");
                scanf("%llu", &date_2); //getchar();//printf("%llu <- \n", date);
                scanf("%d", &pnum);
                scanf(" %[^\n]s\n", title); //getchar();//printf("%s <- \n", title);
                scanf(" %[^\n]s\n", details); //getchar();//printf("%s <- \n", details);
                input_2 = editWhileIterate(date, input, date_2, title, details, pnum);
                printf("result value: %d\n2: no such record 1: bookmark collision 0: success\n", input_2);
                break;
            case 16:
                printf("Type scan target YYYYMMDD: \n");
                scanf("%llu", &date); //getchar();
                printf("result: %d\n", isBookMarked(date));
                break;
            case 17:
                printf("Type scan target YYYYMMDD: \n");
                scanf("%llu", &date); //getchar();
                printf("result: %d\n", isHoliday(date));
                break;
            case 18:
                printf("Type scan target YYYYMMDD: \n");
                scanf("%llu", &date); //getchar();
                printf("Type Page Number [Alert: not an index]: \n");
                scanf("%d", &input);
                printf("Type mode to set whether D+(0) or D-(1): \n");
                scanf("%d", &input_2);
                input_3 = checkDdayWhileIterate(date, input, input_2);
                if (input_3 == 1) {
                    printf("same content alert: do you want to delete it? 0 or 1\n");
                    scanf("%d", &input_3);
                    if (input_3 == 1) {
                        delDdayStack(input_2);
                    }
                }
                else if (input_3 == 2) {
                    printf("override alert: do you want to override it? 0 or 1\n");
                    scanf("%d", &input_3);
                    if (input_3 == 1) {
                        setDdayWhileIterate(date, input, input_2);
                    }
                }
                else {
                    setDdayWhileIterate(date, input, input_2);
                }
                break;
            case 19:
                getDday(&dday1, dday1_str, &dday2, dday2_str);
                if (dday1_str[0] != '\0') {
                    printf("D%+d: %s\n", dday1, dday1_str);
                }
                if (dday2_str[0] != '\0') {
                    printf("D%+d: %s\n", dday2, dday2_str);
                }
                break;
            case 20:
                initDday();
                break;
            case 21:
                printf("Type scan target YYYYMMDD: \n");
                scanf("%llu", &date); //getchar();
                printf("Type Page Number [Alert: not an index]: \n");
                scanf("%d", &input);
                input_2 = shareWhileIterate(date, input, shareCode);
                printf("result: %d\n", input_2);
                if (input_2 == 0) {
                    printf("code: %s\n", shareCode);
                }
                break;
            case 22:
                printf("Type invitation code: \n");
                scanf("%s", shareCode);
                input_2 = getFromServer_Highlevel(shareCode);
                printf("result: %d\n", input_2);
                break;
            case 23:
                printf("Type scan target YYYYMMDD: \n");
                scanf("%llu", &date); //getchar();
                printf("result: %d\n", isSharedToDoExisting(date));
                break;
            case -1:
                clearAll();
                r = 0;
                break;
            default:
                break;
        }
        printf("-----------------------------------------------------------\n");
    }

    exit(1);
}

/*-----------------------*/

toDoPtr create_Node(unsigned long long date, int priority_num, const char* title, const char* info) {
    toDoPtr newOne;

    newOne = (toDoPtr)malloc(sizeof(toDo));

    memset(newOne->title, 0x00, 26);
    memset(newOne->details, 0x00, 61);
    memset(newOne->userName, 0x00, 16);
    memset(newOne->code, 0x00, 9);

    if (newOne == NULL) {
        errOcc("malloc"); /* err hndling */
    }

    newOne->hashNum = 0;
    newOne->dateData = date;
    newOne->priority = priority_num;
    newOne->isShared = 0;
    strcpy(newOne->title, title);
    strcpy(newOne->details, info);
    strcpy(newOne->userName, userName);
    strcpy(newOne->code, "---------");

    return newOne;
}

void resizeArr_longer(dayPtr target) {
    int i;
    int size = ++(target->maxIndex);
    if ((target->toDoArr = (toDoPtr*)(realloc(target->toDoArr, sizeof(toDoPtr) * (size + 1)))) == NULL) {
        errOcc("realloc");
    }
    //printf("Size extended to %d->%d\n", size - 1, size);

    /* remap hash */
    for (i = 0; i < target->maxIndex; i++) {
        (target->toDoArr)[i]->hashNum = ((target->toDoArr)[i]->dateData * 1000) + (target->toDoArr)[i]->priority * 100 + i;
    } /* YYYYMMDDHHMMPXX */

    return;
}

void insert_toDo(dayPtr when, toDoPtr target) {
    if (when->maxIndex == -1) {
        when->toDoArr = (toDoPtr*)malloc(sizeof(toDoPtr));
        if (when->toDoArr == NULL) {
            errOcc("malloc"); /* err hndling */
        }
        when->maxIndex = 0;
    }
    else {
        resizeArr_longer(when);
    }
    (when->toDoArr)[when->maxIndex] = target; // SUBJECT TO CHANGE: TO MAX HEAP

    /* remap hash */
    (when->toDoArr)[when->maxIndex]->hashNum = ((when->toDoArr)[when->maxIndex]->dateData * 1000) +
        (when->toDoArr)[when->maxIndex]->priority * 100 + when->maxIndex;
    return;
}

dayPtr create_day(void) {
    dayPtr newOne = (dayPtr)malloc(sizeof(day));

    if (newOne == NULL) {
        errOcc("malloc"); /* err hndling */
    }

    newOne->maxIndex = -1;
    newOne->toDoArr = NULL;
    newOne->isBookMarkExists = 0;
    newOne->isHoliday = 0;
    newOne->sharedToDoExists = 0;

    return newOne;
}

dayPtr findDay(int target, monthPtr db) {
    if (!(db->dates[target])) {
        db->dates[target] = create_day();
    }

    return db->dates[target];
}

monthPtr create_month(void) {
    int i;
    monthPtr newOne = (monthPtr)malloc(sizeof(month));

    if (newOne == NULL) {
        errOcc("malloc"); /* err hndling */
    }

    for (i = 0; i < 32; i++) {
        (newOne->dates)[i] = NULL;
    }

    return newOne;
}

monthPtr findMonth(int target, yearPtr db) {
    if (!(db->months[target])) {
        db->months[target] = create_month();
    }

    if ((target == 2) && ((db->months)[target]->dates[0]) == NULL) {
        /* leap year alloc */
        if ((db->months)[0] == leapYear) {
            ((db->months)[target]->dates[0]) = twenty_Nine;
        }
        else {
            ((db->months)[target]->dates[0]) = twenty_Eight;
        }
    }
    else if (((db->months)[target]->dates[0]) == NULL) {
        /* month limit alloc */
        if (target == 1 || (target == 3 || (target == 5 || (target == 7 || (target == 8 || (target == 10 || (target == 12))))))) {
            ((db->months)[target]->dates[0]) = thirty_one;
        }
        else {
            ((db->months)[target]->dates[0]) = thirty;
        }
    }

    return db->months[target];
}

yearGrp create_yearGrp(int num) {
    int i;
    yearPtr newYear;
    yearGrp newGrp;

    newYear = (yearPtr)malloc(sizeof(year));
    newGrp = (yearGrp)malloc(sizeof(yearsLL));

    if (newYear == NULL || newGrp == NULL) {
        errOcc("malloc"); /* err hndling */
    }

    newGrp->year = num;
    newGrp->target = newYear;
    newGrp->next = newGrp->prev = NULL;

    for (i = 0; i < 13; i++) {
        (newYear->months)[i] = NULL;
    }

    if (num % 4 == 0) { /* Leap year check */
        if (num % 100 == 0) {
            if (num % 400 == 0) {
                (newYear->months)[0] = leapYear;
            }
        }
        else {
            (newYear->months)[0] = leapYear;
        }
    }

    return newGrp;
}

yearPtr findYear(int target, yearGrp* db) {
    yearGrp iterator; yearGrp newOne; yearGrp append;
    int isFound = 0;

    /* iter */
    iterator = key;
    
    if (iterator) {
        while (iterator->prev) {
            iterator = iterator->prev;
        }
    }

    while (iterator) {
        if (iterator->year == target) {
            isFound = 1;
            break;
        }
        else if (iterator->year > target) {
            isFound = -1;
            break;
        }
        append = iterator;
        iterator = iterator->next;
        isFound = 2;
    }

    /* calc */
    if (isFound == 1) {                     /* If exists */
        return iterator->target;
    }
    else if (isFound == -1) {               /* If insertion needed */
        newOne = create_yearGrp(target);
        if (iterator->prev) {               /* If insertion needed: between stuffs */
            newOne->prev = iterator->prev;
            newOne->next = iterator;
            newOne->prev->next = newOne;
            newOne->next->prev = newOne;
            return newOne->target;
        }
        else {                              /* If insertion needed: new start node */
            newOne->prev = NULL;
            newOne->next = iterator;
            newOne->next->prev = newOne;
            return newOne->target;
        }
    }
    else if (isFound == 2) {                /* If insertion needed: need to append */
        newOne = create_yearGrp(target);
        newOne->prev = append;
        newOne->prev->next = newOne;
    }
    else {                                  /* If creating a start node required */
        (*db) = newOne = create_yearGrp(target);
        key = newOne;
    }

    return newOne->target;
}

int insert(yearGrp* db, toDoPtr targetData) {
    /* returns 2 if its a wrong date format
       returns 1 if a bookmarked todo already exists
       returns 0 if done correctly
    */
    int year, month, date, hour, minute;
    yearPtr   yy;
    monthPtr  mm;
    dayPtr    dd;

    /* Split */
    year =   (targetData->dateData) / 100000000;
    month =  (targetData->dateData) % 100000000 / 1000000;
    date =   (targetData->dateData) % 1000000 / 10000;
    hour =   (targetData->dateData) % 10000 / 100;
    minute = (targetData->dateData) % 100;

    /* Search */
    yy = findYear(year, db);
    mm = findMonth(month, yy);
    /* wait, we should check month limit! */
    if ((mm->dates)[0] == thirty_one && date > 31) {
        free(targetData); /* since it's already allocated! */
        return 2;
    }
    if ((mm->dates)[0] == thirty && date > 30) {
        free(targetData); /* since it's already allocated! */
        return 2;
    }
    if ((mm->dates)[0] == twenty_Nine && date > 29) {
        free(targetData); /* since it's already allocated! */
        return 2;
    }
    if ((mm->dates)[0] == twenty_Eight && date > 28) {
        free(targetData); /* since it's already allocated! */
        return 2;
    }
    /* HHMM Check */
    if ((hour > 24 || minute > 60) && !(hour == 99 && minute == 99)) { /* holiday default SHOULD be added */
        free(targetData);
        return 3; /* invaild time or minute */
    }

    /* then save... */
    dd = findDay(date, mm);
    /* one bookmark per day */
    if ((dd->isBookMarkExists) == 0 && targetData->priority) {
        dd->isBookMarkExists = 1;
    }
    else if ((dd->isBookMarkExists) && targetData->priority) {
        free(targetData); /* since it's already allocated! */
        return 1;
    }

    if (targetData->dateData % 10000 == 9999) {
        dd->isHoliday = 1; /* set holiday */
    }

    if (targetData->isShared) {
        dd->sharedToDoExists = 1; /* shared content inside */
    }

    //puts("inserted");
    /* Insert */
    insert_toDo(dd, targetData);
    putSaveData(targetData);

    return 0;
}

void printAll(yearGrp db) {
    int i, j;
    dayPtr temp;

    if (!db) {
        puts("No data");
        return;
    }

    while (db->prev) {
        db = db->prev;
    }

    while (db) { /* wtf */
        printf("Records of the Year %d:\n", db->year);
        for (i = 1; i <= 12; i++) {
            if ((db->target->months)[i]) {
                printf("    ->Records of the Month %d:\n", i);
                for (j = 1; j <= 31; j++) {
                    if (((db->target->months)[i]->dates)[j]) {
                        printf("        -->Records of the Day %d:\n", j);
                        temp = ((db->target->months)[i]->dates)[j];
                        printToday(temp);
                    }
                }
            }
        }
        db = db->next;
    }

    return;
}

dayPtr search_byDate(unsigned long long target) {
    unsigned long long year, month, date; //hour, minute;
    yearPtr   yy = NULL;
    monthPtr  mm = NULL;
    dayPtr    dd = NULL;
    yearGrp iter = key;

    /* Split */
    year =   (target) / 100000000;
    month =  (target) % 100000000 / 1000000;
    date =   (target) % 1000000 / 10000;
    //hour =   (target) % 10000 / 100;
    //minute = (target) % 100;

    /* Search */
    while (iter->prev) iter = iter->prev; /* move for first iterator */
    while (iter) {
        if (iter->year == year) {
            yy = iter->target;
            if (yy->months[month]) {
                mm = yy->months[month]; /* support for the easier deletion; no need to back traverse. */
                if (mm->dates[date] && (mm->dates[date]->maxIndex != -1)) {
                    dd = mm->dates[date];
                    break;
                }
            }
        }
        iter = iter->next;
    }

    if (!dd) { /* if not found */
        return NULL;
    }

    return dd;
}

int get_toDo_byForm(toDoPtr target, char** buf) {
    //int hour, min;
    /* Split */

    //hour =  ((target)->dateData) % 10000 / 100;
    //min =   ((target)->dateData) % 100;

    /* \\ is considered as a new line feed in UX layer */
    //snprintf(*buf, sizeof(char) * 50, "%d:%d \\ priority: %d \\ %s \\ %s... \\ \n", hour, min, target->priority, target->title, target->details);

    return (int)strlen(*buf);
}

void quickSort_byDate(toDoPtr* arr, int from, int to) {
    toDoPtr pivot;
    toDoPtr swapTemp;
    int cursor, pivAble;

    if (from >= to) return;

    //printf("Size: from %d to %d index.\n", from, to);

    pivot = arr[from]; pivAble = from;
    //printf("pivot: %llu\n", pivot->dateData % 10000);
    for (cursor = from + 1; cursor <= to; cursor++) {
        //printf("Comparing %d-%d, each data of %llu, %llu\n", cursor, pivAble, arr[cursor]->dateData % 10000, pivot->dateData % 10000);
        if (arr[cursor]->dateData % 10000 < pivot->dateData % 10000) {
            //printf("SWAP %d <-> %d\n", cursor, pivAble + 1);
            swapTemp = arr[++pivAble];
            arr[pivAble] = arr[cursor];
            arr[cursor] = swapTemp;
        }
    }
    //printf("SWAP %d <-> %d\n", from, pivAble);
    swapTemp = arr[from];
    arr[from] = arr[pivAble];
    arr[pivAble] = swapTemp;
    //printf("Phase %d->%d Safely finished.\n", from, to);

    quickSort_byDate(arr, from, pivAble - 1);
    quickSort_byDate(arr, pivAble + 1, to);

    return;
}

void quickSort_byPriNum(toDoPtr* arr, int from, int to) {
    toDoPtr pivot;
    toDoPtr swapTemp;
    int cursor, pivAble;

    if (from >= to) return;

    //printf("Size: from %d to %d index.\n", from, to);

    pivot = arr[from]; pivAble = from;
    //printf("pivot: %llu\n", pivot->dateData % 10000);
    for (cursor = from + 1; cursor <= to; cursor++) {
        //printf("Comparing %d-%d, each data of %llu, %llu\n", cursor, pivAble, arr[cursor]->dateData % 10000, pivot->dateData % 10000);
        if (arr[cursor]->priority < pivot->priority) {
            //printf("SWAP %d <-> %d\n", cursor, pivAble + 1);
            swapTemp = arr[++pivAble];
            arr[pivAble] = arr[cursor];
            arr[cursor] = swapTemp;
        }
    }
    //printf("SWAP %d <-> %d\n", from, pivAble);
    swapTemp = arr[from];
    arr[from] = arr[pivAble];
    arr[pivAble] = swapTemp;
    //printf("Phase %d->%d Safely finished.\n", from, to);

    quickSort_byPriNum(arr, from, pivAble - 1);
    quickSort_byPriNum(arr, pivAble + 1, to);

    return;
}

void quickSort_byHashNum(toDoPtr* arr, int from, int to) {
    toDoPtr pivot;
    toDoPtr swapTemp;
    int cursor, pivAble;

    if (from >= to) return;

    //printf("Size: from %d to %d index.\n", from, to);

    pivot = arr[from]; pivAble = from;
    //printf("pivot: %llu\n", pivot->dateData % 10000);
    for (cursor = from + 1; cursor <= to; cursor++) {
        //printf("Comparing %d-%d, each data of %llu, %llu\n", cursor, pivAble, arr[cursor]->dateData % 10000, pivot->dateData % 10000);
        if (arr[cursor]->hashNum < pivot->hashNum) {
            //printf("SWAP %d <-> %d\n", cursor, pivAble + 1);
            swapTemp = arr[++pivAble];
            arr[pivAble] = arr[cursor];
            arr[cursor] = swapTemp;
        }
    }
    //printf("SWAP %d <-> %d\n", from, pivAble);
    swapTemp = arr[from];
    arr[from] = arr[pivAble];
    arr[pivAble] = swapTemp;
    //printf("Phase %d->%d Safely finished.\n", from, to);

    quickSort_byHashNum(arr, from, pivAble - 1);
    quickSort_byHashNum(arr, pivAble + 1, to);

    return;
}

void sortGivenDateToDos(dayPtr when, int sortType) {
    /**
     * sortType 1: priority first
     * sortType 2: time first
     * other numbers: regarded as priority
    */
    toDoPtr temp = NULL;

    /* err hndl */
    if (!when) {
        //puts("No datas to sort");
        return;
    }
    if (when->maxIndex == -1) {
        return; /* no records to sort */
    }

    /* kind of radix(bucket) */
    if (sortType == 1) {
        quickSort_byPriNum(when->toDoArr, 0, when->maxIndex);
        if ((when->toDoArr)[when->maxIndex]->dateData % 10000 == 9999) { /* support for public holiday-sort */
            temp = (when->toDoArr)[when->maxIndex];
            (when->toDoArr)[when->maxIndex] = (when->toDoArr)[0];
            (when->toDoArr)[0] = temp;
            quickSort_byPriNum(when->toDoArr, 1, when->maxIndex);
        }
    }
    else {
        quickSort_byDate(when->toDoArr, 0, when->maxIndex);
        if ((when->toDoArr)[when->maxIndex]->dateData % 10000 == 9999) { /* support for public holiday-sort */
            temp = (when->toDoArr)[when->maxIndex];
            (when->toDoArr)[when->maxIndex] = (when->toDoArr)[0];
            (when->toDoArr)[0] = temp;
            quickSort_byDate(when->toDoArr, 1, when->maxIndex);
        }
    }

    return;
}

void printToday(dayPtr when) {
    int i;

    if (!when) {
        puts("No data");
        return;
    }

    for (i = 0; i <= when->maxIndex; i++) { /* safe approach */
        if ((when->toDoArr)[i]->isShared == 1) {
            printf("            Shared by %s::\n            %llu: [%d] %s || %s\n", (when->toDoArr)[i]->userName,
                (when->toDoArr)[i]->dateData, (when->toDoArr)[i]->priority, (when->toDoArr)[i]->title, (when->toDoArr)[i]->details);
        }
        else if ((when->toDoArr)[i]->isShared == 2) {
            printf("            ShareCode %s::\n            %llu: [%d] %s || %s\n", (when->toDoArr)[i]->code,
                (when->toDoArr)[i]->dateData, (when->toDoArr)[i]->priority, (when->toDoArr)[i]->title, (when->toDoArr)[i]->details);  
        }
        else {
            printf("            %llu: [%d] %s || %s\n", 
                (when->toDoArr)[i]->dateData, (when->toDoArr)[i]->priority, (when->toDoArr)[i]->title, (when->toDoArr)[i]->details);
        }
    }

    return;
}

/*--------API------------------------------------------------------------------------------------------------------------------------------------*/

int getNumOfSchedule(unsigned long long targetDate) {
    dayPtr dd = NULL;

    dd = search_byDate(targetDate * 10000); // YYYYMMDD

    if (!dd) return 0;

    /* we know maxIndex + 1 is an actual number of records */
    return dd->maxIndex + 1;
}
void getUpcomingSchedule(unsigned long long today, char* strbuf, int scrSize) { /* deprecated */
    char* temp;
    int limit; int i;
    dayPtr dd = NULL;
    char* strs;

    i = 0;
    limit = scrSize;
    strs = (char*)malloc(sizeof(char) * scrSize); strs[0] = '\0';
    temp = (char*)malloc(sizeof(char) * scrSize); temp[0] = '\0';

    dd = search_byDate(today);

    if (!dd) {
        strcpy(strbuf, "No data");
        return;
    }
    else {
        while (limit) {
            /* is given screen size capable of */
            limit /= get_toDo_byForm((dd->toDoArr)[i++], &temp);
            strcat(strs, temp);
        }
        strcpy(strbuf, temp);
    }
    
    return;
}
int setSchedule(unsigned long long today, char* title, char* details, int priority) {
    toDoPtr in;

    in = create_Node(today, priority, title, details);
    //insert(&key, in);

    return insert(&key, in); /* 0: inserted. 1: Bookmarked ToDo already exists. 2: invaild date */
}
void getTodaySchedule(unsigned long long today, int sortType, char* strbuf, int scrSize) {
    //char* strs;
    //char* temp;
    //int limit;
    dayPtr dd = NULL;

    //limit = scrSize;
    //strs = (char*)malloc(sizeof(char) * scrSize); strs[0] = '\0';
    //temp = (char*)malloc(sizeof(char) * scrSize); temp[0] = '\0';

    dd = search_byDate(today * 10000); // YYYYMMDD

    if (!dd) {
        puts("No data.");
        //strcpy(*buf, "No data");
    }
    else {
        sortGivenDateToDos(dd, sortType);
        printToday(dd);
        //while (limit) {
        //    /* is given screen size capable of */
        //    limit /= get_toDo_byForm(dd, temp);
        //    strcat(strs, temp);
        //}
        //strcpy(*strbuf, temp);
    }
    
    return;
}
/* active high-level APIs */
int getTodaySchedule_Summarized(unsigned long long today, char* strbuf) {
    /* Prefix codes
        [^: for Time, [[: make new line, ^: tab inside
        ]^: for title, ]]: for details, *: Bookmarked
    */
    
    dayPtr dd = NULL;
    char str[BUFSIZ] = {'\0', };
    char temp[BUFSIZ] = {'\0', };
    int i;

    dd = search_byDate(today * 10000); // YYYYMMDD

    if (!dd || dd->maxIndex == -1) {
        strcpy(str, "no_data\n");
        return -1;
    }
    else {
        sortGivenDateToDos(dd, 2);
        for (i = 0; i <= dd->maxIndex; i++) {
            if ((dd->toDoArr)[i]->isShared) { /* if the pointed todo is shared one */
                sprintf(temp, "[^%04llu*%d@]^%-25s", (dd->toDoArr)[i]->dateData % 10000, (dd->toDoArr)[i]->priority, (dd->toDoArr)[i]->title);
            }
            else if ((dd->toDoArr)[i]->priority) /* if bookmarked */ {
                /*            Time->BookMarked->Title*/
                sprintf(temp, "[^%04llu*%d]^%-25s", (dd->toDoArr)[i]->dateData % 10000, (dd->toDoArr)[i]->priority, (dd->toDoArr)[i]->title);
            }
            else {
                /*            Time->Title*/
                sprintf(temp, "[^%04llu]^%-25s", (dd->toDoArr)[i]->dateData % 10000, (dd->toDoArr)[i]->title);
            }
            /* Ver 2.0, 0504 */
            strcat(str, temp);
        }
    }
    strcat(str, "\0");
    strcpy(strbuf, str);

    return 0;
}
int getTodaySchedule_withDetails(unsigned long long today, char* strbuf, int direction) {
    /* implementing *pageIterator* */
    /* Prefix codes
        [^: for Time, [[: make new line, ^: tab inside
        ]^: for title, ]]: for details, *: Bookmarked 
    */
    /* 0504 fixed: no need to tab nor new line */
    /* 0508 added: direction 0: init(start) 1: forward, direction 2: backward */
    dayPtr dd = NULL;
    char str[BUFSIZ] = {'\0', };
    char temp[BUFSIZ] = {'\0', };
    int i = 0;;

    dd = search_byDate(today * 10000); // YYYYMMDD

    if (!dd || dd->maxIndex == -1) {
        strcpy(str, "no_data\n");
        return -1;
    }
    else {
        if (direction == 1) { /* forward */
            pageIterator = (pageIterator + 1) % (dd->maxIndex + 1);
        }
        else if (direction == 2) { /* backward */
            pageIterator = (pageIterator - 1);
            if (pageIterator < 0) {
                pageIterator = dd->maxIndex;
            }
        }
        else { /* init start */
            pageIterator = 0;
        }
        i = pageIterator;

        sortGivenDateToDos(dd, 2);

        if ((dd->toDoArr)[i]->isShared == 1) { /* shared: 공유 받은 */
            sprintf(temp, "[^%04llu*%d@%-15s]^%-25s]]%s", (dd->toDoArr)[i]->dateData % 10000, (dd->toDoArr)[i]->priority, (dd->toDoArr)[i]->userName, (dd->toDoArr)[i]->title, (dd->toDoArr)[i]->details);
        }
        else if ((dd->toDoArr)[i]->isShared == 2) { /* being shared: 공유 해 주는 */
            sprintf(temp, "[^%04llu*%d@@%-8s]^%-25s]]%s", (dd->toDoArr)[i]->dateData % 10000, (dd->toDoArr)[i]->priority, (dd->toDoArr)[i]->code, (dd->toDoArr)[i]->title, (dd->toDoArr)[i]->details);
        }
        else if ((dd->toDoArr)[i]->priority) { /* if bookmarked */
            /*         Time->BookMarked->Title(NL)->Details*/
            sprintf(temp, "[^%04llu*%d]^%-25s]]%s", (dd->toDoArr)[i]->dateData % 10000, (dd->toDoArr)[i]->priority, (dd->toDoArr)[i]->title, (dd->toDoArr)[i]->details);
        }
        else {
            /*         Time->Title(NL)->Details*/
            sprintf(temp, "[^%04llu]^%-25s]]%s", (dd->toDoArr)[i]->dateData % 10000, (dd->toDoArr)[i]->title, (dd->toDoArr)[i]->details);
        }
        strcat(str, temp);
    }

    strcpy(strbuf, str);

    return i;
}
void getTodaySchedule_withDetails_iterEnd(void) {
    pageIterator = 0;

    return;
}

/*------------------------------------------------------------------------------------------*/

void coreInit(void) {
    allocReminder();
    initDday();
    setUserName("Gildong_Hong");
    
    load();
    /* dummy datas just for leap year / month limit check */
    leapYear     = (monthPtr)malloc(sizeof(month));
    twenty_Eight = (dayPtr)malloc(sizeof(day));
    twenty_Nine  = (dayPtr)malloc(sizeof(day));
    thirty       = (dayPtr)malloc(sizeof(day));
    thirty_one   = (dayPtr)malloc(sizeof(day));

    return;
}

/* saving features start */
void resizeSaveMem(void) {
    (saveLink->maxIndex)++;
    saveLink->toDoData = (toDoPtr*)realloc(saveLink->toDoData, (saveLink->maxIndex + 1) * sizeof(toDoPtr));

    if (saveLink->toDoData == NULL) {
        errOcc("realloc");
    }

    return;
}
int load(void) {
    int fd_bin; int chunk;
    toDoPtr buf = NULL;

    if (ifAlreadyLoaded) {
        return 1;
    }

    freeAllMem(); /* erases node entry */
    initSaveMem(); /* erases nodes and hashLinks */
    initDday();

    fd_bin = open(bin_fileName, O_CREAT | O_RDWR); /* if exists, then read, or not, then creat */
    if (fd_bin == -1) {
        errOcc("open");
    }

    /* read username */
    if ((chunk = read(fd_bin, userName, sizeof(userName))) == -1) {
        errOcc("read");
    }

    /* read reminder */
    if ((chunk = read(fd_bin, rmdr, sizeof(reminder))) == -1) {
        errOcc("read");
    }
    restoreReminder(); /* then restore it */
    /* read D-day */
    loadDday(&fd_bin); /* file descriptor gets refreshed when keep reading/accessing to the file: at the point where last access occured. */

    buf = (toDoPtr)malloc(sizeof(toDo));
    while ((chunk = read(fd_bin, buf, sizeof(toDo))) != 0) {
        if (chunk == -1) break;
        insert(&key, buf);
        buf = (toDoPtr)malloc(sizeof(toDo)); // memory leak alert!!! -> freed.
    }

    free(buf);

    if (chunk == -1) {
        errOcc("read");
    }
    

    ifAlreadyLoaded = 1;

    close(fd_bin);

    fd_bin = open(pub_fileName, O_RDWR);
    if (fd_bin == -1) {
        errOcc("load");
    }

    getHolidayInfos(&fd_bin);

    close(fd_bin);

    return 0;
}
int save(void) {
    int fd_bin; /* hr for human readable */
    FILE* fd_hr;
    int written;
    //int time, min;
    //unsigned long long dates = 0;  
    //time = 0; min = 0;

    if (!key) { /* if none to save */
        return 1;
    }

    ifAlreadyLoaded = 0;

    fd_bin = open(bin_fileName, O_CREAT | O_WRONLY | O_TRUNC, 0641);
    fd_hr = fopen(hr_fileName, "w+");

    if (fd_bin == -1 || fd_hr == NULL) {
        errOcc("open");
    }

    /* username write */
    if ((written = write(fd_bin, userName, sizeof(userName))) != sizeof(userName)) {
        errOcc("write");
    }

    quickSort_byHashNum(saveLink->toDoData, 0, saveLink->maxIndex); /* sort by hash, then save. */

    /* reminder write */
    if ((written = write(fd_bin, rmdr, sizeof(reminder))) != sizeof(reminder)) {
        errOcc("write");
    }
    /* D-day write */
    saveDday(&fd_bin);

    /* bin */
    for (int i = 0; i <= saveLink->maxIndex; i++) {
        if ((saveLink->toDoData)[i]->dateData % 10000 == 9999) { /* if system default(public holiday) */
            continue; /* don't save: it's loaded from other source. */
        }
        if ((written = write(fd_bin, ((saveLink->toDoData)[i]), sizeof(toDo))) != sizeof(toDo)) {
            errOcc("write");
        }
    }
    /* hr */
    save_hr(fd_hr);
    /** unsigned long long dateData;
        int priority;
        char title[31];
        // SOCKET FEATURE {method}();
        char details[256];
    */
    
    /*
    for (int i = 0; i <= saveLink->maxIndex; i++) {
        dates = (saveLink->toDoData)[i]->dateData / 10000;
        time = (int)((saveLink->toDoData)[i]->dateData % 10000 / 100);
        min = (int)((saveLink->toDoData)[i]->dateData % 100);
        fprintf(fd_hr, "%llu | %02d:%02d -> [Priority %d], Title: %12s, Details: %s\n", dates, time, min, 
            (saveLink->toDoData)[i]->priority, (saveLink->toDoData)[i]->title, (saveLink->toDoData)[i]->details);
    }
    */

    fclose(fd_hr);
    close(fd_bin);

    ifAlreadyLoaded = 0;

    return 0;
}

int save_hr(FILE* fp) {
    yearGrp db = key;
    dayPtr temp;
    toDoPtr lnk;
    unsigned long long time;
    int i, j, k;
    char timeSttStr[100] = {'\0', };
    char timeEndStr[100] = {'\0', };
    /*---------------------------------------*/
    if (rmdr->isSet) {
        strcpy(timeSttStr, ctime(&(rmdr->start)));
        strcpy(timeEndStr, ctime(&(rmdr->end)));
    }
    /*---------------------------------------*/
    /* important: in C, there is no such thing like GC, so most of the syscall/lib returns static array! */

    if (!db) {
        return 1;
    }

    while (db->prev) {
        db = db->prev;
    }

    if (rmdr->isSet) { /* reminder */
        fprintf(fp, "Reminder Title: %s\nReminder Start: %s | Reminder End: %s\n\n", rmdr->info, timeSttStr, timeEndStr);
    }
    else {
        fprintf(fp, "Reminder has not set yet.\n\n");
    }

    if (dStack->isPlusExist == 0) { /* D-day */
        fprintf(fp, "D+ has not set yet.\n\n");
    }
    else {
        i = 0;
        fprintf(fp, "#%d: %llu, %s\n", i, (dStack->dDayArr)[i].dateData / 10000, (dStack->dDayArr)[i].title);
    }
    if (dStack->isPlusExist == 0) { /* D-day */
        fprintf(fp, "D- has not set yet.\n\n");
    }
    else {
        i = 1;
        fprintf(fp, "#%d: %llu, %s\n", i, (dStack->dDayArr)[i].dateData / 10000, (dStack->dDayArr)[i].title);
    }
    

    while (db) { /* wtf */
        fprintf(fp, "Records of the Year %d:\n", db->year);
        for (i = 1; i <= 12; i++) {
            if ((db->target->months)[i]) {
                fprintf(fp, "    -> Records of the Month %02d:\n", i);
                for (j = 1; j <= 31; j++) {
                    if (((db->target->months)[i]->dates)[j]) {
                        fprintf(fp, "        --> Records of the Day %02d:\n", j);
                        fprintf(fp, "           [ %d Record(s) in this day. ]\n", ((db->target->months)[i]->dates)[j]->maxIndex + 1);
                        temp = ((db->target->months)[i]->dates)[j];
                        /* Quick Sort */
                        
                        sortGivenDateToDos(temp, 2);
                        for (k = 0; k <= temp->maxIndex; k++) {
                            lnk = (temp->toDoArr)[k];
                            time = lnk->dateData % 10000;
                            if (lnk->priority) {
                                fprintf(fp, "             > [BookMarked]%02llu:%02llu -> Title: %12s, Details: %s\n", time / 100, time % 100,
                                lnk->title, lnk->details);
                            }
                            else {
                                fprintf(fp, "             >             %02llu:%02llu -> Title: %12s, Details: %s\n", time / 100, time % 100,
                                lnk->title, lnk->details);
                            }
                        }
                    }
                }
            }
        }
        db = db->next;
    }

    return 0;
}
void freeAllMem(void) {
    /* erases toDo "Entry", not the toDo itself; it is erased in initSaveMem() */
    yearGrp db = key;
    yearGrp tmp;
    int i, j, k;
    dayPtr temp;

    if (!db) {
        return;
    }

    while (db->prev) {
        db = db->prev;
    }

    while (db) { /* wtf */
        for (i = 1; i <= 12; i++) {
            if ((db->target->months)[i]) {
                for (j = 1; j <= 31; j++) {
                    if (((db->target->months)[i]->dates)[j]) {
                        temp = ((db->target->months)[i]->dates)[j];
                        for (k = 0; k <= temp->maxIndex; k++) {
                            free((temp->toDoArr)[k]);
                        }
                        free(((db->target->months)[i]->dates)[j]->toDoArr);
                        free(((db->target->months)[i]->dates)[j]);
                    }
                }
                //free(((db->target->months)[i]->dates)[0]);
                free((db->target->months)[i]);
            }
            //free((db->target->months)[i]->dates);
            //free((db->target->months)[i]);
        }
        if (db->target) {
            //free(db->target->months);
            free(db->target);
        }
        db = db->next;
    }

    db = key;
    while (db) {
        tmp = db;
        db = db->next;
        free(tmp);
    }

    key = NULL; /* entry reset */

    return;
}
void initSaveMem(void) {
    /* to free all memory before load something */
    //int i = 0;
    //toDoPtr temp;

    if (!saveLink) {
        return;
    }

    //for (i = 0; i <= saveLink->maxIndex; i++) {
        /* erasing toDos here! */
    //    temp = (saveLink->toDoData)[i];
    //    free(temp);
    //    (saveLink->toDoData)[i] = NULL;
    //}
    free(saveLink->toDoData);
    free(saveLink);

    saveLink = NULL; /* entry reset */

    return;
}
void putSaveData(toDoPtr target) {
    if (!saveLink) {
        saveLink = (savePtr)malloc(sizeof(saveData));
        saveLink->maxIndex = 0;
        saveLink->toDoData = (toDoPtr*)malloc(sizeof(toDoPtr));
        (saveLink->toDoData)[0] = target;
    }
    else {
        resizeSaveMem();
        (saveLink->toDoData)[saveLink->maxIndex] = target;
    }

    return;
}
toDoPtr getSaveData(void) {

    return NULL;
}
/* saving features ended */

void errOcc(const char* str) {
    fprintf(stderr, "Plan_it: ");
    perror(str);
    exit(1);
}

/*--Debug-only features----------------------------------*/

void printMarkUP(char* str, int lineLimit) {
    /* Prefix codes
        [^: for Time, [[: make new line, ^: tab inside
        ]^: for title, ]]: for details, *: for bookmarks

        How it is decoded
            pseudo code:
                algorithm PrintStringInForm <- (String target)
                    if [ found
                        then push [ to the stack
                    elif ] found
                        then push ] to the stack
                    elif ^ found
                        then push ^ to the stack
                        then pop every element of the stack
                    
                    interpret stack instructions: [^: for Time, [[: make new line, ^: tab inside, ]^: for title, ]]: for details

                    do jobs with interpreted data

                end algorithm PrintStringInForm
    */
    int cur = 0;
    int isStillInLine = 0;
    int cnter = 4; /* tab space */

    while (str[cur]) {
        switch (str[cur]) {
            /* simulates stack */
            case '[': 
                cur++;
                if (str[cur] == '^') { /* date */
                    printf("Time: ");
                }
                else { /* mk nl */
                    printf("\n");  
                }
                isStillInLine = 0;
                cnter = 4;
                break;
            case ']': 
                cur++;
                if (str[cur] == '^') { /* title form */
                    printf("[Title]\n");
                }
                else { /* details form */
                    printf("[Details]\n");  
                }
                isStillInLine = 0;
                cnter = 4;
                break;
            case '^':
                printf("    ");
                isStillInLine = 1;
                break;
            case '*':
                printf("\nBookmarked!\n");
                isStillInLine = 0;
                break;
            default:  
                if (isStillInLine) {
                    cnter++;
                    if (cnter == lineLimit) {
                        puts("");
                        printf("    ");
                        cnter = 4; /* simulates tab */
                    }
                }
                printf("%c", str[cur]);
        }
        cur++;
    }

    return;
}

void printUsage(void) {
    puts("Plan_it: ./pln [options] [argument 1...]");
    puts("      [options]:  NONE: Launch Plan_it in Normal Mode.");
    puts("      [options]:    -d: Launch Plan_it in Core Part Debugging Mode.");
    puts("      [options]:   -cl: Launch Plan_it in Client Part Debugging Mode.");
    puts("      [options]:   -in: using CLI interface, you can insert data with arguments");
    puts("      -----------------------------------------------------------------------");
    puts("      [arguments]: [YYYYMMDD][HHMM][Priority_Num][Title][Details]");

    return;
}

void __launchOptions(int argc, char* argv[]){
    if (argc == 2) {
        if (strcmp(argv[1], dbDebug) == 0) {
            __dbDebug();
        }
        if (strcmp(argv[1], clDebug) == 0) {
            __clDebug();
        }
        else {
            fprintf(stderr, "Plan_it: Unsupported command - '%s'\n", argv[1]);
            printUsage();
            exit(1);
        }
    }
    else {
        if (argc == 7 && strcmp(argv[1], cli_input)) {
            /* parts here */
        }
        else {
            if (argc != 7) {
                fprintf(stderr, "Plan_it: Unsupported Arguments - %s\n", argv[1]);
                printUsage();
                exit(1);
            }
            fprintf(stderr, "Plan_it: Unsupported command - '%s'\n", argv[1]);
            printUsage();
            exit(1);
        }
    }

    return;
}

/*-------------------------------------------------------*/

/* For Bookmark Features */
toDoPtr getBookMarked(unsigned long long src, int distance) {
    /* src: YYYYMMDDHHMM */
    toDoPtr retVal = NULL;
    int i;

    src *= 1000; /* to compare with hash */

    /* implementing hash */

    if (!saveLink) {
        return NULL;
    }

    quickSort_byHashNum(saveLink->toDoData, 0, saveLink->maxIndex);

    for (i = 0; (i <= saveLink->maxIndex && distance); i++) {
        if ((saveLink->toDoData)[i]->hashNum >= src && (saveLink->toDoData)[i]->priority) {
            distance--;
            if (!distance) {
                retVal = (saveLink->toDoData)[i];
                break;
            }
        }
    }

    return retVal;
}

void getBookMarkedInDate(unsigned long long today, int counter, char* str) {
    /* today: YYYYMMDDHHMM */
    /* returns *str w/bookmarked todo strings, based on today and number of bookmarks UX layer wants.
       important: this function returns only "upcoming" bookmarks.
        [^: for Time, [[: make new line, ^: tab inside
        ]^: for title, ]]: for details, *: for bookmarks
    */
    char retstr[BUFSIZ] = {'\0', };
    char tempstr[100] = {'\0', };
    int i;
    toDoPtr temp; /* bcggd8g6 */
    toDoPtr compare = NULL;

    //memset(retstr, 0x00, BUFSIZ);
    //memset(tempstr, 0x00, 100);

    retstr[0] = '\0';

    for (i = 1; i <= counter; i++) {
        if ((temp = getBookMarked(today, i)) && temp != compare) {
            memset(tempstr, 0x00, 100);
            if (temp->isShared) {
                sprintf(tempstr, "*%d@[^%llu]^%-25s", temp->priority, temp->dateData, temp->title);
            }
            else {
                sprintf(tempstr, "*%d[^%llu]^%-25s", temp->priority, temp->dateData, temp->title);
            }
            compare = temp;
            strcat(retstr, tempstr);
        }
    }

    strcpy(str, retstr);

    return;
}

/* For Deletion Features - 0.0.3 added */
int deleteRecord(dayPtr when, int index) {
    int i;
    int svidx = -1;

    if (index > when->maxIndex || (index < 0 || when->maxIndex == -1)) {
        return 1; /* err hndl: no such record exists */
    }
    if ((when->toDoArr)[index]->dateData % 10000 == 9999) {
        return 2; /* err hndl: cannot erase public holiday! */
    }

    /* prepare for the back structure adjustment */
    for (i = 0; i <= saveLink->maxIndex; i++) {
        if ((saveLink->toDoData)[i] == (when->toDoArr)[index]) {
            svidx = i;
            break;
        }
    }

    /* free in the tree structure */
    if ((when->toDoArr)[index]->priority != 0) { /* to support multi colored bookmarks */
        when->isBookMarkExists = 0; /* erase bookmark when target has the key */
    }

    free((when->toDoArr)[index]);

    for (i = index; i < when->maxIndex; i++) {
        (when->toDoArr)[i] = (when->toDoArr)[i + 1];
    }

    /* free in the back array structure and adjust */
    
    if (svidx != -1) { /* just to be safe */
        for (i = svidx; i < saveLink->maxIndex; i++) {
            (saveLink->toDoData)[i] = (saveLink->toDoData)[i + 1];
        }
        --(saveLink->maxIndex);
    }

    --(when->maxIndex);

    return 0;
}
int deleteWhileIterate(unsigned long long src, int pageNum) {
    /* src = YYYYMMDD */
    /*yearPtr ytmp; monthPtr mtmp; */dayPtr dtmp;

    //src /= 10000;
    /* this function is not encapsulated at the UX Layer. So we use raw 'src' value with no adjustment. */
    dtmp = search_byDate(src); // YYYYMMDD

    //ytmp = findYear(src / 10000, &key);
    //mtmp = findMonth(src % 10000 / 100, ytmp);
    //dtmp = findDay(src % 100, mtmp);

    if (dtmp->toDoArr == NULL || pageNum < 1 || pageNum > dtmp->maxIndex + 1) { /* just to be safe */
        return 1; /* nothing to del */
    }

    sortGivenDateToDos(dtmp, 2); /* being ready for index-based iteration */

    /* don't delete public holiday */
    if ((dtmp->toDoArr)[pageNum - 1]->dateData % 10000 == 9999) return 2;

    if ((dtmp->toDoArr)[pageNum - 1]->priority != 0) { /* if bookmarked... */
        dtmp->isBookMarkExists = 0;
    }

    return deleteRecord(dtmp, pageNum - 1);
}
int editWhileIterate(unsigned long long src, int pageNum, unsigned long long t_day, char* t_title, char* t_details, int t_priority) {
    /* being able to edit bookmark, date, time, title, details */
    /* if t_day := 1234        user doesn't want to edit this section of the record */
    /* if t_title[0] := NULL   user doesn't want to edit this section of the record */
    /* if t_details[0] := NULL user doesn't want to edit this section of the record */
    /* if t_priority := -1     user doesn't want to edit this section of the record */
    /* src = YYYYMMDD */

    //src /= 10000;

    yearPtr ytmp; monthPtr mtmp; dayPtr dtmp;

    ytmp = findYear(src / 10000, &key);
    mtmp = findMonth(src % 10000 / 100, ytmp);
    dtmp = findDay(src % 100, mtmp);

    if (dtmp->toDoArr == NULL || pageNum < 1 || pageNum > dtmp->maxIndex + 1) { /* just to be safe */
        return 2; /* nothing to del */
    }

    sortGivenDateToDos(dtmp, 2); /* being ready for index-based iteration */

    if (t_priority == 1 && dtmp->isBookMarkExists != 0 && (dtmp->toDoArr)[pageNum - 1]->priority != 1) {
        return 1; /* if already bookmarked todo exists */
    }

    if ((dtmp->toDoArr)[pageNum - 1]->priority != 0) { /* if tries to edit what's bookmarked... */
        dtmp->isBookMarkExists = 0;
    }

    /* actual editing start */
    /*                                                    Don't allow users to edit system default holiday */
    if (t_day != 1234 && (dtmp->toDoArr)[pageNum - 1]->dateData != 9999)              (dtmp->toDoArr)[pageNum - 1]->dateData = t_day;
    if (t_title[0] != '\0')                                                           strcpy((dtmp->toDoArr)[pageNum - 1]->title, t_title);
    if (t_details[0] != '\0')                                                         strcpy((dtmp->toDoArr)[pageNum - 1]->details, t_details);
    if (t_priority != -1)                                                             (dtmp->toDoArr)[pageNum - 1]->priority = t_priority;
    /* actual editing ended */
    if ((dtmp->toDoArr)[pageNum - 1]->priority == 1) {
        dtmp->isBookMarkExists = 1; /* if it has passed the test at the first */
    }
    
    /* don't use deletion based editing anymore; we actually edit it */
    //deleteRecord(dtmp, pageNum - 1);

    /* insert does the thing about bookmark check */
    //insert(&key, create_Node(t_day, t_priority, t_title, t_details));

    return 0; /* safely executed: 0 */
}
/* For Reminder Features - 0.0.5 added */
void reminderHandler(int signum) {
    /* this is a debug feature */
    /* so UX team needs to implement this function in UX Environment */

    /* UX Team MUST implement parts below in UX Core Codes */

    if (rmdr->repeatCounter == 0) {
        setReminderHandler(0, 0, 0); /* turn off alarm */
        /* some_UX_Actions Need Here when the timer has just been expired */
        /* below is just a sample for CLI Debugging environment */
        printf("Timer Expired\n");
        /* sample ended */

        return;
    }
    
    /* sample output */
    printf("*******[%s]: Timer %d Second(s) Left*******\n", rmdr->info, (int)(rmdr->repeatCounter) * (int)(rmdr->intervals));
    /* important */
    (rmdr->repeatCounter)--;

    return;
}
void setReminderHandler(int status, unsigned long init, unsigned long repeat) {
    /* status 1 for set, 0 for off */
    /* static */
    static struct sigaction inmode;
    static struct sigaction origin;
    static struct itimerval set;
    static struct itimerval org;

    if (status == 1) {
        inmode.sa_handler = reminder_extends_popup;
        inmode.sa_flags &= ~SA_RESETHAND;
        inmode.sa_flags &= ~SA_SIGINFO;

        if (sigaction(SIGALRM, &inmode, &origin) == -1) {
            errOcc("sigaction");
        }
    }
    else { /* OFF */
        if (sigaction(SIGALRM, &origin, NULL) == -1) {
            errOcc("sigaction");
        }
    }

    if (status == 1) {
        set.it_value.tv_sec = init;
        set.it_value.tv_usec = 0;
        set.it_interval.tv_sec = repeat;
        set.it_interval.tv_usec = 0;

        if (setitimer(ITIMER_REAL, &set, &org) == -1) {
            errOcc("getitimer");
        }
    }
    else { /* OFF */
        rmdr->isSet = 0;
        if (setitimer(ITIMER_REAL, &org, NULL) == -1) {
            errOcc("getitimer");
        }
    }

    return;
}
int setReminder(time_t current, time_t delta, int repeatCnter, char* what, int intervals) {
    /*      time_t: current, delta */
    /* delta means: current + delta = reminder target time */
    /*   intervals: in seconds form */
    /*       delta: in seconds form*/

    //intervals *= 60; /* then turn it into secs */

    /* if not possible */
    if (delta < intervals || delta < intervals * repeatCnter) {
        //printf("err1\n");
        return 1; /* err: cannot set */
    }

    /* if there's already a reminder exists */
    if (rmdr->isSet) {
        //printf("err2\n");
        return 2; /* err: already exists */
    }

    /* setting start */
    rmdr->isSet = 1;
    rmdr->start = current;
    rmdr->end = current + delta;
    rmdr->intervals = intervals;
    rmdr->repeatCounter = repeatCnter;
    strcpy(rmdr->info, what);
    /*                                                                                 magic number*/
    setReminderHandler(1, rmdr->end - rmdr->start - rmdr->intervals * rmdr->repeatCounter + 1, rmdr->intervals);
    //printf("safe\n");
    return 0; /* safely executed. */
}
int isReminderSetAlready(char* str) {
    if (rmdr->isSet) {
        strcpy(str, rmdr->info);
    }

    return rmdr->isSet;
}
void restoreReminder(void) {
    if (rmdr->isSet) {
        //printf("check\n");
        if (time(NULL) < rmdr->end) {
            //printf("check\n");
            rmdr->isSet = 0; /* due to the algorithm */
            setReminder(time(NULL), rmdr->end - time(NULL), rmdr->repeatCounter, rmdr->info, rmdr->intervals);
        }
        else {
            //printf("check\n");
            turnOffReminder(); /* if it's already expired */
        }
    }

    return;
}
void allocReminder(void) {
    rmdr = (reminderPtr)malloc(sizeof(reminder));

    rmdr->isSet = 0;
    rmdr->start = 0;
    rmdr->end = 0;
    rmdr->intervals = 0;
    rmdr->repeatCounter = 0;
    (rmdr->info)[0] = '\0';

    return;
}
void turnOffReminder(void) {
    setReminderHandler(0, 0, 0);
    rmdr->isSet = 0;
    free(rmdr);
    allocReminder();

    return;
}
void myscanf(FILE* fd, char* target) {
    /* assume that fd is stdin, so we don't open() */
    char c = '\0';
    char temp[BUFSIZ] = {'\0', }; /* warning: max len: SYSBUFSIZ - 1 */
    int idx = 0;

    while ((c = getchar()) != EOF) {
        temp[idx++] = c;
    }
    temp[idx] = '\0';

    strcpy(target, temp);

    return;
}

int isBookMarked(unsigned long long targetDate) { /* YYYYMMDD */
    yearPtr ytmp; monthPtr mtmp; dayPtr dtmp;
    int i;

    ytmp = findYear(targetDate / 10000, &key);
    mtmp = findMonth(targetDate % 10000 / 100, ytmp);
    dtmp = findDay(targetDate % 100, mtmp);

    if (dtmp->toDoArr == NULL) { /* just to be safe */
        return 0; /* no record existed at the date at first */
    }

    for (i = 0; i <= dtmp->maxIndex; i++) {
        if ((dtmp->toDoArr)[i]->priority) {
            return (dtmp->toDoArr)[i]->priority; /* return bookmark number */
        }
    }

    /* if no bookmarks exists */
    return 0;
}

/* For further implementations: Basic search algorithm
    yearPtr ytmp; monthPtr mtmp; dayPtr dtmp;

    ytmp = findYear(src / 10000, &key);
    mtmp = findMonth(src % 10000 / 100, ytmp);
    dtmp = findDay(src % 100, mtmp);
*/
void initDday(void) {
    if (dStack) {
        free(dStack);
    }

    dStack = (dDayPtr)malloc(sizeof(dDay));
    dStack->maxIndex = 1;
    dStack->isPlusExist = 0;
    dStack->isMinusExist = 0;

    if (dStack == NULL) {
        errOcc("initDday malloc");
    }

    return;
}
void setDdayStack(toDoPtr target, int addto) {
    /* addto := 0 when D+, addto := 1 when D- */
    switch (addto) {
        case 0: /* D+ add */
            //if (dStack->isPlusExist) {
                //free(dStack->dDayArr[0]);
            //}
            //(dStack->dDayArr)[0] = (toDoPtr)malloc(sizeof(toDo));
            //if ((dStack->dDayArr)[0] == NULL) {
            //    errOcc("allocDdayStack");
            //}
            //memcpy((dStack->dDayArr)[0], target, sizeof(toDo)); /* memcpy */
            (dStack->dDayArr)[0].dateData = target->dateData;
            strcpy((dStack->dDayArr)[0].title, target->title);
            dStack->isPlusExist = 1;
            break;
        case 1:
            //if (dStack->isMinusExist) {
                //free(dStack->dDayArr[1]);
            //}
            //(dStack->dDayArr)[1] = (toDoPtr)malloc(sizeof(toDo));
            //if ((dStack->dDayArr)[1] == NULL) {
            //    errOcc("allocDdayStack");
            //}
            //memcpy((dStack->dDayArr)[1], target, sizeof(toDo)); /* memcpy */
            (dStack->dDayArr)[1].dateData = target->dateData;
            strcpy((dStack->dDayArr)[1].title, target->title);
            dStack->isMinusExist = 1;
            break;
        default:
            break;
    }

    return;
}
void delDdayStack(int whatto) {
    /* whatto := 0 when D+, whatto := 1 when D- */
    switch (whatto) {
        case 0: /* D+ add */
            //if (dStack->isPlusExist) {
                //free(dStack->dDayArr[0]);
                (dStack->dDayArr)[0].title[0] = '\0';
                dStack->isPlusExist = 0;
            //}
            break;
        case 1:
            //if (dStack->isMinusExist) {
                //free(dStack->dDayArr[1]);
                (dStack->dDayArr)[1].title[0] = '\0';
                dStack->isMinusExist = 0;
            //}
            break;
        default:
            break;
    }

    return;
}
int isBothDdayTheSame(toDoPtr s, toDo o) {
    /* we can't compare the identity of toDoPtrs by memory address; it may vary by the time when it was stored on the memory */
    if ((o.title)[0] == '\0') {
        return 0; /* new */
    }
    if (s->dateData == o.dateData && strcmp(s->title, o.title) == 0) {
        return 1; /* to del */
    }

    return 2; /* to override */
}
int checkDdayWhileIterate(unsigned long long src, int pageNum, int mode) {
    /* src = YYYYMMDD, mode 0 if D+, mode 1 if D- */
    yearPtr ytmp; monthPtr mtmp; dayPtr dtmp;

    ytmp = findYear(src / 10000, &key);
    mtmp = findMonth(src % 10000 / 100, ytmp);
    dtmp = findDay(src % 100, mtmp);

    if (dtmp->toDoArr == NULL || pageNum < 1 || pageNum > dtmp->maxIndex + 1) { /* just to be safe */
        return 3; /* no such data- */
    }

    sortGivenDateToDos(dtmp, 2); /* being ready for index-based iteration */

    /* (toDoPtr, toDo) */
    return isBothDdayTheSame((dtmp->toDoArr)[pageNum - 1], (dStack->dDayArr)[mode]);
}
void setDdayWhileIterate(unsigned long long src, int pageNum, int mode) {
    /* src = YYYYMMDD */
    yearPtr ytmp; monthPtr mtmp; dayPtr dtmp;

    ytmp = findYear(src / 10000, &key);
    mtmp = findMonth(src % 10000 / 100, ytmp);
    dtmp = findDay(src % 100, mtmp);

    if (dtmp->toDoArr == NULL || pageNum < 1 || pageNum > dtmp->maxIndex + 1) { /* just to be safe */
        return; /* no such data- */
    }

    sortGivenDateToDos(dtmp, 2); /* being ready for index-based iteration */

    setDdayStack((dtmp->toDoArr)[pageNum - 1], mode);

    return;
}
/*  

    int dplus;
    getDday(&dplus)
    printf("D+%d\n", dplus);
*/
void getDday(int* slot1, char* title1, int* slot2, char* title2) { /* returns NULL if no existence for each title[n] */
    struct tm dt;
    struct tm today;
    time_t curdat;

    //today = (struct tm*)malloc(sizeof(struct tm));
    time(&curdat);

    //localtime_r(&curdat, &today); /* they say localtime() can fail... */
    today = *localtime(&curdat);

    /* D+ handling */
    if (dStack->isPlusExist == 0) {
        title1[0] = '\0';
    }
    else {
        /* deprecated
        sprintf(str1, "%04llu-%02llu-%02llu\n", (dStack->toDoStack)[0]->dateData / 10000, (dStack->toDoStack)[0]->dateData % 10000 / 100,
            (dStack->toDoStack)[0]->dateData % 100);
        strptime(str1, "%F", &dt);
        */
        dt.tm_year = ((dStack->dDayArr)[0].dateData / 10000) / 10000 - 1900;
        dt.tm_mon = (((dStack->dDayArr)[0].dateData / 10000) % 10000) / 100 - 1; /* struct tm's day range: [1, 31], but month: [0, 11] why??? */
        dt.tm_mday = ((dStack->dDayArr)[0].dateData / 10000) % 100;
        *slot1 = julian_day(&today) - julian_day(&dt);
        if (*slot1 == 0) {
            *slot1 = 1; /* For the purpose of calculating the D-day according to social conventions */
        }
        //printf("%d %d %d to %d %d %d\n", dt.tm_year, dt.tm_mon, dt.tm_mday, today->tm_year, today->tm_mon, today->tm_mday);
        if (strlen((dStack->dDayArr)[0].title) <= 18) {
            strcpy(title1, (dStack->dDayArr)[0].title);
        }
        else {
            strncpy(title1, (dStack->dDayArr)[0].title, 15);
            strcat(title1, "...\0");
        }
    }

    /* D- Handling */
    if (dStack->isMinusExist == 0) {
        title2[0] = '\0';
    }
    else {
        /* deprecated
        sprintf(str1, "%04llu-%02llu-%02llu\n", (dStack->toDoStack)[1]->dateData / 10000, ((dStack->toDoStack)[1]->dateData % 10000) / 100,
            (dStack->toDoStack)[1]->dateData % 100);
        strptime(str1, "%F", &dt);
        */
        dt.tm_year = ((dStack->dDayArr)[1].dateData / 10000) / 10000 - 1900;
        dt.tm_mon = (((dStack->dDayArr)[1].dateData / 10000) % 10000) / 100 - 1; /* struct tm's day range: [1, 31], but month: [0, 11] why??? */
        dt.tm_mday = ((dStack->dDayArr)[1].dateData / 10000) % 100;
        //printf("%d %d %d to %d %d %d\n", dt.tm_year, dt.tm_mon, dt.tm_mday, today->tm_year, today->tm_mon, today->tm_mday);
        *slot2 = julian_day(&today) - julian_day(&dt);
        if (strlen((dStack->dDayArr)[1].title) <= 18) {
            strcpy(title2, (dStack->dDayArr)[1].title);
        }
        else {
            strncpy(title2, (dStack->dDayArr)[1].title, 15);
            strcat(title2, "...\0");
        }
    }

    return;
}
void saveDday(int* fd) {
    int i = 0; int btw = 0;

    btw = write(*fd, &(dStack->maxIndex), sizeof(int));
    if (btw == -1) {
        errOcc("saveDday");
    }
    for (i = 0; i <= dStack->maxIndex; i++) {
        btw = write(*fd, &(dStack->dDayArr)[i], sizeof(toDo));
        if (btw == -1) {
            errOcc("saveDday");
        }
    }
}
void loadDday(int *fd) {
    int i = 0; int btw = 0;
    toDoPtr buf;

    btw = read(*fd, &i, sizeof(int));
    if (btw == -1) errOcc("loadDday");

    dStack->maxIndex = i;
    
    for (i = 0; i <= dStack->maxIndex; i++) {
        buf = (toDoPtr)malloc(sizeof(toDo));
        if (buf == NULL) errOcc("loadDday");
        btw = read(*fd, buf, sizeof(toDo));
        if (btw == -1) errOcc("loadDday");
        setDdayStack(buf, i);
        /* we SHOULD free it since it is being copied to the memory - memcpy*/
        free(buf);
    }
    
    return;
}
int isHoliday(unsigned long long target) { /* YYYYMMDD */
    yearPtr ytmp; monthPtr mtmp; dayPtr dtmp;

    ytmp = findYear(target / 10000, &key);
    mtmp = findMonth(target % 10000 / 100, ytmp);
    dtmp = findDay(target % 100, mtmp);

    if (!dtmp) return 0; /* no record at first */

    return dtmp->isHoliday;
}
void getHolidayInfos(int* fd) { /* let fp := "public.dsv", dsv stands for default save */
    int bytesRead = 0;
    toDoPtr buf;

    if (!(*fd)) {
        errOcc("getHolidayInfos");
    }

    buf = (toDoPtr)malloc(sizeof(toDo));
    while ((bytesRead = read(*fd, buf, sizeof(toDo))) != 0) {
        if (bytesRead == -1) {
            errOcc("getHolidayInfos");
        }
        insert(&key, buf);
        buf = (toDoPtr)malloc(sizeof(toDo));
    } free(buf); /* last dummy one */

    //puts("why this isnt working");

    return;
}
void clearAll(void) {
    freeAllMem();
    initSaveMem();

    free(leapYear);
    free(twenty_Eight);
    free(twenty_Nine);
    free(thirty);
    free(thirty_one);

    free(rmdr);
    free(dStack);

    /* all heap blocks were freed -- no leaks are possible! */

    return;
}
/* 0519 added for network features */
void setUserName(char* myName) {
    int i = 0;

    memset(userName, 0x00, sizeof(userName));
    strcpy(userName, myName);

    if (saveLink == NULL) {
        return;
    }

    /* remap names */
    for (i = 0; i <= saveLink->maxIndex; i++) {
        if ((saveLink->toDoData)[i]->isShared == 0) { /* if not shared */
            strcpy((saveLink->toDoData)[i]->userName, userName);
        }
    }

    return;
}
int isSharedToDoExisting(unsigned long long targetDate) {
    /* YYYYMMDD */
    yearPtr ytmp; monthPtr mtmp; dayPtr dtmp;

    ytmp = findYear(targetDate / 10000, &key);
    mtmp = findMonth(targetDate % 10000 / 100, ytmp);
    dtmp = findDay(targetDate % 100, mtmp);

    if (!dtmp) return 0; /* no record at first */

    return ((dtmp)->sharedToDoExists); /* 1: shared, 2: being shared */
}
int shareWhileIterate(unsigned long long src, int pageNum, char* shareCode) {
    /* src = YYYYMMDD, (dtmp->toDoArr)[pageNum - 1] */
    int rtnval;
    yearPtr ytmp; monthPtr mtmp; dayPtr dtmp; toDoPtr temp;

    ytmp = findYear(src / 10000, &key);
    mtmp = findMonth(src % 10000 / 100, ytmp);
    dtmp = findDay(src % 100, mtmp);

    if (dtmp->toDoArr == NULL || pageNum < 1 || pageNum > dtmp->maxIndex + 1) { /* just to be safe */
        return 3; /* no such data- */
    }

    sortGivenDateToDos(dtmp, 2); /* being ready for index-based iteration */

    temp = (dtmp->toDoArr)[pageNum - 1];

    rtnval = pushToServer(temp, shareCode);

    if (rtnval == 0) { /* only if success */
        temp->isShared = 2;
        dtmp->sharedToDoExists = 1;
        strcpy(temp->code, shareCode);
    }

    return rtnval; /* 2: cannot connect/server connection error, 0: success*/
}
int pushToServer(toDoPtr o, char* shareCode) {
    return cli_pushToDoDataToServer(o, shareCode);
}
void deBookMarkInDate(unsigned long long src) {
    int i;
    yearPtr ytmp; monthPtr mtmp; dayPtr dtmp;

    ytmp = findYear(src / 10000, &key);
    mtmp = findMonth(src % 10000 / 100, ytmp);
    dtmp = findDay(src % 100, mtmp);

    if (dtmp == NULL) {
        return;
    }

    for (i = 0; i <= dtmp->maxIndex; i++) {
        if ((dtmp->toDoArr)[i]->priority != 0) {
            (dtmp->toDoArr)[i]->priority = 0;
            break;
        }
    }
    dtmp->isBookMarkExists = 0;

    return;
}
int getFromServer_Highlevel(char* shareCode) {
    int rtn;
    toDoPtr temp = (toDoPtr)malloc(sizeof(toDo));
    memset(temp->title, 0x00, 26);
    memset(temp->details, 0x00, 61);

    if (temp == NULL) errOcc("gFS_Hl err");

    if ((rtn = cli_getToDoDataFromServer(temp, shareCode)) == 1) {
        free(temp);
        return 1; /* no such data */
    }
    else if (rtn == 2) {
        free(temp);
        return 2; /* cannot connect to the server */
    }
    else {
        if (temp->priority != 0) {
            deBookMarkInDate(temp->dateData / 10000);
        }
        insert(&key, temp);
        //printf("%s\n", temp->title);
    }

    return 0;
}
int getFromServer(toDoPtr target, char* shareCode) {
    return cli_getToDoDataFromServer(target, shareCode);
}
char* getUserName(void) {
    return userName;
}