@echo off
del pal3.dll
gcc -Wall -shared -o pal3.dll pal3unpack.c pal3unpack_asm.S pal3unpack.def
pause
