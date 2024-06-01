Plan_it Project README(Manual)

목차
	I.   Project software: pln (프로젝트 애플리케이션 실행 파일)
	II.  Helper software: defaultRecordGenerator
	III. Helper software: randomRecordGenerator
	IV. Project software: plnit_serverCore
	V.  Helper documentation: Documentation_About_Methods.docx

1. Project software: pln (프로젝트 애플리케이션 실행 파일)

컴파일 방법: 
	1. make
		컴파일:
			$make
		Phony target:
			refresh: make 시에 생성되는 파일 모두 다 삭제
				$make refresh
			cleanup: 실행파일(pln)을 제외한, make 시에 생성되는 컴파일 관련 나머지 파일 삭제
				$make cleanup
	2. gcc를 통한 수동 컴파일
		gcc -Wall -g plnit_clientCore.c plnit_clientCore.h plnit_dbAPI.h plnit_dbCore.h plnit_dbCore.c plnit_uxCore.c plnit_uxCore.h -lcurses -o pln
		
	* curses 라이브러리가 설치된 상태여야 함.
	** 컴파일 대상 파일에 대한 권한이 확보 된 상태여야 함.

-------

실행 방법: 
	1. 일반 실행
		실행: $./pln 
	2. 디버그 모드 실행
		DB Core	   디버깅 메뉴: -d 옵션 (./pln -d)
		Client Socket 디버깅 메뉴: -cl 옵션 (./pln -cl)

	* public.dsv가 존재해야하고, 이에 대한 권한(0644)이 확보된 상황이어야 함.
	** 만약 todos.sv와 todos.txt가 존재한다면, 이들 파일에 대한 권한(0644)이 확보되어야 함.

-------

실행환경: 
	1. Kernal / OS
		Kernal: Linux 5.15.0-107-generic 이상
		OS:     Ubuntu 20.04.6 LTS 이상
	2. 컴파일러
		GCC 9.4.0 이상

-------

개발환경: 
	1. Kernal / OS
		Kernal: Linux 5.15.0-107-generic 
		OS:     Ubuntu 20.04.6 LTS 
	2. 컴파일러
		GCC 9.4.0 
	3. 코드 에디터
		VS Code, Vim

-------

기능: 
	기본 기능:
		1. 무한 스크롤 가능한, 직관적인 UX
		2. 일정 삽입 / 삭제 / 수정 기능
		3. 일정 저장 / 로드 기능
		4. 일정 관람 기능: 요약 보기
		5. 일정 관람 기능: 상세 보기
	추가 기능:
		1. 북마크
			1.1. 하루에 한 개의 북마크된 일정만 존재 할 수 있으며, 이 일정이 달력 칸의 컬러 스트립을 결정합니다.
			1.2. 공유 받은 일정이 북마크 값을 보유하고 있고, 해당 일정이 삽입될 날짜에 북마크 된 일정이 존재한다면, 전자를 우선시합니다. 후자는 삭제되지는
				않으나, 북마크 값을 지니지 않도록 바뀝니다.
		2. 다가오는 북마크
		3. 스마트 D-day
			3.1. 현재 시점보다 이전에 존재하면 D+, 현재 시점보다 이후에 존재하면 D-로 자동 설정됩니다.
			3.2. 별도의 타이핑 필요 없이, 상세 일정 열람 화면에서, 현재 페이지가 가리키고 있는 컨텐츠를 바로 D-day화 합니다.
		4. 공휴일 DB
			4.1. public.dsv가 생성하는 공휴일 DB에 기반한, 공휴일 열람 기능을 제공합니다.
				4.1.1. public.dsv를 생성하는 별도의 helper software에 대해서는 아래에 별도 manual이 존재합니다.
		5. 리마인더(미리 알림)
			5.1. 지정한 제목의 컨텐츠를, 일정 시간 간격으로 지속적으로 환기하는 기능을 제공합니다.
			5.2. 상세 작동법
				5.2.1. Type reminder title below: 미리 알림 제목을 입력하시면 됩니다.
				5.2.2. Time(seconds) to set the reminder: 초 단위로, 미리 알림이 만료될 시간을 정의하시면 됩니다.
				5.3.3. Number of times to repeat: 미리 알림 만료 전, 환기 메시지를 몇 번 출력 할 지, 횟수를 입력하시면 됩니다.
				5.3.4. interval for the reminder: 미리 알림 만료 전까지, 환기 메시지의 출력 간격 시간을 초 단위로 정의하시면 됩니다.
				5.3.5. 주의: 만료 시간 간격보다 환기 메시지의 출력 간격 시간의 합이 더 큰 경우 등, 논리적 정의에 맞지 않는 경우에는 설정을 버립니다.
		6. 일정 공유 기능
			6.1. 공유 송신 측: 상세 일정 보기 화면에서 S키를 누르면, 공유 코드가 제공됩니다.
			6.2. 공유 수신 측: 이동 모드에서 g키를 누르면, 공유 코드를 입력 받을 수 있습니다.
		7. 개인화
			개인의 이름을 저장 할 수 있고, 생성한 일정들은 이 이름을 기반으로 한 소유권을 가집니다.
			또한, 공유 송신시 해당 일정의 공유자를 이름을 기반으로 표기합니다.
		8. Human-Readable 저장 파일 생성
	
-------

기능키 및 입력 모드 설명:
	1. 일자간 이동 모드(1단계 심도)
		- 방향키(i, j, k, l)로 달력 이동
		g : 공유일정 받아오기
		z : 변경사항 저장
		x : 저장된 데이터 불러오기
		q : 리마인더 설정
		w : 리마인더 해제
		s : 달력 날짜 선택: 상세 일정 보기 모드로 전환

	2. 상세 일정 보기 모드(2단계 심도)
		- 방향키(j, l)로 일정 페이지 넘기기
		I : 일정 추가
		d : 일정 삭제
		m : 일정 수정
		D : 디데이 설정
		S : 일정 공유
		e : 상세 일정 보기 모드 나가기: 일자간 이동 모드로 복귀
	
	3. 전역 키
		. : 사용자 이름 변경
		0 : 오늘 날짜로 이동

-------

제약 사항 모음 :
	1. 문자열 길이
		제목 -> 최대 25자
		내용 -> 최대 60자
		공유코드 -> 8자 고정
		사용자이름 -> 최대 15자

	2. 입력 양식
		YYYYMMDDHHMM
			위 방식으로 날짜를 표기하는 방법의 일부 혹은 전체 스트링을 요구할 경우가 있습니다.
			이때, 양식에 맞게 입력 해 주시면 됩니다.
	
-------

2. Helper software: defaultRecordGenerator

프로그램 설명:
	공휴일 DB인 public.dsv를 생성하는 Linux 응용프로그램입니다.

컴파일 방법:
	gcc -o plnit_recmkr plnit_publicRecordMaker.c

사용 방법:
	targetText.txt의 내용을 자동으로 읽어서, public.dsv로 만들어 주는 프로그램입니다.

	targetText.txt에서의 공휴일 일정은 반드시 세 줄로 이루어져 있어야 하며,

	첫 줄은 YYYYMMDD 형식의 일자,
	두번째 줄은 25자 이하의 영문으로 된 공휴일명,
	세번째 줄은 60자 이하의 영문으로 된 세부사항입니다.

	각 일정과 일정 사이에는 엔터 없이 그대로 쭉 이어져 있으며,

	별도의 커맨드로 조작할 필요 없이 
	 - 기존에 존재했던 public.dsv를 지우고(O_TRUNC가 달려있긴 하지만 버그인지 가끔씩 제대로 덮어씌워지지 않는 경우가 있습니다.)
	 - plnit_recmkr를 실행 해 주시기만 하면(./plnit_recmkr)
 	 - public.dsv가 생성됩니다. 이 파일을 기존에 고지된 사용법에 맞게 적절한 경로에 두시면 됩니다.

주의사항:
	리눅스에서와 Windows 계열에서는 new line을 다루는 방법이 다르기 때문에, 윈도우 메모장에서 작성한 .txt는 오류를 일으킵니다.
	따라서, VS Code 등을 통해서 CRLF->LF로 바꾸셔야 정상 작동합니다.

	또한, 한번 바꾸었다고 LF로 상시 작동하는 것이 아니기 때문에(Git에 의한 변경 등)
	plnit_recmkr 실행 전 
	- CRLF/LF 확인
	- 실행 전 텍스트 파일 맨 마지막에 엔터가 있는지 확인
	을 한 번 씩 거쳐주시면 감사하겠습니다.


3. Helper software: randomRecordGenerator
	
프로그램 설명:
	프로그램 테스트를 위한 더미 레코드를 생성 해 주는 프로그램입니다.
	첫 입력: 연도의 범위를 입력받고,
	두번째 입력: 생성할 레코드의 수를 입력받고,
	세번째 입력: 
		1 입력시: 주어진 범위 내 무작위 일자, 무작위 시간, 무작위 스트링을 기반으로 한 레코드
		2 입력시: 2024년 5월 8일이라는 한정된 범위 내의 무작위 스트링을 기반으로 한 레코드
	
	주로, Plan_it 프로그램을 Core Debug 모드로 구동하여 insert 옵션으로
	수천개의 레코드를 한번에 삽입하는 용도로 사용합니다.

컴파일 방법:
	Windows 환경에서 MinGW를 활용하여 빌드 하시면 됩니다.


4. Project software: plnit_serverCore
	
프로그램 설명:
	공유 일정 기능 구현을 위한, 서버 구동용 파일입니다.

컴파일 방법:
	1. make
		컴파일:
			$make
		Phony target:
			refresh: make 시에 생성되는 파일 모두 다 삭제
				$make refresh

	2. gcc를 통한 수동 컴파일
		gcc plnit_serverCore.c -o pln_sv

사용 방법:
	- 공유 일정 관리 서버를 구동합니다.
	- 클라이언트 접속 정보와, 공유 마크업 스트링을 표시합니다.
	- ^C를 입력하면, accesspairs.ssv라는 바이너리 파일에 지금까지 기록된 DB를 저장 한 후 종료합니다.

주의사항:
	- O_SYNC 옵션이 accesspairs.ssv 접근 과정에 동반됩니다.

5. Helper documentation: Documentation_About_Methods.docx

파일 설명:
	내부 로직 및 메소드의 개발 과정 및 개발 결과, 각 메소드별 요구 인자 등의 내용을 담고 있는 문서입니다.