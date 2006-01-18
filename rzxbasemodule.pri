TEMPLATE = lib
CONFIG += qt plugin warn_on
DEFINES += QT_DLL
LANGUAGE = c++

TRANSLATIONS = $$ROOT/resources/translations/$$(MODULENAME)_fr.ts

include($$ROOT/rzxglobal.pri)

INCLUDEPATH += $$ROOT/core $$ROOT

win32 {
   LIBS += -L$$ROOT -lqrezix2
} else {
   LIBS += -L$$ROOT -lqrezix
}

