# KNU CSE 2021114026 Jeongwoo Kim
# Libraries below written by: 2023011393 Nawon Kim, KNU CSE 2021114026 Jeongwoo Kim, 2023013565 Dahye Jeong
# Phony target: "refresh": deletes all of (pre)compiled files
# Phony target: "cleanup": deletes all of (pre)compiled files EXCEPT for an executable.
# Build target: "pln", an executable calendar program
# Important: 'curses' Library should be installed in your machine.

.PHONY: refresh
.PHONY: cleanup

pln: plnit_dbCore.o plnit_uxCore.o plnit_clientCore.o
	gcc -Wall -o pln -g plnit_dbCore.o plnit_uxCore.o plnit_clientCore.o -lcurses

plnit_dbcore.o: plnit_dbAPI.h plnit_cliAPI.h plnit_dbCore.h plnit_dbCore.c
	gcc -Wall -c plnit_dbAPI.h plnit_cliAPI.h plnit_dbCore.h plnit_dbCore.c

plnit_uxCore.o: plnit_dbAPI.h plnit_uxCore.h plnit_uxCore.c
	gcc -Wall -c plnit_dbAPI.h plnit_uxCore.h plnit_uxCore.c

plnit_clientCore.o: plnit_clientCore.h plnit_clientCore.c
	gcc -Wall -c plnit_clientCore.h plnit_clientCore.c

refresh:
	rm pln plnit_dbCore.o plnit_uxCore.o plnit_clientCore.o plnit_dbAPI.h.gch plnit_cliAPI.h.gch plnit_clientCore.h.gch plnit_dbCore.h.gch plnit_uxCore.h.gch

cleanup:
	rm plnit_dbCore.o plnit_uxCore.o plnit_clientCore.o plnit_dbAPI.h.gch plnit_cliAPI.h.gch plnit_clientCore.h.gch plnit_dbCore.h.gch plnit_uxCore.h.gch