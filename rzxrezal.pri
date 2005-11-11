ROOT = ../..

TARGET = $$sprintf(rezal%1, $$MODULENAME)
include($$ROOT/rzxbasemodule.pri)
DESTDIR = $$ROOT/rezals

INCLUDEPATH += $$ROOT/modules/mainui
LIBS += -L$$ROOT/modules -lrzxmainui
