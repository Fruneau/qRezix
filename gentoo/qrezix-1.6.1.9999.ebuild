# Copyright 1999-2005 Binet R�seau
# Distributed under the terms of the GNU General Public License v2
# $Header$

inherit kde-functions cvs
need-qt 3.3

DESCRIPTION="qRezix : a xNet client made in Qt"
HOMEPAGE="http://frankiz/"

# Point to any required sources; these will be automatically downloaded by
# Portage.
LICENSE="GPL-2"
SLOT="0"

KEYWORDS="x86"
IUSE=""

DEPEND=">=x11-libs/qt-3.3.3"

S=${WORKDIR}/${PN}

ECVS_SERVER="gwennoz:/home/anoncvs/"
ECVS_MODULE="qrezix"
ECVS_BRANCH="qrezix-tcp"
ECVS_USER="anoncvs"
ECVS_PASS=""

src_compile() {
	econf || die "econf failed"
	emake || die "emake failed"
}

src_install() {
	make DESTDIR=${D} install || die
}
