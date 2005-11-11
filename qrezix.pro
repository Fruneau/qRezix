TEMPLATE = subdirs
LANGUAGE = C++
CONFIG	+= qt warn_on debug
DEFINES += QT_DLL
INCLUDEPATH += core

SUBDIRS += core main modules net rezals
system(cd resources/translations && lrelease *.ts && cd ../..)

include(rzxinstall.pri)
mainlib.files = libqrezix*
networks.files = net/librzxnet*
modules.files = modules/librzx*
rezal.files = rezals/librezal*
subnets.files = subnet.ini
themes.files = resources/themes/*
translations.files = resources/translations/*.qm

mac {
	mainlib.path = qRezix.app/Contents/Frameworks
	networks.path = qRezix.app/Contents/Resources/net
	modules.path = qRezix.app/Contents/Resources/modules
	rezal.path = qRezix.app/Contents/Resources/rezals
	subnets.path = qRezix.app/Contents/Resources
	translations.path = qRezix.app/Contents/Resources/translations
	themes.path = qRezix.app/Contents/Resources/themes

	icone.files = resources/*.icns
	icone.path = qRezix.app/Contents/Resources

	info.files = resources/Info.plist ./resources/PkgInfo
	info.path = qRezix.app/Contents

	qrezix.files = qRezix.app
	qrezix.path = ../macosx/root/Applications
	INSTALL += icone \
		info

} else:unix {
	mainlib.path = $$DEST/lib
	networks.path = $$DEST/lib/qrezix/net
	modules.path = $$DEST/lib/qrezix/modules
	rezal.path = $$DEST/lib/qrezix/rezals
	subnets.path = $$DEST/share/qrezix
	themes.path = $$DEST/share/qrezix/themes
	translations.path = $$DEST/share/qrezix/translations
	qrezix.files = qrezix
	qrezix.path = $$DEST/bin
}

INSTALLS += mainlib \
	networks \
	modules \
	rezal \
	subnets \
	translations \
	themes \
	qrezix
