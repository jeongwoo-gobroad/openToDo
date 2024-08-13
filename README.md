# openToDo
an openToDo project: (aka Plan-it Project)

  creating a terminal-based calendar app for Linux environment,
  using ncurses library to manage it like GUI interactions.

  리눅스 환경을 위한, 터미널 기반에서 구동되는 달력형 일정 관리 프로그램입니다.
  ncurses 라이브러리를 활용한, pseudo-GUI 환경을 제공합니다.

  Demonstration Video: https://youtu.be/kHII943qwfw

0. Manual
      
      README folder 내부의 README.txt를 참고해주시면 감사하겠습니다.
  
1. How To Build

    1) makefile

          $ chmod 777 *

          $ make
            
            1.1) makefile PHONY targets

                  1.1.1) make refresh

                        Deletes the compilation residue, including executable file.
                  
                  1.1.2) make cleanup

                        Deletes the compilation residue, excluding executable file.

    2) Explicit build method

          $ gcc -Wall [COMPILE_ALL_HEADERS_AND_SOURCES] -lcurses

    todos.sv is a save file for testing. Feel free to use it.

    public.dsv is a system default save file for implementation of public holidays. It CANNOT be altered or deleted.

3. Launch Options

    Plan_it: ./pln [options] [argument 1...]
   
          [options]:  NONE: Launch Plan_it in Normal Mode.
   
          [options]:    -d: using CLI interface, Launch Plan_it in Core Part Debugging Mode.
   
          [options]:   -cl: using CLI interface, Launch Plan_it in Client Part Debugging Mode.
   
          -----------------------------------------------------------------------
   
  
4. Version History

    0.0.1: First Build Version

    0.0.2: Scrollable Calendar Method Implemented.

    0.0.3: 

          Core: 삽입시 기존에 북마크가 있을 경우 새 자료 무시
   
          Core: 윤년, 28, 29, 30, 31일 구분 및 잘못된 날짜를 보유한 데이터는 처음부터 생성되지 않도록 조치 (기존에는 UX단에처 처리, 이제는 자료구조상에서 구현)
   
          Core: human readable 파일 생성시, 3년의 인터벌을 두는 범위(2023-2025 등)의 경우 첫 연도의 트리가 잘못 형성되는 오류를 제거
   
          Core: 해시값 구성 조건 새로 구성
   
          Core: 오류 종류에 따른 insert의 리턴값을 통해 UX단에서 적절한 처리가 가능하도록 수정
   
          Core: 중요: DB 구조 완전 개선으로 이전 버전들과 저장 파일 호환 불가.
   
          UX: 화면 영역 새로 다듬음
   
          UX: 오늘인 날짜 하이라이트

    0.0.5: 
   
          (Core) 레코드 삭제 기능 구현
   
          (Core) 반복 세이브/로드시 메모리 누수 문제 해결, 레코드 필드에 대한 동적 할당 해제 기능 적용
   
          (Core) getTodaySchedule_계열 함수 반환형 int로 업데이트, 반환 데이터가 의미를 가짐.
   
          (Core) sighandler 및 itimer 기반 리마인더 기능 구현
   
          (Core) 리마인더 세이브/로드 기능 구현(저장 파일과 함께 저장됨), Human-readable 파일 내에도 Reminder 정보 제공
   
          (Core) 현재 실행 중인 리마인더 삭제 / 확인 기능 구현
   
          (Core) 실행시 Core부 초기화 함수 coreInit() 내부적 업데이트
   
          (UX) Core부 함수 참조 부분에 대해 반환형 개선 및 신규 함수 추가
   

    0.0.6: 
   
          (Core) UX 화면 뿐만 아니라 -d 옵션으로 진입 가능한 디버깅 메뉴에서도 띄어쓰기가 포함된 문자열 입력 가능
   
          (Core) 수정 기능 관련 함수 구현 중
   
          (Core) 양방향 iterator 기반 상세 일정 순회 기능으로 확장
   
          (UX) 일정 세부사항 조회 기능 추가
   
          (UX) 일정 세부사항 조회에 양방향 iterator 기반 양방향 순회 추가
   

    0.0.7: 
   
          (Core) 띄어쓰기가 포함된 문자열 기능 안정화
   
          (Core) 동적 할당 해제 알고리즘 개선
   
          (Core) UX레이어와의 상호작용을 위해 일부 함수 내부 구조 개선
   
          (UX) 메뉴 심도 3단계 -> 2단계로 단순화
   
          (UX) 양방향 순회 방식 일자 탐색, 선택되어 있는 대상을 바로바로 지울 수 있도록 개선 (메뉴 심도의 단축과도 관련 있는 기능)
   
          (UX) 삽입 함수를 낮취진 심도인 2단계에 삽입
   
          (UX) 일정 개수 출력 함수의 string refresh 관련 알고리즘 개선
   

    0.0.8:
   
          (Core) 북마크의 스티커 색을 지정할 수 있게 됨, 따라서 일자에 따른 북마크의 색을 get 할 수 있는 API 및 내부 로직 추가.
   
          (Core) D-day 관련 기능 대폭 추가
   
              (D-day feature) Page Iterator 기반으로, 어떠한 일정을 바로 D-day 대상으로 삼을 수 있도록 하였음
   
              (D-day feature) 지정된 D-day에 대해, getter 방식 메서드를 구현하여, 내용과 D-day 수를 취할 수 있음
   
              (D-day feature) Stack 기반으로 저장되며, 이론상 무한개를 저장 할 수 있지만, UX 구현의 한계상 최대 2개로 제한하였음. 자세한 정보는 도큐멘트 참조 바람.
   
          (Core) 공휴일 DB화: 프로그램 default 레코드를 저장하는 읽기 전용의 .dsv 파일 제공, 이를 통해 '삭제 할 수 없는' 공휴일 일정 구현
   
          (Core) 특정 일이 공휴일인지 알 수 있는 메서드 구현

          (Core) Insert, Delete 함수 리턴 값 제공, 특히 Delete는 System default(공휴일 등)일정을 삭제하지 못하도록 막음.

          (UX)   컬러 스트립(스티커 기반 북마크) 기능 구현

          (UX)   공휴일 일정에 대한 특수한 스트링 출력 기능 구현

          (기타) randomRecordGenerator의 샘플 개수를 30개로 늘리고, 타이틀 및 텍스트를 로렘 입숨에서 그럴듯 한 텍스트로 바꾸었음

          (기타) .dsv 파일을 생성 가능한 defaultRecordGenerator 제공.

          (Server) 초대코드 및 마크업 스트링 기반의 공유 시스템 구현, proof-of-concept 단계.


    0.0.9:

          (Server) 초대코드 및 마크업 스트링 기반의 공유 시스템 구현, proof-of-concept 단계.

          (Client) 초대코드 및 마크업 스트링 기반의 공유 시스템 구현, proof-of-concept 단계.

          (Core) 동적 할당 해제 알고리즘 일부 개선

          (Core) default 레코드 생성 프로그램 개선, 편의성 제고

          (Core) 추상화를 거치지 않고 UX에서 활용되는 함수들의 오류 수정, 각주 추가

          (UX) 색상별 북마크 스티커의 구현 및 달력/우측 화면에의 표시

          (UX) 요일별 일자 표시 색 적용

          (UX) 공휴일 적색으로 표시, 공휴일 관련 DB 함수 완전 적용 완료

          (UX) Reminder의 UX-Level 구현

          (UX) getCommandScreen() 범용 함수 구현

          (UX) UX 음영 및 색 조합 관련 개선으로 가독성 향상

    0.1.0: 

          (Server-Client Interaction Protocol) 안정적인 통신 프로토콜 구현으로 데이터 손실 및 버퍼 언더/오버런 해결.

          (Client) 초대코드 및 마크업 스트링 기반의 공유 시스템 구현, API화 완료.

          (Core) DB 구조 업그레이드, 이전 버전과 저장 파일 호환 불가.

          (Core) 유저 이름 기반 개인화 시스템 구성.

          (Core) Default 레코드 생성 프로그램 업데이트

          (Core) 메모리 안정성 강화: 구조체 포인터 할당 관련 Memory-safe 액션 추가.

          (Core) 메모리 안정성 강화: 반복 Save-load시 Valgrind-safe 함.

          (Core) 메모리 안정성 강화: 라이브러리 콜 간 Stack smashing 문제 해결.

          (UX / Core) 스마트 D-day 설정 및 출력 기능 구현

          (UX / Core) 현재 시각 기준의 정밀한 [다가오는 북마크] 기능 구현

          (UX) 북마크 삽입 질의시 북마크 스트립 관련 직관성 향상.

          (UX) 공유 일정에 대한 북마크 스트립 추가 강조 기능 구현.

          (UX) 커서 및 음영, 색 조합 관련 개선으로 가독성 향상

    1.0.1: 

          (Build) makefile 및 소스 분할, API 라이브러리화 적용

          (Core) 세이브 파일 구조 업그레이드, 이전 버전과 저장 파일 호환 불가.

          (Core) 유저 이름 기반 개인화 시스템 구성: 개인 일정 소유권 관련 업데이트, 개인 프로필의 세이브파일화 적용.

          (Core) D+의 경우, 사회적 통념에 맞게 +1일 하여 표기하도록 내부 구조 개선.

          (UX) 적응형 스크린 출력: D-day 관련 출력 개선.

          (UX) 홈 화면(오늘의 날짜)으로 즉시 이동 관련 매커니즘 추가.

      
    1.0.2: [current]

          (Build) server 구동기 makefile화.

          (Manual) README 폴더에 상세한 매뉴얼과 문서 제공.

          (Core) 로드 관련 문제 해결.

          (Core) 북마크 관련 레거시 코드 충돌 문제 해결.

          (Core) 타 일자로의 수정 액션 관련 이슈 해결.

          (Core) 수정 메소드에 memcpy 관련 액션 추가.

          (UX) 로드 관련 문제 해결.

          (UX) 수정 방식 개선.
