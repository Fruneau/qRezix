SOURCES += $$ROOT/rezals/map/rzxrezalmap.cpp

HEADERS += $$ROOT/rezals/map/rzxrezalmap.h

maps.files = $$ROOT/resources/maps/*.png
mapscampus.files = $$ROOT/resources/maps/campus/*.png
mapini.files = $$ROOT/rezals/map/map.ini
mac {
        maps.path = $$ROOT/qRezix.app/Contents/Resources/maps
	mapscampus.path = $$ROOT/qRezix.app/Contents/Resources/maps/campus
	mapini.path = $$ROOT/qRezix.app/Contents/Resources
} else:unix {
	maps.path = $$SYSDEST/maps
	mapscampus.path = $$SYSDEST/maps/campus
	mapini.path = $$SYSDEST/
} else:win32 {
	maps.path = $$DEST/maps
	mapscampus.path = $$DEST/maps/campus
	mapini.path = $$DEST/
}
INSTALLS += maps mapscampus mapini
