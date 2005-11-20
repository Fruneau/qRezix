TEMPLATE = subdirs

ROOT = ..
include(../rzxglobal.pri)

!contains(DEFINES, NO_MAINUI) {
	!contains(DEFINES, RZX_RZLVIEW_BUILTIN):!contains(DEFINES, NO_RZLVIEW):SUBDIRS += view
	!contains(DEFINES, RZX_RZLMAP_BUILTIN):!contains(DEFINES, NO_RZLMAP):SUBDIRS += map
	!contains(DEFINES, RZX_RZLDETAIL_BUILTIN):!contains(DEFINES, NO_RZLDETAIL):SUBDIRS += detail
	!contains(DEFINES, RZX_RZLINDEX_BUILTIN):!contains(DEFINES, NO_RZLINDEX):SUBDIRS += index
}
