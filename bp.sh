#!/bin/sh
if gcc -Wall plnit_dbCore.c plnit_uxCore.c -o pln -lcurses -g
then
    echo Build Complete, Launch it with ./pln, also you can debug it with gdb.
else
    echo Failed! Two Possibilities:
    echo    1. GCC reported compile error
    echo    2. Permission error:
    echo        Change build permission or login to the shell as superuser: sudo su
fi