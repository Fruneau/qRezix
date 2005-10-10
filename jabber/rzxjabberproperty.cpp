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
		subsub = new Tag ( sub, "USERID", email.toStdString());
	}
	if(!phone.isEmpty()){
		sub = new Tag( tag , "TEL");
		subsub = new Tag( sub, "HOME");
		subsub = new Tag( sub, "VOICE");
		subsub = new Tag ( sub, "NUMBER", phone.toStdString());
	}
	if(!website.isEmpty())
		sub = new Tag(tag, "URL", website.toStdString());
	if(!nick.isEmpty())
		sub = new Tag(tag, "NICKNAME", nick.toStdString());
	if(!name.isEmpty())
		sub = new Tag(tag, "FN", name.toStdString());
	if(!birthday.isEmpty())
		sub = new Tag(tag, "BDAY", birthday.toStdString());
	if(!description.isEmpty())
		sub = new Tag(tag, "DESC", description.toStdString());
	if(! ( street.isEmpty() && postCode.isEmpty() && city.isEmpty() && region.isEmpty() && country.isEmpty() ) ){
		sub = new Tag( tag , "ADDRESS");
		if(!street.isEmpty()) subsub = new Tag ( sub, "STREET", street.toStdString());
		if(!postCode.isEmpty()) subsub = new Tag ( sub, "PCODE", postCode.toStdString());
		if(!city.isEmpty()) subsub = new Tag ( sub, "LOCALITY", city.toStdString());
		if(!region.isEmpty()) subsub = new Tag ( sub, "REGION", region.toStdString());
		if(!country.isEmpty()) subsub = new Tag ( sub, "CTRY", country.toStdString());
	}
	if(!orgName.isEmpty()||!orgUnit.isEmpty()){
		sub = new Tag( tag , "ORG");
		if(!orgName.isEmpty()) subsub = new Tag ( sub, "ORGNAME", orgName.toStdString());
		if(!orgUnit.isEmpty()) subsub = new Tag ( sub, "ORGUNIT", orgUnit.toStdString());
	}
	return top;
}
