TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= qrezix.h \
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
		  defaults.h
SOURCES		= main.cpp \
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
		  rzxmessagebox.cpp
INTERFACES	= qrezixui.ui \
		  rzxchatui.ui \
		  rzxpropertyui.ui
TRANSLATIONS	= translations/qrezix.ts translations/qrezix_fr.ts
LIBS		+=IMM32.LIB
DEFINES         +=QT_DLL
RC_FILE          =icone.rc