SOURCES += $$ROOT/modules/tray/rzxtrayicon.cpp

HEADERS += $$ROOT/modules/tray/rzxtrayicon.h \
	$$ROOT/modules/tray/rzxtrayconfig.h

FORMS += $$ROOT/modules/tray/rzxtraypropui.ui

win32:LIBS += -lgdi32 -lshell32 -luser32