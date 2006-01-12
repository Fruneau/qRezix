SOURCES += $$ROOT/modules/chat/rzxchatlister.cpp \
	$$ROOT/modules/chat/rzxclientlistener.cpp \
	$$ROOT/modules/chat/rzxchatsocket.cpp \
	$$ROOT/modules/chat/rzxchat.cpp \
	$$ROOT/modules/chat/rzxchatconfig.cpp \
	$$ROOT/modules/chat/rzxtextedit.cpp \
	$$ROOT/modules/chat/rzxsmileyui.cpp \
	$$ROOT/modules/chat/rzxsmileys.cpp \
	$$ROOT/modules/chat/rzxchatpopup.cpp \
	$$ROOT/modules/chat/rzxchatbrowser.cpp
	

HEADERS += $$ROOT/modules/chat/rzxchatlister.h \
	$$ROOT/modules/chat/rzxclientlistener.h \
	$$ROOT/modules/chat/rzxchatsocket.h \
	$$ROOT/modules/chat/rzxchat.h \
	$$ROOT/modules/chat/rzxchatconfig.h \
	$$ROOT/modules/chat/rzxtextedit.h \
	$$ROOT/modules/chat/rzxsmileyui.h \
	$$ROOT/modules/chat/rzxsmileys.h \
	$$ROOT/modules/chat/rzxchatpopup.h \
	$$ROOT/modules/chat/rzxchatbrowser.h

FORMS += $$ROOT/modules/chat/rzxchatprop.ui
mac {
	FORMS += $$ROOT/modules/chat/rzxchat_mac.ui
} else {
	FORMS += $$ROOT/modules/chat/rzxchat.ui
}


smileys.files = $$ROOT/resources/smileys

mac {
	smileys.path = $$ROOT/qRezix.app/Contents/Resources/
} else:unix {
	smileys.path = $$SYSDEST/
} else:win32 {
	smileys.path = $$DEST/
}
INSTALLS += smileys
