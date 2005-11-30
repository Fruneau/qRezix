ROOT = ../..

mac:QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Resources/modules
TARGET = $$sprintf(rzx%1, $$MODULENAME)
include($$ROOT/rzxbasemodule.pri)

DESTDIR = $$ROOT/modules

lib.files = $$ROOT/modules/lib$$TARGET*
mac {
	lib.path = $$ROOT/qRezix.app/Contents/Resources/modules
} else:unix {
        lib.path = $$DEST/lib/qrezix/modules
	lib.extra = cd $$DEST/lib && ln -sf qrezix/modules/* ./
} else:win32 {
	lib.path = $$DEST/modules
}
INSTALLS += lib
