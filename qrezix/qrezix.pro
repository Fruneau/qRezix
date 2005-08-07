TEMPLATE = app
LANGUAGE = C++
CONFIG	+= qt warn_on debug
DEFINES += QT_DLL \
	RZX_MAINUI_BUILTIN \
	RZX_TRAYICON_BUILTIN \
	RZX_CHAT_BUILTIN \
	RZX_NOTIFIER_BUILTIN
win32:LIBS += IMM32.LIB
INCLUDEPATH += core

SUBDIRS += core

include(core/core.pri)
contains(DEFINES, RZX_MAINUI_BUILTIN):include(mainui/mainui.pri)
contains(DEFINES, RZX_TRAYICON_BUILTIN):include(tray/tray.pri)
contains(DEFINES, RZX_NOTIFIER_BUILTIN):include(notifier/notifier.pri)
contains(DEFINES, RZX_CHAT_BUILTIN):include(chat/chat.pri)

SOURCES	+= main.cpp

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

