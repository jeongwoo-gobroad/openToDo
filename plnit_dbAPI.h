// KNU CSE 2021114026 Jeongwoo Kim
// recent update: 240530 v1.0.1 build 32
#include <stdio.h>
#include <time.h>

void errOcc(const char* str);

int save(void);
int save_hr(FILE* fp);
int load(void);
void coreInit(void);

int getNumOfSchedule(unsigned long long targetDate);
void getUpcomingSchedule(unsigned long long today, char* strbuf, int scrSize); 
int setSchedule(unsigned long long today, char* title, char* details, int priority); 
void getTodaySchedule(unsigned long long today, int sortType, char* strbuf, int scrSize); 

int getTodaySchedule_Summarized(unsigned long long today, char* strbuf); 
int getTodaySchedule_withDetails(unsigned long long today, char* strbuf, int direction); 
void getTodaySchedule_withDetails_iterEnd(void); 
void getBookMarkedInDate(unsigned long long today, int counter, char* str);

int deleteWhileIterate(unsigned long long src, int pageNum); 
int editWhileIterate(unsigned long long src, int pageNum, unsigned long long t_day, char* t_title, char* t_details, int t_priority); 
int setReminder(time_t current, time_t delta, int repeatCnter, char* what, int intervals); 
int isReminderSetAlready(char* str); 

void turnOffReminder(void);
int __dbDebug(void);
void __launchOptions(int argc, char* argv[]); 

int isBookMarked(unsigned long long targetDate); 

void getDday(int* slot1, char* title1, int* slot2, char* title2);
void setDdayWhileIterate(unsigned long long src, int pageNum, int mode);

int isHoliday(unsigned long long target); 
int isSharedToDoExisting(unsigned long long targetDate); 

int checkDdayWhileIterate(unsigned long long src, int pageNum, int mode); 
void delDdayStack(int whatto); 

int shareWhileIterate(unsigned long long src, int pageNum, char* shareCode); 
int getFromServer_Highlevel(char* shareCode); 
char* getUserName(void); 
void setUserName(char* myName); 