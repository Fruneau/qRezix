TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release

win32:LIBS	+= IMM32.LIB

DEFINES	+= QT_DLL

HEADERS	+= qrezix.h \
	rzxchat.h \
	rzxclientlistener.h \
	rzxcomputer.h \
	rzxconfig.h \
	rzxhostaddress.h \
	rzxitem.h \
	rzxprotocole.h \
	rzxrezal.h \
	rzxserverlistener.h \
	rzxproperty.h \
	rzxmessagebox.h \
	defaults.h \
	trayicon.h \
	rzxquit.h \
	rzxplugin.h \
	rzxpluginloader.h \
	rzxutilslauncher.h \
	rzxconnectionlister.h \
	md5.h \
	rzxtraywindow.h

SOURCES	+= main.cpp \
	qrezix.cpp \
	rzxchat.cpp \
	rzxclientlistener.cpp \
	rzxcomputer.cpp \
	rzxconfig.cpp \
	rzxhostaddress.cpp \
	rzxitem.cpp \
	rzxprotocole.cpp \
	rzxrezal.cpp \
	rzxproperty.cpp \
	rzxserverlistener.cpp \
	rzxmessagebox.cpp \
	trayicon.cpp \
	rzxquit.cpp \
	rzxplugin.cpp \
	rzxpluginloader.cpp \
	rzxutilslauncher.cpp \
	rzxconnectionlister.cpp \
	md5.cpp \
	rzxtraywindow.cpp

FORMS	= qrezixui.ui \
	rzxchatui.ui \
	rzxpropertyui.ui \
	rzxquitui.ui \
	rzxwrongpassui.ui \
	rzxchangepassui.ui

TRANSLATIONS	= ./translations/qrezix.ts ./translations/qrezix_fr.ts
RC_FILE         = icone.rc

macx {
  translations.files = ./translations/*
  translations.path = ./qrezix.app/Contents/Resources/translations
  themes.files = ../icons/themes/*
  themes.path = ./qrezix.app/Contents/Resources/themes
  icone.files = ./application.icns
  icone.path = ./qrezix.app/Contents/Resources

  movecommand.files =
  movecommand.path =
  movecommand.commands = mv -f qrezix.app ../macosx/root/Applications/qrezix.app

  INSTALLS += translations \
           themes \
           icone \
           movecommand

}
