TEMPLATE = subdirs

!contains(DEFINES, RZX_XNET_BUILTIN):!contains(DEFINES, NO_XNET):SUBDIRS += xnet
!contains(DEFINES, RZX_JABBER_BUILTIN):!contains(DEFINES, NO_JABBER):SUBDIRS += jabber

