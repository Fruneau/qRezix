DEFINES += RZX_BUILD_MAINUI

INCLUDEPATH += $$ROOT/modules/mainui

SOURCES += $$ROOT/modules/mainui/rzxui.cpp \
	$$ROOT/modules/mainui/qrezix.cpp \
	$$ROOT/modules/mainui/rzxrezal.cpp \
	$$ROOT/modules/mainui/rzxrezalmodel.cpp \
	$$ROOT/modules/mainui/rzxrezalpopup.cpp \
	$$ROOT/modules/mainui/rzxrezalsearch.cpp \
	$$ROOT/modules/mainui/rzxrezaldrag.cpp \
	$$ROOT/modules/mainui/rzxquit.cpp

HEADERS += $$ROOT/modules/mainui/rzxui.h \
	$$ROOT/modules/mainui/qrezix.h \
	$$ROOT/modules/mainui/rzxrezal.h \
	$$ROOT/modules/mainui/rzxrezalmodel.h \
	$$ROOT/modules/mainui/rzxrezalpopup.h \
	$$ROOT/modules/mainui/rzxrezalsearch.h \
	$$ROOT/modules/mainui/rzxrezaldrag.h \
	$$ROOT/modules/mainui/rzxdict.h \
	$$ROOT/modules/mainui/rzxquit.h \
	$$ROOT/modules/mainui/rzxmainuiconfig.h \
	$$ROOT/modules/mainui/rzxmainuiglobal.h

FORMS += $$ROOT/modules/mainui/rzxstatus.ui \
	$$ROOT/modules/mainui/rzxquit.ui \
	$$ROOT/modules/mainui/rzxmainuiprop.ui

contains(DEFINES, RZX_RZLVIEW_BUILTIN):include($$ROOT/rezals/view/view.pri)
contains(DEFINES, RZX_RZLDETAIL_BUILTIN):include($$ROOT/rezals/detail/detail.pri)
contains(DEFINES, RZX_RZLINDEX_BUILTIN):include($$ROOT/rezals/index/index.pri)
contains(DEFINES, RZX_RZLMAP_BUILTIN):include($$ROOT/rezals/map/map.pri)
