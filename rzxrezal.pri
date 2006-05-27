ROOT = ../..

macx:QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Resources/rezals/
TARGET = $$sprintf(rezal%1, $$MODULENAME)
LIBNAME = $$sprintf(librezal%1.so, $$MODULENAME)
include($$ROOT/rzxbasemodule.pri)
DESTDIR = $$ROOT/rezals

INCLUDEPATH += $$ROOT/modules/mainui
LIBS += -L$$ROOT/modules -lrzxmainui

lib.files = $$ROOT/rezals/lib$$TARGET*
mac {
	lib.path = $$ROOT/qRezix.app/Contents/Resources/rezals
} else:unix {
        lib.path = $$LIBDEST/rezals
        lib.extra = cd $$DEST/lib && ln -sf $$LIBREL/rezals/$$LIBNAME ./
} else:win32 {
	lib.path = $$DEST/rezals
}
INSTALLS += lib
