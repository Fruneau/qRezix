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
	Tag::TagList l = q->children();
	Tag::TagList::const_iterator it;
	for(it= l.begin(); it != l.end(); it++)
	{
		Tag *tag = (*it);
		if(tag->name()=="EMAIL")
			email = QString::fromStdString(tag->findChild("USERID")->cdata());
		else if(tag->name()=="TEL")
			phone = QString::fromStdString(tag->findChild("NUMBER")->cdata());
		else if(tag->name()=="URL")
			website = QString::fromStdString(tag->cdata());
		else if(tag->name()=="NICKNAME")
			nick = QString::fromStdString(tag->cdata());
		else if(tag->name()=="FN")
			name = QString::fromStdString(tag->cdata());
		else if(tag->name()=="BDAY")
			birthday = QString::fromStdString(tag->cdata());
		else if(tag->name()=="DESC")
			description = QString::fromStdString(tag->cdata());
		else if(tag->name()=="ADR"){
			street = QString::fromStdString(tag->findChild("STREET")->cdata());
			postCode = QString::fromStdString(tag->findChild("PCODE")->cdata());
			city = QString::fromStdString(tag->findChild("LOCALITY")->cdata());
			region = QString::fromStdString(tag->findChild("REGION")->cdata());
			country = QString::fromStdString(tag->findChild("CTRY")->cdata());
		}else if(tag->name()=="ORG"){
			orgName = QString::fromStdString(tag->findChild("ORGNAME")->cdata());
			orgUnit = QString::fromStdString(tag->findChild("ORGUNIT")->cdata());
		}
	}
	emit receivedProperties(computer);
	return true;
};

QString RzxJabberProperty::toMsg(){
	QString props;
	if(!email.isNull())
		props += tr("Email") + "|" + email  + "|";
	if(!phone.isNull())
		props += tr("Phone") + "|" + phone + "|";
	if(!website.isNull())
		props += tr("Website") + "|" + website + "|";
	if(!nick.isNull())
		props += tr("Nickname") + "|" + nick  + "|";
	if(!name.isNull())
		props += tr("Name") + "|" + name + "|";
	if(!birthday.isNull())
		props += tr("Birthday") + "|" + birthday + "|";
	if(description.isNull())
		props += tr("Description") + "|" + description + "|";
	if(! ( street.isNull() && postCode.isNull() && city.isNull() && region.isNull() && country.isNull() ) ){
		props += tr("Address") + "|";
		if(!street.isNull()) props += street + "\n";
		if(!postCode.isNull()) props += postCode + " ";
		if(!city.isNull()) props += city + "\n";
		if(!region.isNull()) props += region + "\n";
		if(!country.isNull()) props += country;
		props += "|";
	}
	if(!orgName.isNull()||!orgUnit.isNull()){
		props += tr("Organisation") + "|";
		if(!orgName.isNull()) props += orgName + " ";
		if(!orgUnit.isNull()) props += orgUnit;
		props += "|";
	}
	props.chop(1);
	return props;
}

Tag * RzxJabberProperty::toIq(){
	Tag *tag,*sub,*subsub;
	tag = new Tag( "iq" );
	tag->addAttrib( "type", "set" );
	sub = new Tag( tag, "vcard" );
	sub->addAttrib( "xmlns", "vcard-temp" );
	if(email.isNull()){
		sub = new Tag( tag , "EMAIL");
		subsub = new Tag( sub, "INTERNET");
		subsub = new Tag ( sub, "USERID", email.toStdString());
	}
	if(phone.isNull()){
		sub = new Tag( tag , "TEL");
		subsub = new Tag( sub, "HOME");
		subsub = new Tag( sub, "VOICE");
		subsub = new Tag ( sub, "NUMBER", phone.toStdString());
	}
	if(website.isNull())
		sub = new Tag(tag, "URL", website.toStdString());
	if(nick.isNull())
		sub = new Tag(tag, "NICKNAME", nick.toStdString());
	if(name.isNull())
		sub = new Tag(tag, "FN", name.toStdString());
	if(birthday.isNull())
		sub = new Tag(tag, "BDAY", birthday.toStdString());
	if(description.isNull())
		sub = new Tag(tag, "DESC", description.toStdString());
	if(! ( street.isNull() && postCode.isNull() && city.isNull() && region.isNull() && country.isNull() ) ){
		sub = new Tag( tag , "ADDRESS");
		if(!street.isNull()) subsub = new Tag ( sub, "STREET", street.toStdString());
		if(!postCode.isNull()) subsub = new Tag ( sub, "PCODE", postCode.toStdString());
		if(!city.isNull()) subsub = new Tag ( sub, "LOCALITY", city.toStdString());
		if(!region.isNull()) subsub = new Tag ( sub, "REGION", region.toStdString());
		if(!country.isNull()) subsub = new Tag ( sub, "CTRY", country.toStdString());
	}
	if(!orgName.isNull()||!orgUnit.isNull()){
		sub = new Tag( tag , "ORG");
		if(!orgName.isNull()) subsub = new Tag ( sub, "ORGNAME", orgName.toStdString());
		if(!orgUnit.isNull()) subsub = new Tag ( sub, "ORGUNIT", orgUnit.toStdString());
	}
	return tag;
}
