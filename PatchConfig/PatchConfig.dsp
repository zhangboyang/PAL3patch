# Microsoft Developer Studio Project File - Name="PatchConfig" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PatchConfig - Win32 Unicode Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PatchConfig.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PatchConfig.mak" CFG="PatchConfig - Win32 Unicode Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PatchConfig - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PatchConfig - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "PatchConfig - Win32 Unicode Debug" (based on "Win32 (x86) Application")
!MESSAGE "PatchConfig - Win32 Unicode Release" (based on "Win32 (x86) Application")
!MESSAGE "PatchConfig - Win32 Static Release" (based on "Win32 (x86) Application")
!MESSAGE "PatchConfig - Win32 Static Unicode Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PatchConfig - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib d3d9.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "PatchConfig - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib d3d9.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "PatchConfig - Win32 Unicode Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PatchConfig___Win32_Unicode_Debug"
# PROP BASE Intermediate_Dir "PatchConfig___Win32_Unicode_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Unicode_Debug"
# PROP Intermediate_Dir "Unicode_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_UNICODE" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib d3d9.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib d3d9.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "PatchConfig - Win32 Unicode Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PatchConfig___Win32_Unicode_Release"
# PROP BASE Intermediate_Dir "PatchConfig___Win32_Unicode_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Unicode_Release"
# PROP Intermediate_Dir "Unicode_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_UNICODE" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib d3d9.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib d3d9.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "PatchConfig - Win32 Static Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PatchConfig___Win32_Static_Release"
# PROP BASE Intermediate_Dir "PatchConfig___Win32_Static_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Static_Release"
# PROP Intermediate_Dir "Static_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib d3d9.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib d3d9.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "PatchConfig - Win32 Static Unicode Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PatchConfig___Win32_Static_Unicode_Release"
# PROP BASE Intermediate_Dir "PatchConfig___Win32_Static_Unicode_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Static_Unicode_Release"
# PROP Intermediate_Dir "Static_Unicode_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_UNICODE" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib d3d9.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib d3d9.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /machine:I386

!ENDIF 

# Begin Target

# Name "PatchConfig - Win32 Release"
# Name "PatchConfig - Win32 Debug"
# Name "PatchConfig - Win32 Unicode Debug"
# Name "PatchConfig - Win32 Unicode Release"
# Name "PatchConfig - Win32 Static Release"
# Name "PatchConfig - Win32 Static Unicode Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
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

SOURCE=.\D3DEnum.cpp
# End Source File
# Begin Source File

SOURCE=.\d3denumeration.cpp
# End Source File
# Begin Source File

SOURCE=.\dxutil.cpp
# End Source File
# Begin Source File

SOURCE=.\FontEnum.cpp
# End Source File
# Begin Source File

SOURCE=.\HASH_SHA1.cpp
# End Source File
# Begin Source File

SOURCE=.\MouseMsgButton.cpp
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

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
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

SOURCE=.\FontEnum.h
# End Source File
# Begin Source File

SOURCE=.\HASH_SHA1.h
# End Source File
# Begin Source File

SOURCE=.\MouseMsgButton.h
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

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
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

SOURCE=.\default.conf
# End Source File
# Begin Source File

SOURCE=.\PatchConfig.manifest
# End Source File
# End Target
# End Project
