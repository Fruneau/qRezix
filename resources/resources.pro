TEMPLATE = subdirs

ROOT = ..
include(../rzxglobal.pri)

SUBDIRS += translations

themes.files = themes/*
translations.files = translations/*.qm

mac {
        translations.path = $$ROOT/qRezix.app/Contents/Resources/translations
        themes.path = $$ROOT/qRezix.app/Contents/Resources/themes

        icone.files = *.icns
        icone.path = $$ROOT/qRezix.app/Contents/Resources

        info.files = Info.plist ./resources/PkgInfo
        info.path = $$ROOT/qRezix.app/Contents

        INSTALLS += icone \
                info
} else:unix {
	themes.path = $$SYSDEST/themes
        translations.path = $$SYSDEST/translations
} else:win32 {
        themes.path = $$DEST/themes
        translations.path = $$DEST/translations
}

INSTALLS += translations \
        themes	
