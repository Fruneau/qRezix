SOURCES += $$ROOT/rezals/map/rzxrezalmap.cpp

HEADERS += $$ROOT/rezals/map/rzxrezalmap.h

maps.files = $$ROOT/rezals/map/*.png $$ROOT/rezals/map/*.jpg
mapini.files = $$ROOT/rezals/map/map.ini
mac {
        maps.path = $$ROOT/qRezix.app/Contents/Resources/maps
	mapini.path = $$ROOT/qRezix.app/Contents/Resources
} else:unix {
	maps.path = $$DEST/share/qrezix/maps
	mapini.path = $$DEST/share/qrezix
} else:win32 {
	maps.path = $$DEST/maps
	mapini.path = $$DEST/
}
INSTALLS += maps mapini
