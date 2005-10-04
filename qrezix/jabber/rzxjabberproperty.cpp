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
#include "rzxjabberproperty.h"
#include <QString>
#include <QtDebug>
using namespace gloox;

RzxJabberProperty::RzxJabberProperty(RzxComputer* c)
{
	computer = c;
}


RzxJabberProperty::~RzxJabberProperty()
{
}

bool RzxJabberProperty::handleIq (Stanza *stanza){};

bool RzxJabberProperty::handleIqID (Stanza *stanza, int context){
	QString props;
	Tag *q = stanza->findChild( "vCard" );
	Tag::TagList l = q->children();
	Tag::TagList::const_iterator it;
	for(it= l.begin(); it != l.end(); it++)
	{
		Tag *tag = (*it);
		if(tag->name()=="EMAIL")
			props += tr("Email") + "|" + QString::fromStdString(tag->findChild("USERID")->cdata())  + "|";
		else if(tag->name()=="TEL")
			props += tr("Phone") + "|" + QString::fromStdString(tag->findChild("NUMBER")->cdata())  + "|";
		else if(tag->name()=="URL")
			props += tr("Website") + "|" + QString::fromStdString(tag->cdata())  + "|";
		else if(tag->name()=="NICKNAME")
			props += tr("Nickname") + "|" + QString::fromStdString(tag->cdata())  + "|";
		else if(tag->name()=="FN")
			props += tr("Name") + "|" + QString::fromStdString(tag->cdata())  + "|";
		else if(tag->name()=="BDAY")
			props += tr("Birthday") + "|" + QString::fromStdString(tag->cdata())  + "|";
		else if(tag->name()=="DESC")
			props += tr("Description") + "|" + QString::fromStdString(tag->cdata())  + "|";
		else if(tag->name()=="ADR")
			props += tr("Address") + "|" + QString::fromStdString(tag->findChild("STREET")->cdata()) + "\n" + QString::fromStdString(tag->findChild("PCODE")->cdata()) + " " + QString::fromStdString(tag->findChild("LOCALITY")->cdata()) + "\n" + QString::fromStdString(tag->findChild("REGION")->cdata()) + "\n" +  QString::fromStdString(tag->findChild("CTRY")->cdata()) + "|";
		else if(tag->name()=="ORG")
			props += tr("Organisation") + "|" + QString::fromStdString(tag->findChild("ORGNAME")->cdata()) + " - " + QString::fromStdString(tag->findChild("ORGUNIT")->cdata()) + "|";
		else
			props += QString::fromStdString(tag->name()) + "|" + QString::fromStdString(tag->cdata())  + "|";
	}
	props.chop(1);
	emit receivedProperties(props,this);
};
