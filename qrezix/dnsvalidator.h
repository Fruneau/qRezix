/* Fichier à supprimer */

#ifndef DNSVALIDATOR_H
#define DNSVALIDATOR_H

#include <qvalidator.h>

class DnsValidator : public QRegExpValidator
{
	Q_OBJECT
	
	public:
		DnsValidator( QWidget* parent = 0, const char *name = 0 );
		~DnsValidator();
};

#endif

