TEMPLATE = app
LANGUAGE = C++
CONFIG	+= qt warn_on
DEFINES += QT_DLL

ROOT = ..
include(../rzxglobal.pri)

win32 {
	LIBS += -L$$ROOT -lqrezix1
} else {
	LIBS += -L$$ROOT -lqrezix
}

INCLUDEPATH += $$ROOT $$ROOT/core

SOURCES	+= main.cpp

TRANSLATIONS	= $$ROOT/resources/translations/qrezix_fr.ts
RC_FILE		= $$ROOT/resources/icone.rc

DESTDIR = $$ROOT
mac {
	TARGET = qRezix
} else {
	TARGET = qrezix
}
