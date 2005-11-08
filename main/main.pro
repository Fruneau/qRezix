TEMPLATE = app
LANGUAGE = C++
CONFIG	+= qt warn_on
DEFINES += QT_DLL


include(../rzxglobal.pri)

LIBS += -L.. -lqrezix
INCLUDEPATH += .. ../core

SOURCES	+= main.cpp

TRANSLATIONS	= translations/qrezix_fr.ts
RC_FILE		= ../icone.rc

DESTDIR = ..
mac {
	TARGET = qRezix
} else {
	TARGET = qrezix
}
