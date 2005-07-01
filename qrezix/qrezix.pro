############################################
###       Fichier projet de qRezix       ###
############################################

# Ce fichier permet de générer les Makefiles
# > qmake qrezix.pro

############################################
### Partie 1 : Configuration de Qt       ###
############################################
# Type de projet
TEMPLATE	= app
LANGUAGE	= C++

# Modules Qt nécessaires
QT += qt3support network

# Configuration de la compilation
CONFIG	+= qt warn_on
DEFINES += QT_DLL

# Options supplémentaires
win32:LIBS	+= IMM32.LIB

############################################
### Partie 2 : Fichiers sources          ###
############################################
# Fichier headers -> moc
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

# Fichier sources -> c++
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

# Fichiers ui -> uic
FORMS = rzxpropertyui.ui \
	rzxquitui.ui \
	rzxwrongpassui.ui \
	rzxchangepassui.ui \
	qrezixui.ui
macx {
	FORMS += rzxchatui_mac.ui
}
!macx {
	FORMS += rzxchatui.ui
}

# Ressources supplémentaire du projet
TRANSLATIONS = ./translations/qrezix.ts \
	./translations/qrezix_fr.ts
RC_FILE         = icone.rc

############################################
### Partie 3 : Installation              ###
############################################
# Uniquement définie sur MacOS
# Consiste à mettre les thèmes/traductions
# dans le package du programme
macx {
	# Nom du programme
	TARGET = qRezix

	# Définie les règles pour installer
	#  regle.files = sources
	#  regle.path  = destination
	# Traduction :
	translations.files = ./translations/*.qm
	translations.path = ./qRezix.app/Contents/Resources/translations
	# Thèmes
	themes.files = ../icons/themes/*
	themes.path = ./qRezix.app/Contents/Resources/themes
	# Icône du programme
	icone.files = ./*.icns
	icone.path = ./qRezix.app/Contents/Resources
	# Informations sur le programme
	info.files = ./Info.plist
	info.path = ./qRezix.app/Contents

	# Déplace le paquet pour le mettre dans le répertoire
	# où il sera utiliser pour compiler un installer qvb
	qrezix.files = ./qRezix.app
	qrezix.path = ../macosx/root/Applications

	# Definition des modules à installer
	INSTALLS += translations \
		themes \
		icone \
		info \
		qrezix
}
