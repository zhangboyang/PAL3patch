@ECHO OFF

ECHO scAsmTool - Assemble Test [please run Disassemble Test first]
ECHO.
PAUSE

MKDIR scetest

SET SCASMTOOL=scAsmTool.exe

%SCASMTOOL% /3 /T basedata\init.sce scetest\init.asm
%SCASMTOOL% /3 /T basedata\TestLyp.sce scetest\TestLyp.asm
REM %SCASMTOOL% /3 /T basedata\SubGame\HockShop\Data\InvestHS.sce scetest\InvestHS.asm
%SCASMTOOL% /3 /T basedata\ui\BigMap\BigMap.sce scetest\BigMap.asm
%SCASMTOOL% /3 /T scene\M01\m01.sce scetest\m01.asm
%SCASMTOOL% /3 /T scene\M02\m02.sce scetest\m02.asm
%SCASMTOOL% /3 /T scene\M03\m03.sce scetest\m03.asm
%SCASMTOOL% /3 /T scene\M04\m04.sce scetest\m04.asm
%SCASMTOOL% /3 /T scene\M05\m05.sce scetest\m05.asm
%SCASMTOOL% /3 /T scene\M06\m06.sce scetest\m06.asm
%SCASMTOOL% /3 /T scene\m08\m08.sce scetest\m08.asm
%SCASMTOOL% /3 /T scene\M09\m09.sce scetest\m09.asm
%SCASMTOOL% /3 /T scene\m10\m10.sce scetest\m10.asm
%SCASMTOOL% /3 /T scene\m11\m11.sce scetest\m11.asm
%SCASMTOOL% /3 /T scene\M15\m15.sce scetest\m15.asm
%SCASMTOOL% /3 /T scene\M16\m11.sce scetest\M16_m11.asm
%SCASMTOOL% /3 /T scene\M16\m16.sce scetest\m16.asm
%SCASMTOOL% /3 /T scene\M17\m17.sce scetest\m17.asm
%SCASMTOOL% /3 /T scene\M18\m18.sce scetest\m18.asm
%SCASMTOOL% /3 /T scene\M19\m19.sce scetest\m19.asm
%SCASMTOOL% /3 /T scene\M20\m20.sce scetest\m20.asm
%SCASMTOOL% /3 /T scene\M21\m21.sce scetest\m21.asm
%SCASMTOOL% /3 /T scene\M22\m22.sce scetest\m22.asm
%SCASMTOOL% /3 /T scene\M23\m11.sce scetest\M23_m11.asm
%SCASMTOOL% /3 /T scene\M23\m23.sce scetest\m23.asm
%SCASMTOOL% /3 /T scene\M24\m24.sce scetest\m24.asm
%SCASMTOOL% /3 /T scene\M25\m25.sce scetest\m25.asm
%SCASMTOOL% /3 /T scene\M26\m26.sce scetest\m26.asm
%SCASMTOOL% /3 /T scene\Q01\Q01.sce scetest\Q01.asm
%SCASMTOOL% /3 /T scene\Q02\Q02.sce scetest\Q02.asm
%SCASMTOOL% /3 /T scene\Q03\Q03.sce scetest\Q03.asm
%SCASMTOOL% /3 /T scene\Q04\Q04.sce scetest\Q04.asm
%SCASMTOOL% /3 /T scene\Q05\Q05.sce scetest\Q05.asm
%SCASMTOOL% /3 /T scene\Q06\Q06.sce scetest\Q06.asm
%SCASMTOOL% /3 /T scene\Q07\Q07.sce scetest\Q07.asm
%SCASMTOOL% /3 /T scene\Q08\Q08.sce scetest\Q08.asm
%SCASMTOOL% /3 /T scene\Q09\Q09.sce scetest\Q09.asm
%SCASMTOOL% /3 /T scene\Q10\Q10.sce scetest\Q10.asm
%SCASMTOOL% /3 /T scene\Q11\Q11.sce scetest\Q11.asm
%SCASMTOOL% /3 /T scene\Q12\Q12.sce scetest\Q12.asm
%SCASMTOOL% /3 /T scene\Q13\Q13.sce scetest\Q13.asm
%SCASMTOOL% /3 /T scene\Q14\Q14.sce scetest\Q14.asm
%SCASMTOOL% /3 /T scene\Q15\Q15.sce scetest\Q15.asm
%SCASMTOOL% /3 /T scene\Q16\Q16.sce scetest\Q16.asm
%SCASMTOOL% /3 /T scene\Q17\Q17.sce scetest\Q17.asm
%SCASMTOOL% /3 /T scene\Q17\Q17\Q17.sce scetest\Q17_Q17.asm
REM %SCASMTOOL% /3 /T scene\T01\T01.sce scetest\T01.asm
REM %SCASMTOOL% /3 /T scene\T02\T02.sce scetest\T02.asm

ECHO.
ECHO ALL FINISHED!
PAUSE
