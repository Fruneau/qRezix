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
#include <qprocess.h>
#include <qstringlist.h>
#include "rzxcomputer.h"
#include "rzxserverlistener.h"
#include "rzxprotocole.h"
#include "rzxconfig.h"
#include "defaults.h"
#include "rzxchat.h"
#include "rzxproperty.h"
#include "rzxmessagebox.h"
#include "rzxpluginloader.h"

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
	delayScan = new QTimer();
	connect(delayScan, SIGNAL(timeout()), this, SLOT(scanServers()));
	options.ServerFlags = options.Server = 0;
}

RzxComputer::RzxComputer()
{
	delayScan = NULL;
}

RzxComputer::~RzxComputer()
{
	if(delayScan) delete delayScan;
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
#ifdef Q_OS_MACX
	options.SysEx = 5;
#else
	options.SysEx = 3;
#endif
#endif
}

QString RzxComputer::serialize(bool stamp) {
	QString ret;
	options_t test = options;
	test.Server &= test.ServerFlags;
	Q_UINT32 opts = *((Q_UINT32*) &test);
	Q_UINT32 vers = *((Q_UINT32*) &version);
	
	ret = name + " " +
		QString::number(opts, 16).right(8) + " " +				
		QString::number(vers, 16).right(8) + " ";
		
	if (stamp) ret += "0 ";
	
	ret += QString::number((Q_UINT32)flags, 16).right(8) + " " +
			remarque;
		
	return ret;
}

void RzxComputer::setName(const QString& newName) 
{
	name = newName;
}
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
{ return name; }
RzxComputer::options_t RzxComputer::getOptions() const 
{ return options; }
RzxComputer::version_t RzxComputer::getVersion() const 
{ return version; }
RzxHostAddress RzxComputer::getIP() const 
{ return ip; }
unsigned long RzxComputer::getFlags() const
{ return flags; }
QString RzxComputer::getRemarque() const 
{ return remarque; }
QPixmap RzxComputer::getIcon() const 
{ return icon; }
int RzxComputer::getServers() const
{ return options.Server; }
int RzxComputer::getServerFlags() const
{ return options.ServerFlags; }

///Retourne le client utilis� avec la version
/**Permet d'obtenir le nom et le num�ro de version du client xNet utilis�*/
QString RzxComputer::getClient() const
{
	QString client;
	switch(version.Client)
	{
		case 1: client = "Rezix"; break;
		case 2: client = "XNet"; break;
		case 3: client = "MacXNet"; break;
		case 4: client = "CPANet"; break;
		case 5: client = "CocoaXNet"; break;
		case 6: case 0x60: client = "qRezix"; break; //g�re le cas de la version erron�e
		case 7: client = "mxNet"; break;
		default : client = tr("Unknown"); break;
	}
	client += QString(" %1.%2.%3").arg(version.MajorVersion).arg(version.MinorVersion).arg(version.FunnyVersion);
	return client;
}



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
	int servers = 0;
#ifdef WIN32
  //Bon, c pas beau, mais si j'essaye de travailler sur le meme socket pour tous les test
	//j'arrive toujours � de faux r�sultats... c ptet un bug de windows (pour changer)
	QSocketDevice detectFTP, detectHTTP, detectNEWS, detectSMB;

	//scan du ftp
	if(!detectFTP.bind(ip, 21))
		servers |= RzxComputer::SERVER_FTP;

	//scan du http
	if(!detectHTTP.bind(ip, 80))
		servers |= RzxComputer::SERVER_HTTP;

	//scan du nntp
	if(!detectNEWS.bind(ip, 119))
		servers |= RzxComputer::SERVER_NEWS;

	//scan du samba
	if(!detectSMB.bind(ip, 139))
		servers |= RzxComputer::SERVER_SAMBA;
#else
#ifdef Q_OS_MACX
	servers = RzxComputer::SERVER_FTP | RzxComputer::SERVER_HTTP | RzxComputer::SERVER_NEWS | RzxComputer::SERVER_SAMBA;
#else
  QProcess *netstat;
  QStringList res;
  
  netstat = new QProcess();
  netstat->addArgument("netstat");
  netstat->addArgument("-ltn");

  //On ex�ctue netstat pour obtenir les infos sur les diff�rents ports utilis�s
  if(netstat->start())
  {
    while(netstat->isRunning());
    while(netstat->canReadLineStdout())
    {
      QString *q;
      q = new QString(netstat->readLineStdout());
      res += *q;
    }
    delete netstat;

    //lecture des diff�rents port pour voir s'il sont listen
    if(!(res.grep(":21 ")).isEmpty()) servers |= RzxComputer::SERVER_FTP;
    if(!(res.grep(":80 ")).isEmpty()) servers |= RzxComputer::SERVER_HTTP;
    if(!(res.grep(":119 ")).isEmpty()) servers |= RzxComputer::SERVER_NEWS;
    if(!(res.grep(":139 ")).isEmpty()) servers |= RzxComputer::SERVER_SAMBA;
  }
  //au cas o� netstat fail ou qu'il ne soit pas install�
  else
		servers = RzxComputer::SERVER_FTP | RzxComputer::SERVER_HTTP | RzxComputer::SERVER_NEWS | RzxComputer::SERVER_SAMBA;
#endif //MACX
#endif //WIN32
	int oldServers = getServers();
	
	if(servers != getServers())
	{
		setServers(servers);
		RzxProperty::serverUpdate();
	}

	if((servers & RzxComputer::SERVER_FTP) ^ (oldServers & RzxComputer::SERVER_FTP))
		RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_SERVERFTP, NULL);
	if((servers & RzxComputer::SERVER_HTTP) ^ (oldServers & RzxComputer::SERVER_HTTP))
		RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_SERVERHTTP, NULL);
	if((servers & RzxComputer::SERVER_NEWS) ^ (oldServers & RzxComputer::SERVER_NEWS))
		RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_SERVERNEWS, NULL);
	if((servers & RzxComputer::SERVER_SAMBA) ^ (oldServers & RzxComputer::SERVER_SAMBA))
		RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_SERVERSMB, NULL);

	delayScan->start(30000); //bon, le choix de 30s, c vraiment al�atoire
							//1 ou 2 minutes, �'aurait pas �t� mal, mais bon
}
