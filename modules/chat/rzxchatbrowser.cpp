/***************************************************************************
              rzxchatbrowser  -  permet de parcourir les chats
                             -------------------
    begin                : Wed Jan 12 2006
    copyright            : (C) 2006 by Florent Bruneau
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
#include "rzxfilesocket.h"
#include "rzxchatlister.h"
#include "rzxfilelistener.h"


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

///Pour éviter de suivre les url dans la fenêtre de chat...
/** De toute façon ça ne marcherait pas !
 */
void RzxChatBrowser::setSource(const QUrl&)
{
}

///Lance le client adapté à l'url cliquée
void RzxChatBrowser::launchUrl(const QUrl& url)
{
	const QString scheme = url.scheme();
	const QString authority = url.authority();
	if (url.scheme() == "RzxTransfer")
	{
		fileTransfer(url.authority());
	}
	else
		RzxUtilsLauncher::run(url);
}

void RzxChatBrowser::fileTransfer(const QString& id)
{
	bool ok = false;
	int n = id.toInt(&ok);
	if (ok)
		fileTransfer(n);
}

// Valide le transfert de fichier 
void RzxChatBrowser::fileTransfer(int id)
{
	RzxFileSocket* socket = RzxChatLister::global()->fileListener()->getSocket(id);
	if(socket && socket->widget)
		emit socket->widget->emitAccept();
}
