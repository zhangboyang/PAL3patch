@ECHO OFF

ECHO scAsmTool - Disassemble Test
ECHO.
PAUSE

MKDIR scetest

SET SCASMTOOL=scAsmTool.exe

%SCASMTOOL% /3A /D basedata\init.sce scetest\init.asm
%SCASMTOOL% /3A /D basedata\ui\BigMap\BigMap.sce scetest\BigMap.asm
%SCASMTOOL% /3A /D scene\Sce\m01.sce scetest\m01.asm
%SCASMTOOL% /3A /D scene\Sce\m02.sce scetest\m02.asm
%SCASMTOOL% /3A /D scene\Sce\m03.sce scetest\m03.asm
%SCASMTOOL% /3A /D scene\Sce\m04.sce scetest\m04.asm
%SCASMTOOL% /3A /D scene\Sce\m05.sce scetest\m05.asm
%SCASMTOOL% /3A /D scene\Sce\m06.sce scetest\m06.asm
%SCASMTOOL% /3A /D scene\Sce\m07.sce scetest\m07.asm
%SCASMTOOL% /3A /D scene\Sce\m08.sce scetest\m08.asm
%SCASMTOOL% /3A /D scene\Sce\m09.sce scetest\m09.asm
%SCASMTOOL% /3A /D scene\Sce\m10.sce scetest\m10.asm
%SCASMTOOL% /3A /D scene\Sce\m11.sce scetest\m11.asm
%SCASMTOOL% /3A /D scene\Sce\m12.sce scetest\m12.asm
%SCASMTOOL% /3A /D scene\Sce\m13.sce scetest\m13.asm
%SCASMTOOL% /3A /D scene\Sce\m14.sce scetest\m14.asm
%SCASMTOOL% /3A /D scene\Sce\m15.sce scetest\m15.asm
%SCASMTOOL% /3A /D scene\Sce\m16.sce scetest\m16.asm
%SCASMTOOL% /3A /D scene\Sce\m17.sce scetest\m17.asm
%SCASMTOOL% /3A /D scene\Sce\m18.sce scetest\m18.asm
%SCASMTOOL% /3A /D scene\Sce\m19.sce scetest\m19.asm
%SCASMTOOL% /3A /D scene\Sce\m20.sce scetest\m20.asm
%SCASMTOOL% /3A /D scene\Sce\q01.sce scetest\q01.asm
%SCASMTOOL% /3A /D scene\Sce\q02.sce scetest\q02.asm
%SCASMTOOL% /3A /D scene\Sce\q03.sce scetest\q03.asm
%SCASMTOOL% /3A /D scene\Sce\q04.sce scetest\q04.asm
%SCASMTOOL% /3A /D scene\Sce\q05.sce scetest\q05.asm
%SCASMTOOL% /3A /D scene\Sce\q06.sce scetest\q06.asm
%SCASMTOOL% /3A /D scene\Sce\q07.sce scetest\q07.asm
%SCASMTOOL% /3A /D scene\Sce\q08.sce scetest\q08.asm
%SCASMTOOL% /3A /D scene\Sce\q09.sce scetest\q09.asm
%SCASMTOOL% /3A /D scene\Sce\q10.sce scetest\q10.asm
%SCASMTOOL% /3A /D scene\Sce\q11.sce scetest\q11.asm
%SCASMTOOL% /3A /D scene\Sce\y01.sce scetest\y01.asm

ECHO.
ECHO ALL FINISHED!
PAUSE
