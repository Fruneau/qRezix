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

#include <RzxComputer>

#include <RzxConfig>
#include <RzxUtilsLauncher>
#include <RzxConnectionLister>
#include <RzxIconCollection>
#include <RzxApplication>

const char *RzxComputer::promalText[4] = {
    "?",
    QT_TR_NOOP("Orange") ,
    QT_TR_NOOP("Rouje"),
    QT_TR_NOOP("Jone")
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
RzxComputer::RzxComputer(RzxNetwork *network, const RzxHostAddress& c_ip, const QString& c_name, quint32 c_options, quint32 c_version, quint32 c_stamp, quint32 c_flags, const QString& c_remarque)
	: delayScan(NULL), m_network(network), m_name(c_name), m_remarque(c_remarque), m_ip(c_ip), m_flags(c_flags), m_stamp(c_stamp)
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
	m_network = NULL;
	connect(delayScan, SIGNAL(timeout()), this, SLOT(scanServers()));

	//Ip mise à jour par RzxServerListener
	m_ip = RzxHostAddress::fromRezix(0);

	setName(RzxConfig::dnsName());
	setRemarque(RzxConfig::remarque());
	setPromo(RzxConfig::promo());
	setState(RzxConfig::repondeur());
	setServerFlags((ServerFlags)RzxConfig::servers());
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
	m_options.SysEx = Rzx::SYSEX_UNKNOWN;
#ifdef WIN32
	if (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)
		m_options.SysEx = Rzx::SYSEX_WINNT;
	else
		m_options.SysEx = Rzx::SYSEX_WIN9X;
#endif
#ifdef Q_OS_LINUX
	m_options.SysEx = Rzx::SYSEX_LINUX;
#endif
#ifdef Q_OS_BSD4
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
void RzxComputer::setServers(Servers servers) 
{ m_options.Server = servers; }
///Définition de la liste des serveurs envisageables sur la machine (localhost uniquement)
void RzxComputer::setServerFlags(Servers serverFlags) 
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

///Récupération du module auquel est ataché l'entrée
RzxNetwork *RzxComputer::network() const
{ return m_network; }

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
RzxComputer::Servers RzxComputer::servers() const
{ return (ServerFlags)m_options.Server; }
///Récupération de la liste des serveurs présents (localhost)
RzxComputer::Servers RzxComputer::serverFlags() const
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
{ return RzxHostAddress::isSameGateway(ip(), ref.ip()); }
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
int RzxComputer::rezal() const
{
	return m_ip.rezal();
}

/** No descriptions */
void RzxComputer::loadIcon()
{
	QPixmap temp = RzxIconCollection::global()->hashedIcon(m_stamp);
	if(temp.isNull())
	{
		if(m_stamp && m_network)
			m_network->getIcon(ip());
		setIcon(RzxIconCollection::global()->osPixmap(sysEx(), true));
	}
	else
		setIcon(temp);
}

/*********** Lancement des clients sur la machine ************/
///Lance un client ftp sur l'ordinateur indiqué
/** On lance le client sur le ftp de l'ordinateur indiqué en s'arrangeant pour parcourir
 * le répertoire \a path qui est indiqué
 */
void RzxComputer::ftp(const QString& path) const
{
	if(hasFtpServer())
		RzxUtilsLauncher::ftp(ip(), path);
}

///Lance un client http sur l'ordinateur indiqué
/** On lance le client sur le http de l'ordinateur indiqué en s'arrangeant pour parcourir
 * le répertoire \a path qui est indiqué
 */
void RzxComputer::http(const QString& path) const
{
	if(hasHttpServer())
		RzxUtilsLauncher::http(ip(), path);
}

///Lance un client samba sur l'ordinateur indiqué
/** On lance le client sur le samba de l'ordinateur indiqué en s'arrangeant pour parcourir
 * le répertoire \a path qui est indiqué
 */
void RzxComputer::samba(const QString& path) const
{
	if(hasSambaServer())
		RzxUtilsLauncher::samba(ip(), path);
}

///Lance un client news sur l'ordinateur indiqué
/** On lance le client sur le serveur news de l'ordinateur indiqué en s'arrangeant pour parcourir
 * le newsgroup \a path qui est indiqué
 */
void RzxComputer::news(const QString& path) const
{
	if(hasNewsServer())
		RzxUtilsLauncher::news(ip(), path);
}

/********** Gestion des propriétés ***************************/
///Définition des propriétés
void RzxComputer::setProperties(const QString& prop)
{
	RzxConfig::addCache(ip(), prop);
}

///Récupère les propriétés
QString RzxComputer::properties() const
{
	return RzxConfig::cache(ip());
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
void RzxComputer::history()
{
	if(RzxApplication::chatUiModule())
		RzxApplication::chatUiModule()->history(this);
}

///Check des propriétés
void RzxComputer::checkProperties()
{
	if(network()->type() & RzxNetwork::TYP_PROPERTIES)
		network()->properties(this);
	else if(RzxApplication::propertiesModule())
		RzxApplication::propertiesModule()->properties(this);
}

///Lance un chat avec la machine correspondante
void RzxComputer::chat()
{
	if(!RzxApplication::chatUiModule()) return;
	RzxApplication::chatUiModule()->chat(this);
}

///Envoie un message à la machine
void RzxComputer::sendChat(Rzx::ChatMessageType type, const QString& msg)
{
	if(network()->type() & RzxNetwork::TYP_CHAT)
		network()->sendChatMessage(this, type, msg);
	else if(RzxApplication::chatModule())
		RzxApplication::chatModule()->sendChatMessage(this, type, msg);
}

///Reçoit un message depuis la machine
void RzxComputer::receiveChat(Rzx::ChatMessageType type, const QString& msg)
{
	if(!RzxApplication::chatUiModule()) return;
	RzxApplication::chatUiModule()->receiveChatMessage(this, type, msg);
}


/****************** Analyse de la machine ********************/
///Scan des servers ouverts
/** Rerchercher parmi les protocoles affichés par qRezix lesquels sont présents sur la machine */
void RzxComputer::scanServers()
{
	Servers newServers;
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

	#if defined(Q_OS_MAC) || defined(Q_OS_BSD4)
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
	#if defined(Q_OS_MAC) || defined(Q_OS_BSD4)
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
	const Servers oldServers = servers();
	
	if(newServers != servers())
	{
		setServers(newServers);
		emitStateChanged();
	}

	delayScan->start(30000); //bon, le choix de 30s, c vraiment aléatoire
							//1 ou 2 minutes, ç'aurait pas été mal, mais bon
}
