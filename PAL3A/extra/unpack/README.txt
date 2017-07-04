requirements:
(1) DUMPBIN from VC6 (ver 6.00.8447 is tested)
(2) some LIB files from VC6 and DXSDK (dxsdk_sum2004.exe is tested), see md5sum.txt for file names
(3) MinGW gcc (gcc 3.4.2 from Dev-C++ 4.9.9.2 is tested)
(4) original PAL3A.EXE and the crack patch PAL3A.DLL

steps:
(1) run 'make'
(2) copy the file listed in md5sum.txt to this directory
     (note: the crack patch 'pal3a.dll' is renamed to pal3aunpack.dll) 
(3) run 'unpack.bat'
     (note: 'pal3aunpacked.exe' will be generated)

cleanup:
(1) make clean && rm -f dump* && rm -f `ls *.txt | grep -E -v '(info|README|pal3asections|md5sum)'` && rm -f *.bin && rm -f PAL3A.dll PAL3Aunpacked.exe
(2) rm -f `ls | grep -E -i '(dll|lib|exe)$' | sort`

