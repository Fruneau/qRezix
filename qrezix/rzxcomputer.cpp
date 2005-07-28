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
#include <QStringList>
#include <QPixmap>

//Pour l'analyse de localhost
#include <QRegExp>
#include <QProcess>
#include <QApplication>

#include "defaults.h"
#include "rzxcomputer.h"

#include "rzxconfig.h"
#include "rzxutilslauncher.h"
#include "rzxchatsocket.h"
#include "rzxconnectionlister.h"
#include "rzxiconcollection.h"

const char *RzxComputer::promalText[4] = { 
    "?", 
    QT_TR_NOOP("Orange") ,
    QT_TR_NOOP("Rouje"), 
    QT_TR_NOOP("Jone")
};

const char *RzxComputer::rezalText[Rzx::RZL_NUMBER][2] = {
	{ QT_TR_NOOP("World"), QT_TR_NOOP("World") },
	{ "Binets & Kès", "Binets" },
	{ "BR", "BR" },
	{ "BEM Bat. A", "BEM.A" },
	{ "BEM Bat. D", "BEM.D" },
	{ "Foch 0", "Foch.0" },
	{ "Foch 1", "Foch.1" },
	{ "Foch 2", "Foch.2" },
	{ "Foch 3", "Foch.3" },
	{ "Fayolle 0", "Fay.0" },
	{ "Fayolle 1", "Fay.1" },
	{ "Fayolle 2", "Fay.2" },
	{ "Fayolle 3", "Fay.3" },
	{ "PEM", "PEM" },
	{ "Joffre 0", "Jof.0" },
	{ "Joffre 1", "Jof.1" },
	{ "Joffre 2", "Jof.2" },
	{ "Joffre 3", "Jof.3" },
	{ "Maunoury 0", "Mau.0" },
	{ "Maunoury 1", "Mau.1" },
	{ "Maunoury 2", "Mau.2" },
	{ "Maunoury 3", "Mau.3" },
	{ "Batiment 411", "Bat.411" },
	{ "Batiment 70", "Bat.70" },
	{ "Batiment 71", "Bat.71" },
	{ "Batiment 72", "Bat.72" },
	{ "Batiment 73", "Bat.73" },
	{ "Batiment 74", "Bat.74" },
	{ "Batiment 75", "Bat.75" },
	{ "Batiment 76", "Bat.76" },
	{ "Batiment 77", "Bat.77" },
	{ "Batiment 78", "Bat.78" },
	{ "Batiment 79", "Bat.79" },
	{ "Batiment 80", "Bat.80" },
	{ "Wifi", "Wifi" },
	{ "X", "X" }
};

const char *RzxComputer::osText[7] = {
	QT_TR_NOOP("Unknown"),
	"Windows 9x/Me",
	"Windows NT/2k/XP",
	"Linux",
	"MacOS",
	"MacOS X",
	"BSD"
};

RzxComputer *RzxComputer::m_localhost = NULL;

///Construction d'un RzxComputer
/** La construction n'initialise pas le RzxComputer en un objet utilisable.
 */
RzxComputer::RzxComputer():delayScan(NULL) { }

///Consturuction d'un RzxCompuer
/** La construction initialise le RzxComputer à partir des données obtenues
 * Cette construction est suffisante uniquement pour un computer distant
 * et n'est pas adapté à la construction de localhost
 */
RzxComputer::RzxComputer(const RzxHostAddress& c_ip, const QString& c_name, quint32 c_options, quint32 c_version, quint32 c_stamp, quint32 c_flags, const QString& c_remarque)
	:m_name(c_name), m_ip(c_ip), m_flags(c_flags), m_stamp(c_stamp), m_remarque(c_remarque), delayScan(NULL)
{
	*((quint32 *) &m_options) = c_options;
	*((quint32 *) &m_version) = c_version;
	loadIcon();
	connected = false;
}	

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
void RzxComputer::initLocalhost()
{
	Rzx::beginModuleLoading("Local computer image");
	delayScan = new QTimer();
	connect(delayScan, SIGNAL(timeout()), this, SLOT(scanServers()));

	//Ip mise à jour par RzxServerListener
	m_ip = RzxHostAddress::fromRezix(0);

	setName(RzxConfig::dnsname());
	setRemarque(RzxConfig::remarque());
	setPromo(RzxConfig::promo());
	setState(RzxConfig::repondeur());
	setServerFlags(RzxConfig::servers());
	autoSetOs();

	m_version.Client = RZX_CLIENT_ID;
	m_version.MajorVersion = RZX_MAJOR_VERSION;
	m_version.FunnyVersion = RZX_FUNNY_VERSION;
	m_version.MinorVersion = RZX_MINOR_VERSION;
	
	m_options.Capabilities = Rzx::CAP_ON; //capable d'utiliser Capabilities et le chat

	m_flags = 0;
	connected = true;

	scanServers();
	Rzx::endModuleLoading("Local computer image");
}

///Mise à jour du RzxComputer lors de la réception de nouvelle infos
void RzxComputer::update(const QString& c_name, quint32 c_options, quint32 c_stamp, quint32 c_flags, const QString& c_remarque)
{
	Rzx::ConnectionState oldState = state();

	m_name = c_name;
	*((quint32 *) &m_options) = c_options;
	m_flags = c_flags;
	m_remarque = c_remarque;
	connected = true;

	if(m_stamp != c_stamp)
	{
		m_stamp = c_stamp;
		loadIcon();
	}

	if(oldState != state())
		emitStateChanged();
	emit update(this);
}

///Détermine le système d'exploitation du système local
/** Les différents OS supportés sont Windows 9X (1) ou NT (2), Linux (3), MacOS (4), MacOSX(5), BSD(6) */
void RzxComputer::autoSetOs()
{
#ifdef WIN32
	if (QApplication::winVersion() & Qt::WV_NT_based)
		m_options.SysEx = Rzx::SYSEX_WINNT;
	else
		m_options.SysEx = Rzx::SYSEX_WIN9X;
#endif
#ifdef Q_OS_UNIX
	m_options.SysEx = Rzx::SYSEX_LINUX;
#endif
#ifdef Q_OS_BSD
	m_options.SysEx = Rzx::SYSEX_BSD;
#endif
#ifdef Q_OS_MAC
	m_options.SysEx = Rzx::SYSEX_MACOSX;
#endif
}

///Retourne une chaîne de caractère représentant l'objet ajencée selon le pattern
/** Le pattern peut comporter les éléments suivant :
 * - $nn = nom
 * - $do = options en décimal
 * - $xo = options en hexadécimal
 * - $dv = version en décimal
 * - $xv = version en hexadécimal
 * - $di = icône en décimal
 * - $xi = icône en hexadécimal
 * - $ip = ip en héxadécimal
 * - $IP = ip en xxx.xxx.xxx.xxx
 * - $rem = remarque
 * - $df = flags en décimal
 * - $dh = flags en hexadécimal
 */
QString RzxComputer::serialize(const QString& pattern) const
{
	QString message(pattern);
	options_t test = m_options;
	test.Server &= m_serverFlags;

	//Pour préserver la compatibilité avec les versions antérieures qui ne respectent pas le protocole !!!
	if(test.Repondeur == REP_REFUSE) test.Repondeur = REP_ON;
	quint32 opts = *((quint32*) &test);
	quint32 vers = *((quint32*) &m_version);
	
	message.replace("$nn", m_name);
	message.replace("$do", QString::number(opts, 10))
		.replace("$xo", QString::number(opts, 16).right(8));
	message.replace("$dv", QString::number(vers, 10))
		.replace("$xv", QString::number(vers, 16).right(8));
	message.replace("$di", QString::number(m_stamp, 10))
		.replace("$xi", QString::number(m_stamp, 16).right(8));
	message.replace("$df", QString::number(m_flags, 10))
		.replace("$xf", QString::number(m_flags, 16).right(8));
	message.replace("$ip", QString::number((quint32)m_ip.toRezix(), 16).right(8))
		.replace("$IP", m_ip.toString());
	message.replace("$rem", m_remarque);
	return message;
}

/******************** Remplissage du RzxComputer *********************/
///Défition du nom de machine
void RzxComputer::setName(const QString& newName) 
{ m_name = newName; }
///Définition de l'état du répondeur
void RzxComputer::setState(Rzx::ConnectionState state)
{
	if(state == Rzx::STATE_DISCONNECTED)
	{
		connected = false;
		emitStateChanged();
		return;
	}
	connected = true;

	switch(state)
	{
		case Rzx::STATE_HERE:
			m_options.Repondeur = REP_ACCEPT;
			break;
		case Rzx::STATE_AWAY:
			m_options.Repondeur = REP_ON;
			break;
		case Rzx::STATE_REFUSE:
			m_options.Repondeur = REP_REFUSE;
			break;
		default: break;
	}
	emitStateChanged();
}
///Définitioin de l'état du répondeur
/** Fonction surchargée pour pouvoir définir un type on-off 
 *
 * Cette fonction n'a d'utilité que pour localhost
 */
void RzxComputer::setState(bool state)
{
	if(state)
	{
		if(RzxConfig::refuseWhenAway())
			setState(Rzx::STATE_REFUSE);
		else
			setState(Rzx::STATE_AWAY);
	}
	else
		setState(Rzx::STATE_HERE);
}
///Connexion de la machine
void RzxComputer::login()
{
	if(!connected)
	{
		connected = true;
		emitStateChanged();
	}
}
///Déconnexion
void RzxComputer::logout()
{
	if(connected)
	{
		connected = false;
		emitStateChanged();
	}
}
///Définition de l'icône
void RzxComputer::setIcon(const QPixmap& image){
	m_icon = image;
	emit update(this);
}
///Définition de la liste des serveurs présents sur la machine
void RzxComputer::setServers(QFlags<RzxComputer::ServerFlags> servers) 
{ m_options.Server = servers; }
///Définition de la liste des serveurs envisageables sur la machine (localhost uniquement)
void RzxComputer::setServerFlags(QFlags<RzxComputer::ServerFlags> serverFlags) 
{ m_serverFlags = serverFlags; }
///Définition de la promo
void RzxComputer::setPromo(Rzx::Promal promo)
{ m_options.Promo = promo; }
///Définition du commentaire
void RzxComputer::setRemarque(const QString& text)
{ m_remarque = text; }

///Définition de l'IP
/** Utile pour localhost uniquement */
void RzxComputer::setIP(const RzxHostAddress& address)
{ m_ip = address; }
///Ajout d'une feature à la machine
void RzxComputer::addCapabilities(int feature)
{ m_options.Capabilities |= feature; }


/************ Récupération des informations concernant le RzxComputer ***********/
///Récupération du nom de la machine
const QString &RzxComputer::name() const 
{ return m_name; }
///Récupération du commentaire
const QString &RzxComputer::remarque() const 
{ return m_remarque; }
///Récupération de l'état du répondeur
Rzx::ConnectionState RzxComputer::state() const
{
	if(!connected)
		return Rzx::STATE_DISCONNECTED;

	switch((Repondeur)m_options.Repondeur)
	{
		case REP_ACCEPT: return Rzx::STATE_HERE;
		case REP_ON: return Rzx::STATE_AWAY;
		case REP_REFUSE: return Rzx::STATE_REFUSE;
		default: return Rzx::STATE_DISCONNECTED;
	}
}

///Récupération de la promo
Rzx::Promal RzxComputer::promo() const 
{ return (Rzx::Promal)m_options.Promo; }
///Récupération du texte décrivant la promo
QString RzxComputer::promoText() const
{ return promalText[m_options.Promo]; }

///Récupération de l'icône
QPixmap RzxComputer::icon() const 
{ return m_icon; }
///Récupération du stamp de l'icône
quint32 RzxComputer::stamp() const
{ return m_stamp; }

///Récupération des options (OS, Servers, Promo, Répondeur)
RzxComputer::options_t RzxComputer::options() const 
{ return m_options; }

///Récupération de l'OS
Rzx::SysEx RzxComputer::sysEx() const
{ return (Rzx::SysEx)m_options.SysEx; }
///Récupération du nom de l'OS
QString RzxComputer::sysExText() const
{	return tr(osText[sysEx()]); }

///Récupération de flags (euh ça sert à quoi ???)
unsigned long RzxComputer::flags() const
{ return m_flags; }
///Récupération de la liste des servers présents sur la machine (ou demandés par l'utilisateur dans le cas de localhost)
QFlags<RzxComputer::ServerFlags> RzxComputer::servers() const
{ return toServerFlags(m_options.Server); }
///Récupération de la liste des serveurs présents (localhost)
QFlags<RzxComputer::ServerFlags> RzxComputer::serverFlags() const
{ return m_serverFlags; }
bool RzxComputer::hasSambaServer() const
{ return (m_options.Server & SERVER_SAMBA); }
///Indique si on a un serveur ftp
bool RzxComputer::hasFtpServer() const
{ return (m_options.Server & SERVER_FTP); }
///Indique si on a un serveur http
bool RzxComputer::hasHttpServer() const
{ return (m_options.Server & SERVER_HTTP); }
///Indique si on a un serveur news
bool RzxComputer::hasNewsServer() const
{ return (m_options.Server & SERVER_NEWS); }

///Teste de la présence d'une possibilité
/** Retourne true si la machine possède la capacité demandée,
 * ou si elle ne supporte pas l'extension \a cap du protocole xNet */
bool RzxComputer::can(Rzx::Capabilities cap) const
{
	if(cap == Rzx::CAP_ON) return m_options.Capabilities & Rzx::CAP_ON;

	if(!(m_options.Capabilities & Rzx::CAP_ON)) return true;
	else return (m_options.Capabilities & cap);
}

///Récupération de la version du client
RzxComputer::version_t RzxComputer::version() const
{ return m_version; }

///Retourne le client utilisé avec la version
/**Permet d'obtenir le nom et le numéro de version du client xNet utilisé*/
QString RzxComputer::client() const
{
	QString clientName;
	switch(m_version.Client)
	{
		case 1: clientName = "Rezix"; break;
		case 2: clientName = "XNet"; break;
		case 3: clientName = "MacXNet"; break;
		case 4: clientName = "CPANet"; break;
		case 5: clientName = "CocoaXNet"; break;
		case 6: case 0x60: clientName = "qRezix"; break; //gère le cas de la version erronée
		case 7: clientName = "mxNet"; break;
		case 8: clientName = "Rezix.NET"; break;
		default : clientName = tr("Unknown"); break;
	}
	clientName += QString(" %1.%2.%3").arg(m_version.MajorVersion).arg(m_version.MinorVersion).arg(m_version.FunnyVersion);
	return clientName;
}

///Récupération de l'IP sous la forme d'un RzxHostAddress
const RzxHostAddress &RzxComputer::ip() const
{ return m_ip; }
///Indique si on est sur la même passerelle que la personne de référence
bool RzxComputer::isSameGateway(const RzxComputer& ref) const
{ return rezalId() == ref.rezalId(); }
///Fonction surchargée par confort d'utilisation
/** Si \a computer est Null la comparaison est réalisée avec localhost */
bool RzxComputer::isSameGateway(RzxComputer *computer) const
{
	if(!computer)
		computer = localhost();
	return isSameGateway(*computer);
}
///Indique si l'objet indiqué est localhost
/** A priori le test sur le nom de machine est suffisant pour indentifier si un objet est le même que localhost */
bool RzxComputer::isLocalhost() const
{
	return name() == localhost()->name();
}

///Permet de retrouver le 'nom' du sous-réseau sur lequel se trouve la machine
/** Permet de donner un nom au sous-réseau de la machine. A terme cette fonction lira les données à partir d'un fichier qui contiendra les correspondances */
Rzx::RezalId RzxComputer::rezalId() const
{
	QRegExp mask("(\\d{1,3}\\.\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})");

	if(mask.indexIn(m_ip.toString()) == -1) return Rzx::RZL_WORLD;
	
	//Si l'ip n'est pas de l'X
	if(mask.cap(1) != "129.104") return Rzx::RZL_WORLD;
	
	int resal = mask.cap(2).toUInt();
	int adr = mask.cap(3).toUInt();
	//Kes et binet... avec un cas particulier pour le BR :))
	if(resal == 201)
	{
		if(adr >= 50 && adr <= 62) return Rzx::RZL_BR;
		return Rzx::RZL_BINETS;
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
			case (224<<1)+0: return Rzx::RZL_BAT71;
			case (224<<1)+1: return Rzx::RZL_BAT70;
			case (225<<1)+0: return Rzx::RZL_BAT73;
			case (225<<1)+1: return Rzx::RZL_BAT74;
			case (226<<1)+0: return Rzx::RZL_BAT75;
			case (226<<1)+1: return Rzx::RZL_BAT80;
			case (227<<1)+0: return Rzx::RZL_BAT76;
			case (227<<1)+1: return Rzx::RZL_BAT77;
			case (228<<1)+0: return Rzx::RZL_BAT78;
			case (228<<1)+1: return Rzx::RZL_BAT72;
			case (229<<1)+0: return Rzx::RZL_BAT79;
		}
	}

	switch(resal)
	{
		//Pour le BEM
		case 203: return Rzx::RZL_BEMA;
		case 204: return Rzx::RZL_BEMD;

		//Pour le PEM & BAT411
		case 214: return Rzx::RZL_PEM;
		case 223: return Rzx::RZL_BAT411;

		//Pour Foch, Fayolle, Joffre et Maunoury
		case 205: return Rzx::RZL_FOCH0;
		case 206: return Rzx::RZL_FOCH1;
		case 207: return Rzx::RZL_FOCH2;
		case 208: return Rzx::RZL_FOCH3;
		case 209: return Rzx::RZL_FAYOLLE0;
		case 210: return Rzx::RZL_FAYOLLE1;
		case 211: return Rzx::RZL_FAYOLLE2;
		case 212: return Rzx::RZL_FAYOLLE3;
		case 215: return Rzx::RZL_JOFFRE0;
		case 216: return Rzx::RZL_JOFFRE1;
		case 217: return Rzx::RZL_JOFFRE2;
		case 218: return Rzx::RZL_JOFFRE3;
		case 219: return Rzx::RZL_MAUNOURY0;
		case 220: return Rzx::RZL_MAUNOURY1;
		case 221: return Rzx::RZL_MAUNOURY2;
		case 222: return Rzx::RZL_MAUNOURY3;

		//Pour le wifi
		case 230: return Rzx::RZL_WIFI;

		//Si y'a rien, on sait au moins que c'est à l'X...
		default: return Rzx::RZL_X;
	}
}

/** No descriptions */
void RzxComputer::loadIcon()
{
	QPixmap temp = RzxIconCollection::global()->hashedIcon(m_stamp);
	if(temp.isNull())
	{
		if(m_stamp)
			emit needIcon(ip());
		setIcon(RzxIconCollection::global()->osPixmap(sysEx(), true));
	}
	else
		setIcon(temp);
}

///Génère un tooltip formaté correspondant à l'objet
/** Le tooltip généré selon les préférences exprimées par l'utilisateur, avec les informations qui constitue le RzxComputer :
 * 	- NOM
 * 	- Informations :
 * 		- serveurs
 * 		- promo
 * 		- ip/rezal
 * 		- client/modules
 * 	- Propriétés (dernière propriétés en cache pour ce client)
 */
QString RzxComputer::tooltipText() const
{
	int tooltipFlags = RzxConfig::tooltip();
	if(!(tooltipFlags & (int)RzxConfig::Enable) || tooltipFlags==(int)RzxConfig::Enable) return "";
	
	QString tooltip = "<b>"+ name() + " </b>";
	if(tooltipFlags & (int)RzxConfig::Promo)
		tooltip += "<i>(" + promoText() + ")</i>";
	tooltip += "<br/><br/>";
 	tooltip += "<b><i>" + tr("Informations :") + "</b></i><br/>";
	
	if(hasFtpServer() && (tooltipFlags & (int)RzxConfig::Ftp))
		tooltip += "<b>-></b>&nbsp;" + tr("ftp server : ") + tr("<b>on</b>") + "<br/>";
	if(hasHttpServer() && (tooltipFlags & (int)RzxConfig::Http))
		tooltip += "<b>-></b>&nbsp;" + tr("web server : ") + tr("<b>on</b>") + "<br/>";
	if(hasNewsServer() && (tooltipFlags & (int)RzxConfig::News))
		tooltip += "<b>-></b>&nbsp;" + tr("news server : ") + tr("<b>on</b>") + "<br/>";
	if(hasSambaServer() && (tooltipFlags & (int)RzxConfig::Samba))
		tooltip += "<b>-></b>&nbsp;" + tr("samba server : ") + tr("<b>on</b>") + "<br/>";
	if(tooltipFlags & (int)RzxConfig::OS)
		tooltip += "<b>-></b>&nbsp;os : " + sysExText() + "<br/>";
	if(tooltipFlags & (int)RzxConfig::Client)
		tooltip += "<b>-></b>&nbsp;" + client() + "<br/>";
	if(tooltipFlags & (int)RzxConfig::Features)
	{
		if(can(Rzx::CAP_ON))
		{
			int nb = 0;
			tooltip += "<b>-></b>&nbsp;" + tr("features : ");
			if(can(Rzx::CAP_CHAT))
			{
				tooltip += tr("chat");
				nb++;
			}
			if(can(Rzx::CAP_XPLO))
			{
				tooltip += QString(nb?", ":"") + "Xplo";
				nb++;
			}
			if(!nb) tooltip += tr("none");
			tooltip += "<br/>";
		}
	}
	if(tooltipFlags & (int)RzxConfig::IP)
		tooltip += "<b>-></b>&nbsp;ip : <i>" + ip().toString() + "</i><br/>";
	if(tooltipFlags & (int)RzxConfig::Resal)
		tooltip += "<b>-></b>&nbsp;" + tr("place : ") + rezal(false) + "<br/>";
	
	if(tooltipFlags & (int)RzxConfig::Properties)
	{
		tooltip += "<br/>";
		QString msg = RzxConfig::cache(ip());
		if(msg.isNull())
		{
			tooltip += "<i>" + tr("No properties in cache") + "</i>";
		}
		else
		{
			QString date = RzxConfig::getCacheDate(ip());
			tooltip += "<b><i>" + tr("Properties checked on ")  + date + " :</i></b><br/>";
			QStringList list = msg.split("|");
			for(int i = 0 ; i < list.size() - 1 ; i+=2)
				tooltip += "<b>-></b>&nbsp;" + list[i] + " : " + list[i+1] + "<br/>";
		}
	}

	return tooltip;
}

/*********** Lancement des clients sur la machine ************/
///Lance un client ftp sur l'ordinateur indiqué
/** On lance le client sur le ftp de l'ordinateur indiqué en s'arrangeant pour parcourir
 * le répertoire \a path qui est indiqué
 */
void RzxComputer::ftp(const QString& path) const
{
	RzxUtilsLauncher::ftp(ip(), path);
}

///Lance un client http sur l'ordinateur indiqué
/** On lance le client sur le http de l'ordinateur indiqué en s'arrangeant pour parcourir
 * le répertoire \a path qui est indiqué
 */
void RzxComputer::http(const QString& path) const
{
	RzxUtilsLauncher::http(ip(), path);
}

///Lance un client samba sur l'ordinateur indiqué
/** On lance le client sur le samba de l'ordinateur indiqué en s'arrangeant pour parcourir
 * le répertoire \a path qui est indiqué
 */
void RzxComputer::samba(const QString& path) const
{
	RzxUtilsLauncher::samba(ip(), path);
}

///Lance un client news sur l'ordinateur indiqué
/** On lance le client sur le serveur news de l'ordinateur indiqué en s'arrangeant pour parcourir
 * le newsgroup \a path qui est indiqué
 */
void RzxComputer::news(const QString& path) const
{
	RzxUtilsLauncher::news(ip(), path);
}

/********** Modification de l'état de préférence *************/
///Ban l'ordinateur
void RzxComputer::ban()
{
	RzxConfig::global()->addToBanlist(*this);
	emit update(this);
}

///Unban l'ordinateur
void RzxComputer::unban()
{
	RzxConfig::global()->delFromBanlist(*this);
	emit update(this);
}

///Ajout au favoris
void RzxComputer::addToFavorites()
{
	RzxConfig::global()->addToFavorites(*this);
	emit update(this);
}

///Supprime des favoris
void RzxComputer::removeFromFavorites()
{
	RzxConfig::global()->delFromFavorites(*this);
	emit update(this);
}

/******** Indique que l'état du favoris a changé *************/
///Indique que le favoris a changé d'état
/** L'état est :
 * 	- Connecté
 * 	- Absent
 * 	- Présent
 * 	- Déconnecté
 *
 * et ceci n'est émis que dans le cas où l'objet est un favoris != localhost
 */
void RzxComputer::emitStateChanged()
{
	emit stateChanged(this);
	if(isFavorite() && !isLocalhost())
		emit favoriteStateChanged(this);
}

/*********** Lancement des données liées au chat *************/
///Affichage de l'historique des communications
void RzxComputer::historique()
{
	emit wantHistorique(this);
}

///Check des propriétés
void RzxComputer::proprietes()
{
	emit wantProperties(this);
}

///Lance un chat avec la machine correspondante
void RzxComputer::chat()
{
	emit wantChat(this);
}

/****************** Analyse de la machine ********************/
///Scan des servers ouverts
/** Rerchercher parmi les protocoles affichés par qRezix lesquels sont présents sur la machine */
void RzxComputer::scanServers()
{
	QFlags<ServerFlags> newServers;
#ifdef WIN32
    //Bon, c pas beau, mais si j'essaye de travailler sur le meme socket pour tous les test
	//j'arrive toujours à de faux résultats... c ptet un bug de windows (pour changer)
	QTcpServer detecter;

	//scan du ftp
	if(!detecter.listen(ip, 21))
		newServers |= SERVER_FTP;

	//scan du http
	if(!detecter.listen(ip, 80))
		newServers |= SERVER_HTTP;

	//scan du nntp
	if(!detecter.listen(ip, 119))
		newServers |= SERVER_NEWS;

	//scan du samba
	if(!detecter.listen(ip, 445))
		newServers |= SERVER_SAMBA;
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
		res = QString(netstat.readAllStandardOutput()).split('\n');
	#if defined(Q_OS_MAC) || defined(Q_OS_BSD)
			res = res.filter(QRegExp("LISTEN"));
			if(!(res.filter(QRegExp("\\.21\\s")).isEmpty())) newServers |= SERVER_FTP;
			if(!(res.filter(QRegExp("\\.80\\s")).isEmpty())) newServers |= SERVER_HTTP;
			if(!(res.filter(QRegExp("\\.119\\s")).isEmpty())) newServers |= SERVER_NEWS;
			if(!(res.filter(QRegExp("\\.445\\s")).isEmpty())) newServers |= SERVER_SAMBA;
	#else
			//lecture des différents port pour voir s'il sont listen
			if(!(res.filter(":21 ")).isEmpty()) newServers |= SERVER_FTP;
			if(!(res.filter(":80 ")).isEmpty()) newServers |= SERVER_HTTP;
			if(!(res.filter(":119 ")).isEmpty()) newServers |= SERVER_NEWS;
			if(!(res.filter(":445 ")).isEmpty()) newServers |= SERVER_SAMBA;
	#endif
	}

	//au cas où netstat fail ou qu'il ne soit pas installé
	else
	{
		qDebug("Netstat didn't succeed : %s", netstat.errorString().toAscii().constData());
		newServers |= SERVER_FTP;
		newServers |= SERVER_HTTP;
		newServers |= SERVER_NEWS;
		newServers |= SERVER_SAMBA;
	}

#endif //WIN32
	const QFlags<ServerFlags> oldServers = servers();
	
	if(newServers != servers())
	{
		setServers(newServers);
		emitStateChanged();
	}

	delayScan->start(30000); //bon, le choix de 30s, c vraiment aléatoire
							//1 ou 2 minutes, ç'aurait pas été mal, mais bon
}
