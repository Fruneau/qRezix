CONFIG *= release
CONFIG -= debug
QT *= network

!debug:macx {
        QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
        CONFIG+=x86 ppc
}	

include($$ROOT/rzxinstall.pri)

HEADERS += $$ROOT/core/RzxGlobal \
	$$ROOT/core/RzxApplication \
	$$ROOT/core/RzxHostAddress \
	$$ROOT/core/RzxSubnet \
	$$ROOT/core/RzxConnectionLister \
	$$ROOT/core/RzxComputer \
	$$ROOT/core/RzxMessageBox \
	$$ROOT/core/RzxUtilsLauncher \
	$$ROOT/core/RzxProperty \
	$$ROOT/core/RzxAbstractConfig \
	$$ROOT/core/RzxConfig \
	$$ROOT/core/RzxIconCollection \
	$$ROOT/core/RzxThemedIcon \
	$$ROOT/core/RzxModule \
	$$ROOT/core/RzxBaseModule \
	$$ROOT/core/RzxBaseLoader \
	$$ROOT/core/RzxNetwork \
	$$ROOT/core/RzxWrongPass \
	$$ROOT/core/RzxChangePass \
	$$ROOT/core/RzxSound \
	$$ROOT/core/RzxTranslator \
	$$ROOT/core/RzxStyle \
	$$ROOT/core/RzxIntro \
	$$ROOT/core/RzxLoaderProp \
	$$ROOT/core/RzxInfoMessage \
	$$ROOT/core/RzxComputerList \
	$$ROOT/core/RzxFavoriteList \
	$$ROOT/core/RzxBanList \
	$$ROOT/core/RzxListEdit \
	$$ROOT/core/RzxComputerListWidget \
	$$ROOT/core/RzxQuickRun

win32:DEFINES += RZX_ALL_BUILTIN
!contains(DEFINES, NO_JABBER):!existLib(gloox):DEFINES += NO_JABBER
contains(DEFINES, RZX_ALL_BUILTIN) {
	!contains(DEFINES, NO_MAINUI):DEFINES += RZX_MAINUI_BUILTIN
	!contains(DEFINES, NO_TRAYICON):DEFINES += RZX_TRAYICON_BUILTIN
	!contains(DEFINES, NO_CHAT):DEFINES += RZX_CHAT_BUILTIN
	!contains(DEFINES, NO_NOTIFIER):DEFINES += RZX_NOTIFIER_BUILTIN
	!contains(DEFINES, NO_XNET):DEFINES += RZX_XNET_BUILTIN
	!contains(DEFINES, NO_JABBER):DEFINES += RZX_JABBER_BUILTIN
	!contains(DEFINES, NO_RZLVIEW):DEFINES += RZX_RZLVIEW_BUILTIN
	!contains(DEFINES, NO_RZLINDEX):DEFINES += RZX_RZLINDEX_BUILTIN
	!contains(DEFINES, NO_RZLDETAIL):DEFINES += RZX_RZLDETAIL_BUILTIN
	!contains(DEFINES, NO_RZLMAP):DEFINES += RZX_RZLMAP_BUILTIN
	!contains(DEFINES, NO_RZLBOB):DEFINES += RZX_RZLBOB_BUILTIN
}
