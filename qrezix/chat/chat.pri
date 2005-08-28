SOURCES += ../chat/rzxchatlister.cpp \
	../chat/rzxclientlistener.cpp \
	../chat/rzxchatsocket.cpp \
	../chat/rzxchat.cpp

HEADERS += ../chat/rzxchatlister.h \
	../chat/rzxclientlistener.h \
	../chat/rzxchatsocket.h \
	../chat/rzxchat.h \
	../chat/rzxchatconfig.h

FORMS += ../chat/rzxchatpropui.ui
mac {
	FORMS += ../chat/rzxchatui_mac.ui
} else {
	FORMS += ../chat/rzxchatui.ui
}
