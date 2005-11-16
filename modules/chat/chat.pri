SOURCES += $$ROOT/modules/chat/rzxchatlister.cpp \
	$$ROOT/modules/chat/rzxclientlistener.cpp \
	$$ROOT/modules/chat/rzxchatsocket.cpp \
	$$ROOT/modules/chat/rzxchat.cpp \
	$$ROOT/modules/chat/rzxchatconfig.cpp \
	$$ROOT/modules/chat/rzxtextedit.cpp \
	$$ROOT/modules/chat/rzxsmileyui.cpp
	

HEADERS += $$ROOT/modules/chat/rzxchatlister.h \
	$$ROOT/modules/chat/rzxclientlistener.h \
	$$ROOT/modules/chat/rzxchatsocket.h \
	$$ROOT/modules/chat/rzxchat.h \
	$$ROOT/modules/chat/rzxchatconfig.h \
	$$ROOT/modules/chat/rzxtextedit.h \
	$$ROOT/modules/chat/rzxsmileyui.h

FORMS += $$ROOT/modules/chat/rzxchatpropui.ui
mac {
	FORMS += $$ROOT/modules/chat/rzxchatui_mac.ui
} else {
	FORMS += $$ROOT/modules/chat/rzxchatui.ui
}


smileys.files = $$ROOT/resources/smileys/*

mac {
	smileys.path = qRezix.app/Contents/Resources/smileys
} else:unix {
	smileys.path = $$DEST/share/qrezix/smileys
}