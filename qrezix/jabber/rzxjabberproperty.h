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

using namespace gloox;
class RzxJabberComputer;
/**
@author Guillaume Porcher
*/
class RzxJabberProperty : public QObject, public IqHandler
{
	Q_OBJECT
public:
	RzxJabberProperty(RzxJabberComputer*);
	RzxJabberComputer* computer;
	~RzxJabberProperty();
	virtual bool handleIq (Stanza *stanza);
	virtual bool handleIqID (Stanza *stanza, int context);
	QString email, phone, website, nick, name, birthday, description, address, organisation;
	QString toMsg();
	Tag * toIq();

signals:
	void receivedProperties(RzxJabberComputer*);
};

#endif
