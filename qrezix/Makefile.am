####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = qrezix

## INCLUDES were found outside kdevelop specific part

qrezix_SOURCES = rzxchatui.ui qrezixui.ui rzxquitui.ui rzxpropertyui.ui rzxquit.cpp dnsValidator.cpp trayicon.cpp rzxchat.cpp rzxproperty.cpp rzxserverlistener.cpp rzxrezal.cpp rzxprotocole.cpp rzxitem.cpp rzxhostaddress.cpp rzxconfig.cpp rzxcomputer.cpp rzxclientlistener.cpp qrezix.cpp main.cpp rzxmessagebox.cpp rzxmessagebox.h defaults.h dnsvalidator.h klocale.h qrezix.h qrezixui.h resource.h rzxchat.h rzxchatui.h rzxclientlistener.h rzxcomputer.h rzxconfig.h rzxconfiguration.h rzxhostaddress.h rzxitem.h rzxmessagebox.h rzxproperty.h rzxpropertyui.h rzxprotocole.h rzxrezal.h rzxserverlistener.h trayicon.h rzxquit.h

qrezix_LDADD   =  $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)


EXTRA_DIST = main.cpp qrezix.cpp qrezix.h qrezix.desktop lo32-app-qrezix.png lo16-app-qrezix.png lo16-app-kdexnet.png lo32-app-kdexnet.png qrezixui.ui rzxchatui.ui rzxclientlistener.cpp rzxclientlistener.h rzxcomputer.cpp rzxcomputer.h rzxconfig.cpp rzxconfig.h rzxhostaddress.cpp rzxhostaddress.h rzxitem.cpp rzxitem.h rzxprotocole.cpp rzxprotocole.h rzxrezal.cpp rzxrezal.h rzxserverlistener.cpp rzxserverlistener.h rzxpropertyui.ui rzxproperty.cpp rzxproperty.h rzxchat.cpp rzxchat.h trayicon.cpp trayicon.h dnsValidator.cpp dnsvalidator.h rzxmessagebox.cpp rzxmessagebox.h defaults.h rzxquit.h rzxquit.cpp rzxquitui.ui

#ignorer cette partie pour la version QT?
install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/qrezix.desktop $(kde_appsdir)/Applications/qrezix.desktop
	$(mkinstalldirs) $(kde_icondir)/locolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/lo32-app-qrezix.png $(kde_icondir)/locolor/32x32/apps/qrezix.png
	$(mkinstalldirs) $(kde_icondir)/locolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/lo16-app-qrezix.png $(kde_icondir)/locolor/16x16/apps/qrezix.png

uninstall-local:
	-rm -f $(kde_appsdir)/Applications/qrezix.desktop
	-rm -f $(kde_icondir)/locolor/32x32/apps/qrezix.png
	-rm -f $(kde_icondir)/locolor/16x16/apps/qrezix.png

####### kdevelop will overwrite this part!!! (end)############
# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# set the include path for X, qt and KDE
INCLUDES = $(all_includes) -I/usr/include/kde

METASOURCES = AUTO

# the library search path. 
qrezix_LDFLAGS = $(all_libraries) $(KDE_RPATH)

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
SUBDIRS = 
