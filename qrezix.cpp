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
#include <qpushbutton.h>
#include <qstring.h>
#include <qfile.h>

#include "qrezix.h"
#include "rzxconfig.h"
#include "rzxrezal.h"
#include "rzxproperty.h"

#include "defaults.h"

QRezix::QRezix(QWidget *parent, const char *name)
	: QRezixUI(parent, name), m_properties(0), tray(0)
	
{
	connect((QObject *) btnPreferences, SIGNAL(clicked()), this, SLOT(boitePreferences()));
	connect((QObject *) btnMAJcolonnes, SIGNAL(clicked()), rezal, SLOT(adapteColonnes()));
	connect((QObject *) btnAutoResponder, SIGNAL(clicked()), this, SLOT(toggleAutoResponder()));
	connect((QObject *) btnFavorites, SIGNAL(clicked()), this, SLOT(activateFavorites()));
	
	connect(RzxClientListener::object(), SIGNAL(chatSent()), this, SLOT(chatSent()));

	activateAutoResponder( RzxConfig::autoResponder() != 0 );
	if(RzxConfig::favoritesMode())
		btnFavorites -> toggle();

	setCaption(caption() + " v" + VERSION);
#ifdef WIN32
	setCaption(caption() + " [Qt]");
#endif
	clearWFlags(WStyle_SysMenu|WStyle_Minimize);
	alreadyOpened=false;
	connect(rezal, SIGNAL(status(const QString&,bool)), this, SLOT(status(const QString&, bool)));
	connect(rezal, SIGNAL(countChange(const QString&)), lblCount, SLOT(setText(const QString&)));
	connect(rezal, SIGNAL(countChange(const QString&)), this, SIGNAL(setToolTip(const QString&)));
	
	if(!QFile(RzxConfig::globalConfig()->find()).exists()) {
		m_properties = new RzxProperty(this);
		m_properties -> exec();
	} 
	//RzxConfig::loadTranslators();
	rezal -> initConnection();
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

void QRezix::closeEvent(QCloseEvent * e){
  QSize s = size();       // store size
	QString height="";
	height.sprintf("%4d",s.height());
	QString width = "";
	width.sprintf("%4d",s.width());
	QString windowSize;
	if( statusMax ) windowSize = "100000000";
	else windowSize="0"+height+width;
	RzxConfig::globalConfig()->writeWindowSize(windowSize);
	
	RzxConfig::globalConfig() -> write();
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
	if(e->type()==QEvent::ShowMinimized || e->type()==QEvent::Hide){
		if(RzxConfig::globalConfig()->useSystray()) hide();
		return true;
	}
	else if( e->type() == QEvent::Resize && alreadyOpened )
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


void QRezix::toggleAutoResponder(){
	activateAutoResponder( btnAutoResponder -> isOn() );
}


void QRezix::activateAutoResponder( bool state ){
	if( btnAutoResponder -> isOn() != state ) btnAutoResponder -> toggle();

	if (state == (RzxConfig::autoResponder() != 0)) return;
	RzxConfig::setAutoResponder( state );
	RzxServerListener * server = RzxServerListener::object();
	RzxComputer * localhostObject = RzxConfig::localHost();
	server -> sendRefresh(localhostObject);
}

void QRezix::activateFavorites(){
	RzxConfig::setFavoritesMode(btnFavorites->isOn()?1:0);
	rezal->sort();
}

void QRezix::toggleVisible(){
	if(isVisible())
		hide();
	else{
		showNormal(); 
		if(statusMax) showMaximized();
			else showNormal();
	show();
		setActiveWindow();raise();
		alreadyOpened=true;
		rezal->adapteColonnes();
	}
}

void QRezix::chatSent() {
	// Desactive le répondeur lorsqu'on envoie un chat
	activateAutoResponder( false );
}

