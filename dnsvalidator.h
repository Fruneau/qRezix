#ifndef DNSVALIDATOR_H
#define DNSVALIDATOR_H

#include <qvalidator.h>

class DnsValidator : public QValidator{
	Q_OBJECT
public:
    DnsValidator(QWidget* parent=0, const char *name=0);
    ~DnsValidator();
	
void fixup ( QString & ) const;
QValidator::State validate( QString & , int & ) const;

};

#endif

