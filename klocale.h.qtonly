// probleme avec am_edit: il travaille AVANT configure
// ce qui fait qu'on ne peut pas lui filer ses options
// via automake/autoconf
// solution: creer un klocale.h bidon qui annulle
// ses modifs

/*
#include <qobject.h>
#include <qstring.h>

inline QString tr2i18n(const char * msg, const char * desc=0) {
	return QObject::tr(msg, desc);
}*/

#define tr2i18n tr
