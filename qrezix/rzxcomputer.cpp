/***************************************************************************
                          rzxcomputer.cpp  -  description
                             -------------------
    begin                : Thu Jan 24 2002
    copyright            : (C) 2002 by Sylvain Joyeux
    email                : sylvain.joyeux@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qapplication.h>
#include <qdir.h>
#include <qfileinfo.h>
#include "rzxcomputer.h"
#include "rzxserverlistener.h"
#include "rzxprotocole.h"
#include "rzxconfig.h"
#include "defaults.h"
#include "rzxchat.h"
#include "rzxproperty.h"
#include "rzxmessagebox.h"

const char *RzxComputer::promalText[4] = { "?", QT_TR_NOOP("Orange") , QT_TR_NOOP("Rouje"), QT_TR_NOOP("Jone") };


/** Creation de localhost */
void RzxComputer::initLocalHost( void )
{
	version.Client = RZX_CLIENT_ID;
	version.MajorVersion = RZX_MAJOR_VERSION;
	version.FunnyVersion = RZX_FUNNY_VERSION;
	version.MinorVersion = RZX_MINOR_VERSION;

	autoSetOs();

	ip = RzxHostAddress::fromRezix(0);
	detect = new QSocketDevice();
	delayScan = new QTimer();
	connect(delayScan, SIGNAL(timeout()), this, SLOT(scanServers()));
	options.ServerFlags = options.Server = 0;
}


/** Recuperation des parametres d'ordinateur a partir d'un message Xnet
Retourne true en cas d'echec, false sinon */
bool RzxComputer::parse(const QString& params){
	if (params.isEmpty()) return true;
		
	QStringList args = RzxProtocole::split(' ', params, RzxProtocole::ServerCounts[RzxProtocole::SERVER_JOIN]);
	if (args.count() != RzxProtocole::ServerCounts[RzxProtocole::SERVER_JOIN])
		return true;

	QString temp; bool ok; QStringList::Iterator it;
	unsigned long tempNumb;
	
	it = args.begin();	
	temp = *it;	
	ip = RzxHostAddress::fromRezix(temp.toULong(&ok, 16));
	if (!ok) return true;
	
	name = *(++it);
	if (name.isEmpty()) return true;

	temp = *(++it);	
	tempNumb = temp.toULong(&ok, 16);
	if (!ok) return true;
	*((unsigned long *) &options) = tempNumb;
	
	temp = *(++it);
	tempNumb = temp.toULong(&ok, 16);
	if (!ok) return true;
	*((unsigned long *) &version) = tempNumb;
	
	temp = *(++it);
	stamp = temp.toULong(&ok, 16);
	if (!ok) return true;
	
	temp = *(++it);
	flags = temp.toULong(&ok, 16);
	if (!ok) return true;
	
	// maintenant qu'on a le stamp et l'ip, on peut essayer de charger l'icone
	loadIcon();
	
	remarque = *(++it);
	return false;
}

void RzxComputer::autoSetOs() //0=Inconnu, 1=Win9X, 2=WinNT, 3=Linux, 4=MacOS, 5=MacOS X
{
#ifdef WIN32
	if (QApplication::winVersion() & Qt::WV_NT_based)
		options.SysEx = 2;
	else
		options.SysEx = 1;
#else
	options.SysEx = 3;
#endif
}

QString RzxComputer::serialize(bool stamp) {
	QString ret;
	options_t test = options;
	test.Server &= test.ServerFlags;
	unsigned long opts = *((unsigned long*) &test);
	unsigned long vers = *((unsigned long*) &version);
	
	ret = name + " " +
		QString::number(opts, 16) + " " +				
		QString::number(vers, 16) + " ";
		
	if (stamp) ret += "0 ";
	
	ret += QString::number(flags, 16) + " " +
			remarque;
		
	return ret;
}

void RzxComputer::setName(const QString& newName) 
{ name = newName; }
void RzxComputer::setRepondeur(bool i){
	options.Repondeur = i ? REP_ON : REP_ACCEPT;
}
void RzxComputer::setIcon(const QPixmap& image){
	icon = image;
	emit isUpdated();
}
void RzxComputer::setServers(int servers) 
{ options.Server = servers; }
void RzxComputer::setServerFlags(int serverFlags) 
{ options.ServerFlags = serverFlags; }
void RzxComputer::setPromo(int promo)
{ options.Promo = promo; }
void RzxComputer::setRemarque(const QString& text)
{ remarque = text; }



bool RzxComputer::getRepondeur() const 
{ return options.Repondeur; }

int RzxComputer::getPromo() const 
{ return(options.Promo); }

QString RzxComputer::getPromoText() const
{ return tr(promalText[options.Promo]); }

QString RzxComputer::getFilename() const 
{ return QString::number(stamp, 16) + "-" + QString::number(ip.ip4Addr(),16) + ".png"; }
QString RzxComputer::getName() const 
{ return name; };
RzxComputer::options_t RzxComputer::getOptions() const 
{ return options; };
RzxComputer::version_t RzxComputer::getVersion() const 
{ return version; };
RzxHostAddress RzxComputer::getIP() const 
{ return ip; };
unsigned long RzxComputer::getFlags() const
{ return flags; };
QString RzxComputer::getRemarque() const 
{ return remarque; };
QPixmap RzxComputer::getIcon() const 
{ return icon; }
int RzxComputer::getServers() const
{ return options.Server; }
int RzxComputer::getServerFlags() const
{ return options.ServerFlags; }




/** No descriptions */
void RzxComputer::loadIcon(){
	removePreviousIcons();
	QString file = getFilename();
	if (!icon.load(RzxConfig::computerIconsDir().absFilePath(file))) {
		icon = *(RzxConfig::osIcons(true)[(int)options.SysEx]);
		emit needIcon(getIP());
	}
}

void RzxComputer::removePreviousIcons(){
	QString newFile=getFilename();
	QDir d(RzxConfig::computerIconsDir());
	if(!d.exists(newFile)){
		d.setFilter(QDir::Files);
		d.setNameFilter("*-" + QString::number(ip.ip4Addr(),16) + ".png");
		
		const QFileInfoList *list = d.entryInfoList();
		QFileInfoListIterator it( *list );      // create list iterator
		while ( QFileInfo * fi = it()) {// for each file...
			QString name=fi->fileName();
			if(!(name==newFile)){
				QFile::remove(fi->absFilePath());
			}
		}
	}
}


//Scan des servers ouverts
void RzxComputer::scanServers()
{
	QSocketDevice detectFTP, detectHTTP, detectNEWS, detectSMB;
	if(!detect) return;
	int servers = 0;

	//scan du ftp
	if(!detectFTP.bind(ip, 21)) servers |= RzxComputer::SERVER_FTP;
//	else servers &= ~RzxComputer::SERVER_FTP;

	//scan du http
	if(!detectHTTP.bind(ip, 80)) servers |= RzxComputer::SERVER_HTTP;
//	else servers &= ~RzxComputer::SERVER_HTTP;

	//scan du nntp
	if(!detectNEWS.bind(ip, 119)) servers |= RzxComputer::SERVER_NEWS;
//	else servers &= ~RzxComputer::SERVER_NEWS;

	//scan du samba
	if(!detectSMB.bind(ip, 139)) servers |= RzxComputer::SERVER_SAMBA;
//	else servers &= ~RzxComputer::SERVER_SAMBA;

	if(servers != getServers())
	{
		setServers(servers);
		RzxProperty::serverUpdate();
	}
	delayScan->start(30000);
}
