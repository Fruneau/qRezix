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
#include <qregexp.h>

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
	
	options.Capabilities = 3; //capable d'utiliser Capabilities et le chat
	options.Capabilities |= RzxPlugInLoader::global()->getFeatures();

	autoSetOs();

	ip = RzxHostAddress::fromRezix(0);
	delayScan = new QTimer();
	connect(delayScan, SIGNAL(timeout()), this, SLOT(scanServers()));
	ServerFlags = options.Server = 0;
	flags = 0;
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
	test.Server &= ServerFlags;
	//Pour préserver la compatibilité avec les versions antérieures qui ne respectent pas le protocole !!!
	if(test.Repondeur == REP_REFUSE) test.Repondeur = REP_ON;
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
	options.Repondeur = i ? (RzxConfig::refuseWhenAway() ? REP_REFUSE :REP_ON) : REP_ACCEPT;
}
void RzxComputer::setIcon(const QPixmap& image){
	icon = image;
	emit isUpdated();
}
void RzxComputer::setServers(int servers) 
{ options.Server = servers; }
void RzxComputer::setServerFlags(int serverFlags) 
{ ServerFlags = serverFlags; }
void RzxComputer::setPromo(int promo)
{ options.Promo = promo; }
void RzxComputer::setRemarque(const QString& text)
{ remarque = text; }



int RzxComputer::getRepondeur() const 
{ return options.Repondeur; }

int RzxComputer::getPromo() const 
{ return(options.Promo); }

QString RzxComputer::getPromoText() const
{ return tr(promalText[options.Promo]); }

QString RzxComputer::getFilename() const 
{ return QString::number(stamp, 16) + ".png"; }
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
{ return ServerFlags; }
bool RzxComputer::can(unsigned int cap)
{
	if(!(options.Capabilities & CAP_ON)) return true;
	else return (options.Capabilities & cap);
}

///Retourne le client utilisé avec la version
/**Permet d'obtenir le nom et le numéro de version du client xNet utilisé*/
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
		case 6: case 0x60: client = "qRezix"; break; //gère le cas de la version erronée
		case 7: client = "mxNet"; break;
		case 8: client = "Rezix.NET"; break;
		default : client = tr("Unknown"); break;
	}
	client += QString(" %1.%2.%3").arg(version.MajorVersion).arg(version.MinorVersion).arg(version.FunnyVersion);
	return client;
}

///Permet de retrouver le 'nom' du sous-réseau sur lequel se trouve la machine
/** Permet de donner un nom au sous-réseau de la machine. A terme cette fonction lira les données à partir d'un fichier qui contiendra les correspondances */
QString RzxComputer::getResal(bool shortname) const
{
	QString m_ip = ip.toString();
	QRegExp mask("(\\d{1,3}\\.\\d{1,3})\\.(\\d{1,3})\\.\\d{1,3}");
	
	if(mask.search(m_ip) == -1) return tr("Unknown");
	
	//Si l'ip n'est pas de l'X
	if(mask.cap(1) != "129.104") return tr("World");
	
	int resal = mask.cap(2).toUInt();
	if(resal == 201) return (shortname?"Binets":"Binets et Kès");
	if(resal == 203 || resal == 204) return "BEM";
	if(resal >= 205 && resal <= 208) return (shortname?"Foch":"Foch ") + QString::number(resal - 205);
	if(resal >= 209 && resal <= 212) return (shortname?"Fay.":"Fayolle ") + QString::number(resal - 209);
	if(resal == 214) return "PEM";
	if(resal >= 215 && resal <= 218) return (shortname?"Jof.":"Joffre ") + QString::number(resal - 215);
	if(resal >= 219 && resal <= 222) return (shortname?"Mau.":"Maunoury ") + QString::number(resal - 219);
	if(resal == 223) return (shortname?"B.411":"Bat. 411");
	return "X";
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
	//j'arrive toujours à de faux résultats... c ptet un bug de windows (pour changer)
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
	QProcess *netstat;
	QStringList res;
  
	netstat = new QProcess();
	netstat->addArgument("netstat");
  
	#ifdef Q_OS_MACX
		netstat->addArgument("-f inet");
		netstat->addArgument("-an");
	#else
		netstat->addArgument("-ltn");
	#endif

	//On exéctue netstat pour obtenir les infos sur les différents ports utilisés
	if(netstat->start())
	{
		while(netstat->isRunning());
		while(netstat->canReadLineStdout())
			res += netstat->readLineStdout();
		delete netstat;

		#ifdef Q_OS_MACX
			res = res.grep("LISTEN");
			if(!(res.grep(".21 ")).isEmpty()) servers |= RzxComputer::SERVER_FTP;
			if(!(res.grep(".80 ")).isEmpty()) servers |= RzxComputer::SERVER_HTTP;
			if(!(res.grep(".119 ")).isEmpty()) servers |= RzxComputer::SERVER_NEWS;
			if(!(res.grep(".139 ")).isEmpty()) servers |= RzxComputer::SERVER_SAMBA;
		#else
			//lecture des différents port pour voir s'il sont listen
			if(!(res.grep(":21 ")).isEmpty()) servers |= RzxComputer::SERVER_FTP;
			if(!(res.grep(":80 ")).isEmpty()) servers |= RzxComputer::SERVER_HTTP;
			if(!(res.grep(":119 ")).isEmpty()) servers |= RzxComputer::SERVER_NEWS;
			if(!(res.grep(":139 ")).isEmpty()) servers |= RzxComputer::SERVER_SAMBA;
		#endif
	}
	//au cas où netstat fail ou qu'il ne soit pas installé
	else
		servers = RzxComputer::SERVER_FTP | RzxComputer::SERVER_HTTP | RzxComputer::SERVER_NEWS | RzxComputer::SERVER_SAMBA;
		
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

	delayScan->start(30000); //bon, le choix de 30s, c vraiment aléatoire
							//1 ou 2 minutes, ç'aurait pas été mal, mais bon
}
