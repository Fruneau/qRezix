/* Ce fichier doit être supprimé dans un futur proche, car il n'apporte rien */
#include <qregexp.h>
#include <qwidget.h>

#include "dnsvalidator.h"

DnsValidator::DnsValidator(QWidget* parent, const char *name)
	:QRegExpValidator(parent, name)
{
	setRegExp(QRegExp("[a-zA-Z0-9\-]{1,63}"));
}

DnsValidator::~DnsValidator(){
}
