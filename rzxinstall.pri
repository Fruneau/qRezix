unix {
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

win32 {
	DEST = $$ROOT/packages/windows/install
}

defineTest(existLib) {
	LIB_PATH = /usr/lib /usr/local/lib /lib /sw/lib /sw/usr/lib /sw/usr/local/lib
	LIB = $$1

	for(path, LIB_PATH) {
		exists($$path/lib$${LIB}.so):return(true)
		mac:exists($$path/lib$${LIB}.dylib):return(true)
	}

	return(false)
}

