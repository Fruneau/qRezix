!mac:unix {
	isEmpty(PREFIX):PREFIX = /usr/local
	isEmpty(LIBDIR) {
		LIBDIR = $$PREFIX/lib/qrezix
		LIBREL = qrezix/
	}
	isEmpty(SYSDIR):SYSDIR = $$PREFIX/share/qrezix
	isEmpty(BINDIR):BINDIR = $$PREFIX/bin
	LIBDEST = $$DEST/$$LIBDIR
	SYSDEST = $$DEST/$$SYSDIR
	BINDEST = $$DEST/$$BINDIR
	DEST = $$DEST/$$PREFIX
}

!win32:debug {
    QMAKE_CXXFLAGS += -O2 -g -fstrict-aliasing -Wall -Wextra -Werror -Wchar-subscripts \
                      -Wundef -Wcast-align -Wwrite-strings -Wsign-compare -Wunused \
                      -Wno-unused-parameter -Wuninitialized -Winit-self -Wpointer-arith \
                      -Wredundant-decls -Wformat-nonliteral -Wno-format-y2k -Wmissing-format-attribute
}

win32 {
	DEST = $$ROOT/packages/windows/install
}

defineTest(existLib) {
	LIB_PATH = /usr/lib /usr/local/lib /lib /sw/lib /sw/usr/lib /sw/usr/local/lib /opt/local/lib
	LIB = $$1

	for(path, LIB_PATH) {
		exists($$path/lib$${LIB}.so):return(true)
		mac:exists($$path/lib$${LIB}.dylib):return(true)
	}

	return(false)
}

