TEMPLATE = subdirs

ROOT = ..
include(../rzxglobal.pri)

!contains(DEFINES, RZX_MAINUI_BUILTIN):!contains(DEFINES, NO_MAINUI):SUBDIRS += mainui
!contains(DEFINES, RZX_TRAYICON_BUILTIN):!contains(DEFINES, NO_TRAYICON):SUBDIRS += tray
!contains(DEFINES, RZX_NOTIFIER_BUILTIN):!contains(DEFINES, NO_NOTIFIER):SUBDIRS += notifier
!contains(DEFINES, RZX_CHAT_BUILTIN):!contains(DEFINES, NO_CHAT):SUBDIRS += chat

