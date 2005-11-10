SOURCES += $$ROOT/modules/chat/rzxchatlister.cpp \
	$$ROOT/modules/chat/rzxclientlistener.cpp \
	$$ROOT/modules/chat/rzxchatsocket.cpp \
	$$ROOT/modules/chat/rzxchat.cpp \
	$$ROOT/modules/chat/rzxchatconfig.cpp

HEADERS += $$ROOT/modules/chat/rzxchatlister.h \
	$$ROOT/modules/chat/rzxclientlistener.h \
	$$ROOT/modules/chat/rzxchatsocket.h \
	$$ROOT/modules/chat/rzxchat.h \
	$$ROOT/modules/chat/rzxchatconfig.h

FORMS += $$ROOT/modules/chat/rzxchatpropui.ui
mac {
	FORMS += $$ROOT/modules/chat/rzxchatui_mac.ui
} else {
	FORMS += $$ROOT/modules/chat/rzxchatui.ui
}
