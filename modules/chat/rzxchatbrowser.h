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
#ifndef RZXCHATBROWSER_H
#define RZXCHATBROWSER_H

#include <QTextBrowser>

#include <RzxUtilsLauncher>

class QUrl;

/**
 @author Florent Bruneau
 */

///Fenêtre d'affichage du chat...
/** Cette fenêtre est munie d'un interpréteur d'url qui permet à l'utilisateur
 * de lancer le client adapté en cliquant sur les url dans le chat...
 */
class RzxChatBrowser: public QTextBrowser
{
	Q_OBJECT

	public:
		RzxChatBrowser(QWidget * = NULL);
		~RzxChatBrowser();

	public slots:
		virtual void setSource(const QUrl&);

	protected slots:
		void launchUrl(const QUrl&);
};

#endif
