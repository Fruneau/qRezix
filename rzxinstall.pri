unix {
	isEmpty(PREFIX):PREFIX = /usr
	DEST = $$DEST/$$PREFIX
}

win32 {
	DEST = $$ROOT/packages/windows/install
}
