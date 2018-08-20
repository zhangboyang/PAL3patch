@ECHO OFF

IF "%~1" == "" GOTO USAGE


SET SRCFILE=%~f1
SET OUTFILE=%~d1%~p1%~n1.dll
SET DEFFILE=%~d1%~p1%~n1.def

CD /D %~dp0
tcc -run preprocess.c "%SRCFILE%" | tcc - -DBUILD_FOR_PAL3A -shared -o "%OUTFILE%" -lgdi32 -lcomdlg32 -luser32 -lkernel32 -ladvapi32 -lshell32 -lPAL3Apatch
IF %ERRORLEVEL% EQU 0 (ECHO 编译成功。) ELSE (ECHO 编译失败。)
IF EXIST "%DEFFILE%" DEL "%DEFFILE%"

GOTO END


:USAGE
ECHO.
ECHO 本程序可以将 C 源代码形式的插件编译为 DLL 形式的插件。
ECHO.
ECHO 使用方法：将待编译的 C 源代码托放到本程序的图标上即可。
ECHO.


:END
PAUSE