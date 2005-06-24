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
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStringList>
#include <QRegExp>
#include <QPixmap>
#include <QVector>

#include "defaults.h"
#include "rzxcomputer.h"

#include "rzxserverlistener.h"
#include "rzxprotocole.h"
#include "rzxconfig.h"
#include "rzxchat.h"
#include "rzxproperty.h"
#include "rzxmessagebox.h"
#include "rzxpluginloader.h"

const char *RzxComputer::promalText[4] = { 
    "?", 
    QT_TR_NOOP("Orange") ,
    QT_TR_NOOP("Rouje"), 
    QT_TR_NOOP("Jone")
};


///Construction d'un RzxComputer
/** La construction n'initialise pas le RzxComputer en un objet utilisable.
 * Selon le cas on utilise initLocalHost our parse pour initialiser l'objet.
 */
RzxComputer::RzxComputer():delayScan(NULL) { }

///Destruction...
RzxComputer::~RzxComputer()
{
	if(delayScan) delete delayScan;
}


/********************** Initialisation d'un RzxComputer *****************/
///Création d'un Computer représentant localhost
/** L'objet créé est un objet global qui regroupe les informations importantes
 * concernant localhost. Cet objet n'est pas affiché dans le rzxrezal. La machine
 * qui représente localhost sur le resal est en fait la copie de cet objet transmise
 * par le réseau via le protocol xNet
 */
void RzxComputer::initLocalHost()
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

///Création d'un Computer à partir des données obtenue via le protocole xNet
/** Permet la représentation des hosts distants...
 */
bool RzxComputer::parse(const QString& params){
	if (params.isEmpty()) return true;
		
	QStringList args = RzxProtocole::split(' ', params, RzxProtocole::ServerCounts[RzxProtocole::SERVER_JOIN]);
	if (args.count() != (int)RzxProtocole::ServerCounts[RzxProtocole::SERVER_JOIN])
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

///Détermine le système d'exploitation du système local
/** Les différents OS supportés sont Windows 9X (1) ou NT (2), Linux (3), MacOS (4), MacOSX(5), BSD(6) */
void RzxComputer::autoSetOs()
{
#ifdef WIN32
	if (QApplication::winVersion() & Qt::WV_NT_based)
		options.SysEx = SYSEX_WINNT;
	else
		options.SysEx = SYSEX_WIN9X;
#endif
#ifdef Q_OS_UNIX
	options.SysEx = SYSEX_LINUX;
#endif
#ifdef Q_OS_BSD
        options.SysEx = SYSEX_BSD;
#endif
#ifdef Q_OS_MAC
	options.SysEx = SYSEX_MACOSX;
#endif
}

///Serialise le RzxComputer dans le format utilisé par le protocole xNet
QString RzxComputer::serialize(bool stamp)
{
	QString ret;
	options_t test = options;
	test.Server &= ServerFlags;
	//Pour préserver la compatibilité avec les versions antérieures qui ne respectent pas le protocole !!!
	if(test.Repondeur == REP_REFUSE) test.Repondeur = REP_ON;
	quint32 opts = *((Q_UINT32*) &test);
	quint32 vers = *((Q_UINT32*) &version);
	
	ret = name + " " +
		QString::number(opts, 16).right(8) + " " +				
		QString::number(vers, 16).right(8) + " ";
		
	if (stamp) ret += "0 ";
	
	ret += QString::number((Q_UINT32)flags, 16).right(8) + " " +
			remarque;
		
	return ret;
}

/******************** Remplissage du RzxComputer *********************/
///Défition du nom de machine
void RzxComputer::setName(const QString& newName) 
{ name = newName; }
///Définition de l'état du répondeur
void RzxComputer::setRepondeur(bool i)
{ options.Repondeur = i ? (RzxConfig::refuseWhenAway() ? REP_REFUSE :REP_ON) : REP_ACCEPT; }
///Définition de l'icône
void RzxComputer::setIcon(const QPixmap& image){
	icon = image;
	emit isUpdated();
}
///Définition de la liste des serveurs présents sur la machine
void RzxComputer::setServers(int servers) 
{ options.Server = servers; }
///Définition de la liste des serveurs envisageables sur la machine (localhost uniquement)
void RzxComputer::setServerFlags(int serverFlags) 
{ ServerFlags = serverFlags; }
///Définition de la promo
void RzxComputer::setPromo(int promo)
{ options.Promo = promo; }
///Définition du commentaire
void RzxComputer::setRemarque(const QString& text)
{ remarque = text; }


/************ Récupération des informations concernant le RzxComputer ***********/
///Récupération du nom de la machine
QString RzxComputer::getName() const 
{ return name; }
///Récupération du commentaire
QString RzxComputer::getRemarque() const 
{ return remarque; }
///Récupération de l'état du répondeur
int RzxComputer::getRepondeur() const 
{ return options.Repondeur; }

///Récupération de la promo
int RzxComputer::getPromo() const 
{ return(options.Promo); }
///Récupération du texte décrivant la promo
QString RzxComputer::getPromoText() const
{ return tr(promalText[options.Promo]); }

///Récupération de l'icône
QPixmap RzxComputer::getIcon() const 
{ return icon; }
///Récupération du nom du fichier contenant l'icône
QString RzxComputer::getFilename() const 
{ return QString::number(stamp, 16) + ".png"; }

///Récupération des options (OS, Servers, Promo, Répondeur)
RzxComputer::options_t RzxComputer::getOptions() const 
{ return options; }

///Récupération de flags (euh ça sert à quoi ???)
unsigned long RzxComputer::getFlags() const
{ return flags; }
///Récupération de la liste des servers présents sur la machine (ou demandés par l'utilisateur dans le cas de localhost)
int RzxComputer::getServers() const
{ return options.Server; }
///Récupération de la liste des serveurs présents (localhost)
int RzxComputer::getServerFlags() const
{ return ServerFlags; }

///Teste de la présence d'une possibilité
/** Retourne true si la machine possède la capacité demandée,
 * ou si elle ne supporte pas l'extension Capabilities du protocole xNet */
bool RzxComputer::can(unsigned int cap)
{
	if(cap == CAP_ON) return options.Capabilities & CAP_ON;

	if(!(options.Capabilities & CAP_ON)) return true;
	else return (options.Capabilities & cap);
}

///Récupération de la version du client
RzxComputer::version_t RzxComputer::getVersion() const 
{ return version; }
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

///Récupération de l'IP sous la forme d'un RzxHostAddress
RzxHostAddress RzxComputer::getIP() const 
{ return ip; }
///Permet de retrouver le 'nom' du sous-réseau sur lequel se trouve la machine
/** Permet de donner un nom au sous-réseau de la machine. A terme cette fonction lira les données à partir d'un fichier qui contiendra les correspondances */
QString RzxComputer::getResal(bool shortname) const
{
	QString m_ip = ip.toString();
	QRegExp mask("(\\d{1,3}\\.\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})");
	
	if(mask.search(m_ip) == -1) return tr("Unknown");
	
	//Si l'ip n'est pas de l'X
	if(mask.cap(1) != "129.104") return tr("World");
	
	int resal = mask.cap(2).toUInt();
	int adr = mask.cap(3).toUInt();
	//Kes et binet... avec un cas particulier pour le BR :))
	if(resal == 201)
	{
		if(adr >= 50 && adr <= 62) return "BR";
		return (shortname?"Binets":"Binets & Kès");
	}
	
	//Cas des bar d'étages
	//Comment ça y'a pas de bar au premier étage ? faudra dire ça à NC :þ
	if(resal == 208 || resal == 212 || resal == 218 || resal == 222)
	{
		if(adr >= 97 && adr <= 100) resal -= 2;
		if(adr >= 101 && adr <= 104) resal--;
	}

	//Pour prendre en compte les nouveaux caserts...
	//En espérant tout de même qu'un jour ils porteronts des vrais nom :(
	if(resal >= 224 && resal <= 229)
	{
		int greaterMask = (resal<<1) + (adr>>7);
		switch(greaterMask)
		{
			case (224<<1)+0: return "B71";
			case (224<<1)+1: return "B70";
			case (225<<1)+0: return "B73";
			case (225<<1)+1: return "B74";
			case (226<<1)+0: return "B75";
			case (226<<1)+1: return "B80";
			case (227<<1)+0: return "B76";
			case (227<<1)+1: return "B77";
			case (228<<1)+0: return "B78";
			case (228<<1)+1: return "B72";
			case (229<<1)+0: return "B79";
		}
	}

	//Pour le wifi
	if(resal == 230) return "Wifi";
	
	//Distribution (stribution) des bâtiments en fonction du sous-réseau
	if(resal == 203 || resal == 204) return "BEM";
	if(resal >= 205 && resal <= 208) return (shortname?"Foch.":"Foch ") + QString::number(resal - 205);
	if(resal >= 209 && resal <= 212) return (shortname?"Fay.":"Fayolle ") + QString::number(resal - 209);
	if(resal == 214) return "PEM";
	if(resal >= 215 && resal <= 218) return (shortname?"Jof.":"Joffre ") + QString::number(resal - 215);
	if(resal >= 219 && resal <= 222) return (shortname?"Mau.":"Maunoury ") + QString::number(resal - 219);
	if(resal == 223) return (shortname?"B.411":"Bat. 411");
	
	//Si y'a rien, on sait au moins que c'est à l'X...
	return "X";
}

/** No descriptions */
void RzxComputer::loadIcon(){
	QString file = getFilename();
	if(!stamp || !icon.load(RzxConfig::computerIconsDir().absFilePath(file))) {
		icon = *(RzxConfig::osIcons(true)[(int)options.SysEx]);
		if(stamp) emit needIcon(getIP());
	}
}


/****************** Analyse de la machine ********************/
///Scan des servers ouverts
/** Rerchercher parmi les protocoles affichés par qRezix lesquels sont présents sur la machine */
void RzxComputer::scanServers()
{
	int servers = 0;
#ifdef WIN32
    //Bon, c pas beau, mais si j'essaye de travailler sur le meme socket pour tous les test
	//j'arrive toujours à de faux résultats... c ptet un bug de windows (pour changer)
	QTcpServer detecter;

	//scan du ftp
	if(!detecter.listen(ip, 21))
		servers |= RzxComputer::SERVER_FTP;

	//scan du http
	if(!detecter.listen(ip, 80))
		servers |= RzxComputer::SERVER_HTTP;

	//scan du nntp
	if(!detecter.listen(ip, 119))
		servers |= RzxComputer::SERVER_NEWS;

	//scan du samba
	if(!detecter.listen(ip, 445))
		servers |= RzxComputer::SERVER_SAMBA;
#else
	QProcess netstat;
	QStringList res;
     
	#if defined(Q_OS_MAC) || defined(Q_OS_BSD)
        res << "-anf" << "inet";
	#else
		res << "-ltn";
	#endif

	//On exécute netstat pour obtenir les infos sur les différents ports utilisés
	//Seul problème c'est que la syntaxe de netstat n'est pas figée :
	// Sous linux : netstat -ltn | grep ':port '
	// Sous BSD : netstat -anf inet | grep LISTEN | grep '.port ' 
	netstat.start("netstat", res);
	if(netstat.waitForFinished(1000))
	{
		res = QStringList::split('\n', netstat.readAllStandardOutput());
    	#if defined(Q_OS_MAC) || defined(Q_OS_BSD)
			res = res.grep("LISTEN");
			if(!(res.grep(QRegExp("\\.21\\s")).isEmpty())) servers |= RzxComputer::SERVER_FTP;
			if(!(res.grep(QRegExp("\\.80\\s")).isEmpty())) servers |= RzxComputer::SERVER_HTTP;
			if(!(res.grep(QRegExp("\\.119\\s")).isEmpty())) servers |= RzxComputer::SERVER_NEWS;
			if(!(res.grep(QRegExp("\\.445\\s")).isEmpty())) servers |= RzxComputer::SERVER_SAMBA;
		#else
			//lecture des différents port pour voir s'il sont listen
			if(!(res.grep(":21 ")).isEmpty()) servers |= RzxComputer::SERVER_FTP;
			if(!(res.grep(":80 ")).isEmpty()) servers |= RzxComputer::SERVER_HTTP;
			if(!(res.grep(":119 ")).isEmpty()) servers |= RzxComputer::SERVER_NEWS;
			if(!(res.grep(":445 ")).isEmpty()) servers |= RzxComputer::SERVER_SAMBA;
		#endif
	}
	
	//au cas où netstat fail ou qu'il ne soit pas installé
	else
	{
        qDebug("Netstat didn't succeed : " + netstat.errorString());
		servers = RzxComputer::SERVER_FTP | RzxComputer::SERVER_HTTP | RzxComputer::SERVER_NEWS | RzxComputer::SERVER_SAMBA;
    }

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
