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

#ifndef WIN32
#include <unistd.h>

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#ifdef Q_OS_MACX
#define QREZIX_DATA_DIR "qrezix.app/Contents/Resources/"
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

#include "rzxhostaddress.h"
#include "rzxconfig.h"
#include "rzxcomputer.h"
#include "qrezix.h"
#include "rzxrezal.h"
#include "rzxitem.h"
#include "rzxserverlistener.h"
#include "rzxpluginloader.h"
#include "defaults.h"


#define CONF_REZIX "rezix.conf"
#define CONF_FAVORITES "favorites.conf"


RzxConfig * RzxConfig::Config = 0;
const QString RzxConfig::logPath("log");
const QString RzxConfig::themePath("themes");
QDict<QTranslator> RzxConfig::translations;
QTranslator* RzxConfig::currentTranslator=NULL;

void RzxConfig::loadTranslators(){
	if(currentTranslator) qApp->removeTranslator(currentTranslator);
	currentTranslator=NULL;
	translations.setAutoDelete(true);
	translations.clear();
	loadTranslatorsInDir(globalConfig()->m_systemDir);
	//loadTranslatorsInDir(globalConfig()->m_userDir);
}

void RzxConfig::loadTranslatorsInDir(QDir sourceDir){
	if(!sourceDir.cd("translations")) qDebug(QString("Cannot cd to %1/translations").arg(sourceDir.canonicalPath()));
	else qDebug(QString("Exploring %1/translations").arg(sourceDir.canonicalPath()));
	sourceDir.setNameFilter("*.qm");
	QStringList trans=sourceDir.entryList(QDir::Files|QDir::Readable);
	QTranslator* cur=0;
	qDebug(QString("Found %1 translations in %2").arg(trans.count()).arg(sourceDir.canonicalPath()));
	for(QStringList::Iterator it=trans.begin(); it!=trans.end(); ++it){
		qDebug(QString("Loading translation file %1").arg(*it));
		cur=new QTranslator(0);
		cur->load(*it, sourceDir.path());
		qApp->installTranslator(cur);
		qDebug("Language is "+tr("English"));
		translations.insert(tr("English"), cur);
		qApp->removeTranslator(cur);
	}
}
		
void RzxConfig::setLanguage(QString language){
	if(language!=tr("English")){
		if(currentTranslator) qApp->removeTranslator(currentTranslator);
		currentTranslator=NULL;
		if(language!="English"){
			currentTranslator=translations[language];
			qApp->installTranslator(currentTranslator);
		}
		qDebug("Config->languageChanged() emitted");
		emit Config->languageChanged();
	}
}

/**
*/
RzxConfig::RzxConfig()
	: QObject(0, "Config"), allIcons(USER_HASH_TABLE_LENGTH){
	if(!Config) Config=this;
	fileEntries.setAutoDelete(true);
	favorites=new QDict<QString>(USER_HASH_TABLE_LENGTH,false);
	favorites->setAutoDelete(true);
	computer = 0;
	
#ifdef WIN32
	m_systemDir = m_userDir = QDir::currentDirPath();	
#else
#ifdef Q_OS_MACX
	m_systemDir.setPath(QREZIX_DATA_DIR);
	m_userDir.setPath(QREZIX_DATA_DIR);
#else
	m_systemDir.setPath(QREZIX_DATA_DIR);
	m_userDir = QDir::home();
	if (!m_userDir.cd(".rezix")) {
		if (!m_userDir.mkdir(".rezix")) {
			QString msg;
			msg = tr("QRezix cannot create %1, which is the folder\nin which its configuration is saved\n")
				.arg(m_userDir.absFilePath(".rezix"));
			msg += tr("You will not be able to save your configuration");
			QMessageBox::critical(0, "QRezix", msg);
		} else {
			m_userDir.cd(".rezix");
		}
	}
	
	if (!m_systemDir.exists()) {
		qWarning(tr("%s doesn't exist"), m_systemDir.canonicalPath().latin1());
		m_systemDir = m_userDir;
	}
#endif //MACX
#endif //WIN32
	qDebug("System path set to "+m_systemDir.path());
	qDebug("Personnal path set to "+m_userDir.path());

	//ATTENTION, LE NOUVEAU FORMAT NE PERMET PAS DE CONVERTIR LES DONNÉES DE CONFIGURATION
	//DEPUIS L'ANCIEN FORMAT VERS LE NOUVEAU
	//Ouverture du fichier de configuration
	settings = new QSettings();
	settings->insertSearchPath(QSettings::Unix,m_userDir.canonicalPath());
	readFavorites();

	//CHargment de données diverses (icons, traductions)
	loadTranslators();
	allIcons.setAutoDelete(true);
	progIcons.setAutoDelete(true);
	QString lg = readEntry("language", "English");
	qDebug("Language is set to"+ lg);
	setLanguage(lg);
	qDebug("Trying to open "+ findData("action.png",  themePath + "/" + iconTheme()));	
	
	QMimeSourceFactory::defaultFactory()->setImage( "action", findData("action.png", themePath+"/"+iconTheme()) );	//TODO trouver le répertoire du thême cournat
	
	//Chargement des données QVB sur les fontes du système
	qDebug("Loading system Fonts !");
	QFontDatabase fdb;
	fontProperties = new QDict<FontProperty>(907); //nombre premier plus grand que le nombre de polices supposé
    	fontFamilies = fdb.families();
    	for ( QStringList::Iterator f = fontFamilies.begin(); f != fontFamilies.end();) {
        	QString family = *f;
        	QStringList styles = fdb.styles( family );
			if(styles.contains("Normal")!=0) {
				QValueList<int> size = fdb.smoothSizes(family, "Normal");
				bool b = styles.contains("Bold")!=0;
				bool i = styles.contains("Italic")!=0 || styles.contains("Oblique")!=0;
				FontProperty * fp = new FontProperty( b, i, size);
				fontProperties -> insert(family, fp);
				++f;
				qDebug("Inserted "+family+" into QDict");
			}
			else {
				f=fontFamilies.remove(f);
				qDebug("Removed "+family+" from family list");	
			}
        	/*for ( QStringList::Iterator s = styles.begin(); s != styles.end(); ++s ) {
            		QString style = *s;
            		QString dstyle = "\t" + style + " (";
            		QValueList<int> smoothies = fdb.smoothSizes( family, style );
            		for ( QValueList<int>::Iterator points = smoothies.begin();
                  		points != smoothies.end(); ++points ) {
                		dstyle += QString::number( *points ) + " ";
            		}
            		dstyle = dstyle.left( dstyle.length() - 1 ) + ")";
            		qDebug( dstyle );
        	}*/
    	}
	
}

/**
*/
RzxConfig::~RzxConfig(){
	delete fontProperties;
}

/**
*/
RzxConfig * RzxConfig::globalConfig() {
	if (!Config) {
		Config = new RzxConfig();
	}
	return Config;
}


RzxConfig::FontProperty::FontProperty(bool b, bool i, QValueList<int> pS)
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
		qWarning(tr("QRezix can't find  %s/%s"), relative.local8Bit().data(), name.local8Bit().data());
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

void RzxConfig::flush()
{
	if(settings) delete settings;
	settings = new QSettings();
	settings->insertSearchPath(QSettings::Unix,m_userDir.canonicalPath());
	RzxPlugInLoader::global()->setSettings();
}

QPixmap * RzxConfig::icon(const QString& name) {
	RzxConfig * config = globalConfig();
	return icon(name, config -> allIcons);
}

QPixmap * RzxConfig::themedIcon(const QString& name) {
	RzxConfig * config = globalConfig();
	return icon(name, config -> progIcons, themePath + "/" + iconTheme());
}

QPixmap * RzxConfig::icon(const QString& name, QDict<QPixmap>& cache, const QString& subdir) {
	RzxConfig * config = globalConfig();
	QString qualifiedName = subdir.isNull() ? name : (subdir + "_" + name);
	QPixmap * ret = cache.find(qualifiedName);
	if (ret) return ret;
	
	ret = new QPixmap();
	cache.insert(qualifiedName, ret);
	
	QString fileName = config -> findData(name + ".png", subdir);
	if (fileName.isNull())
		return ret;
	ret -> load(fileName);
	return ret;
}

void RzxConfig::saveIcon(const QString& name, const QPixmap& image){
	RzxConfig * cfgObject = globalConfig();
	QPixmap * ret = cfgObject -> allIcons.find(name);
	if (ret)	cfgObject -> allIcons.remove(name);
		
	ret = new QPixmap;
	*ret = image;
	ret -> save(cfgObject -> m_userDir.absFilePath(name + ".png"), "PNG");
	cfgObject -> allIcons.insert(name, ret);
}

/******************************************************************************
* FONCTIONS DE LECTURE DES ENTREES															*
******************************************************************************/

/** Renvoie la liste des familles de fonte initialisée au début */
QStringList RzxConfig::getFontList() {	return fontFamilies; }

/** Renvoie la liste des tailles acceptées par cette police */
QValueList<int> RzxConfig::getSizes(const QString& family) {
	FontProperty * fp = fontProperties->find(family);
	if(!fp)
		qDebug("Problème, chargement de la police "+family+" impossible");
	return fp->sizes;
}

bool RzxConfig::isItalicSupported(const QString& family) {
	FontProperty * fp = fontProperties->find(family);
	if(!fp)
		qDebug("Problème, chargement de la police "+family+" impossible");
	return fp->italic;
}

bool RzxConfig::isBoldSupported(const QString& family) {
	FontProperty * fp = fontProperties->find(family);
	if(!fp)
		qDebug("Problème, chargement de la police "+family+" impossible");
	return fp->bold;
}

/** Renvoie une chaine correspondant ï¿½la taille de la fenï¿½re principale*/
QString RzxConfig::readWindowSize(){return globalConfig() -> readEntry("window_size",DEFAULT_WINDOWSIZE); }

/** Renvoie une chaine correspondant ï¿½la taille de la fenï¿½re principale*/
void RzxConfig::writeWindowSize(QString ws){globalConfig() -> writeEntry("window_size",ws); }

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

/** Renvoie si on utilise le systray ou non */
#ifndef Q_OS_MACX
int RzxConfig::useSystray(){ return globalConfig() -> readEntry("useSystray", 1); }
#else
int RzxConfig::useSystray(){ return globalConfig() -> readEntry("useSystray", 0); }
#endif

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

#ifdef WIN32
QString RzxConfig::sambaCmd(){ return globalConfig() -> readEntry("samba_cmd", "standard");}
QString RzxConfig::ftpCmd(){ return globalConfig() -> readEntry("ftp_cmd", "standard"); }
QString RzxConfig::httpCmd(){ return globalConfig() -> readEntry("http_cmd", "standard"); }
QString RzxConfig::newsCmd(){ return globalConfig() -> readEntry("newsCmd", "standard"); }
#else
#ifdef Q_OS_MACX
QString RzxConfig::sambaCmd(){ return globalConfig() -> readEntry("samba_cmd", "open"); }
QString RzxConfig::ftpCmd(){ return globalConfig() -> readEntry("ftp_cmd", "Default"); }
QString RzxConfig::httpCmd(){ return globalConfig() -> readEntry("http_cmd", "Default"); }
QString RzxConfig::newsCmd(){ return globalConfig() -> readEntry("newsCmd", "Default"); }
#else
QString RzxConfig::sambaCmd(){ return globalConfig() -> readEntry("samba_cmd", "standard"); }
QString RzxConfig::ftpCmd(){ return globalConfig() -> readEntry("ftp_cmd", "gftp"); }
QString RzxConfig::httpCmd(){ return globalConfig() -> readEntry("http_cmd", "konqueror"); }
QString RzxConfig::newsCmd(){ return globalConfig() -> readEntry("newsCmd", "knode"); }
#endif //MACX
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

QString RzxConfig::propLastName(){ return globalConfig() -> readEntry("txtFirstname", "");}
QString RzxConfig::propName(){ return globalConfig() -> readEntry("txtName", "");}
QString RzxConfig::propSurname(){ return globalConfig() -> readEntry("txtSurname", "");}
QString RzxConfig::propTel(){ return globalConfig() -> readEntry("txtPhone", "");}
QString RzxConfig::propSport(){ return globalConfig() -> readEntry("txtSport", "");}
int RzxConfig::numSport(){ return globalConfig() -> readEntry("numSport", 0);}
QString RzxConfig::propMail(){ return globalConfig() -> readEntry("txtMail", "");}
QString RzxConfig::propWebPage(){ return globalConfig() -> readEntry("txtWeb", "");}
QString RzxConfig::propCasert(){ return globalConfig() -> readEntry("txtCasert", "");}
QString	RzxConfig::iconTheme(){ return globalConfig() -> readEntry("theme", DEFAULT_THEME);}
QString	RzxConfig::FTPPath(){ return globalConfig() -> readEntry("FTPPath", "");}

int RzxConfig::menuTextPosition() { return globalConfig() ->readEntry("menuTextPos", 2); }
int RzxConfig::menuIconSize() { return globalConfig() ->readEntry("menuIconSize", 2); }

QString RzxConfig::propPromo() { return localHost() -> getPromoText(); }
int RzxConfig::quitMode(){ return globalConfig() -> readEntry("quitmode", 1); }
bool RzxConfig::showQuit(){ return globalConfig() -> readEntry("showquit", true); }
void RzxConfig::writeQuitMode(int mode){ globalConfig() -> writeEntry("quitmode", mode); }
void RzxConfig::writeShowQuit(bool mode){ globalConfig() -> writeEntry("showquit", mode); }

/** Renvoie le password xnet */
QString RzxConfig::pass()
{
	QString i = globalConfig() -> readEntry(RzxServerListener::object()->getServerIP().toString() + "/pass", QString::null);
	if(!i) //Pour la compatibilité avec les anciennes formes de stockage sous nux
	{
		i = globalConfig() -> readEntry("pass", QString::null);
//		globalConfig()->setPass(i);
	}
	return i;
}

/** Renvoie la variable correspondant aux colonnes ï¿½afficher */
int RzxConfig::colonnes() { return globalConfig() -> readEntry("colonnes", 0xFFFF); }

/** Change le mode du rï¿½ondeur */
void RzxConfig::setAutoResponder(bool val) { localHost() -> setRepondeur( val ); }

/** Lit le mode du rï¿½ondeur */
bool RzxConfig::autoResponder() { return localHost() -> getRepondeur(); }

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
	globalConfig() -> progIcons.clear();
	((QRezix *) parent) -> changeTheme();
	emit globalConfig()->themeChanged();
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
QArray<QPixmap *> RzxConfig::yesnoIcons(){
	QArray<QPixmap *> ret(10);
	ret[0] = themedIcon("no_samba");
	ret[1] = themedIcon("no_ftp");
	ret[2] = themedIcon("no_hotline");
	ret[3] = themedIcon("no_http");
	ret[4] = themedIcon("no_news");
	ret[5] = themedIcon("samba");
	ret[6] = themedIcon("ftp");
	ret[7] = themedIcon("hotline");
	ret[8] = themedIcon("http");
	ret[9] = themedIcon("news");
	return ret;
}

QArray<QPixmap *> RzxConfig::gatewayIcons(){
	QArray<QPixmap *> ret(2);
	ret[0] = themedIcon("diff_gateway");
	ret[1] = themedIcon("same_gateway");
	return ret;
}

QArray<QPixmap *> RzxConfig::promoIcons(){
	QArray<QPixmap *> ret(3);
	ret[0] = themedIcon("orange");
	ret[1] = themedIcon("rouje");
	ret[2] = themedIcon("jone");
	return ret;
}

/** No descriptions */
QArray<QPixmap *> RzxConfig::osIcons(bool large){
	QString suffix;
	if (large)
		suffix = "_large";
	
	QArray<QPixmap *> ret(6);
	for (int idx = 0; idx < 6; idx++)
		ret[idx] = themedIcon("os_" + QString::number(idx) + suffix);
	
	return ret;
}

/* Retourne l'icone du bouton son de la fenetre de chat */
QPixmap * RzxConfig::soundIcon(bool sound) {
	QString q= !sound ? "haut_parleur1" : "haut_parleur2";
	return themedIcon(q);
}

/** No descriptions */
QPixmap * RzxConfig::localhostIcon(){
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
		if (!file.open(IO_ReadOnly | IO_Translate)) {
			QMessageBox::critical(0, tr("qRezix error"), 
				tr("Unable to open favorites file %1").arg(favoritesFile));
			return;
		}
	
		QTextStream stream(&file);

		favorites->clear();
		QString line;
		line = stream.readLine();
		while(!line.isNull()) {
			favorites->insert(line, new QString("1"));
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
		QStringList favoriteList =  settings->readListEntry("/qRezix/general/favorites");
		QStringList::iterator it;
		for(it = favoriteList.begin() ; it != favoriteList.end() ; it++)
			favorites->insert(*it, new QString("1"));
	}
}


void RzxConfig::writeFavorites()
{
	QStringList favoriteList;
	QDictIterator<QString> strConfig2(*favorites);
	for (; strConfig2.current(); ++strConfig2) {
		favoriteList << strConfig2.currentKey();
	}
	settings->writeEntry("/qRezix/general/favorites", favoriteList);
}
