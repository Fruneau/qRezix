# Microsoft Developer Studio Project File - Name="qrezix" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=qrezix - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "qrezix.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "qrezix.mak" CFG="qrezix - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "qrezix - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "qrezix - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "qrezix - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FD /c
# ADD CPP /nologo /MD /W3 /O1 /I "$(QTDIR)\include" /D "NDEBUG" /D "NO_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib imm32.lib winmm.lib wsock32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib imm32.lib winmm.lib wsock32.lib imm32.lib wsock32.lib winmm.lib $(QTDIR)\lib\qt-mt230nc.lib $(QTDIR)\lib\qtmain.lib IMM32.LIB /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "qrezix - Win32 Debug"

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
# ADD BASE CPP /nologo /MD /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /ZI /Od /I "$(QTDIR)\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /FR /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib imm32.lib winmm.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib imm32.lib winmm.lib wsock32.lib imm32.lib wsock32.lib winmm.lib $(QTDIR)\lib\qt-mt230nc.lib $(QTDIR)\lib\qtmain.lib IMM32.LIB /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc" /out:"Debug/rezix.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "qrezix - Win32 Release"
# Name "qrezix - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\dnsValidator.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_dnsvalidator.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_qrezix.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_qrezixui.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_rzxchat.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_rzxchatui.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_rzxclientlistener.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_rzxcomputer.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_rzxconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_rzxitem.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_rzxproperty.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_rzxpropertyui.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_rzxprotocole.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_rzxrezal.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_rzxserverlistener.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_trayicon.cpp
# End Source File
# Begin Source File

SOURCE=.\qrezix.cpp
# End Source File
# Begin Source File

SOURCE=.\qrezixui.cpp
# End Source File
# Begin Source File

SOURCE=.\rzxchat.cpp
# End Source File
# Begin Source File

SOURCE=.\rzxchatui.cpp
# End Source File
# Begin Source File

SOURCE=.\rzxclientlistener.cpp
# End Source File
# Begin Source File

SOURCE=.\rzxcomputer.cpp
# End Source File
# Begin Source File

SOURCE=.\rzxconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\rzxhostaddress.cpp
# End Source File
# Begin Source File

SOURCE=.\rzxitem.cpp
# End Source File
# Begin Source File

SOURCE=.\rzxmessagebox.cpp
# End Source File
# Begin Source File

SOURCE=.\rzxproperty.cpp
# End Source File
# Begin Source File

SOURCE=.\rzxpropertyui.cpp
# End Source File
# Begin Source File

SOURCE=.\rzxprotocole.cpp
# End Source File
# Begin Source File

SOURCE=.\rzxrezal.cpp
# End Source File
# Begin Source File

SOURCE=.\rzxserverlistener.cpp
# End Source File
# Begin Source File

SOURCE=.\trayicon.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\defaults.h
# End Source File
# Begin Source File

SOURCE=.\dnsvalidator.h

!IF  "$(CFG)" == "qrezix - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\dnsvalidator.h
InputName=dnsvalidator

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "qrezix - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\dnsvalidator.h
InputName=dnsvalidator

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\q.xpm
# End Source File
# Begin Source File

SOURCE=.\qrezix.h

!IF  "$(CFG)" == "qrezix - Win32 Release"

# Begin Custom Build - Moc'ing qrezix.h...
InputPath=.\qrezix.h

"moc_qrezix.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe qrezix.h -o moc_qrezix.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "qrezix - Win32 Debug"

# Begin Custom Build - Moc'ing qrezix.h...
InputPath=.\qrezix.h

"moc_qrezix.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe qrezix.h -o moc_qrezix.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\qrezixui.h
# End Source File
# Begin Source File

SOURCE=.\rzxchat.h

!IF  "$(CFG)" == "qrezix - Win32 Release"

# Begin Custom Build - Moc'ing rzxchat.h...
InputPath=.\rzxchat.h

"moc_rzxchat.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe rzxchat.h -o moc_rzxchat.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "qrezix - Win32 Debug"

# Begin Custom Build - Moc'ing rzxchat.h...
InputPath=.\rzxchat.h

"moc_rzxchat.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe rzxchat.h -o moc_rzxchat.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rzxchatui.h
# End Source File
# Begin Source File

SOURCE=.\rzxclientlistener.h

!IF  "$(CFG)" == "qrezix - Win32 Release"

# Begin Custom Build - Moc'ing rzxclientlistener.h...
InputPath=.\rzxclientlistener.h

"moc_rzxclientlistener.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe rzxclientlistener.h -o moc_rzxclientlistener.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "qrezix - Win32 Debug"

# Begin Custom Build - Moc'ing rzxclientlistener.h...
InputPath=.\rzxclientlistener.h

"moc_rzxclientlistener.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe rzxclientlistener.h -o moc_rzxclientlistener.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rzxcomputer.h

!IF  "$(CFG)" == "qrezix - Win32 Release"

# Begin Custom Build - Moc'ing rzxcomputer.h...
InputPath=.\rzxcomputer.h

"moc_rzxcomputer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe rzxcomputer.h -o moc_rzxcomputer.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "qrezix - Win32 Debug"

# Begin Custom Build - Moc'ing rzxcomputer.h...
InputPath=.\rzxcomputer.h

"moc_rzxcomputer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe rzxcomputer.h -o moc_rzxcomputer.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rzxconfig.h

!IF  "$(CFG)" == "qrezix - Win32 Release"

# Begin Custom Build - Moc'ing rzxconfig.h...
InputPath=.\rzxconfig.h

"moc_rzxconfig.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe rzxconfig.h -o moc_rzxconfig.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "qrezix - Win32 Debug"

# Begin Custom Build - Moc'ing rzxconfig.h...
InputPath=.\rzxconfig.h

"moc_rzxconfig.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe rzxconfig.h -o moc_rzxconfig.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rzxhostaddress.h
# End Source File
# Begin Source File

SOURCE=.\rzxitem.h

!IF  "$(CFG)" == "qrezix - Win32 Release"

# Begin Custom Build - Moc'ing rzxitem.h...
InputPath=.\rzxitem.h

"moc_rzxitem.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe rzxitem.h -o moc_rzxitem.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "qrezix - Win32 Debug"

# Begin Custom Build - Moc'ing rzxitem.h...
InputPath=.\rzxitem.h

"moc_rzxitem.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe rzxitem.h -o moc_rzxitem.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rzxmessagebox.h
# End Source File
# Begin Source File

SOURCE=.\rzxproperty.h

!IF  "$(CFG)" == "qrezix - Win32 Release"

# Begin Custom Build - Moc'ing rzxproperty.h...
InputPath=.\rzxproperty.h

"moc_rzxproperty.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe rzxproperty.h -o moc_rzxproperty.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "qrezix - Win32 Debug"

# Begin Custom Build - Moc'ing rzxproperty.h...
InputPath=.\rzxproperty.h

"moc_rzxproperty.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe rzxproperty.h -o moc_rzxproperty.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rzxpropertyui.h
# End Source File
# Begin Source File

SOURCE=.\rzxprotocole.h

!IF  "$(CFG)" == "qrezix - Win32 Release"

# Begin Custom Build - Moc'ing rzxprotocole.h...
InputPath=.\rzxprotocole.h

"moc_rzxprotocole.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe rzxprotocole.h -o moc_rzxprotocole.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "qrezix - Win32 Debug"

# Begin Custom Build - Moc'ing rzxprotocole.h...
InputPath=.\rzxprotocole.h

"moc_rzxprotocole.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe rzxprotocole.h -o moc_rzxprotocole.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rzxrezal.h

!IF  "$(CFG)" == "qrezix - Win32 Release"

# Begin Custom Build - Moc'ing rzxrezal.h...
InputPath=.\rzxrezal.h

"moc_rzxrezal.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe rzxrezal.h -o moc_rzxrezal.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "qrezix - Win32 Debug"

# Begin Custom Build - Moc'ing rzxrezal.h...
InputPath=.\rzxrezal.h

"moc_rzxrezal.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe rzxrezal.h -o moc_rzxrezal.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rzxserverlistener.h

!IF  "$(CFG)" == "qrezix - Win32 Release"

# Begin Custom Build - Moc'ing rzxserverlistener.h...
InputPath=.\rzxserverlistener.h

"moc_rzxserverlistener.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe rzxserverlistener.h -o moc_rzxserverlistener.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "qrezix - Win32 Debug"

# Begin Custom Build - Moc'ing rzxserverlistener.h...
InputPath=.\rzxserverlistener.h

"moc_rzxserverlistener.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe rzxserverlistener.h -o moc_rzxserverlistener.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\trayicon.h

!IF  "$(CFG)" == "qrezix - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\trayicon.h
InputName=trayicon

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "qrezix - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\trayicon.h
InputName=trayicon

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icone.rc
# End Source File
# Begin Source File

SOURCE=.\QRezix.ico
# End Source File
# End Group
# Begin Group "Interfaces"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qrezixui.ui

!IF  "$(CFG)" == "qrezix - Win32 Release"

# Begin Custom Build - Uic'ing qrezixui.ui...
InputPath=.\qrezixui.ui

BuildCmds= \
	%QTDIR%\bin\uic qrezixui.ui -o qrezixui.h \
	%QTDIR%\bin\uic qrezixui.ui -i qrezixui.h -o qrezixui.cpp \
	%QTDIR%\bin\moc qrezixui.h -o moc_qrezixui.cpp \
	

"qrezixui.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"qrezixui.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_qrezixui.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "qrezix - Win32 Debug"

# Begin Custom Build - Uic'ing qrezixui.ui...
InputPath=.\qrezixui.ui

BuildCmds= \
	%QTDIR%\bin\uic qrezixui.ui -o qrezixui.h \
	%QTDIR%\bin\uic qrezixui.ui -i qrezixui.h -o qrezixui.cpp \
	%QTDIR%\bin\moc qrezixui.h -o moc_qrezixui.cpp \
	

"qrezixui.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"qrezixui.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_qrezixui.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rzxchatui.ui

!IF  "$(CFG)" == "qrezix - Win32 Release"

# Begin Custom Build - Uic'ing rzxchatui.ui...
InputPath=.\rzxchatui.ui

BuildCmds= \
	%QTDIR%\bin\uic rzxchatui.ui -o rzxchatui.h \
	%QTDIR%\bin\uic rzxchatui.ui -i rzxchatui.h -o rzxchatui.cpp \
	%QTDIR%\bin\moc rzxchatui.h -o moc_rzxchatui.cpp \
	

"rzxchatui.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"rzxchatui.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_rzxchatui.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "qrezix - Win32 Debug"

# Begin Custom Build - Uic'ing rzxchatui.ui...
InputPath=.\rzxchatui.ui

BuildCmds= \
	%QTDIR%\bin\uic rzxchatui.ui -o rzxchatui.h \
	%QTDIR%\bin\uic rzxchatui.ui -i rzxchatui.h -o rzxchatui.cpp \
	%QTDIR%\bin\moc rzxchatui.h -o moc_rzxchatui.cpp \
	

"rzxchatui.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"rzxchatui.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_rzxchatui.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rzxpropertyui.ui

!IF  "$(CFG)" == "qrezix - Win32 Release"

# Begin Custom Build - Uic'ing rzxpropertyui.ui...
InputPath=.\rzxpropertyui.ui

BuildCmds= \
	%QTDIR%\bin\uic rzxpropertyui.ui -o rzxpropertyui.h \
	%QTDIR%\bin\uic rzxpropertyui.ui -i rzxpropertyui.h -o rzxpropertyui.cpp \
	%QTDIR%\bin\moc rzxpropertyui.h -o moc_rzxpropertyui.cpp \
	

"rzxpropertyui.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"rzxpropertyui.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_rzxpropertyui.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "qrezix - Win32 Debug"

# Begin Custom Build - Uic'ing rzxpropertyui.ui...
InputPath=.\rzxpropertyui.ui

BuildCmds= \
	%QTDIR%\bin\uic rzxpropertyui.ui -o rzxpropertyui.h \
	%QTDIR%\bin\uic rzxpropertyui.ui -i rzxpropertyui.h -o rzxpropertyui.cpp \
	%QTDIR%\bin\moc rzxpropertyui.h -o moc_rzxpropertyui.cpp \
	

"rzxpropertyui.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"rzxpropertyui.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_rzxpropertyui.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
