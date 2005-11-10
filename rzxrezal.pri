ROOT = ../..

TARGET = $$sprintf(rezal%1, $$MODULENAME)
include($$ROOT/rzxbasemodule.pri)
DESTDIR = $$ROOT/rezals

LIBS += -L$$ROOT/modules -lrzxmainui
