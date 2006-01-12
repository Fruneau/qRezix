ROOT = ../..

mac:QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Resources/net
TARGET = $$sprintf(rzxnet%1, $$MODULENAME)
LIBNAME = $$sprintf(librzxnet%1.so, $$MODULENAME)
include($$ROOT/rzxbasemodule.pri)


DESTDIR = $$ROOT/net

lib.files = $$ROOT/net/lib$$TARGET*
mac {
        lib.path = $$ROOT/qRezix.app/Contents/Resources/net
} else:unix {
        lib.path = $$LIBDEST/net
        !isEmpty(LIBREL):lib.extra = cd $$DEST/lib && ln -sf $$LIBREL/net/$$LIBNAME ./
} else:win32 {
	lib.path = $$DEST/net
}
INSTALLS += lib
