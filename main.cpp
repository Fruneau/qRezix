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

#include <QApplication>
#include <QTextCodec>
#include <QPixmap>
#include <QBitmap>
#include <QMenu>
#include <QDir>
#include <QString>

#include "rzxglobal.h"

#include "rzxconfig.h"
#include "qrezix.h"
#include "rzxtrayicon.h"
#include "rzxpluginloader.h"

#if defined(Q_OS_UNIX) && !defined(Q_OS_BSD) && !defined(Q_OS_MAC)
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
	for(int i=1; i<argc; i++)
		if(strncmp(argv[i],"--log-debug=",12)==0)
		{
			int len = strlen(argv[i])-12;
			char *logfile_name = new char[len+1];
			memcpy(logfile_name,argv[i]+12,len+1);
			FILE *logfile = fopen(logfile_name,"a");
			Rzx::useOutputFile(logfile);
			delete[] logfile_name;
			break;
		}

	Rzx::installMsgHandler();
	qDebug((QString("qRezix ") + VERSION + RZX_TAG_VERSION + "\n").toAscii().constData());
	
#if defined(Q_OS_UNIX) && !defined(Q_OS_BSD) && !defined(Q_OS_MAC)
	default_segv_handler = signal( SIGSEGV, fatalHandler );
	default_pipe_handler = signal( SIGPIPE, SIG_IGN );
	default_term_handler = signal( SIGTERM, sigTermHandler );
	default_int_handler = signal( SIGINT, sigTermHandler );
#endif
	
	QRezix *rezix = QRezix::global();
	if(rezix->wellInit)
	{
		a.setWindowIcon(QRezix::qRezixIcon());
		QObject::connect(RzxConfig::global(), SIGNAL(languageChanged()), rezix, SLOT(languageChanged()));
		
//		rezix->setWindowIcon(QRezix::qRezixIcon());
		rezix->tray = new TrayIcon(QRezix::qRezixIcon(), "Rezix", rezix );
	
		QObject::connect(rezix->tray,SIGNAL(clicked(const QPoint&)),rezix,SLOT(toggleVisible()));
		QObject::connect(rezix,SIGNAL(setToolTip(const QString &)),rezix->tray,SLOT(setToolTip(const QString &)));
		
		rezix->launchPlugins();
		
		QString windowSize=RzxConfig::readWindowSize();
		#ifndef Q_OS_MAC
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
		
		if(RzxConfig::global()->useSystray())
			rezix->tray->show();
		else
			rezix->show();
		
		QObject::connect(&a, SIGNAL(aboutToQuit()), rezix, SLOT(saveSettings()));
		
		rezix->changeTrayIcon();
		
		int retour = a.exec();
		delete rezix;
		if(RzxConfig::global())
			delete RzxConfig::global();
		Rzx::closeMsgHandler();
		return retour;
	}
	delete rezix;
	Rzx::closeMsgHandler();
	return 0;
}
