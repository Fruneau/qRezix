TEMPLATE = subdirs
LANGUAGE = C++
CONFIG	+= qt warn_on debug
DEFINES += QT_DLL
INCLUDEPATH += core

SUBDIRS += core main modules net rezals
unix:system(find -name 'Makefile' -exec 'rm {}' ";" 2>> /dev/null)
mac:system(find . -name 'Makefile' -exec 'rm {}' ";" 2>> /dev/null)
system(cd resources/translations && lrelease *.ts && cd ../..)

include(rzxinstall.pri)
themes.files = resources/themes/*
translations.files = resources/translations/*.qm

mac {
	translations.path = qRezix.app/Contents/Resources/translations
	themes.path = qRezix.app/Contents/Resources/themes

	icone.files = resources/*.icns
	icone.path = qRezix.app/Contents/Resources

	info.files = resources/Info.plist ./resources/PkgInfo
	info.path = qRezix.app/Contents

	qrezix.files = qRezix.app
	qrezix.path = ../macosx/root/Applications
	INSTALLS += icone \
		info

} else:unix {
	themes.path = $$DEST/share/qrezix/themes
	translations.path = $$DEST/share/qrezix/translations
	qrezix.files = qrezix
	qrezix.path = $$DEST/bin
}

INSTALLS += translations \
	themes \
	qrezix
