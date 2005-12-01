SOURCES += $$ROOT/modules/chat/rzxchatlister.cpp \
	$$ROOT/modules/chat/rzxclientlistener.cpp \
	$$ROOT/modules/chat/rzxchatsocket.cpp \
	$$ROOT/modules/chat/rzxchat.cpp \
	$$ROOT/modules/chat/rzxchatconfig.cpp \
	$$ROOT/modules/chat/rzxtextedit.cpp \
	$$ROOT/modules/chat/rzxsmileyui.cpp \
	$$ROOT/modules/chat/rzxchatpopup.cpp
	

HEADERS += $$ROOT/modules/chat/rzxchatlister.h \
	$$ROOT/modules/chat/rzxclientlistener.h \
	$$ROOT/modules/chat/rzxchatsocket.h \
	$$ROOT/modules/chat/rzxchat.h \
	$$ROOT/modules/chat/rzxchatconfig.h \
	$$ROOT/modules/chat/rzxtextedit.h \
	$$ROOT/modules/chat/rzxsmileyui.h \
	$$ROOT/modules/chat/rzxchatpopup.h

FORMS += $$ROOT/modules/chat/rzxchatpropui.ui
mac {
	FORMS += $$ROOT/modules/chat/rzxchatui_mac.ui
} else {
	FORMS += $$ROOT/modules/chat/rzxchatui.ui
}


smileys.files = $$ROOT/resources/smileys

mac {
	smileys.path = $$ROOT/qRezix.app/Contents/Resources/
} else:unix {
	smileys.path = $$DEST/share/qrezix/
} else:win32 {
	smileys.path = $$DEST/
}
INSTALLS += smileys
