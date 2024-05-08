// KNU CSE 2021114026 Jeongwoo Kim
// recent update: 240506 v0.0.5
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
    unsigned long long hashNum;
    unsigned long long dateData;
    int priority;
    char title[31];
    // SOCKET FEATURE {method}();
    char details[61];
} toDo;
typedef toDo* toDoPtr;

typedef struct day {
    int isBookMarkExists;
    int maxIndex;
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
/**/
const char* bin_fileName = "todos.sv";
const char*  hr_fileName = "todos.txt"; /* hr stands for human readable */
const char*   dbDebug = "-d";
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
void deleteRecord(dayPtr when, int index);

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

void deleteWhileIterate(unsigned long long src, int pageNum);
int editWhileIterate(unsigned long long src, int pageNum, unsigned long long t_day, char* t_title, char* t_details, int t_priority);
int setReminder(time_t current, time_t delta, int repeatCnter, char* what, int intervals);
int isReminderSetAlready(char* str);
void turnOffReminder(void);

void printUsage(void);
void __launchOptions(int argc, char* argv[]);

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

    allocReminder();

    /* dummy datas just for leap year / month limit check */
    leapYear     = (monthPtr)malloc(sizeof(month));
    twenty_Eight = (dayPtr)malloc(sizeof(day));
    twenty_Nine  = (dayPtr)malloc(sizeof(day));
    thirty       = (dayPtr)malloc(sizeof(day));
    thirty_one   = (dayPtr)malloc(sizeof(day));

    while (r) {
        puts("Plan_it DB Core Debugger Menu");
        puts("Basic operation: ");
        puts("(1) to insert\n(2) to print all");
        puts("(3) to save\n(4) to load\n(0) to quit");
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
        printf("Type: ");
        //getchar();
        scanf("%d", &input);
        switch (input) {
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
                else{
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
            case 0:
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

    if (newOne == NULL) {
        errOcc("malloc"); /* err hndling */
    }

    newOne->hashNum = 0;
    newOne->dateData = date;
    newOne->priority = priority_num;
    strcpy(newOne->title, title);
    strcpy(newOne->details, info);

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
    int year, month, date; //hour, minute;
    yearPtr   yy;
    monthPtr  mm;
    dayPtr    dd;

    /* Split */
    year =   (targetData->dateData) / 100000000;
    month =  (targetData->dateData) % 100000000 / 1000000;
    date =   (targetData->dateData) % 1000000 / 10000;
    //hour =   (targetData->dateData) % 10000 / 100;
    //minute = (targetData->dateData) % 100;

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

    if (!when) {
        puts("No datas to sort");
        return;
    }

    /* kind of radix(bucket) */
    if (sortType == 1) {
        quickSort_byDate(when->toDoArr, 0, when->maxIndex);
        quickSort_byPriNum(when->toDoArr, 0, when->maxIndex);
    }
    else {
        quickSort_byPriNum(when->toDoArr, 0, when->maxIndex);
        quickSort_byDate(when->toDoArr, 0, when->maxIndex);
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
        printf("            %llu: [%d] %s || %s\n", (when->toDoArr)[i]->dateData, (when->toDoArr)[i]->priority, (when->toDoArr)[i]->title, (when->toDoArr)[i]->details);
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
            if ((dd->toDoArr)[i]->priority) /* if bookmarked */ {
                /*            Time->BookMarked->Title*/
                sprintf(temp, "[^%04llu*]^%-30s", (dd->toDoArr)[i]->dateData % 10000, (dd->toDoArr)[i]->title);
            }
            else {
                /*            Time->Title*/
                sprintf(temp, "[^%04llu]^%-30s", (dd->toDoArr)[i]->dateData % 10000, (dd->toDoArr)[i]->title);
            }
            /* Ver 2.0, 0504 */
            strcat(str, temp);
        }
    }

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

        if ((dd->toDoArr)[i]->priority) { /* if bookmarked */
            /*         Time->BookMarked->Title(NL)->Details*/
            sprintf(temp, "[^%04llu*]^%-30s]]%s", (dd->toDoArr)[i]->dateData % 10000, (dd->toDoArr)[i]->title, (dd->toDoArr)[i]->details);
        }
        else {
            /*         Time->Title(NL)->Details*/
            sprintf(temp, "[^%04llu]^%-30s]]%s", (dd->toDoArr)[i]->dateData % 10000, (dd->toDoArr)[i]->title, (dd->toDoArr)[i]->details);
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

    fd_bin = open(bin_fileName, O_CREAT | O_RDWR); /* if exists, then read, or not, then creat */
    if (fd_bin == -1) {
        errOcc("open");
    }

    /* read reminder */
    if ((chunk = read(fd_bin, rmdr, sizeof(reminder))) == -1) {
        errOcc("read");
    }
    restoreReminder(); /* then restore it */

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

    quickSort_byHashNum(saveLink->toDoData, 0, saveLink->maxIndex); /* sort by hash, then save. */

    /* reminder write */
    if ((written = write(fd_bin, rmdr, sizeof(reminder))) != sizeof(reminder)) {
        errOcc("write");
    }
    /* bin */
    for (int i = 0; i <= saveLink->maxIndex; i++) {
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
    strcpy(timeSttStr, ctime(&(rmdr->start)));
    strcpy(timeEndStr, ctime(&(rmdr->end)));
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
                                fprintf(fp, "             > \t\t\t%02llu:%02llu -> Title: %12s, Details: %s\n", time / 100, time % 100,
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
    int i, j;

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
                        free(((db->target->months)[i]->dates)[j]->toDoArr);
                        free(((db->target->months)[i]->dates)[j]);
                    }
                }
            }
            //free((db->target->months)[i]->dates);
            free((db->target->months)[i]);
        }
        if (db->target) {
            //free(db->target->months);
            free(db->target);
        }
        db = db->next;
    }

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
    int i = 0;
    toDoPtr temp;

    if (!saveLink) {
        return;
    }

    for (i = 0; i <= saveLink->maxIndex; i++) {
        /* erasing toDos here! */
        temp = (saveLink->toDoData)[i];
        free(temp);
        //(saveLink->toDoData)[i] = NULL;
    }
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
    toDoPtr retVal = NULL;
    int i;

    src *= 10000;
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
    /* returns *str w/bookmarked todo strings, based on today and number of bookmarks UX layer wants.
       important: this function returns only "upcoming" bookmarks.
        [^: for Time, [[: make new line, ^: tab inside
        ]^: for title, ]]: for details, *: for bookmarks
    */
    char* retstr = NULL;
    char tempstr[50];
    int i;
    toDoPtr temp;

    retstr = (char*)malloc(sizeof(char) * 50 * counter); /* alloc, just to be safe */
    if (!retstr) {
        errOcc("malloc");
    }
    retstr[0] = '\0';

    for (i = 1; i <= counter; i++) {
        if ((temp = getBookMarked(today, i))) {
            sprintf(tempstr, "*[^%04llu]^%-30s", temp->dateData % 10000, temp->title);
        }
        strcat(retstr, tempstr);
    }

    strcpy(str, retstr);

    return;
}

/* For Deletion Features - 0.0.3 added */
void deleteRecord(dayPtr when, int index) {
    int i;
    int svidx = -1;

    /* prepare for the back structure adjustment */
    for (i = 0; i <= saveLink->maxIndex; i++) {
        if ((saveLink->toDoData)[i] == (when->toDoArr)[index]) {
            svidx = i;
            break;
        }
    }

    /* free in the tree structure */
    if (index > when->maxIndex || (index < 0 || when->maxIndex == -1)) {
        return; /* err hndl */
    }

    if ((when->toDoArr)[index]->priority == 1) {
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

    return;
}
void deleteWhileIterate(unsigned long long src, int pageNum) {
    /* src = YYYYMMDD */
    yearPtr ytmp; monthPtr mtmp; dayPtr dtmp;

    ytmp = findYear(src / 10000, &key);
    mtmp = findMonth(src % 10000 / 100, ytmp);
    dtmp = findDay(src % 100, mtmp);

    if (dtmp->toDoArr == NULL || pageNum < 1 || pageNum > dtmp->maxIndex + 1) { /* just to be safe */
        return; /* nothing to del */
    }

    sortGivenDateToDos(dtmp, 2); /* being ready for index-based iteration */
    if ((dtmp->toDoArr)[pageNum - 1]->priority == 1) { /* if bookmarked... */
        dtmp->isBookMarkExists = 0;
    }

    deleteRecord(dtmp, pageNum - 1);
    

    return;
}
int editWhileIterate(unsigned long long src, int pageNum, unsigned long long t_day, char* t_title, char* t_details, int t_priority) {
    /* being able to edit bookmark, date, time, title, details */
    /* if t_day := 1234        user doesn't want to edit this section of the record */
    /* if t_title[0] := NULL   user doesn't want to edit this section of the record */
    /* if t_details[0] := NULL user doesn't want to edit this section of the record */
    /* if t_priority := -1     user doesn't want to edit this section of the record */
    /* src = YYYYMMDD */
    yearPtr ytmp; monthPtr mtmp; dayPtr dtmp;

    ytmp = findYear(src / 10000, &key);
    mtmp = findMonth(src % 10000 / 100, ytmp);
    dtmp = findDay(src % 100, mtmp);

    if (dtmp->toDoArr == NULL || pageNum < 1 || pageNum > dtmp->maxIndex + 1) { /* just to be safe */
        return 2; /* nothing to del */
    }

    sortGivenDateToDos(dtmp, 2); /* being ready for index-based iteration */

    if (t_priority == 1 && dtmp->isBookMarkExists == 1 && (dtmp->toDoArr)[pageNum - 1]->priority != 1) {
        return 1; /* if already bookmarked todo exists */
    }

    if ((dtmp->toDoArr)[pageNum - 1]->priority == 1) { /* if tries to edit what's bookmarked... */
        dtmp->isBookMarkExists = 0;
    }

    /* actual editing start */
    if (t_day != 1234)              (dtmp->toDoArr)[pageNum - 1]->dateData = t_day;
    if (t_title[0] != '\0')         strcpy((dtmp->toDoArr)[pageNum - 1]->title, t_title);
    if (t_details[0] != '\0')       strcpy((dtmp->toDoArr)[pageNum - 1]->details, t_details);
    if (t_priority != -1)           (dtmp->toDoArr)[pageNum - 1]->priority = t_priority;
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
        inmode.sa_handler = reminderHandler;
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