/***************************************************************************
           rzxfavoritelist  -  gestion d'une liste de favoris
                             -------------------
    begin                : Wed Dec 14 2005
    copyright            : (C) 2005 by Florent Bruneau
    email                : florent.bruneau@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QUrl>

#include <RzxUtilsLauncher>

#include "rzxchatbrowser.h"

///Construction
RzxChatBrowser::RzxChatBrowser(QWidget *parent)
	:QTextBrowser(parent)
{
	connect(this, SIGNAL(anchorClicked(const QUrl&)), this, SLOT(launchUrl(const QUrl&)));
}

///Fermeture
RzxChatBrowser::~RzxChatBrowser()
{
}

///Pour �viter de suivre les url dans la fen�tre de chat...
/** De toute fa�on �a ne marcherait pas !
 */
void RzxChatBrowser::setSource(const QUrl&)
{
}

///Lance le client adapt� � l'url cliqu�e
void RzxChatBrowser::launchUrl(const QUrl& url)
{
	RzxUtilsLauncher::run(url);
}
