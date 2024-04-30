// KNU CSE 2021114026 Jeongwoo Kim
// recent update: 240428 v0.0.3
// Basic implementation of ADT:
/**
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct toDo {
    unsigned long long hashNum;
    unsigned long long dateData;
    int priority;
    char title[31];
    // SOCKET FEATURE {method}();
    char details[256];
} toDo;
typedef toDo* toDoPtr;

typedef struct day {
    int maxIndex;
    toDoPtr* toDoArr;
} day;
typedef day* dayPtr;

typedef struct month {
    /**
     * [0] is used for 28/29/30/31 det.
     *      but how???
    */
    dayPtr dates[32];
} month;
typedef month* monthPtr;

typedef struct year {
    /**
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
/**/

/* saving features start */
void resizeSaveMem(void);
int load(void);
int save(void);
void initSaveMem(void);
void putSaveData(toDoPtr target);
toDoPtr getSaveData(void);
/* saving features ended */

void errOcc(const char* str);

toDoPtr create_Node(unsigned long long date, int priority_num, const char* title, const char* info);
void resizeArr_longer(dayPtr target);
void insert_toDo(dayPtr when, toDoPtr target);
dayPtr create_day(void);
dayPtr findDay(int target, monthPtr db);
monthPtr create_month(void);
monthPtr findMonth(int target, yearPtr db);
yearGrp create_yearGrp(int num);
yearPtr findYear(int target, yearGrp* db);
void insert(yearGrp* db, toDoPtr targetData);
void printAll(yearGrp db);
dayPtr search_byDate(unsigned long long target);
int get_toDo_byForm(toDoPtr target, char** buf);

void sortGivenDateToDos(dayPtr when, int sortType);
void quickSort_byPriNum(toDoPtr* arr, int from, int to);
void quickSort_byDate(toDoPtr* arr, int from, int to);

void printToday(dayPtr when);

/*--UX Layer interactive API Methods---------------------*/

int getNumOfSchedule(unsigned long long targetDate);
void getUpcomingSchedule(unsigned long long today, char* strbuf, int scrSize);
void setSchedule(unsigned long long today, char* title, char* details, int priority);
void getTodaySchedule(unsigned long long today, int sortType, char* strbuf, int scrSize); /* debugging only feature */

void getTodaySchedule_Summarized(unsigned long long today, int sortType, char* strbuf, int maxLines, int width);
int getTodaySchedule_withDetails(unsigned long long today, int sortType, char* strbuf, int maxLines, int width);
void getTodaySchedule_withDetails_iterEnd(void);

/*-------------------------------------------------------*/

/*--Debug-only features----------------------------------*/

void printMarkUP(char* str, int lineLimit);

/*-------------------------------------------------------*/

int main(void) {
    int input; int input_2;
    unsigned long long date; int pnum; char title[30]; char details[256];
    int r = 1;
    int i = 0;
    char testStr[BUFSIZ];

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
                    scanf("%s", title); getchar();//printf("%s <- \n", title);
                    scanf("%s", details); getchar();//printf("%s <- \n", details);

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
                printf("Type sorting type(2 for time first, 1 for priority first): \n");
                scanf("%d", &input);
                getTodaySchedule_Summarized(date, input, testStr, 10, 10);
                printf("%s\n", testStr);
                break;
            case 8:
                printf("Type scan target YYYYMMDD: \n");
                scanf("%llu", &date); //getchar();
                printf("Type sorting type(2 for time first, 1 for priority first): \n");
                scanf("%d", &input);
                printf("starting iteration...\nType 1 to move to the next page, 2 to terminate: ");
                scanf("%d", &input_2);
                while (input_2 != 2) {
                    input_2 = getTodaySchedule_withDetails(date, input, testStr, 10, 10);
                    printf("%s\n", testStr);
                    printf("Page: %d\ncontinuing iteration...\nType 1 to move to the next page, 2 to terminate: ", input_2);
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
            case 0:
                r = 0;
                break;
            default:
                break;
        }
        printf("\n\n");
    }

    return 0;
}

/*-----------------------*/

toDoPtr create_Node(unsigned long long date, int priority_num, const char* title, const char* info) {
    toDoPtr newOne;

    newOne = (toDoPtr)malloc(sizeof(toDo));

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
    printf("Size extended to %d->%d\n", size - 1, size);

    /* remap hash */
    for (i = 0; i < target->maxIndex; i++) {
        (target->toDoArr)[i]->hashNum = ((target->toDoArr)[0]->dateData / 100) * 100 + i;
    }

    return;
}

void insert_toDo(dayPtr when, toDoPtr target) {
    if (when->maxIndex == -1) {
        when->toDoArr = (toDoPtr*)malloc(sizeof(toDoPtr));
        when->maxIndex = 0;
    }
    else {
        resizeArr_longer(when);
    }
    (when->toDoArr)[when->maxIndex] = target; // SUBJECT TO CHANGE: TO MAX HEAP

    /* remap hash */
    (when->toDoArr)[when->maxIndex]->hashNum = ((when->toDoArr)[0]->dateData / 100) * 100 + when->maxIndex;
    return;
}

dayPtr create_day(void) {
    dayPtr newOne = (dayPtr)malloc(sizeof(day));

    newOne->maxIndex = -1;
    newOne->toDoArr = NULL;

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

    for (i = 0; i < 32; i++) {
        (newOne->dates)[i] = NULL;
    }

    return newOne;
}

monthPtr findMonth(int target, yearPtr db) {
    if (!(db->months[target])) {
        db->months[target] = create_month();
    }

    return db->months[target];
}

yearGrp create_yearGrp(int num) {
    int i;
    yearPtr newYear;
    yearGrp newGrp;

    newYear = (yearPtr)malloc(sizeof(year));
    newGrp = (yearGrp)malloc(sizeof(yearsLL));

    newGrp->year = num;
    newGrp->target = newYear;
    newGrp->next = newGrp->prev = NULL;

    for (i = 0; i < 13; i++) {
        (newYear->months)[i] = NULL;
    }

    return newGrp;
}

yearPtr findYear(int target, yearGrp* db) {
    yearGrp iterator; yearGrp newOne; yearGrp append;
    int isFound = 0;

    /* iter */
    iterator = (*db);
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

void insert(yearGrp* db, toDoPtr targetData) {
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
    dd = findDay(date, mm);

    /* Insert */
    insert_toDo(dd, targetData);
    putSaveData(targetData);

    return;
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
        for (i = 0; i < 13; i++) {
            if ((db->target->months)[i]) {
                printf("    ->Records of the Month %d:\n", i);
                for (j = 0; j < 32; j++) {
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
    int year, month, date; //hour, minute;
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
                mm = yy->months[month];
                if (mm->dates[date]) {
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

void sortGivenDateToDos(dayPtr when, int sortType) {
    /**
     * sortType 1: priority first
     * sortType 2: time first
     * other numbers: regarded as priority
    */

    if (!when) {
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
    for (i = 0; i <= when->maxIndex; i++) {
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
void getUpcomingSchedule(unsigned long long today, char* strbuf, int scrSize) { // not made yet!
    char* strs;
    char* temp;
    int limit; int i;
    dayPtr dd = NULL;

    i = 0;
    limit = scrSize;
    strs = (char*)malloc(sizeof(char) * scrSize); strs[0] = '\0';
    temp = (char*)malloc(sizeof(char) * scrSize); temp[0] = '\0';

    dd = search_byDate(today);

    if (!dd) {
        strcpy(strbuf, "No data");
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
void setSchedule(unsigned long long today, char* title, char* details, int priority) {
    toDoPtr in;

    in = create_Node(today, priority, title, details);
    insert(&key, in);

    return;
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

void getTodaySchedule_Summarized(unsigned long long today, int sortType, char* strbuf, int maxLines, int width) {
    /* Prefix codes
        [^: for Time, [[: make new line, ^: tab inside
        ]^: for title, ]]: for details
    */
    dayPtr dd = NULL;
    char str[BUFSIZ] = {'\0', };
    char temp[BUFSIZ] = {'\0', };
    int i;

    dd = search_byDate(today * 10000); // YYYYMMDD

    if (!dd) {
        strcpy(str, "No data\n");
    }
    else {
        sortGivenDateToDos(dd, sortType);
        for (i = 0; i <= dd->maxIndex; i++) {
            /*            Time->NL->Tab->NL->NL*/
            sprintf(temp, "[^%04llu[[^%s[[[[", (dd->toDoArr)[i]->dateData % 10000, (dd->toDoArr)[i]->title);
            strcat(str, temp);
        }
    }

    strcpy(strbuf, str);

    return;
}
int getTodaySchedule_withDetails(unsigned long long today, int sortType, char* strbuf, int maxLines, int width) {
    /* implementing *pageIterator* */
    /* Prefix codes
        [^: for Time, [[: make new line, ^: tab inside
        ]^: for title, ]]: for details
    */
    dayPtr dd = NULL;
    char str[BUFSIZ] = {'\0', };
    char temp[BUFSIZ] = {'\0', };
    int i = pageIterator;

    dd = search_byDate(today * 10000); // YYYYMMDD

    if (!dd) {
        strcpy(str, "No data\n");
    }
    else {
        pageIterator = (pageIterator + 1) % (dd->maxIndex + 1);
        sortGivenDateToDos(dd, sortType);
        /*        Time->NL->Title(NL)->Tab->NL->Details(NL)->Tab->NL->NL */
        sprintf(temp, "[^%04llu[[]^^%s[[]]^%s[[[[\n", (dd->toDoArr)[i]->dateData % 10000, (dd->toDoArr)[i]->title, (dd->toDoArr)[i]->details);
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

    fd_bin = open(bin_fileName, O_RDONLY);
    if (fd_bin == -1) {
        errOcc("open");
    }

    buf = (toDoPtr)malloc(sizeof(toDo));
    while ((chunk = read(fd_bin, buf, sizeof(toDo))) != 0) {
        if (chunk == -1) break;
        insert(&key, buf);
        buf = (toDoPtr)malloc(sizeof(toDo)); // memory leak alert!!!
    }
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
    int time, min;
    unsigned long long dates = 0;
    time = 0; min = 0;

    fd_bin = open(bin_fileName, O_CREAT | O_WRONLY | O_TRUNC, 0641);
    fd_hr = fopen(hr_fileName, "w+");

    if (fd_bin == -1 || fd_hr == NULL) {
        errOcc("open");
    }

    /* bin */
    for (int i = 0; i <= saveLink->maxIndex; i++) {
        if ((written = write(fd_bin, ((saveLink->toDoData)[i]), sizeof(toDo))) != sizeof(toDo)) {
            errOcc("write");
        }
    }
    /* hr */
    /** unsigned long long dateData;
        int priority;
        char title[31];
        // SOCKET FEATURE {method}();
        char details[256];
    */
    for (int i = 0; i <= saveLink->maxIndex; i++) {
        dates = (saveLink->toDoData)[i]->dateData / 10000;
        time = (int)((saveLink->toDoData)[i]->dateData % 10000 / 100);
        min = (int)((saveLink->toDoData)[i]->dateData % 100);
        fprintf(fd_hr, "%llu | %d:%d -> [Priority %d], Title: %12s, Details: %s\n", dates, time, min, 
            (saveLink->toDoData)[i]->priority, (saveLink->toDoData)[i]->title, (saveLink->toDoData)[i]->details);
    }

    fclose(fd_hr);
    close(fd_bin);

    ifAlreadyLoaded = 0;

    return 0;
}
void initSaveMem(void) {
    /* ? */
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
        ]^: for title, ]]: for details

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

/*-------------------------------------------------------*/