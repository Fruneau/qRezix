CONFIG *= debug
QT *= network

include(rzxinstall.pri)

HEADERS += ../core/RzxGlobal \
	../core/RzxApplication \
	../core/RzxHostAddress \
	../core/RzxSubnet \
	../core/RzxConnectionLister \
	../core/RzxComputer \
	../core/RzxMessageBox \
	../core/RzxUtilsLauncher \
	../core/RzxProperty \
	../core/RzxAbstractConfig \
	../core/RzxConfig \
	../core/RzxIconCollection \
	../core/RzxThemedIcon \
	../core/RzxModule \
	../core/RzxBaseModule \
	../core/RzxBaseLoader \
	../core/RzxNetwork \
	../core/RzxWrongPass \
	../core/RzxChangePass \
	../core/RzxSound \
	../core/RzxTranslator \
	../core/RzxStyle
