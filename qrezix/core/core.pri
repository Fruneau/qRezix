QT *= network

SOURCES += core/rzxglobal.cpp \
	core/rzxapplication.cpp \
	core/rzxhostaddress.cpp \
	core/rzxconnectionlister.cpp \
	core/rzxserverlistener.cpp \
	core/rzxprotocole.cpp \
	core/rzxcomputer.cpp \
	core/rzxmessagebox.cpp \
	core/rzxutilslauncher.cpp \
	core/rzxproperty.cpp \
	core/rzxconfig.cpp \
	core/rzxiconcollection.cpp \
	core/rzxmodule.cpp \
	core/rzxplugin.cpp \
	core/rzxpluginloader.cpp \
	core/md5.cpp

HEADERS += core/rzxglobal.h \
	core/rzxapplication.h \
	core/rzxhostaddress.h \
	core/rzxconnectionlister.h \
	core/rzxserverlistener.h \
	core/rzxprotocole.h \
	core/rzxcomputer.h \
	core/rzxmessagebox.h \
	core/rzxutilslauncher.h \
	core/rzxproperty.h \
	core/rzxconfig.h \
	core/rzxiconcollection.h \
	core/rzxmodule.h \
	core/rzxplugin.h \
	core/rzxpluginloader.h \
	core/md5.h

FORMS += core/rzxchangepassui.ui \
	core/rzxwrongpassui.ui \
	core/rzxpropertyui.ui
