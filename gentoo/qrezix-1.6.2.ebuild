# Copyright 1999-2005 Binet R�seau
# Distributed under the terms of the GNU General Public License v2
# $Header$

inherit kde-functions
need-qt 3.3

DESCRIPTION="qRezix : a xNet client made in Qt"
HOMEPAGE="http://frankiz/"

# Point to any required sources; these will be automatically downloaded by
# Portage.
SRC_URI="ftp://gwennoz/xshare/linux/reseau/qrezix/${P}.tar.gz"
LICENSE="GPL-2"
SLOT="0"

KEYWORDS="-*"
IUSE=""

DEPEND=">=x11-libs/qt-3.3.3"

S=${WORKDIR}/${PN}

src_compile() {
	econf || die "econf failed"
	emake || die "emake failed"
}

src_install() {
	make DESTDIR=${D} install || die
}
