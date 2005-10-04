/***************************************************************************
                          rzxjabberproperty  -  description
                             -------------------
    begin                : jeu sep 29 2005
    copyright            : (C) 2005 by Guillaume Porcher
    email                : pico@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RZXJABBERPROPERTY_H
#define RZXJABBERPROPERTY_H

#include <gloox/stanza.h>
#include <gloox/iqhandler.h>
#include <qobject.h>

#include <RzxComputer>

using namespace gloox;
/**
@author Guillaume Porcher
*/
class RzxJabberProperty : public QObject, public IqHandler
{
	Q_OBJECT
public:
	RzxJabberProperty(RzxComputer* );
	RzxComputer* computer;
	~RzxJabberProperty();
	virtual bool handleIq (Stanza *stanza);
	virtual bool handleIqID (Stanza *stanza, int context);
signals:
	void receivedProperties(QString props, RzxJabberProperty*);
};

#endif
