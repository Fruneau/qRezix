# Copyright 1999-2005 Binet Réseau
# Distributed under the terms of the GNU General Public License v2
# $Header$

inherit subversion

DESCRIPTION="qRezix : a xNet client made in Qt"
HOMEPAGE="http://frankiz/"

# Point to any required sources; these will be automatically downloaded by
# Portage.
LICENSE="GPL-2"
SLOT="0"

KEYWORDS="-*"
IUSE="chat mainui tray notifier xnet"

DEPEND=">=x11-libs/qt-4.0.1"

ESVN_REPO_URI="svn://skinwel/qrezix/branches/qrezix--qt4/"

DEFINES="NO_JABBER"

use_module() {
	if ! use $1; then
		DEFINES="${DEFINES} NO_$2"
	fi
	return 0; 
}

src_compile() {
	use_module chat CHAT
	use_module mainui MAINUI
	use_module tray TRAYICON
	use_module notifier NOTIFIER
	use_module xnet XNET
	qmake "DEFINES=${DEFINES}" "PREFIX=/usr" "DEST=${D}" qrezix.pro || die
	make || die
	if cd resources/translations; then
		lrelease *.ts
		cd ../..
	fi
}

src_install() {
	qmake "PREFIX=/usr" "DEST=${D}" qrezix.pro || die
	make install || die
}
