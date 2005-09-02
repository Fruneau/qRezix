TEMPLATE = lib

mac:QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Frameworks/
VERSION = 1.7.0
DESTDIR = ..
TARGET = qrezix

INCLUDEPATH += ..
contains(DEFINES, RZX_MAINUI_BUILTIN):include(../mainui/mainui.pri)
contains(DEFINES, RZX_TRAYICON_BUILTIN):include(../tray/tray.pri)
contains(DEFINES, RZX_NOTIFIER_BUILTIN):include(../notifier/notifier.pri)
contains(DEFINES, RZX_CHAT_BUILTIN):include(../chat/chat.pri)
contains(DEFINES, RZX_XNET_BUILTIN):include(../xnet/xnet.pri)
include(../rzxglobal.pri)

SOURCES += rzxglobal.cpp \
	rzxapplication.cpp \
	rzxhostaddress.cpp \
	rzxsubnet.cpp \
	rzxconnectionlister.cpp \
	rzxcomputer.cpp \
	rzxmessagebox.cpp \
	rzxutilslauncher.cpp \
	rzxproperty.cpp \
	rzxabstractconfig.cpp \
	rzxconfig.cpp \
	rzxiconcollection.cpp \
	rzxthemedicon.cpp \
	rzxbasemodule.cpp \
	rzxmodule.cpp \
	rzxnetwork.cpp \
	rzxwrongpass.cpp \
	rzxchangepass.cpp

HEADERS += rzxglobal.h \
	rzxapplication.h \
	rzxhostaddress.h \
	rzxsubnet.h \
	rzxconnectionlister.h \
	rzxcomputer.h \
	rzxmessagebox.h \
	rzxutilslauncher.h \
	rzxproperty.h \
	rzxabstractconfig.h \
	rzxconfig.h \
	rzxiconcollection.h \
	rzxthemedicon.h \
	rzxbasemodule.h \
	rzxbaseloader.h \
	rzxmodule.h \
	rzxnetwork.h \
	rzxwrongpass.h \
	rzxchangepass.h
	
FORMS += rzxpropertyui.ui \
	rzxwrongpassui.ui \
	rzxchangepassui.ui

TRANSLATIONS = ../translations/qrezix_fr.ts
