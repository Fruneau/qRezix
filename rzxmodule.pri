ROOT = ../..

macx:QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Resources/modules/
TARGET = $$sprintf(rzx%1, $$MODULENAME)
LIBNAME = $$sprintf(librzx%1.so, $$MODULENAME)
include($$ROOT/rzxbasemodule.pri)

DESTDIR = $$ROOT/modules

lib.files = $$ROOT/modules/lib$$TARGET*
mac {
	lib.path = $$ROOT/qRezix.app/Contents/Resources/modules
} else:unix {
        lib.path = $$LIBDEST/modules
	!isEmpty(LIBREL):lib.extra = cd $$DEST/lib && ln -sf $$LIBREL/modules/$$LIBNAME ./
} else:win32 {
	lib.path = $$DEST/modules
}
INSTALLS += lib
