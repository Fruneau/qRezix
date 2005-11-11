ROOT = ../..

mac:QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Resources/net
TARGET = $$sprintf(rzxnet%1, $$MODULENAME)
include($$ROOT/rzxbasemodule.pri)


DESTDIR = $$ROOT/net

