ROOT = ../..

mac:QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Resources/net
TARGET = $$sprintf(rzxnet%1, $$MODULENAME)
include($$ROOT/rzxbasemodule.pri)


DESTDIR = $$ROOT/net

lib.files = $$ROOT/net/lib$$TARGET*
mac {
        lib.path = $$ROOT/qRezix.app/Contents/Resources/net
} else:unix {
        lib.path = $$DEST/lib/qrezix/net
        lib.extra = ln -sf $$DEST/lib/qrezix/net/* $$DEST/lib
}
INSTALLS += lib
