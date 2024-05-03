# openToDo
an openToDo project: (aka Plan-it Project)

  creating a terminal-based calendar app for Linux environment,
  using ncurses library to manage it like GUI interactions.

  리눅스 환경을 위한, 터미널 기반에서 구동되는 달력형 일정 관리 프로그램입니다.
  ncurses 라이브러리를 활용한, pseudo-GUI 환경을 제공합니다.
  
1. How To Build
    gcc -w plnit_dbCore.c plnit_uxCore.c -o plnit

2. Launch Options
    Plan_it: ./pln [options] [argument 1...]
          [options]:  NONE: Launch Plan_it in Normal Mode.
          [options]:    -d: Launch Plan_it in Core Part Debugging Mode.
          [options]:   -in: using CLI interface, you can insert data with arguments
          -----------------------------------------------------------------------
          [arguments]: [YYYYMMDD][HHMM][Priority_Num][Title][Details]
  
3. Version History
    0.0.1: [Current] First Build Version