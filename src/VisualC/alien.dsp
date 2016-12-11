# Microsoft Developer Studio Project File - Name="alien" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=alien - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "alien.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "alien.mak" CFG="alien - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "alien - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "alien - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cwcl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "alien - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "ENABLE_DEBUG" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40d /d "NDEBUG"
# ADD RSC /l 0x40d /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=cwlink.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib sdl.lib sdlmain.lib sdl_mixer.lib smpeg.lib msvcrt.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib

!ELSEIF  "$(CFG)" == "alien - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MT /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "ENABLE_DEBUG" /D "__STDC__" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40d /d "_DEBUG"
# ADD RSC /l 0x40d /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=cwlink.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 SDL.lib SDLmain.lib kernel32.lib SDL_mixer.lib msvcrt.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib /pdbtype:sept

!ENDIF 

# Begin Target

# Name "alien - Win32 Release"
# Name "alien - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\animation.c
# End Source File
# Begin Source File

SOURCE=..\cd_iso.c
# End Source File
# Begin Source File

SOURCE=..\client.c
# End Source File
# Begin Source File

SOURCE=..\common.c
# End Source File
# Begin Source File

SOURCE=..\debug.c
# End Source File
# Begin Source File

SOURCE=..\decode.c
# End Source File
# Begin Source File

SOURCE=..\game2bin.c
# End Source File
# Begin Source File

SOURCE=..\getopt.c
# End Source File
# Begin Source File

SOURCE=..\getopt1.c
# End Source File
# Begin Source File

SOURCE=..\lzss.c
# End Source File
# Begin Source File

SOURCE=..\main.c
# End Source File
# Begin Source File

SOURCE=..\music.c
# End Source File
# Begin Source File

SOURCE=..\render.c
# End Source File
# Begin Source File

SOURCE=..\rooms.c
# End Source File
# Begin Source File

SOURCE=..\scale2x.c
# End Source File
# Begin Source File

SOURCE=..\scale3x.c
# End Source File
# Begin Source File

SOURCE=..\screen.c
# End Source File
# Begin Source File

SOURCE=..\sound.c
# End Source File
# Begin Source File

SOURCE=..\sprites.c
# End Source File
# Begin Source File

SOURCE=..\vm.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\animation.h
# End Source File
# Begin Source File

SOURCE=..\cd_iso.h
# End Source File
# Begin Source File

SOURCE=..\client.h
# End Source File
# Begin Source File

SOURCE=..\common.h
# End Source File
# Begin Source File

SOURCE=..\debug.h
# End Source File
# Begin Source File

SOURCE=..\decode.h
# End Source File
# Begin Source File

SOURCE=..\game2bin.h
# End Source File
# Begin Source File

SOURCE=..\getopt.h
# End Source File
# Begin Source File

SOURCE=..\lzss.h
# End Source File
# Begin Source File

SOURCE=..\music.h
# End Source File
# Begin Source File

SOURCE=..\render.h
# End Source File
# Begin Source File

SOURCE=..\rooms.h
# End Source File
# Begin Source File

SOURCE=..\scale2x.h
# End Source File
# Begin Source File

SOURCE=..\scale3x.h
# End Source File
# Begin Source File

SOURCE=..\screen.h
# End Source File
# Begin Source File

SOURCE=..\sound.h
# End Source File
# Begin Source File

SOURCE=..\sprites.h
# End Source File
# Begin Source File

SOURCE=..\vm.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
