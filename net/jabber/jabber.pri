warning("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
warning("! In order to compile the jabber module !")
warning("! you have to install the gloox library !")
warning("! More informations: jabber/README file !")
warning("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
warning("")

HEADERS += $$ROOT/jabber/rzxjabberprotocole.h \
	$$ROOT/jabber/rzxjabberconfig.h \
	$$ROOT/jabber/rzxjabberclient.h \
	$$ROOT/jabber/rzxjabbercomputer.h \
	$$ROOT/jabber/rzxjabberproperty.h

SOURCES += $$ROOT/jabber/rzxjabberprotocole.cpp \
	$$ROOT/jabber/rzxjabberconfig.cpp \
	$$ROOT/jabber/rzxjabberclient.cpp \
	$$ROOT/jabber/rzxjabbercomputer.cpp \
	$$ROOT/jabber/rzxjabberproperty.cpp

FORMS += $$ROOT/jabber/rzxjabberpropui.ui

LIBS += -lgloox

CONFIG += qt warn_on debug thread
