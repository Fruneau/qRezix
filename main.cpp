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

#ifdef WITH_KDE
#include <kapp.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#else
#include <qapplication.h>
#endif

#include "trayicon.h"
#include "rzxpluginloader.h"

#ifdef WITH_KDE
static const char *description =
I18N_NOOP("qReziX");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE


static KCmdLineOptions options[] =
{
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};
#endif

int main(int argc, char *argv[])
{
	
#ifdef WITH_KDE
	KAboutData aboutData( "qrezix", I18N_NOOP("qRezix"),
		VERSION, description, KAboutData::License_GPL,
		"(c) 2002, Sylvain Joyeux", 0, 0, "sylvain.joyeux@m4x.org");
	aboutData.addAuthor("Sylvain Joyeux",0, "sylvain.joyeux@m4x.org");
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
	
	KApplication a;
#else
	QApplication a(argc,argv);
#endif
	QPixmap iconeProg((const char **)q);
	iconeProg.setMask(iconeProg.createHeuristicMask() );
	
	
	QRezix *rezix = new QRezix();
	RzxConfig::globalConfig();
	
	QObject::connect(RzxConfig::globalConfig(), SIGNAL(languageChanged()), rezix, SLOT(languageChanged()));
		
	rezix -> setIcon(iconeProg);
	rezix -> languageChanged();
	rezix -> tray = new TrayIcon(iconeProg, "Rezix", rezix );

	QObject::connect(rezix->tray,SIGNAL(clicked(const QPoint&)),rezix,SLOT(toggleVisible()));
	QObject::connect(rezix,SIGNAL(setToolTip(const QString &)),rezix->tray,SLOT(setToolTip(const QString &)));
	
	a.setMainWidget(rezix);	
	
	QString windowSize=RzxConfig::globalConfig()->readWindowSize();	
	if(windowSize.left(1)=="1")
		rezix->statusMax=true;
	else{
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
		
	return a.exec();
}
