TEMPLATE = subdirs
LANGUAGE = C++
CONFIG	+= qt warn_on debug
DEFINES += QT_DLL
INCLUDEPATH += core

SUBDIRS += core main modules net rezals resources
mac | unix:system( find . -name 'Makefile' -exec rm {} \";\" 2>> /dev/null )
win32:system(for /r %1 in (*Makefile) do @del %1)

include(rzxinstall.pri)

!mac {
	unix {
		qrezix.files = qrezix
		qrezix.path = $$BINDEST/
	} else:win32 {
		qrezix.files = qrezix.exe
		qrezix.path = $$DEST/
	}
	INSTALLS += qrezix
}
