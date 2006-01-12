SOURCES += $$ROOT/rezals/map/rzxrezalmap.cpp

HEADERS += $$ROOT/rezals/map/rzxrezalmap.h

maps.files = $$ROOT/resources/maps/*
mapini.files = $$ROOT/rezals/map/map.ini
mac {
        maps.path = $$ROOT/qRezix.app/Contents/Resources/maps
	mapini.path = $$ROOT/qRezix.app/Contents/Resources
} else:unix {
	maps.path = $$SYSDEST/maps
	mapini.path = $$SYSDEST/
} else:win32 {
	maps.path = $$DEST/maps
	mapini.path = $$DEST/
}
INSTALLS += maps mapini
