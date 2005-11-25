TEMPLATE = subdirs
LANGUAGE = C++
CONFIG	+= qt warn_on debug
DEFINES += QT_DLL
INCLUDEPATH += core

SUBDIRS += core main modules net rezals resources
unix:system(find -name 'Makefile' -exec rm {} ";" 2>> /dev/null)
mac:system(find . -name 'Makefile' -exec rm {} ";" 2>> /dev/null)

include(rzxinstall.pri)

mac {
	qrezix.files = qRezix.app
	qrezix.path = packages/macosx/
} else:unix {
	qrezix.files = qrezix
	qrezix.path = $$DEST/bin
} else:win32 {
	qrezix.files = qrezix.exe
	qrezix.path = $$DEST/
}

INSTALLS += qrezix
