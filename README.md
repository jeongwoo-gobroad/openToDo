# openToDo
an openToDo project: (aka Plan-it Project)

  creating a terminal-based calendar app for Linux environment,
  using ncurses library to manage it like GUI interactions.

  리눅스 환경을 위한, 터미널 기반에서 구동되는 달력형 일정 관리 프로그램입니다.
  ncurses 라이브러리를 활용한, pseudo-GUI 환경을 제공합니다.
  
1. How To Build

    gcc -Wall plnit_dbCore.c plnit_uxCore.c -o [program_name] -lcurses

    todos.sv is a save file for testing. Feel free to use it.

3. Launch Options
    Plan_it: ./pln [options] [argument 1...]
   
          [options]:  NONE: Launch Plan_it in Normal Mode.
   
          [options]:    -d: Launch Plan_it in Core Part Debugging Mode.
   
          [options]:   -in: using CLI interface, you can insert data with arguments -> Cannot use now
   
          -----------------------------------------------------------------------
   
          [arguments]: [YYYYMMDD][HHMM][Priority_Num][Title][Details]
   
  
4. Version History

    0.0.1: First Build Version

    0.0.2: Scrollable Calendar Method Implemented.

    0.0.3: [Current]

          Core: 삽입시 기존에 북마크가 있을 경우 새 자료 무시
   
          Core: 윤년, 28, 29, 30, 31일 구분 및 잘못된 날짜를 보유한 데이터는 처음부터 생성되지 않도록 조치 (기존에는 UX단에처 처리, 이제는 자료구조상에서 구현)
   
          Core: human readable 파일 생성시, 3년의 인터벌을 두는 범위(2023-2025 등)의 경우 첫 연도의 트리가 잘못 형성되는 오류를 제거
   
          Core: 해시값 구성 조건 새로 구성
   
          Core: 오류 종류에 따른 insert의 리턴값을 통해 UX단에서 적절한 처리가 가능하도록 수정
   
          Core: 중요: DB 구조 완전 개선으로 이전 버전들과 저장 파일 호환 불가.
   
          UX: 화면 영역 새로 다듬음
   
          UX: 오늘인 날짜 하이라이트
