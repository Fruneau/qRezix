# Copyright 1999-2005 Binet Réseau
# Distributed under the terms of the GNU General Public License v2
# $Header$

inherit kde-functions subversion
need-qt 3.3

DESCRIPTION="qRezix : a xNet client made in Qt"
HOMEPAGE="http://frankiz/"

# Point to any required sources; these will be automatically downloaded by
# Portage.
LICENSE="GPL-2"
SLOT="0"

KEYWORDS="~x86 ~amd64"
IUSE="xplo smilix"

DEPEND=">=x11-libs/qt-3.3.3
	xplo? ( =x11-plugins/qrezix-xplo-9999 )
	smilix? ( =x11-plugins/qrezix-smilix-9999 )"

ESVN_REPO_URI="svn://skinwel/qrezix/trunk/"

src_compile() {
	econf || die "econf failed"
	emake || die "emake failed"
}

src_install() {
	make DESTDIR=${D} install || die
}
