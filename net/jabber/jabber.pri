warning("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
warning("! In order to compile the jabber module !")
warning("! you have to install the gloox library !")
warning("! More informations: jabber/README file !")
warning("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
warning("")
warning("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
warning("!    This module is not maintained      !")
warning("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
warning("")

HEADERS += $$ROOT/net/jabber/rzxjabberprotocole.h \
	$$ROOT/net/jabber/rzxjabberconfig.h \
	$$ROOT/net/jabber/rzxjabberclient.h \
	$$ROOT/net/jabber/rzxjabbercomputer.h \
	$$ROOT/net/jabber/rzxjabberproperty.h

SOURCES += $$ROOT/net/jabber/rzxjabberprotocole.cpp \
	$$ROOT/net/jabber/rzxjabberconfig.cpp \
	$$ROOT/net/jabber/rzxjabberclient.cpp \
	$$ROOT/net/jabber/rzxjabbercomputer.cpp \
	$$ROOT/net/jabber/rzxjabberproperty.cpp

FORMS += $$ROOT/net/jabber/rzxjabberprop.ui

LIBS += -lgloox

CONFIG += qt warn_on debug thread
