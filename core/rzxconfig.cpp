/***************************************************************************
                          rzxconfig.cpp  -  description
                             -------------------
    begin                : Sun Jan 27 2002
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

#include <stdlib.h>
#include <time.h>

#include <QDate>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QTranslator>
#include <QApplication>

#ifdef WIN32
#	include <math.h>
#	include <windows.h>
#else
#	include <unistd.h>
#endif

#include <RzxConfig>

#include <RzxApplication>
#include <RzxMessageBox>
#include <RzxHostAddress>
#include <RzxComputer>
#include <RzxIconCollection>
#include <RzxTranslator>
#include <RzxStyle>

RZX_CONFIG_INIT(RzxConfig)

///Initialisation des donn�es de configuration
void RzxConfig::init()
{
	Rzx::beginModuleLoading("Config");

	//Chargement de la liste des r�pertoires	
	loadDirs();

	//Initialisation du g�n�rateur de nombre al�atoire...
	//M�me si qRezix l'utilise peu, �a peut toujours �tre utile en particulier pour les plugins
	srand(time(0));

	//Ouverture du fichier de configuration
	readFavorites();
	readIgnoreList();

	//CHargment de donn�es diverses (icons, traductions)
	RzxStyle::global();
	RzxTranslator::global();
	RzxIconCollection::global();
	loadRezals();
	
	//Chargement des donn�es QVB sur les fontes du syst�me
	Rzx::endModuleLoading("Config");
}

///Lib�ration de la m�moire
void RzxConfig::destroy()
{
	Rzx::beginModuleClosing("Config");
	writeFavorites();
	writeIgnoreList();
	delete RzxStyle::global();
	delete RzxTranslator::global();
	delete RzxIconCollection::global();
	Rzx::endModuleClosing("Config");
}

///Indique si c'est le premier chargement
bool RzxConfig::firstLaunch()
{
	if(global()->value("firstload", true).toBool())
	{
		global() -> setValue("firstload", false);
		return true;
	}
	return false;
}

/*****************************************************************************
* REPERTOIRES DES DONNEES																	  *
*****************************************************************************/
///Chargement des r�pertoires de stockage
void RzxConfig::loadDirs()
{
	Rzx::beginModuleLoading("Config Path");

#ifdef Q_OS_MAC
	m_systemDir.setPath(QREZIX_SYSTEM_DIR);
	m_userDir.setPath(QREZIX_DATA_DIR);
	m_libDir.setPath(QREZIX_LIB_DIR);
#else
	QString userSubdir;
	m_userDir = QDir::home();
#ifdef WIN32
	QString dir = settings->value("InstDir").toString();
	userSubdir = "qRezix";
	m_libDir = QDir::currentDirPath();
	if(!dir.isEmpty())
		m_systemDir.setPath(dir);
	else
		m_systemDir = m_userDir;
	m_systemDir = m_libDir;
#else
	userSubdir = ".rezix";
	m_systemDir.setPath(QREZIX_DATA_DIR);
	m_libDir.setPath(QREZIX_LIB_DIR);
#endif 
	if (!m_userDir.cd(userSubdir))
	{
		if (!m_userDir.mkdir(userSubdir))
		{
			QString msg;
			msg = tr("qRezix cannot create %1, which is the folder in which its configuration is saved\n")
				.arg(m_userDir.absoluteFilePath(userSubdir));
			msg += tr("You will not be able to save your configuration");
			RzxMessageBox::critical(0, "qRezix", msg);
		}
		else
			m_userDir.cd(userSubdir);
	}
	
	if (!m_systemDir.exists())
		m_systemDir = m_userDir;

	if(!m_libDir.exists())
		m_libDir = m_systemDir;
#endif //WIN32

	qDebug() << "Current path is" << QDir::current().path();
	qDebug() << "Personnal path set to" << m_userDir.path();
	qDebug() << "System path set to" << m_systemDir.path();
	qDebug() << "Libraries path set to" << m_libDir.path();
	Rzx::endModuleLoading("Config Path");
}

///R�pertoire utilisateur
/** Dans les syst�mes Unix, c'est ~/.rezix/, sous windows, c'est %HOME%\qRezix\.
 * Ce r�pertoire est celui qui est cens� stocker les donn�es de l'utilisateur.
 */
const QDir &RzxConfig::userDir()
{
	return global()->m_userDir;
}

///R�pertoire syst�me
/** Ce r�pertoire est celui d'installation de qRezix
 */
const QDir &RzxConfig::systemDir()
{
	return global() -> m_systemDir;
}

///R�pertoire de stockage des biblioth�ques de qRezix
const QDir &RzxConfig::libDir()
{
	return global() -> m_libDir;
}

///Retourne le r�pertoire o� rechercher les ic�nes des ordinateurs
QDir RzxConfig::computerIconsDir()
{
	return dir(UserDir, "icones", true, true);
}

///Retourne le r�pertoire indiqu�
/** \sa dirList */
QDir RzxConfig::dir(DirFlags dir, const QString &subDir, bool force, bool create)
{
	QDir rep;
	QDir invalid("/invalid/forder/");
	switch(dir)
	{
		case UserDir: rep = userDir(); break;
		case SystemDir: rep = systemDir(); break;
		case LibDir: rep = libDir(); break;
		case CurrentDir: rep = QDir::current(); break;
		case TempDir: rep = QDir::temp(); break;
		default: qDebug("RzxConfig::dir : invalid directory"); return invalid;
	}
	if(!rep.exists()) return invalid;
	if(!subDir.isEmpty() && !rep.cd(subDir))
	{
		if(create && rep.mkdir(subDir))
		{
			if(!rep.cd(subDir) && !force) return invalid;
		}
		else if(!force)
			return invalid;
	}
	return rep;
}

///Retourne la liste des r�pertoires demand�s
/** La liste est renvoy� sans doublons.
 *
 * Si \param subDir est d�fini, la liste renvoie les sous r�pertoires
 * correspondant. \param force indique ce qu'on doit faire dans le cas
 * o� le sous r�pertoire n'existe pas : true renvoie le r�pertoire non
 * modifi�, false ignore le r�pertoire si il n'a pas le sous r�pertoire
 * demand�.
 */
QList<QDir> RzxConfig::dirList(Dir dirs, const QString &subDir, bool force, bool create)
{
	QList<QDir> dirList;
#define add(mdir) addDir(dirList, dir(mdir, subDir, force, create))
	if(dirs & UserDir) add(UserDir);
	if(dirs & SystemDir) add(SystemDir);
	if(dirs & LibDir) add(LibDir);
	if(dirs & CurrentDir) add(CurrentDir);
	if(dirs & TempDir) add(TempDir);
#undef add
	return dirList;
}

///Fonction auxiliaire de libDir
void RzxConfig::addDir(QList<QDir> &dirList, QDir dir)
{
	if(!dir.exists()) return;
	if(!dirList.contains(dir))
		dirList << dir;
}

///Recherche un fichier existant dans les r�pertoires indiqu�s, et retourne son chemin
QString RzxConfig::findFile(const QString& filename, Dir dir, const QString& subdir)
{
	QList<QDir> dirs = dirList(dir, subdir);
	foreach(QDir dir, dirs)
	{
		if(dir.exists(filename))
			return dir.absoluteFilePath(filename);
	}
	return QString();
}


/******************************************************************************
* FONCTION DE GESTION DES SOUS-R�SEAUX                                        *
******************************************************************************/
///Charge la liste des sous-r�seau
/** La liste des rezals doit �tre d�finie dans un fichier nomm� subnet.ini plac�
 * soit dans le r�pertoire d'installation soit dans le r�pertoire utilisateur et
 * donc le contenu d�crit les diff�rents subnets de fa�on pr�cise. Ce fichier
 * est constitu� de 2 parties, la premi�re appel�e Subnets d�crit la liste des
 * sous r�seaux avec un format :
 * \code
 * x_shortname = ip1/mask1,ip2/mask2,...
 * \endcode
 * o� x est un chiffre de priorit� (plus il est petit, plus le sous r�seau est
 * prioritaire... c'est � dire plus il est test� rapidement).
 *
 * La deuxi�me partie s'appelle Names et associe � chaque subnet un nom plus
 * long. Il a un format :
 * \code
 * x_shortname = long name
 * \endcode
 *
 * Un exemple de fichier complet serait de la forme suivante :
 * \code
 * [Subnets]
 * 1_Binet = 129.104.201.0/24
 * 1_Bat70 = 129.104.224.128/25
 * 2_X = 129.104.0.0/16
 * 3_Monde = 0.0.0.0/0
 * 
 * [Names]
 * 1_Binet = Binets & K�s
 * 1_Bat70 = B�timent 70
 * 2_X = X
 * 3_Monde = Internet
 * \endcode
 */
void RzxConfig::loadRezals()
{
	//On place le subnet inconnu par d�faut
	rezalSubnets << (QList<RzxSubnet>() << RzxSubnet::unknown);
	rezalNames << "Unknown";
	rezalLongNames << "Unknown";
	
	//On lit le fichier de conf
	Rzx::beginModuleLoading("Subnets");
	QString file = findFile("subnet.ini");
	if(file.isNull())
	{
		qDebug("No subnet file found...");
		Rzx::endModuleLoading("Subnets", false);
		return;
	}

	QSettings subnets(file, IniFormat);

	//R�cup�ration des r�seaux
	subnets.beginGroup("Subnets");
	QStringList keys = subnets.childKeys();
	foreach(QString key, keys)
	{
		QString value = subnets.value(key).toString();
		QStringList subs = value.split(",", QString::SkipEmptyParts);
		QList<RzxSubnet> rezal;
		foreach(QString sub, subs)
		{
			RzxSubnet subnet(sub);
			if(subnet.isValid())
				rezal << subnet;
		}
		if(!rezal.isEmpty())
		{
			QRegExp name("\\d_(.+)");
			if(name.indexIn(key) != -1)
				rezalNames << name.cap(1);
			else
				rezalNames << key;
			rezalSubnets << rezal;
		}
	}
	subnets.endGroup();

	//R�cup�ration des noms de r�seaux
	subnets.beginGroup("Names");
	foreach(QString key, keys)
	{
		QString longName = subnets.value(key).toString();
		if(longName.isEmpty())
		{
			QRegExp name("\\d_(.+)");
			if(name.indexIn(key) != -1)
				rezalLongNames << name.cap(1);
			else
				rezalLongNames << key;
		}
		else
			rezalLongNames << longName;
	}
	subnets.endGroup();
	qDebug("Found %d subnets", rezalNumber());
	Rzx::endModuleLoading("Subnets");
}

///Retourne le nombre de sous r�seaux
uint RzxConfig::rezalNumber()
{
	return (uint)global()->rezalSubnets.count();
}

///Retourne le rezal auquel appartient le QHostAddress
/** Retourne -1 si aucun sous-r�seau correspondant n'est trouv�
 */
int RzxConfig::rezal(const QHostAddress& addr)
{
	int i = 0;
	foreach(QList<RzxSubnet> rezal, global()->rezalSubnets)
	{
		foreach(RzxSubnet subnet, rezal)
		{
			if(subnet.contains(addr))
				return i;
		}
		i++;
	}
	return -1;
}

///Retourne le nom du rezal indiqu�
/** Si le rezal n'existe pas, cette fonction renvoie QString(),
 * dans les autre cas, le nom du rezal est retourn�. Si bool = true,
 * c'est le nom court qui est retourn�, sinon c'est le nom long
 */
QString RzxConfig::rezalName(int rezalid, bool shortname)
{
	if(rezalid >= (int)rezalNumber() || rezalid < 0)
		return QString();

	if(shortname)
		return global()->rezalNames[rezalid];
	else
		return global()->rezalLongNames[rezalid];
}

///Retourne le nom du rezal auquel appartient le QHostAddress
/** \sa rezalName */
QString RzxConfig::rezalName(const QHostAddress& addr, bool shortname)
{
	return rezalName(rezal(addr), shortname);
}


/*******************************************************************************
* EMISSION DE MESSAGES                                                         *
*******************************************************************************/
///Emet un signal pour informer du changement de format des ic�nes
void RzxConfig::emitIconFormatChanged()
{
	emit global()->iconFormatChange();
}

/******************************************************************************
* INFORMATIONS CONCERNANT LA MACHINE                                          *
******************************************************************************/
///Commentaire de l'ordinateur
QString RzxConfig::remarque(bool def, const QString& defValue)
{
	QString comment = def?QString(defValue):global()->value("comment", "$#x").toString();

	if(comment == "$#x")
	{
		QStringList comments = QString(DEFAULT_COMMENT).split("\n");
		int i = rand()%comments.size();
		comment = comments[i];
	}

	return comment;
}
///Change le commentaire de l'ordinateur
void RzxConfig::setRemarque(const QString &remarque)
{
	global()->value("comment", remarque.isNull()?QString("$#x"):remarque);
}

///Retourne le nom de la promo
QString RzxConfig::propPromo()
{
	return RzxComputer::localhost()->promoText();
}

///Change le mode du r�pondeur
void RzxConfig::setAutoResponder(const bool& val)
{
	RzxComputer::localhost()->setState(val);
}

///Lit le mode du r�pondeur
bool RzxConfig::autoResponder(bool def, const bool& defValue)
{
	return def?defValue:RzxComputer::localhost()->isOnResponder();
}

///Indique si la totalit� des informations sont enregistr�es
bool RzxConfig::infoCompleted()
{
	return propLastName() != "" && propName() != "" && propCasert() != "" 
		&& propMail() != DEFAULT_MAIL && propTel() != "";
}


/******************************************************************************
* GESTION DES FAVORIS                                                         *
******************************************************************************/
void RzxConfig::readFavorites()
{
	/* Pour que la compatiblit� avec l'ancien format soit maintenue
	on lit ce fichier, et on le supprime, sachant que tout ira dans
	le nouveau fichier � l'issue */
	static const QString name = CONF_FAVORITES;
	if (m_userDir.exists(name))
	{
		QString favoritesFile = m_userDir.absoluteFilePath(name);
	
		QFile file(favoritesFile);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			RzxMessageBox::critical(0, tr("qRezix error"), 
				tr("Unable to open favorites file %1").arg(favoritesFile));
			return;
		}
	
		QTextStream stream(&file);

		favorites.clear();
		QString line;
		line = stream.readLine();
		while(!line.isNull()) {
			addToFavorites(line);
			line = stream.readLine();
		}
		file.close();
		file.remove(); //le fichier n'est d�sormais plus n�cessaire
							//tout va dans le qrezixrc
		writeFavorites();
	}
	
	/* La nouvelle technique... les favoris sont maintenant une entr�e dans
	le fichier de config sous forme de liste */
	else
	{
		QStringList favoriteList = value("favorites").toStringList();
		QStringList::iterator it;
		for(it = favoriteList.begin() ; it != favoriteList.end() ; it++)
			addToFavorites(*it);
	}
}

///Indique si l'entr�e est dans les favoris
bool RzxConfig::isFavorite(const QString& nom) const {
	return favorites.contains(nom);
}

///Idem encore mais � partir d'un RzxComputer
bool RzxConfig::isFavorite(const RzxComputer& computer) const {
	return isFavorite(computer.name());
}

///Ajout du pseudo � la liste des favoris
void RzxConfig::addToFavorites(const QString& nom) {
	favorites.insert(nom);
}

///Idem � partir du RzxComputer
void RzxConfig::addToFavorites(const RzxComputer& computer) {
	favorites.insert(computer.name());
}

///Suppression du pseudo de la liste des favoris
void RzxConfig::delFromFavorites(const QString& nom) {
	favorites.remove(nom);
}

///Idem � partir du RzxComputer
void RzxConfig::delFromFavorites(const RzxComputer& computer) {
	favorites.remove(computer.name());
}

void RzxConfig::writeFavorites()
{
	setValue("favorites", QVariant::fromValue<QStringList>(favorites.values()));
}

/******************************************************************************
* GESTION DE L'IGNORE LIST                                                    *
******************************************************************************/
void RzxConfig::readIgnoreList()
{
	/* Pour que la compatiblit� avec l'ancien format soit maintenue
	on lit ce fichier, et on le supprime, sachant que tout ira dans
	le nouveau fichier � l'issue */
	static const QString name = CONF_IGNORELIST;
	if (m_userDir.exists(name))
	{
		QString ignoreListFile = m_userDir.absoluteFilePath(name);
	
		QFile file(ignoreListFile);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			RzxMessageBox::critical(0, tr("qRezix error"), 
				tr("Unable to open ignoreList file %1").arg(ignoreListFile));
			return;
		}
	
		QTextStream stream(&file);

		ignoreList.clear();
		QString line;
		line = stream.readLine();
		while(!line.isNull()) {
			addToBanlist(line);
			line = stream.readLine();
		}
		file.close();
		file.remove(); //le fichier n'est d�sormais plus n�cessaire
							//tout va dans le qrezixrc
		writeIgnoreList();
	}
	
	/* La nouvelle technique... les favoris sont maintenant une entr�e dans
	le fichier de config sous forme de liste */
	else
	{
		QStringList ignoreListList =  value("ignoreList").toStringList();
		QStringList::iterator it;
		for(it = ignoreListList.begin() ; it != ignoreListList.end() ; it++)
			addToBanlist(*it);
	}
}

///Indique si l'ip donn�e est bann�e
bool RzxConfig::isBan(const QString& ip) const {
	return ignoreList.contains(ip);
}

///Idem � partir d'un RzxHostAddress - RzxHostAddress
bool RzxConfig::isBan(const RzxHostAddress& ip) const {
	return isBan(ip.toString());
}

///Idem � partir d'un RzxComputer
bool RzxConfig::isBan(const RzxComputer& computer) const {
	return isBan(computer.ip().toString());
}

void RzxConfig::addToBanlist(const QString& ip) {
	ignoreList.insert(ip);
}

void RzxConfig::addToBanlist(const RzxComputer& ip) {
	ignoreList.insert(ip.ip().toString());
}

void RzxConfig::delFromBanlist(const QString& ip) {
	ignoreList.remove(ip);
}

void RzxConfig::delFromBanlist(const RzxComputer& ip) {
	ignoreList.remove(ip.ip().toString());
}

void RzxConfig::writeIgnoreList()
{
	setValue("ignoreList", QVariant::fromValue<QStringList>(ignoreList.values()));
}


/******************************************************************************
* GESTION DE LA CACHE DES PROPRI�T�S
******************************************************************************/
void RzxConfig::addCache(const RzxHostAddress& address, const QString& msg)
{
	global() -> setValue("cacheprop-" + address.toString(), msg);
	global() ->setValue("cachedate-" + address.toString(),  QDate::currentDate().toString("dd MMMM yyyy"));
}

QString RzxConfig::cache(const RzxHostAddress& address)
{
	return global()->value("cacheprop-" + address.toString()).toString();
}

QString RzxConfig::getCacheDate(const RzxHostAddress& address)
{
	return global()->value("cachedate-" + address.toString()).toString();
}
