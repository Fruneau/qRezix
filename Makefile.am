####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = qrezix

## INCLUDES were found outside kdevelop specific part


qrezix_SOURCES = rzxtraywindow.cpp md5.cpp rzxconnectionlister.cpp rzxchangepassui.ui rzxwrongpassui.ui rzxchatui.ui qrezixui.ui rzxquitui.ui rzxpropertyui.ui rzxutilslauncher.cpp rzxpluginloader.cpp rzxplugin.cpp rzxquit.cpp trayicon.cpp rzxchat.cpp rzxproperty.cpp rzxserverlistener.cpp rzxrezal.cpp rzxprotocole.cpp rzxitem.cpp rzxhostaddress.cpp rzxconfig.cpp rzxcomputer.cpp rzxclientlistener.cpp qrezix.cpp main.cpp rzxmessagebox.cpp rzxmessagebox.h defaults.h qrezix.h resource.h rzxchat.h rzxclientlistener.h rzxcomputer.h rzxconfig.h rzxhostaddress.h rzxitem.h rzxmessagebox.h rzxproperty.h rzxprotocole.h rzxrezal.h rzxserverlistener.h trayicon.h rzxquit.h rzxplugin.h rzxpluginloader.h rzxutilslauncher.h rzxconnectionlister.h md5.h rzxtraywindow.h

qrezix_LDADD   =  $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)


EXTRA_DIST = main.cpp qrezix.cpp qrezix.h qrezix.desktop lo32-app-qrezix.png lo16-app-qrezix.png lo16-app-kdexnet.png lo32-app-kdexnet.png qrezixui.ui rzxchatui.ui rzxclientlistener.cpp rzxclientlistener.h rzxcomputer.cpp rzxcomputer.h rzxconfig.cpp rzxconfig.h rzxhostaddress.cpp rzxhostaddress.h rzxitem.cpp rzxitem.h rzxprotocole.cpp rzxprotocole.h rzxrezal.cpp rzxrezal.h rzxserverlistener.cpp rzxserverlistener.h rzxpropertyui.ui rzxproperty.cpp rzxproperty.h rzxchat.cpp rzxchat.h trayicon.cpp trayicon.h rzxmessagebox.cpp rzxmessagebox.h defaults.h rzxquit.h rzxquit.cpp rzxquitui.ui rzxplugin.h rzxwrongpassui.ui rzxwrongpassui.h rzxwrongpassui.cpp rzxplugin.cpp rzxpluginloader.h rzxpluginloader.cpp rzxutilslauncher.h rzxutilslauncher.cpp rzxchangepassui.ui rzxchangepassui.h rzxchangepassui.cpp rzxconnectionlister.cpp rzxconnectionlister.h md5.cpp md5.h rzxtraywindow.cpp rzxtraywindow.h



# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

METASOURCES = AUTO

# the library search path. 
qrezix_LDFLAGS = $(all_libraries) 

# Uncomment the following two lines if you add a ui.rc file for your application to make use of
# KDE
#rcdir = $(kde_datadir)/qrezix
#rc_DATA = qrezixui.rc

#WARNING: if you use a ui.rc file above, use:

# messages: rc.cpp

# instead of 

# messages:

messages: 
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/qrezix.pot; \
	fi

AM_CFLAGS = 
AM_CXXFLAGS = 
AM_FFLAGS = 
SUBDIRS = translations
