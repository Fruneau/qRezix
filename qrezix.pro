TEMPLATE = app
LANGUAGE = C++
CONFIG	+= qt warn_on debug
DEFINES += QT_DLL
win32:LIBS += IMM32.LIB

include(core/core.pri)
include(mainui/mainui.pri)
include(tray/tray.pri)
include(notifier/notifier.pri)
include(chat/chat.pri)

HEADERS	+= rzxapplication.h
SOURCES	+= main.cpp rzxapplication.cpp

TRANSLATIONS	= translations/qrezix_fr.ts
RC_FILE		= icone.rc

mac {
	TARGET = qRezix
	translations.files = translations/*.qm
	translations.path = qRezix.app/Contents/Resources/translations
	themes.files = ../icons/themes/*
	themes.path = qRezix.app/Contents/Resources/themes
	icone.files = resources/*.icns
	icone.path = qRezix.app/Contents/Resources
	info.files = resources/Info.plist ./resources/PkgInfo
	info.path = qRezix.app/Contents
	qrezix.files = qRezix.app
	qrezix.path = ../macosx/root/Applications
	INSTALLS += translations \
		themes \
		icone \
		info \
		qrezix
}

