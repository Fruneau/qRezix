TEMPLATE	= app
LANGUAGE	= C++
QT += qt3support
CONFIG	+= qt warn_on release

win32:LIBS	+= IMM32.LIB

DEFINES	+= QT_DLL QT3_SUPPORT

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


FORMS = rzxpropertyui.ui \
	rzxquitui.ui \
	rzxwrongpassui.ui \
	rzxchangepassui.ui

macx {
    FORMS += qrezixui_mac.ui \
        rzxchatui_mac.ui
    
}
!macx {
    FORMS += qrezixui.ui \
        rzxchatui.ui
}

TRANSLATIONS	= ./translations/qrezix.ts ./translations/qrezix_fr.ts
RC_FILE         = icone.rc

macx {
  TARGET = qRezix

  translations.files = ./translations/*
  translations.path = ./qRezix.app/Contents/Resources/translations
  themes.files = ../icons/themes/*
  themes.path = ./qRezix.app/Contents/Resources/themes
  icone.files = ./application.icns
  icone.path = ./qRezix.app/Contents/Resources
  info.files = ./Info.plist
  info.path = ./qRezix.app/Contents
  qrezix.files = ./qRezix.app
  qrezix.path = ../macosx/root/Applications

  INSTALLS += translations \
           themes \
           icone \
	       info \
           qrezix

}
#The following line was inserted by qt3to4
QT += network  
#The following line was inserted by qt3to4
CONFIG += uic3

#The following line was inserted by qt3to4
QT +=  
#The following line was inserted by qt3to4
CONFIG += uic3

#The following line was inserted by qt3to4
QT +=  
#The following line was inserted by qt3to4
CONFIG += uic3

