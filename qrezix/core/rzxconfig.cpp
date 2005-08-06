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
#include <QPixmap>
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

#include "rzxconfig.h"

#include "defaults.h"

#include "rzxmessagebox.h"
#include "rzxhostaddress.h"
#include "rzxcomputer.h"
#include "rzxserverlistener.h"
#include "rzxpluginloader.h"
#include "rzxrezalmodel.h"
#include "rzxiconcollection.h"

RzxConfig *RzxConfig::Config = 0;
const QString RzxConfig::logPath("log");
QHash<QString,QTranslator*> RzxConfig::translations;
QTranslator *RzxConfig::currentTranslator=NULL;

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
	loadTranslatorsInDir(m_userDir);
	loadTranslatorsInDir(m_systemDir);
	loadTranslatorsInDir(m_libDir);

	qDebug("Loading translation...");
	setLanguage(language());
	Rzx::endModuleLoading("Translations");
}

///Chargement des traductions contenues dans le répertoire indiqué
void RzxConfig::loadTranslatorsInDir(const QDir &rep)
{
	QDir sourceDir(rep);
	if(!sourceDir.cd("translations"))
		return;
	
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
/** Contrairement à language qui retourne la langue demandée par l'utilisateur
 * cette fonction retourne le nom de la langue actuellement chargée... ce qui
 * peut être différent dans certaines conditions.
 */
QString RzxConfig::translation()
{
	return tr("English");
}

///Sélection de la langue à utiliser
void RzxConfig::setLanguage(const QString& language)
{
	if(language != translation() && translations.keys().contains(language))
	{
		QApplication::removeTranslator(currentTranslator);
		currentTranslator = translations[language];
		QApplication::installTranslator(currentTranslator);
		emit Config->languageChanged();
	}
	qDebug("Language set to %s", tr("English").toLatin1().constData());
}

/**
*/
RzxConfig::RzxConfig()
	: QObject()
{
	Rzx::beginModuleLoading("Config");
	
	if(!Config) Config=this;
	settings = new QSettings("BR", "qRezix");

	Rzx::beginModuleLoading("Config Path");
#ifdef WIN32
	QString dir = settings->value("InstDir").toString();
	m_userDir = QDir::currentDirPath();
	if(!dir.isEmpty())
		m_systemDir.setPath(dir);
	else
		m_systemDir = m_userDir;
	m_libDir = m_systemDir;
#else
#ifdef Q_OS_MAC
	m_systemDir.setPath(QREZIX_DATA_DIR);
	m_userDir.setPath(QREZIX_DATA_DIR);
	m_libDir.setPath(QREZIX_DATA_DIR);
#else
	m_systemDir.setPath(QREZIX_DATA_DIR);
	m_libDir.setPath(QREZIX_LIB_DIR);
	m_userDir = QDir::home();
	if (!m_userDir.cd(".rezix"))
	{
		if (!m_userDir.mkdir(".rezix"))
		{
			QString msg;
			msg = tr("qRezix cannot create %1, which is the folder in which its configuration is saved\n")
				.arg(m_userDir.absoluteFilePath(".rezix"));
			msg += tr("You will not be able to save your configuration");
			RzxMessageBox::critical(0, "qRezix", msg);
		}
		else
			m_userDir.cd(".rezix");
	}
	
	if (!m_systemDir.exists())
		m_systemDir = m_userDir;

	if(!m_libDir.exists())
		m_libDir = m_systemDir;
#endif //MC
#endif //WIN32
	qDebug("Personnal path set to %s", m_userDir.path().toAscii().constData());
	qDebug("System path set to %s", m_systemDir.path().toAscii().constData());
	qDebug("Libraries path set to %s", m_libDir.path().toAscii().constData());
	Rzx::endModuleLoading("Config Path");

	//Initialisation du générateur de nombre aléatoire...
	//Même si qRezix l'utilise peu, ça peut toujours être utile en particulier pour les plugins
	srand(time(0));

	//Ouverture du fichier de configuration
	readFavorites();
	readIgnoreList();

	//CHargment de données diverses (icons, traductions)
	loadTranslators();
	RzxIconCollection::global();
	
	//Chargement des données QVB sur les fontes du système
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
	Rzx::endModuleLoading("Config");
}

/**
*/
RzxConfig::~RzxConfig()
{
	writeFavorites();
	writeIgnoreList();
	settings->sync();
	if(settings)
		delete settings;
	fontProperties.clear();
	translations.clear();
	Config = NULL;
}

/**
*/
RzxConfig * RzxConfig::global() {
	if (!Config)
		Config = new RzxConfig();
	return Config;
}


RzxConfig::FontProperty::FontProperty(bool b, bool i, const QList<int> &pS)
			: bold(b), italic(i), sizes(pS) {
}

RzxConfig::FontProperty::~FontProperty() {
}

/*****************************************************************************
* REPERTOIRES DES DONNEES																	  *
*****************************************************************************/
/** Recherche si la configuration a déjà été réalisée */
bool RzxConfig::find()
{
	if(global() -> readEntry("firstload", 1))
	{
		global() -> writeEntry("firstload", 0);
		return false;
	}

	return true;
}

QDir RzxConfig::userDir() { return global() -> m_userDir; }
QDir RzxConfig::systemDir() { return global() -> m_systemDir; }
QDir RzxConfig::libDir() { return global() -> m_libDir; }

QDir RzxConfig::computerIconsDir() {
	QDir ret(global() -> m_userDir);
	if (!ret.cd("icones")) {
		ret.mkdir(ret.absoluteFilePath("icones"));
		ret.cd("icones");
	}
	return ret;
}

QDir RzxConfig::logDir() {
	QDir ret (global() -> m_userDir);
	if (!ret.cd("log")) {
		ret.mkdir(ret.absoluteFilePath("log")); 
		ret.cd("log");
	}
	return ret;
}

/******************************************************************************
* FONCTIONS DE LECTURE GENERALES																*
******************************************************************************/
QString RzxConfig::readEntry(const QString& name, const QString& def) {
	if(!settings) return def;
	return settings->value("general/" + name, def).toString();
}

int RzxConfig::readEntry(const QString& name, int def) {
	if(!settings) return def;
	return settings->value("general/" + name, def).toInt();
}

QStringList RzxConfig::readEntry(const QString& name) {
	if(!settings) return QStringList();
	return settings->value("general/" + name).toStringList();
}

void RzxConfig::writeEntry(const QString& name, const QString& val) {
	if(!settings) return;
	settings->setValue("general/" + name, val);
	if (name == "localhost")
		RzxComputer::buildLocalhost();
}

void RzxConfig::writeEntry(const QString& name, int val) {
	if(!settings) return;
	settings->setValue("general/" + name, val);
}

void RzxConfig::writeEntry(const QString& name, const QStringList& list) {
	if(!settings) return;
	settings->setValue("general/" + name, list);
}

void RzxConfig::flush()
{
	if(settings) settings->sync();
}

/******************************************************************************
* FONCTIONS DE LECTURE DES ENTREES															*
******************************************************************************/

/** Renvoie la liste des familles de fonte initialisée au début */
QStringList RzxConfig::getFontList() {	return fontFamilies; }

/** Renvoie la liste des tailles acceptées par cette police */
const QList<int> RzxConfig::getSizes(const QString& family) const {
	const FontProperty &fp = fontProperties[family];
	return fp.sizes;
}

bool RzxConfig::isItalicSupported(const QString& family) const {
	const FontProperty &fp = fontProperties[family];
	return fp.italic;
}

bool RzxConfig::isBoldSupported(const QString& family) const {
	const FontProperty &fp = fontProperties[family];
	return fp.bold;
}

/** Renvoie une chaine correspondant ï¿½la taille de la fenï¿½re principale*/
QString RzxConfig::readWindowSize(){return global() -> readEntry("window_size",DEFAULT_WINDOWSIZE); }

/** Renvoie une chaine correspondant ï¿½la taille de la fenï¿½re principale*/
void RzxConfig::writeWindowSize(QString ws){global() -> writeEntry("window_size",ws); }

/** Renvoie le point où devrait se placer la fenêtre */
QPoint RzxConfig::readWindowPosition()
{
	QString pos = global() -> readEntry("windowPosition", "00020024");
	return QPoint(pos.left(4).toInt(), pos.right(4).toInt());
}

/** Ecrie la chaine correspondant à la position de la fenêtre */
void RzxConfig::writeWindowPosition(const QPoint &position)
{
	QString pos = QString("%1%2").arg(position.x(), 4).arg(position.y(), 4).replace(' ', '0');
	global() -> writeEntry("windowPosition", pos);
}

/** Renvoie le temps de reconnection en ms, 0 pour pas de reconnection */
int RzxConfig::reconnection(){ return global() -> readEntry("reconnection", DEFAULT_RECONNECTION); }

/** Renvoie le port de chat */
int RzxConfig::chatPort(){ return global() -> readEntry("chat_port", DEFAULT_CHATPORT); }

/** Renvoie le port de communication avec le serveur */
int RzxConfig::serverPort(){ return global() -> readEntry("server_port", DEFAULT_PORT); }

/** Renvoie le port de communication avec le serveur */
int RzxConfig::pingTimeout(){ return global() -> readEntry("ping_timeout", DEFAULT_TIMEOUT); }

/** Renvoie le port de communication avec le serveur */
QString RzxConfig::serverName(){ return global() -> readEntry("server_name", DEFAULT_SERVER); }

/** Renvoie le rle du double clic (0 pour Chat, 1 pour FTP)*/
int RzxConfig::doubleClicRole(){ return global() -> readEntry("doubleClic", 0); }

/** Renvoie l'état de tooltips à utiliser */
int RzxConfig::tooltip(){ return global()->readEntry("tooltip", 0); }

/** Renvoie si on utilise le systray ou non */
#ifndef Q_OS_MAC
int RzxConfig::useSystray(){ return global() -> readEntry("useSystray", 1); }
#else
int RzxConfig::useSystray(){ return global() -> readEntry("useSystray", 0); }
#endif

/** Renvoie la taille de la tray icon */
int RzxConfig::traySize() { return global()->readEntry("traysize", 22); }

int RzxConfig::useSearch(){ return global() -> readEntry("useSearch", 1); }
int RzxConfig::defaultTab(){ return global() -> readEntry("defaultTab", 1); }
/* Renvoie si une boite d'info doit etre ouverte quand qqun checke nos propriétés ou pas */
int RzxConfig::warnCheckingProperties() { return global()-> readEntry("warnCheckingProperties", 0); }

/* Renvoie si l'heure doit etre affichée devant chaque message dans la fenetre de chat */
int RzxConfig::printTime() {return global()-> readEntry("printTime", 1);}

/** Renvoie si on est beepï¿½ï¿½chaque fois qu'on reï¿½it un message */
int RzxConfig::beep(){ return global() -> readEntry("beep", 0); }
QString RzxConfig::beepCmd(){ return global() -> readEntry("txtBeepCmd", "play"); }
QString RzxConfig::beepSound(){ return global() -> readEntry("txtBeep", ""); }

/** pour les beeps à la connection des autres clients (favoris) */
int RzxConfig::beepConnection(){ return global() -> readEntry("beepConnection", 0); }
QString RzxConfig::connectionSound(){ return global() -> readEntry("txtBeepConnection", ""); }

/** Affichage de notifications pour le changement d'état d'autres clients */
bool RzxConfig::showConnection() { return global()->readEntry("showConnection", 1); }

#ifdef WIN32
QString RzxConfig::sambaCmd(){ return global() -> readEntry("samba_cmd", "standard");}
QString RzxConfig::ftpCmd(){ return global() -> readEntry("ftp_cmd", "standard"); }
QString RzxConfig::httpCmd(){ return global() -> readEntry("http_cmd", "standard"); }
QString RzxConfig::newsCmd(){ return global() -> readEntry("newsCmd", "standard"); }
#else
#ifdef Q_OS_MAC
QString RzxConfig::sambaCmd(){ return global() -> readEntry("samba_cmd", "open"); }
QString RzxConfig::ftpCmd(){ return global() -> readEntry("ftp_cmd", "Default"); }
QString RzxConfig::httpCmd(){ return global() -> readEntry("http_cmd", "Default"); }
QString RzxConfig::newsCmd(){ return global() -> readEntry("newsCmd", "Default"); }
#else
QString RzxConfig::sambaCmd(){ return global() -> readEntry("samba_cmd", "standard"); }
QString RzxConfig::ftpCmd(){ return global() -> readEntry("ftp_cmd", "gftp"); }
QString RzxConfig::httpCmd(){ return global() -> readEntry("http_cmd", "konqueror"); }
QString RzxConfig::newsCmd(){ return global() -> readEntry("newsCmd", "knode"); }
#endif //MAC
#endif //WIN32

void RzxConfig::sambaCmd(QString newstr){ global() -> writeEntry("samba_cmd",newstr);}
void RzxConfig::ftpCmd(QString newstr)  { global() -> writeEntry("ftp_cmd", newstr); }
void RzxConfig::httpCmd(QString newstr) { global() -> writeEntry("http_cmd",newstr); }
void RzxConfig::newsCmd(QString newstr) { global() -> writeEntry("newsCmd", newstr); }
void RzxConfig::setPass(const QString& passcode)
{
	global() -> writeEntry(RzxServerListener::object()->getServerIP().toString() + "/pass", passcode);
	global() -> writeEntry("pass", passcode);
	global() -> flush();
}
void RzxConfig::setOldPass(const QString& oldPass)
{
	global() -> writeEntry(RzxServerListener::object()->getServerIP().toString() + "/oldpass", oldPass);
	global() -> flush();
}

// Configuration de base de l'ordinateur
QString RzxConfig::dnsname() { return global()->readEntry("dnsname", QString()); }
QString RzxConfig::remarque()
{
	QString comment = global()->readEntry("comment", "$#x");

	if(comment == "$#x")
	{
		QStringList comments = QString(DEFAULT_COMMENT).split("\n");
		int i = rand()%comments.size();
		comment = comments[i];
	}

	return comment;
}
Rzx::Promal RzxConfig::promo() { return (Rzx::Promal)global()->readEntry("promo", Rzx::PROMAL_UNK); }
Rzx::ConnectionState RzxConfig::repondeur() { return (Rzx::ConnectionState)global()->readEntry("repondeur", Rzx::STATE_HERE); }
QFlags<RzxComputer::ServerFlags> RzxConfig::servers() { return RzxComputer::toServerFlags(global()->readEntry("servers", 0)); }

QString RzxConfig::propLastName(){ return global() -> readEntry("txtFirstname", "");}
QString RzxConfig::propName(){ return global() -> readEntry("txtName", "");}
QString RzxConfig::propSurname(){ return global() -> readEntry("txtSurname", "");}
QString RzxConfig::propTel(){ return global() -> readEntry("txtPhone", "");}
QString RzxConfig::propSport(){ return global() -> readEntry("txtSport", "");}
int RzxConfig::numSport(){ return global() -> readEntry("numSport", 0);}
QString RzxConfig::propMail(){ return global() -> readEntry("txtMail", "");}
QString RzxConfig::propWebPage(){ return global() -> readEntry("txtWeb", "");}
QString RzxConfig::propCasert(){ return global() -> readEntry("txtCasert", "");}
QString	RzxConfig::iconTheme(){ return global() -> readEntry("theme", QString()); }
QString	RzxConfig::FTPPath(){ return global() -> readEntry("FTPPath", "");}

int RzxConfig::menuTextPosition() { return global() ->readEntry("menuTextPos", 2); }
int RzxConfig::menuIconSize() { return global() ->readEntry("menuIconSize", 2); }
QStringList RzxConfig::ignoredPluginsList() { return global()->readEntry("ignoredPlugins"); }

QString RzxConfig::propPromo() { return RzxComputer::localhost()->promoText(); }
int RzxConfig::quitMode(){ return global() -> readEntry("quitmode", 1); }
bool RzxConfig::showQuit(){ return global() -> readEntry("showquit", true); }
bool RzxConfig::refuseWhenAway() { return global() -> readEntry("refuseAway", true); }
void RzxConfig::writeQuitMode(int mode){ global() -> writeEntry("quitmode", mode); }
void RzxConfig::writeShowQuit(bool mode){ global() -> writeEntry("showquit", mode); }
void RzxConfig::writeIgnoredPluginsList(const QStringList& list) { global()->writeEntry("ignoredPlugins", list); }

/** Renvoie le password xnet */
QString RzxConfig::pass()
{
	QString i = global() -> readEntry(RzxServerListener::object()->getServerIP().toString() + "/pass", QString::null);
	if(i.isNull()) //Pour la compatibilité avec les anciennes formes de stockage sous nux
		i = global() -> readEntry("pass", QString::null);
	return i;
}
QString RzxConfig::oldPass()
{
	QString i = global() -> readEntry(RzxServerListener::object()->getServerIP().toString() + "/oldpass", QString::null);
	if(i.isEmpty()) i = QString::null;
	return i;
}

/** Renvoie la variable correspondant aux colonnes à afficher */
int RzxConfig::colonnes() {
	uint colonnes = 0;
	colonnes |= (1<<RzxRezalModel::ColNom);
	colonnes |= (1<<RzxRezalModel::ColRemarque);
	colonnes |= (1<<RzxRezalModel::ColFTP);
	colonnes |= (1<<RzxRezalModel::ColHTTP);
	colonnes |= (1<<RzxRezalModel::ColNews);
	colonnes |= (1<<RzxRezalModel::ColOS);
	colonnes |= (1<<RzxRezalModel::ColPromo);
	colonnes |= (1<<RzxRezalModel::ColRezal);
	return global()->readEntry("colonnes", colonnes);
}

///Indique si la totalité des informations sont enregistrées
bool RzxConfig::infoCompleted()
{
	return propLastName() != "" && propName() != "" && propCasert() != "" 
		&& propMail() != DEFAULT_MAIL && propTel() != "";
}

/** Change le mode du répondeur */
void RzxConfig::setAutoResponder(bool val) { RzxComputer::localhost()->setState(val); }

/** Lit le mode du répondeur */
bool RzxConfig::autoResponder() { return RzxComputer::localhost()->isOnResponder(); }

/** Renvoie le message du répondeur */
QString RzxConfig::autoResponderMsg(){ return global() -> readEntry("txtAutoResponderMsg", "Rï¿½ondeur automatique");}

/** Renvoie la taille des icones des utilsiateurs
 * 0 pour 32x32
 * 1 pour 64x64 */
int RzxConfig::computerIconSize(){ return global() -> readEntry("iconsize", 1); }

///Indique s'il faut highlighté le computer sélectionné
bool RzxConfig::computerIconHighlight() { return global() -> readEntry("iconhighlight", true); }

///Retourne le language actuel
QString RzxConfig::language() { return global() -> readEntry("language", "English"); }

void RzxConfig::closeSettings()
{
	delete settings;
	settings = NULL;
}

QColor RzxConfig::repondeurHighlight() {
	QColor ret;
	ret.setRgb(global() -> readEntry("repondeur_highlight", 0xFD3D3D));
	return ret;
}

QColor RzxConfig::repondeurBase() {
	QColor ret;
	ret.setRgb(global() -> readEntry("repondeur_base", 0xFFEE7C));
	return ret;
}

QColor RzxConfig::repondeurHighlightedText() {
	QColor ret;
	ret.setRgb(global() -> readEntry("repondeur_highlightedtext", 0xFFFFFF));
	return ret;
}

QColor RzxConfig::repondeurNormalText() {
	QColor ret;
	ret.setRgb(global() -> readEntry("repondeur_normaltext", 0x000000));
	return ret;
}

QColor RzxConfig::ignoredBGColor() {
	QColor ret;
	ret.setRgb(global() -> readEntry("ignoredBGColor", 0xCCCCCC));
	return ret;
}

QColor RzxConfig::ignoredText() {
	QColor ret;
	ret.setRgb(global() -> readEntry("ignoredtext", 0xAAAAAA));
	return ret;
}

/** No descriptions */
QColor RzxConfig::errorBackgroundColor(){
	QColor ret;
	ret.setRgb(global() -> readEntry("error_back", 0xFF0000));
	return ret;
}

QColor RzxConfig::errorTextColor(){
	QColor ret;
	ret.setRgb(global() -> readEntry("error_back", 0xFFFFFF));
	return ret;
}

QString RzxConfig::historique(quint32 ip, const QString& hostname) {
	QString filename = QString::number(ip, 16);
	
	QDir logdir = global() -> logDir();
	filename = filename + "[" + hostname + "].html";
	
	if (logdir.exists(filename)) return logdir.absoluteFilePath(filename);
		
	// anciens formats
	QString olds[2];
	olds[0] = QString::number(ip, 16) + "[" + hostname + "].txt";
	olds[1] = QString::number(ip, 16) + ".txt";
	for (int oldidx = 0; oldidx < 2; oldidx++) {
		QString& prevFileName = olds[oldidx];
		if (logdir.exists(prevFileName)) {
			logdir.rename(prevFileName, filename);
			break;
		}
	}
	
	return logdir.absoluteFilePath(filename);
}


/******************************************************************************
* GESTION DES FAVORIS                                                         *
******************************************************************************/
void RzxConfig::readFavorites()
{
	/* Pour que la compatiblité avec l'ancien format soit maintenue
	on lit ce fichier, et on le supprime, sachant que tout ira dans
	le nouveau fichier à l'issue */
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
		file.remove(); //le fichier n'est désormais plus nécessaire
							//tout va dans le qrezixrc
		writeFavorites();
	}
	
	/* La nouvelle technique... les favoris sont maintenant une entrée dans
	le fichier de config sous forme de liste */
	else
	{
		QStringList favoriteList = readEntry("favorites");
		QStringList::iterator it;
		for(it = favoriteList.begin() ; it != favoriteList.end() ; it++)
			addToFavorites(*it);
	}
}

///Indique si l'entrée est dans les favoris
bool RzxConfig::isFavorite(const QString& nom) const {
	return favorites.contains(nom);
}

///Idem encore mais à partir d'un RzxComputer
bool RzxConfig::isFavorite(const RzxComputer& computer) const {
	return isFavorite(computer.name());
}

///Ajout du pseudo à la liste des favoris
void RzxConfig::addToFavorites(const QString& nom) {
	favorites.insert(nom);
}

///Idem à partir du RzxComputer
void RzxConfig::addToFavorites(const RzxComputer& computer) {
	favorites.insert(computer.name());
}

///Suppression du pseudo de la liste des favoris
void RzxConfig::delFromFavorites(const QString& nom) {
	favorites.remove(nom);
}

///Idem à partir du RzxComputer
void RzxConfig::delFromFavorites(const RzxComputer& computer) {
	favorites.remove(computer.name());
}

void RzxConfig::writeFavorites()
{
	writeEntry("favorites", favorites.values());
}

/******************************************************************************
* GESTION DE L'IGNORE LIST                                                    *
******************************************************************************/
void RzxConfig::readIgnoreList()
{
	/* Pour que la compatiblité avec l'ancien format soit maintenue
	on lit ce fichier, et on le supprime, sachant que tout ira dans
	le nouveau fichier à l'issue */
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
		file.remove(); //le fichier n'est désormais plus nécessaire
							//tout va dans le qrezixrc
		writeIgnoreList();
	}
	
	/* La nouvelle technique... les favoris sont maintenant une entrée dans
	le fichier de config sous forme de liste */
	else
	{
		QStringList ignoreListList =  readEntry("ignoreList");
		QStringList::iterator it;
		for(it = ignoreListList.begin() ; it != ignoreListList.end() ; it++)
			addToBanlist(*it);
	}
}

///Indique si l'ip donnée est bannée
bool RzxConfig::isBan(const QString& ip) const {
	return ignoreList.contains(ip);
}

///Idem à partir d'un QHostAddress - RzxHostAddress
bool RzxConfig::isBan(const QHostAddress& ip) const {
	return isBan(ip.toString());
}

///Idem à partir d'un RzxComputer
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
	writeEntry("ignoreList", ignoreList.values());
}


/******************************************************************************
* GESTION DE LA CACHE DES PROPRIÉTÉS
******************************************************************************/
void RzxConfig::addCache(const RzxHostAddress& address, const QString& msg)
{
	global() -> writeEntry("cacheprop-" + address.toString(), msg);
	global() ->writeEntry("cachedate-" + address.toString(),  QDate::currentDate().toString("dd MMM yyyy"));
}

QString RzxConfig::cache(const RzxHostAddress& address)
{
	return global()->readEntry("cacheprop-" + address.toString(), QString::null);
}

QString RzxConfig::getCacheDate(const RzxHostAddress& address)
{
	return global()->readEntry("cachedate-" + address.toString(), QString::null);
}
