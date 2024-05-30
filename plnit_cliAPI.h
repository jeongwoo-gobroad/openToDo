// KNU CSE 2021114026 Jeongwoo Kim
// recent update: 240530 v1.0.1 build 32

/*--server-client interaction related APIs---------------*/
int __clDebug(void);
int cli_pushToDoDataToServer(toDoPtr target, char* code);
int cli_getToDoDataFromServer(toDoPtr target, const char* code);