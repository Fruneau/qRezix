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
#include <QFile>
#include <QTextStream>
#include <QStringList>

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
#include <RzxFavoriteList>
#include <RzxBanList>

RZX_CONFIG_INIT(RzxConfig)

///Définition pour les traductions des propriétés
const char *RzxConfig::propStrings[][2] = {
	{ QT_TR_NOOP("Name"), "RzxConfig" },
	{ QT_TR_NOOP("Last name"), "RzxConfig" },
	{ QT_TR_NOOP("Nick"), "RzxConfig" },
	{ QT_TR_NOOP("Phone"), "RzxConfig" },
	{ QT_TR_NOOP("E-Mail"), "RzxConfig" },
	{ QT_TR_NOOP("Web Page"), "RzxConfig" },
	{ QT_TR_NOOP("Room"), "RzxConfig" },
	{ QT_TR_NOOP("Sport"), "RzxConfig" },
	{ QT_TR_NOOP("Class"), "RzxConfig" },
	{ "Orange", "RzxComputer" },
	{ "Rouje", "RzxComputer" },
	{ "Jone", "RzxComputer" },
	{ "None", "RzxProperty" },
	{ "Athletism", "RzxProperty" },
	{ "Rowing", "RzxProperty" },
	{ "Basketball", "RzxProperty" },
	{ "Orienting", "RzxProperty" },
	{ "Riding", "RzxProperty" },
	{ "Climbing", "RzxProperty" },
	{ "Fencing", "RzxProperty" },
	{ "Football", "RzxProperty" },
	{ "Golf", "RzxProperty" },
	{ "Handball", "RzxProperty" },
	{ "Judo", "RzxProperty" },
	{ "Swimming", "RzxProperty" },
	{ "Rugby", "RzxProperty" },
	{ "Tennis", "RzxProperty" },
	{ "Sailing", "RzxProperty" },
	{ "Volleyball", "RzxProperty" },
	{ "Badminton", "RzxProperty" }
};
#define propStringsSize 12

///Initialisation des données de configuration
void RzxConfig::init()
{
	Rzx::beginModuleLoading("Config");

	//Chargement de la liste des répertoires	
	loadDirs();

	//Initialisation du générateur de nombre aléatoire...
	//Même si qRezix l'utilise peu, ça peut toujours être utile en particulier pour les plugins
	srand(time(0));

	//CHargment de données diverses (icons, traductions)
	RzxStyle::global();
	RzxTranslator::global();
	RzxIconCollection::global();
	RzxFavoriteList::global();
	RzxBanList::global();
	loadRezals();
	
	//Chargement des données QVB sur les fontes du système
	Rzx::endModuleLoading("Config");
}

///Libération de la mémoire
void RzxConfig::destroy()
{
	Rzx::beginModuleClosing("Config");
	delete RzxStyle::global();
	delete RzxTranslator::global();
	delete RzxIconCollection::global();
	delete RzxFavoriteList::global();
	delete RzxBanList::global();
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
///Chargement des répertoires de stockage
void RzxConfig::loadDirs()
{
	Rzx::beginModuleLoading("Config Path");

	QString userSubdir;
	m_userDir = QDir::home();

#ifdef Q_OS_MAC
	m_systemDir.setPath(QREZIX_SYSTEM_DIR);
	m_libDir.setPath(QREZIX_LIB_DIR);
	userSubdir = "Library/Application Support/qRezix";
#else
	#ifdef WIN32
		QSettings regReader(QSettings::SystemScope, "BR", "qRezix");
		QString dir = regReader.value("InstDir").toString();
		userSubdir = "qRezix";
		m_libDir = QDir::current();
		if(!dir.isEmpty())
			m_systemDir.setPath(dir);
		else
			m_systemDir = m_userDir;
	#else
		userSubdir = ".rezix";
		m_systemDir.setPath(QREZIX_DATA_DIR);
		m_libDir.setPath(QREZIX_LIB_DIR);
	#endif //WIN32
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

#ifndef Q_OS_MAC
	if (!m_systemDir.exists())
		m_systemDir = m_userDir;

	if(!m_libDir.exists())
		m_libDir = m_systemDir;
#endif //!Q_OS_MAC

	qDebug() << "Current path is" << QDir::current().path();
	qDebug() << "Personnal path set to" << m_userDir.path();
	qDebug() << "System path set to" << m_systemDir.path();
	qDebug() << "Libraries path set to" << m_libDir.path();
	Rzx::endModuleLoading("Config Path");
}

///Répertoire utilisateur
/** Dans les systèmes Unix, c'est ~/.rezix/, sous windows, c'est %HOME%\qRezix\.
 * Ce répertoire est celui qui est censé stocker les données de l'utilisateur.
 */
const QDir &RzxConfig::userDir()
{
	return global()->m_userDir;
}

///Répertoire système
/** Ce répertoire est celui d'installation de qRezix
 */
const QDir &RzxConfig::systemDir()
{
	return global() -> m_systemDir;
}

///Répertoire de stockage des bibliothèques de qRezix
const QDir &RzxConfig::libDir()
{
	return global() -> m_libDir;
}

///Retourne le répertoire où rechercher les icônes des ordinateurs
QDir RzxConfig::computerIconsDir()
{
	return dir(UserDir, "icones", true, true);
}

///Retourne le répertoire indiqué
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

///Retourne la liste des répertoires demandés
/** La liste est renvoyé sans doublons.
 *
 * Si \param subDir est défini, la liste renvoie les sous répertoires
 * correspondant. \param force indique ce qu'on doit faire dans le cas
 * où le sous répertoire n'existe pas : true renvoie le répertoire non
 * modifié, false ignore le répertoire si il n'a pas le sous répertoire
 * demandé.
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

///Recherche un fichier existant dans les répertoires indiqués, et retourne son chemin
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
* FONCTION DE GESTION DES SOUS-RÉSEAUX                                        *
******************************************************************************/
///Charge la liste des sous-réseau
/** La liste des rezals doit être définie dans un fichier nommé subnet.ini placé
 * soit dans le répertoire d'installation soit dans le répertoire utilisateur et
 * donc le contenu décrit les différents subnets de façon précise. Ce fichier
 * est constitué de 2 parties, la première appelée Subnets décrit la liste des
 * sous réseaux avec un format :
 * \code
 * x_shortname = ip1/mask1,ip2/mask2,...
 * \endcode
 * où x est un chiffre de priorité (plus il est petit, plus le sous réseau est
 * prioritaire... c'est à dire plus il est testé rapidement).
 *
 * La deuxième partie s'appelle Names et associe à chaque subnet un nom plus
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
 * 1_Binet = Binets & Kès
 * 1_Bat70 = Bâtiment 70
 * 2_X = X
 * 3_Monde = Internet
 * \endcode
 */
void RzxConfig::loadRezals()
{
	//On place le subnet inconnu par défaut
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

	//Récupération des réseaux
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

	//Récupération des noms de réseaux
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

///Retourne le nombre de sous réseaux
uint RzxConfig::rezalNumber()
{
	return (uint)global()->rezalSubnets.count();
}

///Retourne l'index du sous-réseau dans la liste si il existe
int RzxConfig::rezal(const RzxSubnet& m_subnet)
{
	int i = 0;
	foreach(QList<RzxSubnet> rezal, global()->rezalSubnets)
	{
		foreach(RzxSubnet subnet, rezal)
		{
			if(subnet == m_subnet)
				return i;
		}
		i++;
	}
	return -1;
}

///Retourne le rezal auquel appartient le QHostAddress
/** Retourne -1 si aucun sous-réseau correspondant n'est trouvé
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

///Retourne le nom du rezal indiqué
/** Si le rezal n'existe pas, cette fonction renvoie QString(),
 * dans les autre cas, le nom du rezal est retourné. Si bool = true,
 * c'est le nom court qui est retourné, sinon c'est le nom long
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
///Emet un signal pour informer du changement de format des icônes
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
	global()->setValue("comment", remarque.isNull()?QString("$#x"):remarque);
}

///Retourne le nom de la promo
QString RzxConfig::propPromo()
{
	return RzxComputer::localhost()->promoText();
}

///Change le mode du répondeur
void RzxConfig::setAutoResponder(const bool& val)
{
	RzxComputer::localhost()->setState(val);
}

///Lit le mode du répondeur
bool RzxConfig::autoResponder(bool def, const bool& defValue)
{
	return def?defValue:RzxComputer::localhost()->isOnResponder();
}

///Indique si la totalité des informations sont enregistrées
bool RzxConfig::infoCompleted()
{
	return propLastName() != "" && propName() != "" && propCasert() != "" 
		&& propMail() != DEFAULT_MAIL && propTel() != "";
}

/******************************************************************************
* GESTION DE LA CACHE DES PROPRIÉTÉS
******************************************************************************/
///Enregistre la cache...
/** Avant de l'enregistrer, on essaye de faire la traduction inverse des différents
 * textes pour pouvoir afficher à terme les propriétés dans la langue actuelle de
 * qRezix
 */
void RzxConfig::addCache(const RzxHostAddress& address, const QString& msg)
{
	//back translations...
	//on recherche dans les traductions disponibles si ça correspond à une chaîne connue
	QStringList list = msg.split("|");
	for(int i = 0 ; i < list.size() ; i++)
	{
		for(int j = 0 ; j < propStringsSize ; j++)
		{
			if(RzxTranslator::backTranslate(list[i], propStrings[j][1], propStrings[j][0]))
			{
				list[i] = QString(propStrings[j][0]);
				break;
			}
		}
	}

	global() -> setValue("cacheprop-" + address.toString(), list.join("|"));
	global() ->setValue("cachedate-" + address.toString(),  QDate::currentDate().toString("dd MMMM yyyy"));
}

///Retourne les props après avoir appliqué les traductions
QString RzxConfig::cache(const RzxHostAddress& address)
{
	QStringList list = global()->value("cacheprop-" + address.toString()).toString().split("|");
	for(int i = 0 ; i < list.size() ; i++)
		list[i] = tr(list[i].toAscii().constData());
	return list.join("|");
}

///Remplie un QTreeWidget avec les propriétés demandées
void RzxConfig::fillWithCache(const RzxHostAddress& ip, QTreeWidget *view)
{
	if(!view) return;

	QStringList props = global()->value("cacheprop-" + ip.toString()).toString().split("|");

	QTreeWidgetItem *item = NULL;
	for(int i = 0 ; i < props.size() - 1 ; i+=2)
	{
		if(props[i+1].isEmpty()) continue;

		item = new QTreeWidgetItem(view, item);
		item->setText(0, tr(props[i].toAscii().constData()));
		item->setText(1, tr(props[i+1].toAscii().constData()));
		if(props[i] == "Web Page")
			item->setIcon(0, RzxIconCollection::getIcon(Rzx::ICON_HTTP));
		else if(props[i] == "Sport")
			item->setIcon(0, RzxIconCollection::getIcon(props[i+1].toLower()));
		else if(props[i] == "Class")
			item->setIcon(0, RzxIconCollection::getIcon(props[i+1] == "Jone" ? Rzx::ICON_JONE : (props[i+1] == "Rouje" ? Rzx::ICON_ROUJE : Rzx::ICON_ORANJE)));
		else if(props[i] == "Room")
			item->setIcon(0, RzxIconCollection::getIcon(Rzx::ICON_SAMEGATEWAY));
		else if(props[i] == "Phone")
			item->setIcon(0, RzxIconCollection::getIcon(Rzx::ICON_PHONE));
		else if(props[i] == "E-Mail")
			item->setIcon(0, RzxIconCollection::getIcon(Rzx::ICON_MAIL));
		else
			item->setIcon(0, RzxIconCollection::getIcon(Rzx::ICON_PROPRIETES));
	}
}

///Retourne l'addresse e-mail de la personne à partir des données issuées des propriétés
QString RzxConfig::getEmailFromCache(const RzxHostAddress& ip)
{
	QStringList props = global()->value("cacheprop-" + ip.toString()).toString().split("|");
	for(int i = 0 ; i < props.size() - 1 ; i+=2)
	{
		if(props[i] == "E-Mail")
			return props[i+1];
	}
	return QString();
}

///Retourne la date d'enregistrement des propriétés
QString RzxConfig::getCacheDate(const RzxHostAddress& address)
{
	return global()->value("cachedate-" + address.toString()).toString();
}
