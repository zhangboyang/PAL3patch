lshint advapi32.lib
lshint binkw32.dll
lshint dinput8.lib
lshint gbengine.dll
lshint kernel32.lib
lshint user32.lib
lshint winmm.lib
lshint comdlg32.lib


copy /y pal3adump.dll pal3a.dll
start /wait pal3a.exe


pal3aanalyse


copy /y pal3atestvalloc.dll pal3.dll
start /wait pal3a.exe


pal3afixdump 1
pal3afixdump 2


pal3amkimport


pal3amakepe


pause