HEADERS += ../jabber/rzxjabberprotocole.h \
	../jabber/rzxjabberconfig.h \
	../jabber/rzxjabberclient.h

SOURCES += ../jabber/rzxjabberprotocole.cpp \
	../jabber/rzxjabberconfig.cpp \
	../jabber/rzxjabberclient.cpp

FORMS += ../jabber/rzxjabberpropui.ui

LIBS += -lgloox

CONFIG += qt warn_on debug thread