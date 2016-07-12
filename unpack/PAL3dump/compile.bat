@echo off
del /q pal3.dll
gcc -Wall -shared -o pal3.dll pal3dump.c pal3dump_asm.S pal3dump.def
pause
