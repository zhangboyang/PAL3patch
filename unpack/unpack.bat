lshint advapi32.lib
lshint binkw32.dll
lshint dinput8.lib
lshint gbengine.dll
lshint kernel32.lib
lshint user32.lib
lshint winmm.lib

copy /y pal3dump.dll pal3.dll
start /wait pal3.exe

pal3analyse

pal3fixdump

pal3mkimport

pal3makepe

pause