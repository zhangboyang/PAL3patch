@ECHO OFF

ECHO scAsmTool - Disassemble Test
ECHO.
PAUSE

MKDIR scetest

SET SCASMTOOL=scAsmTool.exe

%SCASMTOOL% /3 /D basedata\init.sce scetest\init.asm
%SCASMTOOL% /3 /D basedata\TestLyp.sce scetest\TestLyp.asm
REM %SCASMTOOL% /3 /D basedata\SubGame\HockShop\Data\InvestHS.sce scetest\InvestHS.asm
%SCASMTOOL% /3 /D basedata\ui\BigMap\BigMap.sce scetest\BigMap.asm
%SCASMTOOL% /3 /D scene\M01\m01.sce scetest\m01.asm
%SCASMTOOL% /3 /D scene\M02\m02.sce scetest\m02.asm
%SCASMTOOL% /3 /D scene\M03\m03.sce scetest\m03.asm
%SCASMTOOL% /3 /D scene\M04\m04.sce scetest\m04.asm
%SCASMTOOL% /3 /D scene\M05\m05.sce scetest\m05.asm
%SCASMTOOL% /3 /D scene\M06\m06.sce scetest\m06.asm
%SCASMTOOL% /3 /D scene\m08\m08.sce scetest\m08.asm
%SCASMTOOL% /3 /D scene\M09\m09.sce scetest\m09.asm
%SCASMTOOL% /3 /D scene\m10\m10.sce scetest\m10.asm
%SCASMTOOL% /3 /D scene\m11\m11.sce scetest\m11.asm
%SCASMTOOL% /3 /D scene\M15\m15.sce scetest\m15.asm
%SCASMTOOL% /3 /D scene\M16\m11.sce scetest\M16_m11.asm
%SCASMTOOL% /3 /D scene\M16\m16.sce scetest\m16.asm
%SCASMTOOL% /3 /D scene\M17\m17.sce scetest\m17.asm
%SCASMTOOL% /3 /D scene\M18\m18.sce scetest\m18.asm
%SCASMTOOL% /3 /D scene\M19\m19.sce scetest\m19.asm
%SCASMTOOL% /3 /D scene\M20\m20.sce scetest\m20.asm
%SCASMTOOL% /3 /D scene\M21\m21.sce scetest\m21.asm
%SCASMTOOL% /3 /D scene\M22\m22.sce scetest\m22.asm
%SCASMTOOL% /3 /D scene\M23\m11.sce scetest\M23_m11.asm
%SCASMTOOL% /3 /D scene\M23\m23.sce scetest\m23.asm
%SCASMTOOL% /3 /D scene\M24\m24.sce scetest\m24.asm
%SCASMTOOL% /3 /D scene\M25\m25.sce scetest\m25.asm
%SCASMTOOL% /3 /D scene\M26\m26.sce scetest\m26.asm
%SCASMTOOL% /3 /D scene\Q01\Q01.sce scetest\Q01.asm
%SCASMTOOL% /3 /D scene\Q02\Q02.sce scetest\Q02.asm
%SCASMTOOL% /3 /D scene\Q03\Q03.sce scetest\Q03.asm
%SCASMTOOL% /3 /D scene\Q04\Q04.sce scetest\Q04.asm
%SCASMTOOL% /3 /D scene\Q05\Q05.sce scetest\Q05.asm
%SCASMTOOL% /3 /D scene\Q06\Q06.sce scetest\Q06.asm
%SCASMTOOL% /3 /D scene\Q07\Q07.sce scetest\Q07.asm
%SCASMTOOL% /3 /D scene\Q08\Q08.sce scetest\Q08.asm
%SCASMTOOL% /3 /D scene\Q09\Q09.sce scetest\Q09.asm
%SCASMTOOL% /3 /D scene\Q10\Q10.sce scetest\Q10.asm
%SCASMTOOL% /3 /D scene\Q11\Q11.sce scetest\Q11.asm
%SCASMTOOL% /3 /D scene\Q12\Q12.sce scetest\Q12.asm
%SCASMTOOL% /3 /D scene\Q13\Q13.sce scetest\Q13.asm
%SCASMTOOL% /3 /D scene\Q14\Q14.sce scetest\Q14.asm
%SCASMTOOL% /3 /D scene\Q15\Q15.sce scetest\Q15.asm
%SCASMTOOL% /3 /D scene\Q16\Q16.sce scetest\Q16.asm
%SCASMTOOL% /3 /D scene\Q17\Q17.sce scetest\Q17.asm
%SCASMTOOL% /3 /D scene\Q17\Q17\Q17.sce scetest\Q17_Q17.asm
REM %SCASMTOOL% /3 /D scene\T01\T01.sce scetest\T01.asm
REM %SCASMTOOL% /3 /D scene\T02\T02.sce scetest\T02.asm

ECHO.
ECHO ALL FINISHED!
PAUSE
