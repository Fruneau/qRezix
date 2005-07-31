/***************************************************************************
                          rzxapplication  -  description
                             -------------------
    begin                : Wed Jul 27 2005
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
#ifndef RZXAPPLICATION_H
#define RZXAPPLICATION_H

#include <QApplication>

class QRezix;
class RzxTrayIcon;
class RzxChatLister;
class RzxComputer;
class RzxProperty;
class RzxNotifier;

/**
@author Florent Bruneau
*/

///Classe principale de l'application
/** Le but de cette classe est de r�unir tous les composants...
 */
class RzxApplication:public QApplication
{
	Q_OBJECT
	Q_PROPERTY(bool initialised READ isInitialised)

	QRezix *mainui;
	RzxTrayIcon *tray;
	RzxProperty *properties;
	RzxChatLister *chat;
	RzxNotifier *notifier;

	bool wellInit;

	public:
		RzxApplication(int argc, char **argv);
		~RzxApplication();
		bool isInitialised() const;
		bool hasTrayicon() const;
		bool hasMainWindow() const;
		static RzxApplication *instance();
		static QWidget *mainWindow();

	protected:
		bool loadCore();
		bool loadModules();

	public slots:
		void saveSettings();
		void preferences();

	protected slots:
		void toggleResponder();
};

///Indique si l'application a �t� initialis�e sans encombre
/** Si le flags est faux, l'application est reconnue comme n'�tant
 * pas en �tat de fonctionner..., il faut donc faire attention � ne pas
 * lancer l'application ou utiliser des modules
 */
inline bool RzxApplication::isInitialised() const
{
	return wellInit;
}

///Instance de l'application
inline RzxApplication *RzxApplication::instance()
{
	return qobject_cast<RzxApplication*>(QApplication::instance());
}

///Retourne un pointeur vers la fen�tre principale
/** L'int�r�t de cette fonction est de fournir un moyen simple
 * de conna�tre la fen�tre principale pour avoir par un parent...
 */
inline QWidget *RzxApplication::mainWindow()
{
	return (QWidget*)instance()->mainui;
}

///Indique si l'application b�n�ficie d'une trayicon
/** La trayicon a un statut particulier car elle permet � l'application
 * d'avoir une int�raction 'discr�te'...
 */
inline bool RzxApplication::hasTrayicon() const
{
	return tray != NULL;
}

///Indique si l'application a une fen�tre principale
/** La fen�tre principale est une fen�tre permettant un int�raction maximale
 * entre l'utilisateur et le programme.
 */
inline bool RzxApplication::hasMainWindow() const
{
	return mainui != NULL;
}

#endif
