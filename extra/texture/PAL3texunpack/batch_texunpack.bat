@ECHO OFF
SET EXTRACT_PATH="G:\"
SET UNPACKER_PATH="PAL3texunpack.exe"
SET GAME_PATH=".\"

CALL :SETUNPACKER %UNPACKER_PATH%
CALL :SETGAMEPATH %GAME_PATH%

ECHO extract to %EXTRACT_PATH%
CD /D %EXTRACT_PATH%
ECHO remove existing cpk files ...
RD /S cpk
MKDIR cpk
ECHO begin extract ...
CD /D %GAME_PATH%
FOR /R %%i IN (*.cpk) DO CALL :DOEXTRACT "%%i"
ECHO batch finished!
PAUSE
EXIT


:DOEXTRACT
CD /D %EXTRACT_PATH%
%UNPACKER_PATH% %1
TYPE cpk\log.txt >> cpk\texmap.txt
DEL cpk\log.txt
EXIT /B

:SETUNPACKER
SET UNPACKER_PATH="%~f1"
EXIT /B

:SETGAMEPATH
SET GAME_PATH=%~f1
EXIT /B
