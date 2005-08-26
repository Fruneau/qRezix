TEMPLATE = subdirs
LANGUAGE = C++
CONFIG	+= qt warn_on debug
DEFINES += QT_DLL
INCLUDEPATH += core

SUBDIRS += core main

!contains(DEFINES, RZX_MAINUI_BUILTIN):SUBDIRS += mainui
!contains(DEFINES, RZX_TRAYICON_BUILTIN):SUBDIRS += tray
!contains(DEFINES, RZX_NOTIFIER_BUILTIN):SUBDIRS += notifier
!contains(DEFINES, RZX_CHAT_BUILTIN):SUBDIRS += chat

mac {
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

