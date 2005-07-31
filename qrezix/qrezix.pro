# ###########################################
# ##       Fichier projet de qRezix       ###
# ###########################################

# Ce fichier permet de générer les Makefiles
# > qmake qrezix.pro

# ###########################################
# ## Partie 1 : Configuration de Qt       ###
# ###########################################
# Type de projet
TEMPLATE	= app
LANGUAGE	= C++

# Modules Qt nécessaires
QT += network

# Configuration de la compilation
CONFIG	+= qt warn_on debug
DEFINES += QT_DLL

# Options supplémentaires
win32:LIBS	+= IMM32.LIB

# ###########################################
# ## Partie 2 : Fichiers sources          ###
# ###########################################
# Fichier headers -> moc
HEADERS	+= qrezix.h \
	rzxchat.h \
	rzxclientlistener.h \
	rzxcomputer.h \
	rzxconfig.h \
	rzxhostaddress.h \
	rzxprotocole.h \
	rzxserverlistener.h \
	rzxproperty.h \
	rzxmessagebox.h \
	defaults.h \
	rzxtrayicon.h \
	rzxquit.h \
	rzxplugin.h \
	rzxpluginloader.h \
	rzxutilslauncher.h \
	rzxconnectionlister.h \
	md5.h \
	rzxtraywindow.h \
	rzxrezalmodel.h \
	rzxrezalview.h \
	rzxrezalpopup.h \
	rzxrezalsearch.h \
	rzxchatsocket.h \
	rzxglobal.h \
	rzxiconcollection.h \
	rzxapplication.h \
	rzxchatlister.h \
	rzxnotifier.h

# Fichier sources -> c++
SOURCES	+= main.cpp \
	qrezix.cpp \
	rzxchat.cpp \
	rzxclientlistener.cpp \
	rzxcomputer.cpp \
	rzxconfig.cpp \
	rzxhostaddress.cpp \
	rzxprotocole.cpp \
	rzxproperty.cpp \
	rzxserverlistener.cpp \
	rzxmessagebox.cpp \
	rzxtrayicon.cpp \
	rzxquit.cpp \
	rzxplugin.cpp \
	rzxpluginloader.cpp \
	rzxutilslauncher.cpp \
	rzxconnectionlister.cpp \
	md5.cpp \
	rzxtraywindow.cpp \
	rzxrezalmodel.cpp \
	rzxrezalview.cpp \
	rzxrezalpopup.cpp \
	rzxrezalsearch.cpp \
	rzxchatsocket.cpp \
	rzxglobal.cpp \
	rzxiconcollection.cpp \
	rzxapplication.cpp \
	rzxchatlister.cpp \
	rzxnotifier.cpp

# Fichiers ui -> uic
FORMS = rzxpropertyui.ui \
	rzxquitui.ui \
	rzxwrongpassui.ui \
	rzxchangepassui.ui \
	qrezixui.ui \
	rzxchatui.ui
macx {
	FORMS ~= s/rzxchatui.ui/rzxchatui_mac.ui/
}


# Ressources supplémentaire du projet
TRANSLATIONS	= ./translations/qrezix_fr.ts
RC_FILE		= icone.rc


# ###########################################
# ## Partie 3 : Installation              ###
# ###########################################
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
	icone.files = ./resources/*.icns
	icone.path = ./qRezix.app/Contents/Resources
	# Informations sur le programme
	info.files = ./resources/Info.plist ./resources/PkgInfo
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

