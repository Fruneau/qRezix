/***************************************************************************
                          qrezix.cpp  -  description
                             -------------------
    begin                : lun jan 28 16:27:20 CET 2002
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
#include <qmessagebox.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qtoolbox.h>
#include <qpopupmenu.h>
#include <qrect.h>
#include <qpoint.h>
#include <qstring.h>
#include <qfile.h>
#include <qiconset.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qapplication.h>
#include <qlayout.h> 

#include "qrezix.h"
#include "rzxquit.h"
#include "rzxconfig.h"
#include "rzxrezal.h"
#include "rzxproperty.h"
#include "rzxpluginloader.h"

#include "defaults.h"

#include "t.xpm"
#include "q.xpm"

QRezix *QRezix::object = 0;

QRezix::QRezix(QWidget *parent, const char *name)
 : QRezixUI(parent, name), m_properties(0), tray(0)
{
	object = this;
	byTray = false;
	statusFlag = false;
	RzxPlugInLoader::global();
	rezal->showNotFavorites(true);
	rezalFavorites->showNotFavorites(false);
	connect(btnPreferences, SIGNAL(clicked()), this, SLOT(boitePreferences()));
	connect(btnMAJcolonnes, SIGNAL(clicked()), rezal, SLOT(adapteColonnes()));
	connect(btnMAJcolonnes, SIGNAL(clicked()), rezalFavorites, SLOT(adapteColonnes()));
	connect(btnAutoResponder, SIGNAL(toggled(bool)), this, SLOT(activateAutoResponder(bool)));
	connect(btnPlugins, SIGNAL(toggled(bool)), this, SLOT(pluginsMenu(bool)));
	connect(&menuPlugins, SIGNAL(aboutToHide()), btnPlugins, SLOT(toggle()));
 
	RzxClientListener *client = RzxClientListener::object();
	connect(client, SIGNAL(chatSent()), this, SLOT(chatSent()));
 
	// CHAT
	connect(client, SIGNAL(chat(QSocket*, const QString& )), rezal, SLOT(chat(QSocket*, const QString& )));
	// RECEPTION DES PROPRIETES D'UN ORDINATEUR
//	connect(client, SIGNAL(propAnswer(const RzxHostAddress&, const QString&)), rezal, SLOT(showProperties(const RzxHostAddress&, const QString&)));
	connect(client, SIGNAL(propertiesSent(const RzxHostAddress&)), rezal, SLOT(warnProperties(const RzxHostAddress&)));

	// Connexion des �v�nement serveur au r�zal
	RzxServerListener *server = RzxServerListener::object();
	connect(server, SIGNAL(sysmsg(const QString&)), rezal, SLOT(sysmsg(const QString&)));
	connect(server, SIGNAL(fatal(const QString&)), rezal, SLOT(fatal(const QString&)));

	connect(rezal, SIGNAL(needIcon(const RzxHostAddress&)), server, SLOT(getIcon(const RzxHostAddress&)));
 
	// Pr�paration de l'insterface
	activateAutoResponder( RzxConfig::autoResponder() != 0 );

	connect(rezal, SIGNAL(favoriteChanged()), rezalFavorites, SIGNAL(favoriteChanged()));

	clearWFlags(WStyle_SysMenu|WStyle_Minimize);
	alreadyOpened=false;
	connect(rezal, SIGNAL(status(const QString&,bool)), this, SLOT(status(const QString&, bool)));
	connect(rezal, SIGNAL(countChange(const QString&)), lblCount, SLOT(setText(const QString&)));
	connect(rezal, SIGNAL(countChange(const QString&)), this, SIGNAL(setToolTip(const QString&)));


	m_properties = new RzxProperty(this);
	if(!RzxConfig::globalConfig()->find() || !m_properties->infoCompleted())
	{
		m_properties->initDlg();
		m_properties -> exec();
	}

	//RzxConfig::loadTranslators();
	rezal -> initConnection();

	connect(rezal, SIGNAL(selectionChanged(QListViewItem*)), RzxPlugInLoader::global(), SLOT(itemChanged(QListViewItem*)));
	connect(rezalFavorites, SIGNAL(selectionChanged(QListViewItem*)), RzxPlugInLoader::global(), SLOT(favoriteChanged(QListViewItem*)));

	connect(RzxConfig::globalConfig(), SIGNAL(iconFormatChange()), this, SLOT(menuFormatChange()));
	menuFormatChange();
	

	tbRezalContainer->setCurrentIndex(1);
	changeTheme();
}

void QRezix::launchPlugins()
{
	RzxPlugInLoader::global()->init();
}

QRezix *QRezix::global()
{
	return object;
}

void QRezix::languageChanged(){
	qDebug("Language changed");
	languageChange();
	rezal->languageChanged();
	rezalFavorites->languageChanged();
}

QRezix::~QRezix() {
}

void QRezix::status(const QString& msg, bool fatal){
	lblStatus -> setText(msg);
	statusFlag = !fatal;

	if(statusFlag)
		lblStatusIcon->setPixmap(*RzxConfig::themedIcon("on"));
	else
		lblStatusIcon->setPixmap(*RzxConfig::themedIcon("off"));
		
	/* parceque je veux avoir une trace de ce qui s'est pass� ! */
	qDebug( "[%s] status%s = %s", QDateTime::currentDateTime().toString().latin1(),
		    fatal ? " (FATAL)" : "", msg.latin1() );
}

void QRezix::closeByTray()
{
	byTray = true;
	close();
}

///Sauvegarde des donn�es au moment de la fermeture
/** Lance la sauvegarde des donn�es principales lors de la fermeture de rezix. Cette m�thode est cens�e permettre l'enregistrement des donn�es lors de la fermeture de l'environnement graphique... */
void QRezix::saveSettings()
{
	byTray = true;
	QSize s = size();       // store size
	QString height="";
	height.sprintf("%4d",s.height());
	QString width = "";
	width.sprintf("%4d",s.width());
	QString windowSize;
	if( statusMax ) windowSize = "100000000";
	else windowSize="0"+height+width;
	qDebug("Fermeture des plugins");
	delete RzxPlugInLoader::global();
	qDebug("Fermeture de l'enregistrement des configurations");
	RzxConfig::globalConfig()->writeWindowSize(windowSize);
	RzxConfig::globalConfig() -> closeSettings();
	qDebug("Fermeture des fen�tres de chat");
	rezal->closeChats();
	qDebug("Fermeture de l'�coute r�seau");
	RzxClientListener::object()->close();
	qDebug("Fermeture de la connexion avec le serveur");
	if (!rezal -> isSocketClosed()){
		lblStatus -> setText(tr("Closing socket..."));
		rezal -> setEnabled(false);
		connect(rezal, SIGNAL(socketClosed()), this, SLOT(socketClosed()));
		rezal -> closeSocket();
	}
	disconnect(qApp, SIGNAL(aboutToQuit()), this, SLOT(saveSettings()));
	qDebug("Fermeture de qRezix termin�e");
}

void QRezix::closeEvent(QCloseEvent * e){
	//pour �viter de fermer rezix par m�garde, on affiche un boite de dialogue laissant le choix
	//de fermer qrezix, de minimiser la fen�tre principale --> trayicon, ou d'annuler l'action
	if(!byTray && isShown() && !isMinimized())
	{
		int i;
		if(RzxConfig::showQuit())
		{
			RzxQuit quitDialog(this);
			i = quitDialog.exec();
		}
		else
			i = RzxConfig::quitMode();
		if(i!=RzxQuit::selectQuit)
		{
			if(i==RzxQuit::selectMinimize)
				showMinimized();
#ifdef WIN32 //c'est tr�s tr�s tr�s tr�s tr�s tr�s moche, mais g pas trouv� d'autre mani�re de le faire
			 //c'est pas ma fautre � moi si windows se comporte comme de la merde
			QEvent ev(QEvent::WindowDeactivate); 
			event(&ev);
#endif
			e -> ignore();
			return;
		}
	}

	saveSettings();

	if (rezal -> isSocketClosed())
		e -> accept();
	else
		e -> ignore();
}

bool QRezix::event(QEvent * e){
#ifdef WIN32
	if(e->type()==QEvent::WindowDeactivate)
	{
		if(isMinimized() && RzxConfig::globalConfig()->useSystray())
			hide();
		return true;
	}
#else //WIN32
	if(e->type()==QEvent::ShowMinimized || e->type()==QEvent::Hide){
		if(RzxConfig::globalConfig()->useSystray()) hide();
		return true;
	}
#endif //WIN32
	else if( e->type() == QEvent::Resize && alreadyOpened && !isMinimized())
		statusMax = isMaximized();

	return QWidget::event(e);
}

void QRezix::delayedInit() {
	disconnect(m_properties, SIGNAL(end()), this, SLOT(delayedInit()));
	rezal -> initConnection();
}

void QRezix::boitePreferences(){
	if (m_properties) {
		if (!(m_properties -> isVisible())) {
			m_properties -> initDlg();
			m_properties -> show();
		}
	}
	else {
		m_properties = new RzxProperty(this);
		m_properties -> show();
	}
}

void QRezix::socketClosed(){
	close();
}


void QRezix::toggleAutoResponder()
{
	activateAutoResponder( !btnAutoResponder->isOn());
}

void QRezix::toggleButtonResponder()
{
	activateAutoResponder( !btnAutoResponder -> isOn() );
}

void QRezix::activateAutoResponder( bool state )
{
	if(!state == btnAutoResponder->isOn())
	{
		btnAutoResponder->setOn(state);
		return;
	}
	if (state == (RzxConfig::autoResponder() != 0)) return;
	RzxConfig::setAutoResponder( state );
	RzxProperty::serverUpdate();
	RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_AWAY, NULL);
	// Change l'icone dans la tray
	if(!btnAutoResponder->isOn())
	{
		tray->setIcon(QPixmap::QPixmap(q));
	}
	else
	{
		tray->setIcon(QPixmap::QPixmap(t));
	}
}

void QRezix::toggleVisible(){
	if(isVisible())
		hide();
	else{
		bool saveStatusMax = statusMax;
		showNormal();	//pour forcer l'affichage de la fen�tre ==> modifie statusMax
		if(saveStatusMax)
		{
			showMaximized();
			statusMax = saveStatusMax;
		}
		show();
		setActiveWindow();
		raise();
		alreadyOpened=true;
		rezal->adapteColonnes();
	}
}

void QRezix::languageChange()
{
	QRezixUI::languageChange();
	setCaption(caption() + " v" + RZX_VERSION);
#ifdef WIN32
	setCaption(caption() + " [Qt]");
#endif

	//Parce que Qt oublie de traduire les deux noms
	//Alors faut le faire � la main, mais franchement, c du foutage de gueule
	//� mon avis �a leur prendrait 5 minutes chez trolltech pour corriger le pb
	tbRezalContainer->setItemLabel(0, tr("Favorites"));
	tbRezalContainer->setItemLabel(1, tr("Everybody"));
}

void QRezix::chatSent() {
	// Desactive le r�pondeur lorsqu'on envoie un chat
	activateAutoResponder( false );
}

/// Changement du th�me d'icone
/** Cette m�thode met � jour les ic�nes de l'interface principale (menu), mais aussi celles des listes de connect�s */
void QRezix::changeTheme()
{
	rezal -> redrawAllIcons();
	rezalFavorites -> redrawAllIcons();
	QIconSet pi, away, columns, prefs,favorite,not_favorite;
	int icons = RzxConfig::menuIconSize();
	int texts = RzxConfig::menuTextPosition();
	if(icons || !texts)
	{
		pi.setPixmap(*RzxConfig::themedIcon("plugin"), QIconSet::Automatic);
		away.setPixmap(*RzxConfig::themedIcon("away"), QIconSet::Automatic, QIconSet::Normal, QIconSet::Off);
		away.setPixmap(*RzxConfig::themedIcon("here"), QIconSet::Automatic, QIconSet::Normal, QIconSet::On);
		columns.setPixmap(*RzxConfig::themedIcon("column"), QIconSet::Automatic);
		prefs.setPixmap(*RzxConfig::themedIcon("pref"), QIconSet::Automatic);
		favorite.setPixmap(*RzxConfig::themedIcon("favorite"), QIconSet::Automatic);
		not_favorite.setPixmap(*RzxConfig::themedIcon("not_favorite"), QIconSet::Automatic);
	}
	btnPlugins->setIconSet(pi);
	btnAutoResponder->setIconSet(away);
	btnMAJcolonnes->setIconSet(columns);
	btnPreferences->setIconSet(prefs);
	tbRezalContainer->setItemIconSet(1,not_favorite);
	tbRezalContainer->setItemIconSet(0,favorite);
	if(statusFlag)
		lblStatusIcon->setPixmap(*RzxConfig::themedIcon("on"));
	else
		lblStatusIcon->setPixmap(*RzxConfig::themedIcon("off"));
}

///Changement de format des boutons de la barre d'outils
/** Change la position du texte, ou la taille des ic�nes de la barre d'outil */
void QRezix::menuFormatChange()
{
	int icons = RzxConfig::menuIconSize();
	int texts = RzxConfig::menuTextPosition();

	//On transforme le cas 'pas d'ic�nes et pas de texte' en 'petites ic�nes et pas de texte'
	if(!texts && !icons) icons = 1;

	//Si on a pas d'ic�ne, on met le texte sur le c�t�... pour �viter un bug d'affichage
	if(!icons) texts = 1;
	
	//Mise � jour de la taille des ic�nes
	switch(icons)
	{
		case 0: //pas d'ic�ne
			{
				QIconSet empty;
				QPixmap emptyIcon;
				btnPlugins->setIconSet(empty);
				btnAutoResponder->setIconSet(empty);
				btnMAJcolonnes->setIconSet(empty);
				btnPreferences->setIconSet(empty);
				tbRezalContainer->setItemIconSet(0,empty);
				tbRezalContainer->setItemIconSet(1,empty);
				lblStatusIcon->setHidden(TRUE);
			}
			break;
		
		case 1: //petites ic�nes
		case 2: //grandes icones
			{
				bool big = (icons == 2);
				if(btnPlugins->iconSet().isNull()) changeTheme(); //pour recharcher les ic�nes s'il y a besoin
				btnPlugins->setUsesBigPixmap(big);
				btnAutoResponder->setUsesBigPixmap(big);
				btnMAJcolonnes->setUsesBigPixmap(big);
				btnPreferences->setUsesBigPixmap(big);
				lblStatusIcon->setShown(TRUE);
			}
			break;
	}
	
	//Mise � jour de la position du texte
	btnPlugins->setUsesTextLabel(texts);
	btnAutoResponder->setUsesTextLabel(texts);
	btnMAJcolonnes->setUsesTextLabel(texts);
	btnPreferences->setUsesTextLabel(texts);
	if(texts)
	{
		QToolButton::TextPosition pos = (texts == 1)? QToolButton::BesideIcon : QToolButton::BelowIcon;
		btnPlugins->setTextPosition(pos);
		btnAutoResponder->setTextPosition(pos);
		btnMAJcolonnes->setTextPosition(pos);
		btnPreferences->setTextPosition(pos);
		lblStatus->setShown(TRUE);
		spacerStatus->changeSize(1,1,QSizePolicy::Minimum,QSizePolicy::Minimum);
	}
	else
	{
		lblStatus->setShown(FALSE);
		spacerStatus->changeSize(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
	}
}

/// Affichage du menu plug-ins lors d'un clic sur le bouton
/** Les actions sont g�r�es directement par le plug-in s'il a bien �t� programm� */
void QRezix::pluginsMenu(bool show)
{
	if(!show)
		return;
	
	menuPlugins.clear();
	RzxPlugInLoader::global()->menuAction(menuPlugins);
	if(!menuPlugins.count())
		menuPlugins.insertItem("<none>");
	menuPlugins.popup(btnPlugins->mapToGlobal(btnPlugins->rect().topRight()));
}
