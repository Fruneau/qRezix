SOURCES += $$ROOT/modules/chat/rzxchatlister.cpp \
	$$ROOT/modules/chat/rzxclientlistener.cpp \
	$$ROOT/modules/chat/rzxchatsocket.cpp \
	$$ROOT/modules/chat/rzxchat.cpp \
	$$ROOT/modules/chat/rzxchatconfig.cpp \
	$$ROOT/modules/chat/rzxtextedit.cpp \
	$$ROOT/modules/chat/rzxsmileyui.cpp \
	$$ROOT/modules/chat/rzxsmileys.cpp \
	$$ROOT/modules/chat/rzxchatpopup.cpp \
	$$ROOT/modules/chat/rzxchatbrowser.cpp \
	$$ROOT/modules/chat/rzxfilelistener.cpp \
	$$ROOT/modules/chat/rzxfilesocket.cpp \
	$$ROOT/modules/chat/rzxfilewidget.cpp

HEADERS += $$ROOT/modules/chat/rzxchatlister.h \
	$$ROOT/modules/chat/rzxclientlistener.h \
	$$ROOT/modules/chat/rzxchatsocket.h \
	$$ROOT/modules/chat/rzxchat.h \
	$$ROOT/modules/chat/rzxchatconfig.h \
	$$ROOT/modules/chat/rzxtextedit.h \
	$$ROOT/modules/chat/rzxsmileyui.h \
	$$ROOT/modules/chat/rzxsmileys.h \
	$$ROOT/modules/chat/rzxchatpopup.h \
	$$ROOT/modules/chat/rzxchatbrowser.h \
	$$ROOT/modules/chat/rzxfilelistener.h \
	$$ROOT/modules/chat/rzxfilesocket.h \
	$$ROOT/modules/chat/rzxfilewidget.h

FORMS += $$ROOT/modules/chat/rzxchatprop.ui
mac {
	FORMS += $$ROOT/modules/chat/rzxchat_mac.ui
} else {
	FORMS += $$ROOT/modules/chat/rzxchat.ui
}

contains(DEFINES, RZX_CHAT_BUILTIN) {
  INCLUDEPATH += ../modules/chat/
}

smileysBasic2.files = $$ROOT/resources/smileys/Basic2/theme \
	$$ROOT/resources/smileys/Basic2/*.png
smileysBasic.files = $$ROOT/resources/smileys/Basic/theme \
	$$ROOT/resources/smileys/Basic/*.png
smileysPingu.files = $$ROOT/resources/smileys/Pingu/theme \
	$$ROOT/resources/smileys/Pingu/*.png
smileysMSNLike.files = $$ROOT/resources/smileys/MSNLike/theme \
	$$ROOT/resources/smileys/MSNLike/*.png

mac {
	smileysBasic2.path = $$ROOT/qRezix.app/Contents/Resources/smileys/Basic2
	smileysBasic.path = $$ROOT/qRezix.app/Contents/Resources/smileys/Basic
	smileysPingu.path = $$ROOT/qRezix.app/Contents/Resources/smileys/Pingu
	smileysMSNLike.path = $$ROOT/qRezix.app/Contents/Resources/smileys/MSNLike
} else:unix {
	smileysBasic2.path = $$SYSDEST/smileys/Basic2
	smileysBasic.path = $$SYSDEST/smileys/Basic
	smileysPingu.path = $$SYSDEST/smileys/Pingu
	smileysMSNLike.path = $$SYSDEST/smileys/MSNLike
} else:win32 {
	smileysBasic2.path = $$DEST/smileys/Basic2
	smileysBasic.path = $$DEST/smileys/Basic
	smileysPingu.path = $$DEST/smileys/Pingu
	smileysMSNLike.path = $$DEST/smileys/MSNLike
}
INSTALLS += smileysBasic2 smileysBasic smileysMSNLike smileysPingu
