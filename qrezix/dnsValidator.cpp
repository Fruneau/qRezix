#include "dnsvalidator.h"
#include <qwidget.h>
#include "rzxconfig.h"
#include "rzxcomputer.h"

DnsValidator::DnsValidator(QWidget* parent, const char *name) : QValidator(parent, name){
}

DnsValidator::~DnsValidator(){
}

QValidator::State DnsValidator::validate(QString &input, int&) const{
	if(input.find(" ")+1) return QValidator::Intermediate;
	else return QValidator::Acceptable;
}

void DnsValidator::fixup ( QString & input ) const{
	input.replace(QRegExp(" "),"");
}
