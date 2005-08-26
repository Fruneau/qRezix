TEMPLATE = lib

TARGET = ../qrezix
VERSION = 1.7.0
CONFIG += debug

INCLUDEPATH += ..

DEFINES *= RZX_XNET_BUILTIN

contains(DEFINES, RZX_MAINUI_BUILTIN):include(../mainui/mainui.pri)
contains(DEFINES, RZX_TRAYICON_BUILTIN):include(../tray/tray.pri)
contains(DEFINES, RZX_NOTIFIER_BUILTIN):include(../notifier/notifier.pri)
contains(DEFINES, RZX_CHAT_BUILTIN):include(../chat/chat.pri)
include(../rzxglobal.pri)

SOURCES += rzxglobal.cpp \
	rzxapplication.cpp \
	rzxhostaddress.cpp \
	rzxconnectionlister.cpp \
	rzxserverlistener.cpp \
	rzxprotocole.cpp \
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
	md5.cpp

HEADERS += rzxglobal.h \
	rzxapplication.h \
	rzxhostaddress.h \
	rzxconnectionlister.h \
	rzxserverlistener.h \
	rzxprotocole.h \
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
	md5.h

FORMS += rzxchangepassui.ui \
	rzxwrongpassui.ui \
	rzxpropertyui.ui

TRANSLATIONS = ../translations/qrezix_fr.ts
