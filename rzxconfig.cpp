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

#ifndef WIN32
	#include <math.h>
#endif

#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qfontdatabase.h>
#include <qapplication.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <QList>
#include <QTranslator>

#ifndef WIN32
#include <unistd.h>
#endif

#ifdef HAVE_CONFIG_H
#include "../config.h"
#else
#ifdef Q_OS_MAC
#define QREZIX_DATA_DIR "./qrezix.app/Contents/Resources/"
#else
#define QREZIX_DATA_DIR "/usr/share/qrezix"
#define QREZIX_LIB_DIR "/usr/lib/qrezix"
#endif
#endif

#ifdef WIN32
#include <windows.h>
#define RZXCHAT_IMPL_H
#define RZXCLIENTLISTENER_H
class RzxChat;
class RzxClientListener;
class RzxChatSocket;
#endif

#include "rzxmessagebox.h"
#include "rzxhostaddress.h"
#include "rzxconfig.h"
#include "rzxcomputer.h"
#include "qrezix.h"
#include "rzxrezal.h"
#include "rzxitem.h"
#include "rzxserverlistener.h"
#include "rzxpluginloader.h"
#include "defaults.h"

//Obsolete, seulement pour la compatibilité avec qRezix < 1.5
#define CONF_FAVORITES "favorites.conf"
#define CONF_IGNORELIST "ignorelist.conf"


RzxConfig *RzxConfig::Config = 0;
const QString RzxConfig::logPath("log");
const QString RzxConfig::themePath("themes");
QHash<QString,QTranslator*> RzxConfig::translations;
QTranslator *RzxConfig::currentTranslator=NULL;

void RzxConfig::loadTranslators(){
	if(currentTranslator) qApp->removeTranslator(currentTranslator);
	currentTranslator=NULL;
	translations.clear();
	loadTranslatorsInDir(m_userDir);
	loadTranslatorsInDir(m_systemDir);
}

void RzxConfig::loadTranslatorsInDir(const QDir &rep) {
	QDir sourceDir(rep);
	if(!sourceDir.cd("translations"))
	{
		qDebug(QString("Cannot cd to %1/translations").arg(sourceDir.canonicalPath()));
		return;
	}
	
	sourceDir.setNameFilter("*.qm");
	QStringList trans=sourceDir.entryList(QDir::Files|QDir::Readable);
	QString trNames = QString::null;
	for(QStringList::Iterator it=trans.begin(); it!=trans.end(); ++it){
		QTranslator *cur = new QTranslator;
		cur->load(*it, sourceDir.path());
		QString lang = cur->translate("RzxConfig", "English");
		if(!translations[lang])
		{
			translations.insert(lang, cur);
			trNames += " " + lang;
		}
	}
	if(trNames.isNull()) trNames = " <none>";
	qDebug(QString("Translations available in %1 :%2").arg(sourceDir.absPath()).arg(trNames));
}
		
void RzxConfig::setLanguage(QString language){
	if(language != tr("English")){
		if(currentTranslator) qApp->removeTranslator(currentTranslator);
		currentTranslator=NULL;
		if(language != "English"){
			currentTranslator = translations[language];
			qApp->installTranslator(currentTranslator);
		}
		emit Config->languageChanged();
	}
}

/**
*/
RzxConfig::RzxConfig()
	: QObject(), computer(NULL) {
	qDebug("=== Loading config ===");
	
	if(!Config) Config=this;
	settings = new QSettings();

#ifdef WIN32
	QString dir = settings->readEntry("/qRezix/InstDir");
	m_userDir = QDir::currentDirPath();
	if(dir)
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
	if (!m_userDir.cd(".rezix")) {
		if (!m_userDir.mkdir(".rezix")) {
			QString msg;
			msg = tr("qRezix cannot create %1, which is the folder in which its configuration is saved\n")
				.arg(m_userDir.absFilePath(".rezix"));
			msg += tr("You will not be able to save your configuration");
			RzxMessageBox::critical(0, "qRezix", msg);
		} else {
			m_userDir.cd(".rezix");
		}
	}
	
	if (!m_systemDir.exists()) {
		qWarning(tr("%s doesn't exist"), QString(QREZIX_DATA_DIR).latin1());
		m_systemDir = m_userDir;
	}

	if(!m_libDir.exists()) {
		qWarning(tr("%s doesn't exist"), QString(QREZIX_LIB_DIR).latin1());
		m_libDir = m_systemDir;
	}
#endif //MC
#endif //WIN32
	qDebug("Personnal path set to "+m_userDir.path());
	qDebug("System path set to "+m_systemDir.path());
	qDebug("Libraries path set to "+m_libDir.path());

	//Initialisation du générateur de nombre aléatoire...
	//Même si qRezix l'utilise peu, ça peut toujours être utile en particulier pour les plugins
	srand(time(0));

	//Ouverture du fichier de configuration
	settings->insertSearchPath(QSettings::Unix,m_userDir.canonicalPath());
	readFavorites();
	readIgnoreList();

	//CHargment de données diverses (icons, traductions)
	loadTranslators();
	QString lg = readEntry("language", "English");
	setLanguage(lg);
	qDebug("Language is set to "+ lg);
	
	qDebug("Loading theme");
	QString theme = findData("action.png",  themePath + "/" + iconTheme());
	if(theme.isNull())
	{
		qDebug("User's default theme not found, trying with " + iconTheme(false));
		theme = findData("action.png",  themePath + "/" + iconTheme(false));
		if(!theme.isNull())
			setIconTheme(NULL, iconTheme(false)); //NULL pour qu'on ne tente pas de changer les icônes dans un qRezix non encore chargé
	}
	if(theme.isNull())
	{
		qDebug("No icons theme available");
		QMessageBox::critical(NULL, tr("No icons theme"), tr("qRezix was unable to find a usable icons theme.\nPlease, check you installation"), false);
	}
	else
	{
		qDebug("Trying to open theme from " + theme);
	}

	//Chargement des données QVB sur les fontes du système
	QFontDatabase fdb;
	fontFamilies = fdb.families();
	for ( QStringList::Iterator f = fontFamilies.begin(); f != fontFamilies.end();) {
		QString family = *f;
		QStringList styles = fdb.styles( family );
		if(styles.contains("Normal")!=0) {
			QList<int> size = fdb.smoothSizes(family, "Normal");
			bool b = styles.contains("Bold")!=0;
			bool i = styles.contains("Italic")!=0 || styles.contains("Oblique")!=0;
			FontProperty fp( b, i, size);
			fontProperties.insert(family, fp);
			++f;
		}
		else {
			f=fontFamilies.remove(f);
		}
	}
	qDebug(QString("Found %1 fonts families").arg(fontFamilies.count()));
	qDebug("=== Config loaded ===\n");
}

/**
*/
RzxConfig::~RzxConfig(){
	if(settings)
		delete settings;
	fontProperties.clear();
	translations.clear();
	allIcons.clear();
	progIcons.clear();
	fileEntries.clear();
	if(computer)
		delete computer;
	Config = NULL;
}

/**
*/
RzxConfig * RzxConfig::globalConfig() {
	if (!Config) {
		Config = new RzxConfig();
	}
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
	if(globalConfig() -> readEntry("firstload", 1))
	{
		globalConfig() -> writeEntry("firstload", 0);
		return false;
	}

	return true;
}

QDir RzxConfig::userDir() { return globalConfig() -> m_userDir; }
QDir RzxConfig::systemDir() { return globalConfig() -> m_systemDir; }
QDir RzxConfig::libDir() { return globalConfig() -> m_libDir; }

QDir RzxConfig::computerIconsDir() {
	QDir ret (globalConfig() -> m_userDir);
	if (!ret.cd("icones")) {
		ret.mkdir(ret.absFilePath("icones"));
		ret.cd("icones");
	}
	return ret;
}

QDir RzxConfig::logDir() {
	QDir ret (globalConfig() -> m_userDir);
	if (!ret.cd("log")) {
		ret.mkdir(ret.absFilePath("log")); 
		ret.cd("log");
	}
	return ret;
}

QString RzxConfig::findData(const QString& name, const QString& relative, bool important) {
	QDir temp(globalConfig() -> m_userDir);
	temp.cd(relative);
	if (temp.exists(name)) return temp.absFilePath(name);
	
	temp = globalConfig() -> m_systemDir;
	temp.cd(relative);
	if (temp.exists(name)) return temp.absFilePath(name);
	
	if(important)
		qWarning(tr("qRezix can't find  %s/%s"), relative.local8Bit().data(), name.local8Bit().data());
	return QString::null;
}

/******************************************************************************
* FONCTIONS DE LECTURE GENERALES																*
******************************************************************************/
QString RzxConfig::readEntry(const QString& name, const QString& def) {
	if(!settings) return def;
	return settings->readEntry("/qRezix/general/" + name, def);
}

int RzxConfig::readEntry(const QString& name, int def) {
	if(!settings) return def;
	return settings->readNumEntry("/qRezix/general/" + name, def);
}

QStringList RzxConfig::readEntry(const QString& name) {
	if(!settings) return QStringList();
	return settings->readListEntry("/qRezix/general/" + name);
}

void RzxConfig::writeEntry(const QString& name, const QString& val) {
	if(!settings) return;
	settings->writeEntry("/qRezix/general/" + name, val);
	if (name == "localhost")
		loadLocalHost();
}

void RzxConfig::writeEntry(const QString& name, int val) {
	if(!settings) return;
	settings->writeEntry("/qRezix/general/" + name, val);
}

void RzxConfig::writeEntry(const QString& name, const QStringList& list) {
	if(!settings) return;
	settings->writeEntry("/qRezix/general/" + name, list);
}

void RzxConfig::flush()
{
	if(settings) delete settings;
	settings = new QSettings();
	settings->insertSearchPath(QSettings::Unix,m_userDir.canonicalPath());
	RzxPlugInLoader::global()->setSettings();
}

const QPixmap &RzxConfig::icon(const QString& name) {
	RzxConfig * config = globalConfig();
	return icon(name, config -> allIcons);
}

const QPixmap &RzxConfig::themedIcon(const QString& name) {
	RzxConfig * config = globalConfig();
	return icon(name, config -> progIcons, themePath + "/" + iconTheme());
}

const QPixmap &RzxConfig::icon(const QString& name, QHash<QString,QPixmap>& cache, const QString& subdir) {
	RzxConfig *config = globalConfig();
	QString qualifiedName = subdir.isNull() ? name : (subdir + "_" + name);
	QPixmap &ret = cache[qualifiedName];
	if(!ret.isNull()) return ret;
	
	QString fileName = config -> findData(name + ".png", subdir);
	if (fileName.isNull())
	{
		qDebug("Icon "+name+" not found");
		return ret;
	}
	ret.load(fileName);
	cache.insert(qualifiedName, ret);
	return cache[qualifiedName];
}

void RzxConfig::saveIcon(const QString& name, const QPixmap& image){
	RzxConfig *cfgObject = globalConfig();
	cfgObject->allIcons.remove(name);	
	image.save(cfgObject -> m_userDir.absFilePath(name + ".png"), "PNG");
	cfgObject->allIcons.insert(name, QPixmap(image));
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
QString RzxConfig::readWindowSize(){return globalConfig() -> readEntry("window_size",DEFAULT_WINDOWSIZE); }

/** Renvoie une chaine correspondant ï¿½la taille de la fenï¿½re principale*/
void RzxConfig::writeWindowSize(QString ws){globalConfig() -> writeEntry("window_size",ws); }

/** Renvoie le point où devrait se placer la fenêtre */
QPoint RzxConfig::readWindowPosition()
{
	QString pos = globalConfig() -> readEntry("windowPosition", "00020024");
	return QPoint(pos.left(4).toInt(), pos.right(4).toInt());
}

/** Ecrie la chaine correspondant à la position de la fenêtre */
void RzxConfig::writeWindowPosition(const QPoint &position)
{
	QString pos = QString("%1%2").arg(position.x(), 4).arg(position.y(), 4).replace(' ', '0');
	globalConfig() -> writeEntry("windowPosition", pos);
}

/** Renvoie le temps de reconnection en ms, 0 pour pas de reconnection */
int RzxConfig::reconnection(){ return globalConfig() -> readEntry("reconnection", DEFAULT_RECONNECTION); }

/** Renvoie le port de chat */
int RzxConfig::chatPort(){ return globalConfig() -> readEntry("chat_port", DEFAULT_CHATPORT); }

/** Renvoie le port de communication avec le serveur */
int RzxConfig::serverPort(){ return globalConfig() -> readEntry("server_port", DEFAULT_PORT); }

/** Renvoie le port de communication avec le serveur */
int RzxConfig::pingTimeout(){ return globalConfig() -> readEntry("ping_timeout", DEFAULT_TIMEOUT); }

/** Renvoie le port de communication avec le serveur */
QString RzxConfig::serverName(){ return globalConfig() -> readEntry("server_name", DEFAULT_SERVER); }

/** Renvoie le rle du double clic (0 pour Chat, 1 pour FTP)*/
int RzxConfig::doubleClicRole(){ return globalConfig() -> readEntry("doubleClic", 0); }

/** Renvoie l'état de tooltips à utiliser */
int RzxConfig::tooltip(){ return globalConfig()->readEntry("tooltip", 0); }

/** Renvoie si on utilise le systray ou non */
#ifndef Q_OS_MAC
int RzxConfig::useSystray(){ return globalConfig() -> readEntry("useSystray", 1); }
#else
int RzxConfig::useSystray(){ return globalConfig() -> readEntry("useSystray", 0); }
#endif

/** Renvoie la taille de la tray icon */
int RzxConfig::traySize() { return globalConfig()->readEntry("traysize", 22); }

int RzxConfig::useSearch(){ return globalConfig() -> readEntry("useSearch", 1); }
int RzxConfig::defaultTab(){ return globalConfig() -> readEntry("defaultTab", 1); }
/* Renvoie si une boite d'info doit etre ouverte quand qqun checke nos propriétés ou pas */
int RzxConfig::warnCheckingProperties() { return globalConfig()-> readEntry("warnCheckingProperties", 0); }

/* Renvoie si l'heure doit etre affichée devant chaque message dans la fenetre de chat */
int RzxConfig::printTime() {return globalConfig()-> readEntry("printTime", 1);}

/** Renvoie si on est beepï¿½ï¿½chaque fois qu'on reï¿½it un message */
int RzxConfig::beep(){ return globalConfig() -> readEntry("beep", 0); }
QString RzxConfig::beepCmd(){ return globalConfig() -> readEntry("txtBeepCmd", "play"); }
QString RzxConfig::beepSound(){ return globalConfig() -> readEntry("txtBeep", ""); }

/** pour les beeps à la connection des autres clients (favoris) */
int RzxConfig::beepConnection(){ return globalConfig() -> readEntry("beepConnection", 0); }
QString RzxConfig::connectionSound(){ return globalConfig() -> readEntry("txtBeepConnection", ""); }

/** Affichage de notifications pour le changement d'état d'autres clients */
bool RzxConfig::showConnection() { return globalConfig()->readEntry("showConnection", 1); }

#ifdef WIN32
QString RzxConfig::sambaCmd(){ return globalConfig() -> readEntry("samba_cmd", "standard");}
QString RzxConfig::ftpCmd(){ return globalConfig() -> readEntry("ftp_cmd", "standard"); }
QString RzxConfig::httpCmd(){ return globalConfig() -> readEntry("http_cmd", "standard"); }
QString RzxConfig::newsCmd(){ return globalConfig() -> readEntry("newsCmd", "standard"); }
#else
#ifdef Q_OS_MAC
QString RzxConfig::sambaCmd(){ return globalConfig() -> readEntry("samba_cmd", "open"); }
QString RzxConfig::ftpCmd(){ return globalConfig() -> readEntry("ftp_cmd", "Default"); }
QString RzxConfig::httpCmd(){ return globalConfig() -> readEntry("http_cmd", "Default"); }
QString RzxConfig::newsCmd(){ return globalConfig() -> readEntry("newsCmd", "Default"); }
#else
QString RzxConfig::sambaCmd(){ return globalConfig() -> readEntry("samba_cmd", "standard"); }
QString RzxConfig::ftpCmd(){ return globalConfig() -> readEntry("ftp_cmd", "gftp"); }
QString RzxConfig::httpCmd(){ return globalConfig() -> readEntry("http_cmd", "konqueror"); }
QString RzxConfig::newsCmd(){ return globalConfig() -> readEntry("newsCmd", "knode"); }
#endif //MAC
#endif //WIN32

void RzxConfig::sambaCmd(QString newstr){ globalConfig() -> writeEntry("samba_cmd",newstr);}
void RzxConfig::ftpCmd(QString newstr)  { globalConfig() -> writeEntry("ftp_cmd", newstr); }
void RzxConfig::httpCmd(QString newstr) { globalConfig() -> writeEntry("http_cmd",newstr); }
void RzxConfig::newsCmd(QString newstr) { globalConfig() -> writeEntry("newsCmd", newstr); }
void RzxConfig::setPass(const QString& passcode)
{
	globalConfig() -> writeEntry(RzxServerListener::object()->getServerIP().toString() + "/pass", passcode);
	globalConfig() -> writeEntry("pass", passcode);
	globalConfig() -> flush();
}
void RzxConfig::setOldPass(const QString& oldPass)
{
	globalConfig() -> writeEntry(RzxServerListener::object()->getServerIP().toString() + "/oldpass", oldPass);
	globalConfig() -> flush();
}

QString RzxConfig::propLastName(){ return globalConfig() -> readEntry("txtFirstname", "");}
QString RzxConfig::propName(){ return globalConfig() -> readEntry("txtName", "");}
QString RzxConfig::propSurname(){ return globalConfig() -> readEntry("txtSurname", "");}
QString RzxConfig::propTel(){ return globalConfig() -> readEntry("txtPhone", "");}
QString RzxConfig::propSport(){ return globalConfig() -> readEntry("txtSport", "");}
int RzxConfig::numSport(){ return globalConfig() -> readEntry("numSport", 0);}
QString RzxConfig::propMail(){ return globalConfig() -> readEntry("txtMail", "");}
QString RzxConfig::propWebPage(){ return globalConfig() -> readEntry("txtWeb", "");}
QString RzxConfig::propCasert(){ return globalConfig() -> readEntry("txtCasert", "");}
QString	RzxConfig::iconTheme(bool def){
	if(def) return globalConfig() -> readEntry("theme", DEFAULT_THEME);
	return DEFAULT_THEME_HELP;
}
QString	RzxConfig::FTPPath(){ return globalConfig() -> readEntry("FTPPath", "");}

int RzxConfig::menuTextPosition() { return globalConfig() ->readEntry("menuTextPos", 2); }
int RzxConfig::menuIconSize() { return globalConfig() ->readEntry("menuIconSize", 2); }
QStringList RzxConfig::ignoredPluginsList() { return globalConfig()->readEntry("ignoredPlugins"); }

QString RzxConfig::propPromo() { return localHost() -> getPromoText(); }
int RzxConfig::quitMode(){ return globalConfig() -> readEntry("quitmode", 1); }
bool RzxConfig::showQuit(){ return globalConfig() -> readEntry("showquit", true); }
bool RzxConfig::refuseWhenAway() { return globalConfig() -> readEntry("refuseAway", true); }
void RzxConfig::writeQuitMode(int mode){ globalConfig() -> writeEntry("quitmode", mode); }
void RzxConfig::writeShowQuit(bool mode){ globalConfig() -> writeEntry("showquit", mode); }
void RzxConfig::writeIgnoredPluginsList(const QStringList& list) { globalConfig()->writeEntry("ignoredPlugins", list); }

/** Renvoie le password xnet */
QString RzxConfig::pass()
{
	QString i = globalConfig() -> readEntry(RzxServerListener::object()->getServerIP().toString() + "/pass", QString::null);
	if(i.isNull()) //Pour la compatibilité avec les anciennes formes de stockage sous nux
		i = globalConfig() -> readEntry("pass", QString::null);
	return i;
}
QString RzxConfig::oldPass()
{
	QString i = globalConfig() -> readEntry(RzxServerListener::object()->getServerIP().toString() + "/oldpass", QString::null);
	if(i.isEmpty()) i = QString::null;
	return i;
}

/** Renvoie la variable correspondant aux colonnes ï¿½afficher */
int RzxConfig::colonnes() {
	uint colonnes = 0;
	colonnes |= (1<<RzxRezal::ColIcone);
	colonnes |= (1<<RzxRezal::ColNom);
	colonnes |= (1<<RzxRezal::ColRemarque);
	colonnes |= (1<<RzxRezal::ColFTP);
	colonnes |= (1<<RzxRezal::ColHTTP);
	colonnes |= (1<<RzxRezal::ColNews);
	colonnes |= (1<<RzxRezal::ColOS);
	colonnes |= (1<<RzxRezal::ColPromo);
	colonnes |= (1<<RzxRezal::ColResal);
	return globalConfig() -> readEntry("colonnes", colonnes);
}

/** Change le mode du rï¿½ondeur */
void RzxConfig::setAutoResponder(bool val) { localHost() -> setRepondeur( val ); }

/** Lit le mode du rï¿½ondeur */
int RzxConfig::autoResponder() { return localHost() -> getRepondeur(); }

/** Renvoie le message du rï¿½ondeur */
QString RzxConfig::autoResponderMsg(){ return globalConfig() -> readEntry("txtAutoResponderMsg", "Rï¿½ondeur automatique");}

/** Renvoie la taille des icones des utilsiateurs
 * 0 pour 32x32
 * 1 pour 64x64 */
int RzxConfig::computerIconSize(){ return globalConfig() -> readEntry("iconsize", 1); }

///Indique s'il faut highlighté le computer sélectionné
bool RzxConfig::computerIconHighlight() { return globalConfig() -> readEntry("iconhighlight", true); }

void RzxConfig::setIconTheme(QObject * parent, const QString& name) {
	globalConfig() -> writeEntry("theme", name);
	if(parent)
	{
		globalConfig() -> progIcons.clear();
		((QRezix *) parent) -> changeTheme();
		emit globalConfig()->themeChanged();
	}
}

/** Renvoie le RzxComputer identifiant localhost */
RzxComputer * RzxConfig::localHost() {
	RzxComputer * computer = globalConfig() -> computer;
	if (!computer) {
		globalConfig() -> loadLocalHost();
		computer = globalConfig() -> computer;
	}		
	
	return computer;
}

/** charge le RzxComputer local */
void RzxConfig::loadLocalHost() {
/*
#ifndef WIN32
	char localhost[256];
	gethostname(localhost, 255);
#else
	DWORD namelength = MAX_COMPUTERNAME_LENGTH + 1;
	char* localhost = new char[namelength];
	GetComputerNameA(localhost, &namelength);
#endif

	QString dnsname = QString(localhost);
*/
	QString dnsname;
	QString comment;
	int promo = RzxComputer::PROMAL_UNK;
	bool repondeur = false;
	int servers = 0;

	//dnsname = readEntry( "dnsname", dnsname );
	dnsname = readEntry( "dnsname", "" );
	comment = readEntry( "comment", "$#x" ); //chaîne de merde qui identifie le tirage aléatoire d'une chaîne
	promo = readEntry( "promo", promo );
	repondeur = readEntry( "repondeur", repondeur );
	servers = readEntry( "servers", servers );

	if(comment == "$#x")
	{
		QStringList comments = QStringList::split("\n", DEFAULT_COMMENT);
		int i = rand()%comments.size();
		comment = comments[i];
	}

	if (!computer) computer = new RzxComputer();
	computer->initLocalHost();
	computer->setName( dnsname );
	computer->setRemarque( comment );
	computer->setPromo( promo );
	computer->setRepondeur( repondeur );
	computer->setServerFlags( servers );
	computer->scanServers();
}

void RzxConfig::closeSettings()
{
	delete settings;
	settings = NULL;
}

QColor RzxConfig::repondeurHighlight() {
	QColor ret;
	ret.setRgb(globalConfig() -> readEntry("repondeur_highlight", 0xFD3D3D));
	return ret;
}

QColor RzxConfig::repondeurBase() {
	QColor ret;
	ret.setRgb(globalConfig() -> readEntry("repondeur_base", 0xFFEE7C));
	return ret;
}

QColor RzxConfig::repondeurHighlightedText() {
	QColor ret;
	ret.setRgb(globalConfig() -> readEntry("repondeur_highlightedtext", 0xFFFFFF));
	return ret;
}

QColor RzxConfig::repondeurNormalText() {
	QColor ret;
	ret.setRgb(globalConfig() -> readEntry("repondeur_normaltext", 0x000000));
	return ret;
}

QColor RzxConfig::ignoredBGColor() {
	QColor ret;
	ret.setRgb(globalConfig() -> readEntry("ignoredBGColor", 0xCCCCCC));
	return ret;
}

QColor RzxConfig::ignoredText() {
	QColor ret;
	ret.setRgb(globalConfig() -> readEntry("ignoredtext", 0xAAAAAA));
	return ret;
}

/** No descriptions */
QColor RzxConfig::errorBackgroundColor(){
	QColor ret;
	ret.setRgb(globalConfig() -> readEntry("error_back", 0xFF0000));
	return ret;
}

QColor RzxConfig::errorTextColor(){
	QColor ret;
	ret.setRgb(globalConfig() -> readEntry("error_back", 0xFFFFFF));
	return ret;
}


/** No descriptions */
const QVector<QPixmap*> RzxConfig::yesnoIcons(){
	QVector<QPixmap*> ret(10);
	ret[0] = (QPixmap*)&themedIcon("no_samba");
	ret[1] = (QPixmap*)&themedIcon("no_ftp");
	ret[2] = (QPixmap*)&themedIcon("no_hotline");
	ret[3] = (QPixmap*)&themedIcon("no_http");
	ret[4] = (QPixmap*)&themedIcon("no_news");
	ret[5] = (QPixmap*)&themedIcon("samba");
	ret[6] = (QPixmap*)&themedIcon("ftp");
	ret[7] = (QPixmap*)&themedIcon("hotline");
	ret[8] = (QPixmap*)&themedIcon("http");
	ret[9] = (QPixmap*)&themedIcon("news");
	return ret;
}

const QVector<QPixmap*> RzxConfig::gatewayIcons(){
	QVector<QPixmap*> ret(2);
	ret[0] = (QPixmap*)&themedIcon("diff_gateway");
	ret[1] = (QPixmap*)&themedIcon("same_gateway");
	return ret;
}

const QVector<QPixmap*> RzxConfig::promoIcons(){
	QVector<QPixmap*> ret(3);
	ret[0] = (QPixmap*)&themedIcon("orange");
	ret[1] = (QPixmap*)&themedIcon("rouje");
	ret[2] = (QPixmap*)&themedIcon("jone");
	return ret;
}

/** No descriptions */
const QVector<QPixmap*> RzxConfig::osIcons(bool large){
	QString suffix;
	if (large)
		suffix = "_large";
	
	QVector<QPixmap*> ret(6);
	for (int idx = 0; idx < 6; idx++)
		ret[idx] = (QPixmap*)&themedIcon("os_" + QString::number(idx) + suffix);
	
	return ret;
}

/* Retourne l'icone du bouton son de la fenetre de chat */
const QPixmap &RzxConfig::soundIcon(bool sound) {
	QString q= !sound ? "haut_parleur1" : "haut_parleur2";
	return themedIcon(q);
}

/** No descriptions */
const QPixmap &RzxConfig::localhostIcon(){
	return icon("localhost");
}

QString RzxConfig::historique(unsigned long ip, const QString& hostname) {
	QString filename = QString::number(ip, 16);
	
	QDir logdir = globalConfig() -> logDir();
	filename = filename + "[" + hostname + "].html";
	
	if (logdir.exists(filename)) return logdir.absFilePath(filename);
		
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
	
	return logdir.absFilePath(filename);
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
		QString favoritesFile = m_userDir.absFilePath(name);
	
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

bool RzxConfig::isFavorite(const QString& nom) const {
	return favorites.contains(nom);
}

void RzxConfig::addToFavorites(const QString& nom) {
	favorites.insert(nom);
}

void RzxConfig::delFromFavorites(const QString& nom) {
	favorites.remove(nom);
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
		QString ignoreListFile = m_userDir.absFilePath(name);
	
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

bool RzxConfig::isBan(const QString& ip) const {
	return ignoreList.contains(ip);
}

void RzxConfig::addToBanlist(const QString& ip) {
	ignoreList.insert(ip);
}

void RzxConfig::delFromBanlist(const QString& ip) {
	ignoreList.remove(ip);
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
	globalConfig() -> writeEntry("cacheprop-" + address.toString(), msg);
	globalConfig() ->writeEntry("cachedate-" + address.toString(),  QDate::currentDate().toString("dd MMM yyyy"));
}

QString RzxConfig::cache(const RzxHostAddress& address)
{
	return globalConfig()->readEntry("cacheprop-" + address.toString(), QString::null);
}

QString RzxConfig::getCacheDate(const RzxHostAddress& address)
{
	return globalConfig()->readEntry("cachedate-" + address.toString(), QString::null);
}
