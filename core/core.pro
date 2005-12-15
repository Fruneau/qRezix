TEMPLATE = lib
CONFIG += dll qt

mac:QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Frameworks/
DEFINES += RZX_BUILD_CORE RZX_MAJOR_VERSION=2 RZX_MINOR_VERSION=0 RZX_FUNNY_VERSION=0 RZX_SVNVERSION
VERSION = 2.0.0
DESTDIR = ..
TARGET = qrezix

ROOT = ..
INCLUDEPATH += ..
include(../rzxglobal.pri)
contains(DEFINES, RZX_MAINUI_BUILTIN):include($$ROOT/modules/mainui/mainui.pri)
contains(DEFINES, RZX_TRAYICON_BUILTIN):include($$ROOT/modules/tray/tray.pri)
contains(DEFINES, RZX_NOTIFIER_BUILTIN):include($$ROOT/modules/notifier/notifier.pri)
contains(DEFINES, RZX_CHAT_BUILTIN):include($$ROOT/modules//chat/chat.pri)
contains(DEFINES, RZX_XNET_BUILTIN):include($$ROOT/net/xnet/xnet.pri)
contains(DEFINES, RZX_JABBER_BUILTIN):include($$ROOT/net/jabber/jabber.pri)

DEFINES += PREFIX=\"$$PREFIX\"

SOURCES += rzxglobal.cpp \
	rzxapplication.cpp \
	rzxhostaddress.cpp \
	rzxsubnet.cpp \
	rzxconnectionlister.cpp \
	rzxcomputer.cpp \
	rzxmessagebox.cpp \
	rzxutilslauncher.cpp \
	rzxproperty.cpp \
	rzxabstractconfig.cpp \
	rzxconfig.cpp \
	rzxiconcollection.cpp \
	rzxthemedicon.cpp \
	rzxbasemodule.cpp \
	rzxmodule.cpp \
	rzxnetwork.cpp \
	rzxwrongpass.cpp \
	rzxchangepass.cpp \
	rzxsound.cpp \
	rzxtranslator.cpp \
	rzxstyle.cpp \
	rzxintro.cpp \
	rzxloaderprop.cpp \
	rzxinfomessage.cpp \
	rzxcomputerlist.cpp \
	rzxfavoritelist.cpp \
	rzxbanlist.cpp \
	rzxlistedit.cpp

HEADERS += rzxglobal.h \
	rzxapplication.h \
	rzxhostaddress.h \
	rzxsubnet.h \
	rzxconnectionlister.h \
	rzxcomputer.h \
	rzxmessagebox.h \
	rzxutilslauncher.h \
	rzxproperty.h \
	rzxabstractconfig.h \
	rzxconfig.h \
	rzxiconcollection.h \
	rzxthemedicon.h \
	rzxbasemodule.h \
	rzxbaseloader.h \
	rzxmodule.h \
	rzxnetwork.h \
	rzxwrongpass.h \
	rzxchangepass.h \
	rzxsound.h \
	rzxtranslator.h \
	rzxstyle.h \
	rzxintro.h \
	rzxloaderprop.h \
	rzxinfomessage.h \
	rzxcomputerlist.h \
	rzxfavoritelist.h \
	rzxbanlist.h \
	rzxlistedit.h
		
FORMS += rzxpropertyui.ui \
	rzxwrongpassui.ui \
	rzxchangepassui.ui \
	rzxintroui.ui \
	rzxloaderui.ui \
	rzxinfomessageui.ui \
	rzxlisteditui.ui

TRANSLATIONS = $$(ROOT)/resources/translations/qrezix_fr.ts

mainlib.files = $$ROOT/libqrezix*
subnets.files = subnet.ini
mac {
        mainlib.path = $$ROOT/qRezix.app/Contents/Frameworks
	subnets.path = $$ROOT/qRezix.app/Contents/Resources
} else:unix {
        mainlib.path = $$DEST/lib
	subnets.path = $$DEST/share/qrezix
} else:win32 {
	mainlib.path = $$DEST
	subnets.path = $$DEST
}
INSTALLS += mainlib subnets		
