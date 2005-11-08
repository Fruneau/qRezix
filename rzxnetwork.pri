TEMPLATE = lib
CONFIG += qt plugin warn_on
DEFINES += QT_DLL
LANGUAGE = c++

TARGET = $$sprintf(rzxnet%1, $$TARGET)
TRANSLATIONS = ../translations/$$(MODULENAME)_fr.ts

include(../rzxglobal.pri)

DESTDIR = ..
INCLUDEPATH += ../core ..
LIBS += -L.. -lqrezix
