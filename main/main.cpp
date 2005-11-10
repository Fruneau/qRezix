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

#include <RzxApplication>


#ifdef Q_OS_UNIX
/* for signal handling */
#include <signal.h>
#include <stdio.h>

typedef void ( *sighandler_t ) ( int );

sighandler_t default_segv_handler, default_pipe_handler, default_term_handler, default_int_handler;

///Interception des erreurs non fatales
/** Ce qui comprend en particuler le SIGPIPE... qui peut être utile dans certaine condition
 * en cas de problèmes avec le réseau
 */
void nonfatalHandler( int signum )
{
	qWarning( "Received a %i signal, continuing", signum );
}

#ifdef Q_OS_LINUX
#include <execinfo.h>

///Interception des erreurs fatales
/** Sous Linux, cette fonction permet l'affichage d'un backtrace automatiquement lors d'un
 * crash de qRezix
 */
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
#endif

///Interception de SIGTERM
/** Par extension, on l'associe également à SIGINT pour terminer proprement
 * lors de l'arrêt via Ctrl+C
 */
void sigTermHandler(int)
{
	static bool test = true;
	QApplication::exit( 255 );
	if(test)
		qDebug( "Terminated" );
	test = false;
}

///Fonction principale du programme
/** Cette fonction ne fait en fait que lancer le RzxApplication
 */
int main(int argc, char *argv[])
{
#ifdef Q_OS_LINUX
	default_segv_handler = signal( SIGSEGV, fatalHandler );
#else
	default_segv_handler = signal( SIGSEGV, sigTermHandler );
#endif
	default_pipe_handler = signal( SIGPIPE, SIG_IGN );
	default_term_handler = signal( SIGTERM, sigTermHandler );
	default_int_handler = signal( SIGINT, sigTermHandler );
#else
int main(int argc, char *argv[])
{
#endif
	RzxApplication a(argc,argv);
	int returnCode = 0;
	if(a.isInitialised())
		a.exec();
	return returnCode;
}
