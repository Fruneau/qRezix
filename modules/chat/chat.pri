SOURCES += $$ROOT/chat/rzxchatlister.cpp \
	$$ROOT/chat/rzxclientlistener.cpp \
	$$ROOT/chat/rzxchatsocket.cpp \
	$$ROOT/chat/rzxchat.cpp \
	$$ROOT/chat/rzxchatconfig.cpp

HEADERS += $$ROOT/chat/rzxchatlister.h \
	$$ROOT/chat/rzxclientlistener.h \
	$$ROOT/chat/rzxchatsocket.h \
	$$ROOT/chat/rzxchat.h \
	$$ROOT/chat/rzxchatconfig.h

FORMS += $$ROOT/chat/rzxchatpropui.ui
mac {
	FORMS += $$ROOT/chat/rzxchatui_mac.ui
} else {
	FORMS += $$ROOT/chat/rzxchatui.ui
}
