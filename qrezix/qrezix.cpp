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

#include "qrezix.h"
#include "rzxquit.h"
#include "rzxconfig.h"
#include "rzxrezal.h"
#include "rzxproperty.h"
#include "rzxpluginloader.h"

#include "defaults.h"

QRezix *QRezix::object = 0;

QRezix::QRezix(QWidget *parent, const char *name)
	: QRezixUI(parent, name), m_properties(0), tray(0)
{
	object = this;
	byTray = false;
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
	connect(client, SIGNAL(propAnswer(const RzxHostAddress&, const QString&)), rezal, SLOT(showProperties(const RzxHostAddress&, const QString&)));
	connect(client, SIGNAL(propertiesSent(const RzxHostAddress&)), rezal, SLOT(warnProperties(const RzxHostAddress&)));

	activateAutoResponder( RzxConfig::autoResponder() != 0 );

	connect(rezal, SIGNAL(favoriteChanged()), rezalFavorites, SIGNAL(favoriteChanged()));

	clearWFlags(WStyle_SysMenu|WStyle_Minimize);
	alreadyOpened=false;
	connect(rezal, SIGNAL(status(const QString&,bool)), this, SLOT(status(const QString&, bool)));
	connect(rezal, SIGNAL(countChange(const QString&)), lblCount, SLOT(setText(const QString&)));
	connect(rezal, SIGNAL(countChange(const QString&)), this, SIGNAL(setToolTip(const QString&)));


	bool firstlaunch = !RzxConfig::globalConfig()->find();
	m_properties = new RzxProperty(this);
	if(!RzxConfig::globalConfig()->find() || !m_properties->infoCompleted())
	{
		if(!firstlaunch) 
			m_properties->initDlg();
		m_properties -> exec();
	}

	//RzxConfig::loadTranslators();
	rezal -> initConnection();
	RzxPlugInLoader::global()->init();

	connect(rezal, SIGNAL(selectionChanged(QListViewItem*)), RzxPlugInLoader::global(), SLOT(itemChanged(QListViewItem*)));
	connect(rezalFavorites, SIGNAL(selectionChanged(QListViewItem*)), RzxPlugInLoader::global(), SLOT(favoriteChanged(QListViewItem*)));

	connect(RzxConfig::globalConfig(), SIGNAL(iconFormatChange()), this, SLOT(menuFormatChange()));
	menuFormatChange();
	changeTheme();
}

QRezix *QRezix::global()
{
	return object;
}

void QRezix::languageChanged(){
	qDebug("Language changed");
	languageChange();
	rezal->languageChanged();
}

QRezix::~QRezix() {
}

void QRezix::status(const QString& msg, bool fatal){
	lblStatus -> setText(msg);
	if (fatal) {
		lblStatus -> setBackgroundMode(QWidget::FixedColor);
		lblStatus -> setBackgroundColor(RzxConfig::errorBackgroundColor());
	}
	else {	
		lblStatus -> setBackgroundMode(QWidget::PaletteBackground);
	}

	/* parceque je veux avoir une trace de ce qui s'est passé ! */
	qDebug( "[%s] status%s = %s", QDateTime::currentDateTime().toString().latin1(),
		    fatal ? " (FATAL)" : "", msg.latin1() );
}

void QRezix::closeByTray()
{
	byTray = true;
	close();
}

void QRezix::closeEvent(QCloseEvent * e){
	//pour éviter de fermer rezix par mégarde, on affiche un boite de dialogue laissant le choix
	//de fermer qrezix, de minimiser la fenêtre principale --> trayicon, ou d'annuler l'action
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
#ifdef WIN32 //c'est très très très très très très moche, mais g pas trouvé d'autre manière de le faire
			 //c'est pas ma fautre à moi si windows se comporte comme de la merde
			QEvent ev(QEvent::WindowDeactivate); 
			event(&ev);
#endif
			e -> ignore();
			return;
		}
	}

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
	qDebug("Fermeture du rezal");

	if (!rezal -> isSocketClosed()){
		lblStatus -> setText(tr("Closing socket..."));
		rezal -> setEnabled(false);
		connect(rezal, SIGNAL(socketClosed()), this, SLOT(socketClosed()));
		rezal -> closeSocket();
	}
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
}

void QRezix::toggleVisible(){
	if(isVisible())
		hide();
	else{
		bool saveStatusMax = statusMax;
		showNormal();	//pour forcer l'affichage de la fenêtre ==> modifie statusMax
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
	setCaption(caption() + " v" + VERSION);
#ifdef WIN32
	setCaption(caption() + " [Qt]");
#endif

	//Parce que Qt oublie de traduire les deux noms
	//Alors faut le faire à la main, mais franchement, c du foutage de gueule
	//à mon avis ça leur prendrait 5 minutes chez trolltech pour corriger le pb
	tbRezalContainer->setItemLabel(0, tr("Everybody"));
	tbRezalContainer->setItemLabel(1, tr("Favorites"));
}

void QRezix::chatSent() {
	// Desactive le répondeur lorsqu'on envoie un chat
	activateAutoResponder( false );
}

/// Changement du thème d'icone
/** Cette méthode met à jour les icônes de l'interface principale (menu), mais aussi celles des listes de connectés */
void QRezix::changeTheme()
{
	rezal -> redrawAllIcons();
	rezalFavorites -> redrawAllIcons();
	QIconSet pi, away, columns, prefs;
	pi.setPixmap(*RzxConfig::themedIcon("plugin"), QIconSet::Automatic);
	away.setPixmap(*RzxConfig::themedIcon("away"), QIconSet::Automatic, QIconSet::Normal, QIconSet::Off);
	away.setPixmap(*RzxConfig::themedIcon("here"), QIconSet::Automatic, QIconSet::Normal, QIconSet::On);
	columns.setPixmap(*RzxConfig::themedIcon("column"), QIconSet::Automatic);
	prefs.setPixmap(*RzxConfig::themedIcon("pref"), QIconSet::Automatic);
	btnPlugins->setIconSet(pi);
	btnAutoResponder->setIconSet(away);
	btnMAJcolonnes->setIconSet(columns);
	btnPreferences->setIconSet(prefs);
}

///Changement de format des boutons de la barre d'outils
/** Change la position du texte, ou la taille des icônes de la barre d'outil */
void QRezix::menuFormatChange()
{
	int icons = RzxConfig::menuIconSize();
	int texts = RzxConfig::menuTextPosition();

	//On transforme le cas 'pas d'icônes et pas de texte' en 'petites icônes et pas de texte'
	if(!texts && !icons) icons = 1;

	//Si on a pas d'icône, on met le texte sur le côté... pour éviter un bug d'affichage
	if(!icons) texts = 1;
	
	//Mise à jour de la taille des icônes
	switch(icons)
	{
		case 0: //pas d'icône
			{
				QIconSet empty;
				btnPlugins->setIconSet(empty);
				btnAutoResponder->setIconSet(empty);
				btnMAJcolonnes->setIconSet(empty);
				btnPreferences->setIconSet(empty);
			}
			break;
		
		case 1: //petites icônes
		case 2: //grandes icones
			{
				bool big = (icons == 2);
				if(btnPlugins->iconSet().isNull()) changeTheme(); //pour recharcher les icônes s'il y a besoin
				btnPlugins->setUsesBigPixmap(big);
				btnAutoResponder->setUsesBigPixmap(big);
				btnMAJcolonnes->setUsesBigPixmap(big);
				btnPreferences->setUsesBigPixmap(big);
			}
			break;
	}
	
	//Mise à jour de la position du texte
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
	}
}

/// Affichage du menu plug-ins lors d'un clic sur le bouton
/** Les actions sont gérées directement par le plug-in s'il a bien été programmé */
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
