# Microsoft Developer Studio Project File - Name="PatchConfig" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PatchConfig - Win32 Debug PAL3
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PatchConfig.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PatchConfig.mak" CFG="PatchConfig - Win32 Debug PAL3"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PatchConfig - Win32 Release PAL3A" (based on "Win32 (x86) Application")
!MESSAGE "PatchConfig - Win32 Debug PAL3A" (based on "Win32 (x86) Application")
!MESSAGE "PatchConfig - Win32 Release PAL3" (based on "Win32 (x86) Application")
!MESSAGE "PatchConfig - Win32 Debug PAL3" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PatchConfig - Win32 Release PAL3A"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release_PAL3A"
# PROP BASE Intermediate_Dir "Release_PAL3A"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_PAL3A"
# PROP Intermediate_Dir "Release_PAL3A"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "BUILD_FOR_PAL3A" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "BUILD_FOR_PAL3A" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib d3d9.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 unicows.lib kernel32.lib advapi32.lib user32.lib gdi32.lib shell32.lib comdlg32.lib version.lib mpr.lib rasapi32.lib winmm.lib winspool.lib vfw32.lib secur32.lib oleacc.lib oledlg.lib sensapi.lib winmm.lib wininet.lib delayimp.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /machine:I386 /nodefaultlib:"kernel32.lib" /nodefaultlib:"advapi32.lib" /nodefaultlib:"user32.lib" /nodefaultlib:"gdi32.lib" /nodefaultlib:"shell32.lib" /nodefaultlib:"comdlg32.lib" /nodefaultlib:"version.lib" /nodefaultlib:"mpr.lib" /nodefaultlib:"rasapi32.lib" /nodefaultlib:"winmm.lib" /nodefaultlib:"winspool.lib" /nodefaultlib:"vfw32.lib" /nodefaultlib:"secur32.lib" /nodefaultlib:"oleacc.lib" /nodefaultlib:"oledlg.lib" /nodefaultlib:"sensapi.lib" /delayload:advapi32.dll /delayload:winspool.drv /delayload:comdlg32.dll /delayload:shell32.dll /delayload:wininet.dll
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "PatchConfig - Win32 Debug PAL3A"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug_PAL3A"
# PROP BASE Intermediate_Dir "Debug_PAL3A"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_PAL3A"
# PROP Intermediate_Dir "Debug_PAL3A"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "BUILD_FOR_PAL3A" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "BUILD_FOR_PAL3A" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib d3d9.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib wininet.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "PatchConfig - Win32 Release PAL3"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release_PAL3"
# PROP BASE Intermediate_Dir "Release_PAL3"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_PAL3"
# PROP Intermediate_Dir "Release_PAL3"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "BUILD_FOR_PAL3" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "BUILD_FOR_PAL3" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "BUILD_FOR_PAL3" /d "NDEBUG"
# ADD RSC /l 0x804 /d "BUILD_FOR_PAL3" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib d3d9.lib wininet.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /machine:I386
# ADD LINK32 unicows.lib kernel32.lib advapi32.lib user32.lib gdi32.lib shell32.lib comdlg32.lib version.lib mpr.lib rasapi32.lib winmm.lib winspool.lib vfw32.lib secur32.lib oleacc.lib oledlg.lib sensapi.lib winmm.lib wininet.lib delayimp.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /machine:I386 /nodefaultlib:"kernel32.lib" /nodefaultlib:"advapi32.lib" /nodefaultlib:"user32.lib" /nodefaultlib:"gdi32.lib" /nodefaultlib:"shell32.lib" /nodefaultlib:"comdlg32.lib" /nodefaultlib:"version.lib" /nodefaultlib:"mpr.lib" /nodefaultlib:"rasapi32.lib" /nodefaultlib:"winmm.lib" /nodefaultlib:"winspool.lib" /nodefaultlib:"vfw32.lib" /nodefaultlib:"secur32.lib" /nodefaultlib:"oleacc.lib" /nodefaultlib:"oledlg.lib" /nodefaultlib:"sensapi.lib" /delayload:advapi32.dll /delayload:winspool.drv /delayload:comdlg32.dll /delayload:shell32.dll /delayload:wininet.dll
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "PatchConfig - Win32 Debug PAL3"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug_PAL3"
# PROP BASE Intermediate_Dir "Debug_PAL3"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_PAL3"
# PROP Intermediate_Dir "Debug_PAL3"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "BUILD_FOR_PAL3" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "BUILD_FOR_PAL3" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "BUILD_FOR_PAL3" /d "_DEBUG"
# ADD RSC /l 0x804 /d "BUILD_FOR_PAL3" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib d3d9.lib wininet.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib wininet.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "PatchConfig - Win32 Release PAL3A"
# Name "PatchConfig - Win32 Debug PAL3A"
# Name "PatchConfig - Win32 Release PAL3"
# Name "PatchConfig - Win32 Debug PAL3"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BadFiles.cpp
# End Source File
# Begin Source File

SOURCE=.\BufferReader.cpp
# End Source File
# Begin Source File

SOURCE=.\CheckForUpdates.cpp
# End Source File
# Begin Source File

SOURCE=.\ChooseFromListDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigData.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigDescData.cpp
# End Source File
# Begin Source File

SOURCE=.\CPKFixer.cpp
# End Source File
# Begin Source File

SOURCE=.\D3DEnum.cpp
# End Source File
# Begin Source File

SOURCE=.\d3denumeration.cpp
# End Source File
# Begin Source File

SOURCE=.\dxutil.cpp
# End Source File
# Begin Source File

SOURCE=.\FileFixer.cpp
# End Source File
# Begin Source File

SOURCE=.\FontEnum.cpp
# End Source File
# Begin Source File

SOURCE=.\fsutil.cpp
# End Source File
# Begin Source File

SOURCE=.\HASH_SHA1.cpp
# End Source File
# Begin Source File

SOURCE=.\MouseMsgButton.cpp
# End Source File
# Begin Source File

SOURCE=.\OpenConfigTool.cpp
# End Source File
# Begin Source File

SOURCE=.\OpenGameFolder.cpp
# End Source File
# Begin Source File

SOURCE=.\PatchConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\PatchConfig.rc
# End Source File
# Begin Source File

SOURCE=.\PatchConfigDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PatchVersionInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\PleaseWaitDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressObject.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadWriter.cpp
# End Source File
# Begin Source File

SOURCE=.\ReferenceCounter.cpp
# End Source File
# Begin Source File

SOURCE=.\RepairGameData.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\UACVirtualization.cpp
# End Source File
# Begin Source File

SOURCE=.\wal.cpp
# End Source File
# Begin Source File

SOURCE=.\wstr.cpp
# End Source File
# Begin Source File

SOURCE=.\XorRepair.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BadFiles.h
# End Source File
# Begin Source File

SOURCE=.\BufferReader.h
# End Source File
# Begin Source File

SOURCE=.\CheckForUpdates.h
# End Source File
# Begin Source File

SOURCE=.\ChooseFromListDlg.h
# End Source File
# Begin Source File

SOURCE=.\ConfigData.h
# End Source File
# Begin Source File

SOURCE=.\ConfigDescData.h
# End Source File
# Begin Source File

SOURCE=.\ConfigDescLocale.h
# End Source File
# Begin Source File

SOURCE=.\CPKFixer.h
# End Source File
# Begin Source File

SOURCE=.\D3DEnum.h
# End Source File
# Begin Source File

SOURCE=.\d3denumeration.h
# End Source File
# Begin Source File

SOURCE=.\dxstdafx.h
# End Source File
# Begin Source File

SOURCE=.\dxutil.h
# End Source File
# Begin Source File

SOURCE=.\FileFixer.h
# End Source File
# Begin Source File

SOURCE=.\FontEnum.h
# End Source File
# Begin Source File

SOURCE=.\fsutil.h
# End Source File
# Begin Source File

SOURCE=.\HASH_SHA1.h
# End Source File
# Begin Source File

SOURCE=.\MouseMsgButton.h
# End Source File
# Begin Source File

SOURCE=.\OpenConfigTool.h
# End Source File
# Begin Source File

SOURCE=.\OpenGameFolder.h
# End Source File
# Begin Source File

SOURCE=.\PatchConfig.h
# End Source File
# Begin Source File

SOURCE=.\PatchConfigDlg.h
# End Source File
# Begin Source File

SOURCE=.\PatchVersionInfo.h
# End Source File
# Begin Source File

SOURCE=.\PleaseWaitDlg.h
# End Source File
# Begin Source File

SOURCE=.\ProgressObject.h
# End Source File
# Begin Source File

SOURCE=.\ReadWriter.h
# End Source File
# Begin Source File

SOURCE=.\ReferenceCounter.h
# End Source File
# Begin Source File

SOURCE=.\RepairGameData.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\UACVirtualization.h
# End Source File
# Begin Source File

SOURCE=.\wal.h
# End Source File
# Begin Source File

SOURCE=.\wstr.h
# End Source File
# Begin Source File

SOURCE=.\XorRepair.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\PatchConfig.ico
# End Source File
# Begin Source File

SOURCE=.\res\PatchConfig.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\default_PAL3.conf
# End Source File
# Begin Source File

SOURCE=.\default_PAL3A.conf
# End Source File
# Begin Source File

SOURCE=.\PatchConfig.manifest
# End Source File
# End Target
# End Project
