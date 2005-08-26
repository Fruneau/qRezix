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
#include <QFontDatabase>
#include <QStringList>
#include <QTranslator>

#ifdef WIN32
	#include <math.h>
	#include <windows.h>
	//Parce que l'abruti de MSVC++ il a du mal avec les includes
	#define RZXCHAT_IMPL_H
	#define RZXCLIENTLISTENER_H
	class RzxChat;
	class RzxClientListener;
	class RzxChatSocket;
#else
	#include <unistd.h>
#endif

#include <RzxConfig>

#include <RzxMessageBox>
#include <RzxHostAddress>
#include <RzxComputer>
#include <RzxServerListener>
#include <RzxIconCollection>

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
	loadTranslators();
	RzxIconCollection::global();
	
	//Chargement des donn�es QVB sur les fontes du syst�me
	loadFontList();
	Rzx::endModuleLoading("Config");
}

///Lib�ration de la m�moire
void RzxConfig::destroy()
{
	Rzx::beginModuleClosing("Config");
	writeFavorites();
	writeIgnoreList();
	fontProperties.clear();
	translations.clear();
	Rzx::endModuleClosing("Config");
}

/** Recherche si la configuration a d�j� �t� r�alis�e */
bool RzxConfig::find()
{
	if(global()->value("firstload", true).toBool())
	{
		global() -> setValue("firstload", false);
		return false;
	}
	return true;
}

/*****************************************************************************
* GESTION DES TRADUCTIONS                                                    *
*****************************************************************************/
///Chargement des traductions disponibles
void RzxConfig::loadTranslators()
{
	Rzx::beginModuleLoading("Translations");

	if(currentTranslator)
		qApp->removeTranslator(currentTranslator);
	currentTranslator = NULL;
	translations.clear();

	qDebug("Searching for translations...");
	translations.insert("English", NULL);
	QList<QDir> dirs = dirList(AllDirsExceptTemp, "translations", true);
	foreach(QDir dir, dirs)
		loadTranslatorsInDir(dir);

	qDebug("Loading translation...");
	setLanguage(language());
	Rzx::endModuleLoading("Translations");
}

///Chargement des traductions contenues dans le r�pertoire indiqu�
void RzxConfig::loadTranslatorsInDir(const QDir &rep)
{
	QDir sourceDir(rep);

	QStringList trans=sourceDir.entryList(QStringList() << "*.qm", QDir::Files|QDir::Readable);
	foreach(QString it, trans)
	{
		QTranslator *cur = new QTranslator;
		cur->load(it, sourceDir.path());
		QString lang = cur->translate("RzxConfig", "English");
		if(!lang.isEmpty() && (!translations.keys().contains(lang) || translations[lang]))
		{
			translations.insert(lang, cur);
			qDebug("* %s in %s", lang.toAscii().constData(), sourceDir.absolutePath().toAscii().constData());
		}
	}
}

///Retourne la liste des traductions disponibles
QStringList RzxConfig::translationsList()
{
	QStringList list = translations.keys();
	qSort(list);
	return list;
}

///Retourne la traduction actuelle
/** Contrairement � language qui retourne la langue demand�e par l'utilisateur
 * cette fonction retourne le nom de la langue actuellement charg�e... ce qui
 * peut �tre diff�rent dans certaines conditions.
 */
QString RzxConfig::translation()
{
	return tr("English");
}

///S�lection de la langue � utiliser
void RzxConfig::setLanguage(const QString& language)
{
	if(language != translation() && translations.keys().contains(language))
	{
		global()->setValue("language", language);
		QApplication::removeTranslator(currentTranslator);
		currentTranslator = translations[language];
		QApplication::installTranslator(currentTranslator);
		emit global()->languageChanged();
	}
	qDebug("Language set to %s", tr("English").toLatin1().constData());
}

///Retourne le language actuel
QString RzxConfig::language()
{
	return global() -> value("language", "English").toString();
}


/*****************************************************************************
* GESTION DES POLICES DE CARACT�RES                                          *
*****************************************************************************/
///Liste des traduction
QHash<QString,QTranslator*> RzxConfig::translations;

///Traduction actuelle
QTranslator *RzxConfig::currentTranslator=NULL;

RzxConfig::FontProperty::FontProperty(bool b, bool i, const QList<int> &pS)
	: bold(b), italic(i), sizes(pS)
{ }

RzxConfig::FontProperty::~FontProperty()
{ }

///Chargement de la liste des polices de caract�res
void RzxConfig::loadFontList()
{
	Rzx::beginModuleLoading("Fonts");
	QFontDatabase fdb;
	fontFamilies = fdb.families();
	for( QStringList::Iterator f = fontFamilies.begin(); f != fontFamilies.end();)
	{
		QString family = *f;
		QStringList styles = fdb.styles( family );
		if(styles.contains("Normal")) 
		{
			QList<int> size = fdb.smoothSizes(family, "Normal");
			bool b = styles.contains("Bold")!=0;
			bool i = styles.contains("Italic")!=0 || styles.contains("Oblique")!=0;
			FontProperty fp( b, i, size);
			fontProperties.insert(family, fp);
			++f;
		}
		else
			f = fontFamilies.erase(f);
	}
	qDebug("Found %d fonts families", fontFamilies.count());
	Rzx::endModuleLoading("Fonts");
}

/// Renvoie la liste des familles de fonte initialis�e au d�but
QStringList RzxConfig::getFontList()
{
	return fontFamilies;
}

/// Renvoie la liste des tailles accept�es par cette police
const QList<int> RzxConfig::getSizes(const QString& family) const
{
	const FontProperty &fp = fontProperties[family];
	return fp.sizes;
}

///Indique si la police supporte l'italique
bool RzxConfig::isItalicSupported(const QString& family) const
{
	const FontProperty &fp = fontProperties[family];
	return fp.italic;
}

///Indique si la police supporte le gras
bool RzxConfig::isBoldSupported(const QString& family) const
{
	const FontProperty &fp = fontProperties[family];
	return fp.bold;
}

/*****************************************************************************
* REPERTOIRES DES DONNEES																	  *
*****************************************************************************/
///Chargement des r�pertoires de stockage
void RzxConfig::loadDirs()
{
	Rzx::beginModuleLoading("Config Path");

#ifdef Q_OS_MAC
	m_systemDir.setPath(QREZIX_DATA_DIR);
	m_userDir.setPath(QREZIX_DATA_DIR);
	m_libDir.setPath(QREZIX_DATA_DIR);
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

	qDebug("Personnal path set to %s", m_userDir.path().toAscii().constData());
	qDebug("System path set to %s", m_systemDir.path().toAscii().constData());
	qDebug("Libraries path set to %s", m_libDir.path().toAscii().constData());
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
	switch(dir)
	{
		case UserDir: rep = userDir(); break;
		case SystemDir: rep = systemDir(); break;
		case LibDir: rep = libDir(); break;
		case CurrentDir: rep = QDir::current(); break;
		case TempDir: rep = QDir::temp(); break;
		default: qDebug("RzxConfig::dir : invalid directory"); return QDir();
	}
	if(!rep.exists()) return QDir();
	if(!subDir.isEmpty() && !rep.cd(subDir))
	{
		if(create && rep.mkdir(subDir))
		{
			if(!rep.cd(subDir) && !force) return QDir();
		}
		else if(!force)
			return QDir();
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

/******************************************************************************
* FONCTION DE GESTION DES MOTS DE PASSE                                       *
******************************************************************************/
///Change le mot de passe de connexion actuel
void RzxConfig::setPass(const QString& passcode)
{
//	global() -> setValue(RzxServerListener::object()->getServerIP().toString() + "/pass", passcode);
	global() -> setValue("pass", passcode);
	global() -> flush();
}

///Renvoie le password xnet
QString RzxConfig::pass()
{
	QString i = global() -> value(/*RzxServerListener::object()->getServerIP().toString() +*/ "/pass").toString();
	if(i.isNull()) //Pour la compatibilit� avec les anciennes formes de stockage sous nux
		i = global() -> value("pass").toString();
	return i;
}

///Change l'ancien mot de passe de connexion
void RzxConfig::setOldPass(const QString& oldPass)
{
	global() -> setValue(/*RzxServerListener::object()->getServerIP().toString() + */"/oldpass", oldPass);
	global() -> flush();
}

///Renvoie l'ancien mot de passe xnet
QString RzxConfig::oldPass()
{
	QString i = global() -> value(/*RzxServerListener::object()->getServerIP().toString() + */"/oldpass").toString();
	if(i.isEmpty()) i = QString::null;
	return i;
}


///Emet un signal pour informer du changement de format des ic�nes
void RzxConfig::emitIconFormatChanged()
{
	emit global()->iconFormatChange();
}


/******************************************************************************
* INFORMATIONS CONCERNANT LA MACHINE                                          *
******************************************************************************/
///Commentaire de l'ordinateur
QString RzxConfig::remarque()
{
	QString comment = global()->value("comment", "$#x").toString();

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
void RzxConfig::setAutoResponder(bool val)
{
	RzxComputer::localhost()->setState(val);
}

///Lit le mode du r�pondeur
bool RzxConfig::autoResponder()
{
	return RzxComputer::localhost()->isOnResponder();
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
