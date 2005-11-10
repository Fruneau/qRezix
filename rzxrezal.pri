ROOT = ..

include($$ROOT/rzxbasemodule.pri)
DESTDIR = $$ROOT/rezals

LIBS += -L$$ROOT/modules -lrzxmainui
