CONFIG *= debug
QT *= network

include(rzxinstall.pri)

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
	$$ROOT/core/RzxStyle
