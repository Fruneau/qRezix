TEMPLATE = subdirs
LANGUAGE = C++
CONFIG	+= qt warn_on debug
DEFINES += QT_DLL
INCLUDEPATH += core

SUBDIRS += core main

!contains(DEFINES, RZX_MAINUI_BUILTIN):!contains(DEFINES, NO_MAINUI):SUBDIRS += mainui
!contains(DEFINES, RZX_TRAYICON_BUILTIN):!contains(DEFINES, NO_TRAYICON):SUBDIRS += tray
!contains(DEFINES, RZX_NOTIFIER_BUILTIN):!contains(DEFINES, NO_NOTIFIER):SUBDIRS += notifier
!contains(DEFINES, RZX_CHAT_BUILTIN):!contains(DEFINES, NO_CHAT):SUBDIRS += chat
!contains(DEFINES, RZX_XNET_BUILTIN):!contains(DEFINES, NO_XNET):SUBDIRS += xnet

mac {
	mainlib.files = libqrezix*
	mainlib.path = qRezix.app/Contents/Frameworks
	networks.files = librzxnet*
	networks.path = qRezix.app/Contents/Resources/net
	modules.files = librzx*
	modules.path = qRezix.app/Contents/Resources/modules
	rezal.path = librezal*
	rezal.files = qRezix.app/Contents/Resources/rezals
	subnets.files = subnets.ini
	subnets.path = qRezix.app/Contents/Resources
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
	INSTALLS += mainlib \
		networks \
		modules \
		rezal \
		subnets \
		translations \
		themes \
		icone \
		info \
		qrezix
}

