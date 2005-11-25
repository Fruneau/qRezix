unix {
	isEmpty(PREFIX):PREFIX = /usr
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

