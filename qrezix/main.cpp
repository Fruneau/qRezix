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

#ifdef Q_OS_UNIX

/* for signal handling */
#include <signal.h>
#include <stdio.h>

#include <execinfo.h>
typedef void ( *sighandler_t ) ( int );

sighandler_t default_segv_handler, default_pipe_handler, default_term_handler, default_int_handler;

void nonfatalHandler( int signum )
{
	qWarning( "Received a %i signal, continuing", signum );
}

void fatalHandler( int signum )
{
	void * array[ 128 ];
	size_t size;
	char **strings;

	//restores the default behaviour
	signal( SIGSEGV, default_segv_handler );

	size = backtrace ( array, 128 * sizeof( void* ) );
	strings = backtrace_symbols ( array, size );
	qDebug( "%s", strings[ 0 ] );
	for ( uint i = 0; i < size; i++ )
	{
		qDebug( "[frame %i]: %s", i, strings[ i ] );
	}
	qDebug( "Received a %i signal, automatic backtrace", signum );
	qDebug( "State of the stack: %i frames", size );

	delete strings;
	QApplication::exit( 1 );
}

void sigTermHandler( int signum )
{
	void * array[ 128 ];
	size_t size;
	char **strings;

	size = backtrace ( array, 128 * sizeof( void* ) );
	strings = backtrace_symbols ( array, size );
	qDebug( "%s", strings[ 0 ] );
	for ( uint i = 0; i < size; i++ )
	{
		qDebug( "[frame %i]: %s", i, strings[ i ] );
	}
	qDebug( "Received a %i signal, automatic backtrace", signum );
	qDebug( "State of the stack: %i frames", size );

	QApplication::exit( 255 );
	qDebug( "Terminated" );
}
#endif

int main(int argc, char *argv[])
{
	QApplication a(argc,argv);

	QPixmap iconeProg((const char **)q);
	iconeProg.setMask(iconeProg.createHeuristicMask() );

#ifdef Q_OS_UNIX
	default_segv_handler = signal( SIGSEGV, fatalHandler );
	default_pipe_handler = signal( SIGPIPE, SIG_IGN );
	default_term_handler = signal( SIGTERM, sigTermHandler );
	default_int_handler = signal( SIGINT, sigTermHandler );
#endif
	
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
		
		QString windowSize=RzxConfig::readWindowSize();
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
			rezix->move(RzxConfig::readWindowPosition()); //QPoint(2,24));
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
