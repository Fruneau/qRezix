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
#include "rzxjabbercomputer.h"
#include <QString>
#include <QtDebug>
using namespace gloox;

RzxJabberProperty::RzxJabberProperty(RzxJabberComputer* c)
{
	computer = c;
}


RzxJabberProperty::~RzxJabberProperty()
{
}

bool RzxJabberProperty::handleIq (Stanza *stanza){return false;};

bool RzxJabberProperty::handleIqID (Stanza *stanza, int context){
	Tag *q = stanza->findChild( "vCard" );
	if(!q)
		return false;
	Tag::TagList l = q->children();
	Tag::TagList::const_iterator it;
	for(it= l.begin(); it != l.end(); it++)
	{
		Tag *tag = (*it);
		if(tag->name()=="EMAIL")
			email = QString::fromUtf8(tag->findChild("USERID")->cdata().data());
		else if(tag->name()=="TEL")
			phone = QString::fromUtf8(tag->findChild("NUMBER")->cdata().data());
		else if(tag->name()=="URL")
			website = QString::fromUtf8(tag->cdata().data());
		else if(tag->name()=="NICKNAME")
			nick = QString::fromUtf8(tag->cdata().data());
		else if(tag->name()=="FN")
			name = QString::fromUtf8(tag->cdata().data());
		else if(tag->name()=="BDAY")
			birthday = QString::fromUtf8(tag->cdata().data());
		else if(tag->name()=="DESC")
			description = QString::fromUtf8(tag->cdata().data());
		else if(tag->name()=="ADR"){
			street = QString::fromUtf8(tag->findChild("STREET")->cdata().data());
			postCode = QString::fromUtf8(tag->findChild("PCODE")->cdata().data());
			city = QString::fromUtf8(tag->findChild("LOCALITY")->cdata().data());
			region = QString::fromUtf8(tag->findChild("REGION")->cdata().data());
			country = QString::fromUtf8(tag->findChild("CTRY")->cdata().data());
		}else if(tag->name()=="ORG"){
			orgName = QString::fromUtf8(tag->findChild("ORGNAME")->cdata().data());
			orgUnit = QString::fromUtf8(tag->findChild("ORGUNIT")->cdata().data());
		}
	}
	emit receivedProperties(computer);
	return true;
};

QString RzxJabberProperty::toMsg(){
	QString props;
	if(!email.isEmpty())
		props += tr("Email") + "|" + email  + "|";
	if(!phone.isEmpty())
		props += tr("Phone") + "|" + phone + "|";
	if(!website.isEmpty())
		props += tr("Website") + "|" + website + "|";
	if(!nick.isEmpty())
		props += tr("Nickname") + "|" + nick  + "|";
	if(!name.isEmpty())
		props += tr("Name") + "|" + name + "|";
	if(!birthday.isEmpty())
		props += tr("Birthday") + "|" + birthday + "|";
	if(description.isEmpty())
		props += tr("Description") + "|" + description + "|";
	if(! ( street.isEmpty() && postCode.isEmpty() && city.isEmpty() && region.isEmpty() && country.isEmpty() ) ){
		props += tr("Address") + "|";
		if(!street.isEmpty()) props += street + "\n";
		if(!postCode.isEmpty()) props += postCode + " ";
		if(!city.isEmpty()) props += city + "\n";
		if(!region.isEmpty()) props += region + "\n";
		if(!country.isEmpty()) props += country;
		props += "|";
	}
	if(!orgName.isEmpty()||!orgUnit.isEmpty()){
		props += tr("Organisation") + "|";
		if(!orgName.isEmpty()) props += orgName + " ";
		if(!orgUnit.isEmpty()) props += orgUnit;
		props += "|";
	}
	props.chop(1);
	return props;
}

Tag * RzxJabberProperty::toIq(){
	Tag *top,*tag,*sub,*subsub;
	top = new Tag( "iq" );
	top->addAttrib( "type", "set" );
	tag = new Tag( top, "vcard" );
	tag->addAttrib( "xmlns", "vcard-temp" );
	if(!email.isEmpty()){
		sub = new Tag( tag , "EMAIL");
		subsub = new Tag( sub, "INTERNET");
		subsub = new Tag ( sub, "USERID", email.toUtf8().data());
	}
	if(!phone.isEmpty()){
		sub = new Tag( tag , "TEL");
		subsub = new Tag( sub, "HOME");
		subsub = new Tag( sub, "VOICE");
		subsub = new Tag ( sub, "NUMBER", phone.toUtf8().data());
	}
	if(!website.isEmpty())
		sub = new Tag(tag, "URL", website.toUtf8().data());
	if(!nick.isEmpty())
		sub = new Tag(tag, "NICKNAME", nick.toUtf8().data());
	if(!name.isEmpty())
		sub = new Tag(tag, "FN", name.toUtf8().data());
	if(!birthday.isEmpty())
		sub = new Tag(tag, "BDAY", birthday.toUtf8().data());
	if(!description.isEmpty())
		sub = new Tag(tag, "DESC", description.toUtf8().data());
	if(! ( street.isEmpty() && postCode.isEmpty() && city.isEmpty() && region.isEmpty() && country.isEmpty() ) ){
		sub = new Tag( tag , "ADDRESS");
		if(!street.isEmpty()) subsub = new Tag ( sub, "STREET", street.toUtf8().data());
		if(!postCode.isEmpty()) subsub = new Tag ( sub, "PCODE", postCode.toUtf8().data());
		if(!city.isEmpty()) subsub = new Tag ( sub, "LOCALITY", city.toUtf8().data());
		if(!region.isEmpty()) subsub = new Tag ( sub, "REGION", region.toUtf8().data());
		if(!country.isEmpty()) subsub = new Tag ( sub, "CTRY", country.toUtf8().data());
	}
	if(!orgName.isEmpty()||!orgUnit.isEmpty()){
		sub = new Tag( tag , "ORG");
		if(!orgName.isEmpty()) subsub = new Tag ( sub, "ORGNAME", orgName.toUtf8().data());
		if(!orgUnit.isEmpty()) subsub = new Tag ( sub, "ORGUNIT", orgUnit.toUtf8().data());
	}
	return top;
}
