ROOT = ../..

mac:QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Resources/modules
TARGET = $$sprintf(rzx%1, $$MODULENAME)
include($$ROOT/rzxbasemodule.pri)

DESTDIR = $$ROOT/modules

lib.files = $$ROOT/modules/lib$$TARGET*
mac {
	lib.path = qRezix.app/Contents/Resources/modules
} else:unix {
        lib.path = $$DEST/lib/qrezix/modules
}
INSTALLS += lib
