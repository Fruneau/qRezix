ROOT = ../..

mac:QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Resources/rezals
TARGET = $$sprintf(rezal%1, $$MODULENAME)
include($$ROOT/rzxbasemodule.pri)
DESTDIR = $$ROOT/rezals

INCLUDEPATH += $$ROOT/modules/mainui
LIBS += -L$$ROOT/modules -lrzxmainui

lib.files = $$ROOT/rezals/lib$$TARGET*
mac {
	lib.path = $$ROOT/qRezix.app/Contents/Resources/rezals
} else:unix {
        lib.path = $$DEST/lib/qrezix/rezals
        lib.extra = ln -sf $$DEST/lib/qrezix/rezals/* $$DEST/lib
}
INSTALLS += lib
