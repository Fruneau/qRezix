/***************************************************************************
main.cpp  -  description
-------------------
begin                : jeu jan 24 03:26:29 CET 2002
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

#include <qtextcodec.h>
#include "qrezix.h"
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpopupmenu.h>
#include <qdir.h>
#include <qstring.h>
#include "q.xpm"
#include "rzxconfig.h"

#include <qapplication.h>


#include "trayicon.h"
#include "rzxpluginloader.h"


int main(int argc, char *argv[])
{
	QApplication a(argc,argv);

	QPixmap iconeProg((const char **)q);
	iconeProg.setMask(iconeProg.createHeuristicMask() );
	
	
	QRezix *rezix = new QRezix();
	if(rezix->wellInit)
	{
		RzxConfig::globalConfig();
		
		QObject::connect(RzxConfig::globalConfig(), SIGNAL(languageChanged()), rezix, SLOT(languageChanged()));
		
		rezix -> setIcon(iconeProg);
		rezix -> languageChanged();
		rezix -> tray = new TrayIcon(iconeProg, "Rezix", rezix );
	
		QObject::connect(rezix->tray,SIGNAL(clicked(const QPoint&)),rezix,SLOT(toggleVisible()));
		QObject::connect(rezix,SIGNAL(setToolTip(const QString &)),rezix->tray,SLOT(setToolTip(const QString &)));
		
		rezix->launchPlugins();
		
		a.setMainWidget(rezix);
		
		QString windowSize=RzxConfig::globalConfig()->readWindowSize();
		#ifndef Q_OS_MACX
		if(windowSize.left(1)=="1")
			rezix->statusMax=true;
		else
		#endif
		{
			rezix->statusMax=false;
			int height=windowSize.mid(1,4).toInt();
			int width =windowSize.mid(5,4).toInt();
			rezix->resize(QSize(width,height));
			rezix->move(QPoint(0,0));
		}
		
		if(RzxConfig::globalConfig()->useSystray())
			rezix->tray->show();
		else
			rezix->show();
		
		QObject::connect(&a, SIGNAL(aboutToQuit()), rezix, SLOT(saveSettings()));
		
		rezix->changeTrayIcon();
		
		return a.exec();
	}
}
