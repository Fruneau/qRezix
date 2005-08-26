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
#include <QList>

#include <RzxBaseLoader>
#include <RzxModule>

class RzxComputer;
class RzxProperty;

/**
@author Florent Bruneau
*/

///Classe principale de l'application
/** Le but de cette classe est de réunir tous les composants...
 */
class RzxApplication:public QApplication, public RzxBaseLoader<RzxModule>
{
	Q_OBJECT
	Q_PROPERTY(bool initialised READ isInitialised)

	RzxProperty *properties;

	QList<RzxModule*> hiders;
	RzxModule *mainui;
	RzxModule *chat;

	bool wellInit;

	public:
		RzxApplication(int argc, char **argv);
		~RzxApplication();
		bool isInitialised() const;
		bool hasHider() const;
		bool hasMainWindow() const;
		static RzxApplication *instance();
		static QWidget *mainWindow();
		static QList<RzxModule*> modulesList();

	protected:
		bool loadCore();
		
		virtual void loadBuiltins();
		virtual bool installModule(RzxModule*);
		virtual void linkModules();

	public slots:
		void saveSettings();
		void preferences();

	protected slots:
		void toggleResponder();
		void activateResponder();
		void deactivateResponder();

	signals:
		///Simple relais pour RzxModule::haveProperties
		void haveProperties(RzxComputer*, bool *);
};

///Indique si l'application a été initialisée sans encombre
/** Si le flags est faux, l'application est reconnue comme n'étant
 * pas en état de fonctionner..., il faut donc faire attention à ne pas
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

///Indique si l'application bénéficie d'une trayicon
/** La trayicon a un statut particulier car elle permet à l'application
 * d'avoir une intéraction 'discrète'...
 */
inline bool RzxApplication::hasHider() const
{
	return hiders.count();
}

///Indique si l'application a une fenêtre principale
/** La fenêtre principale est une fenêtre permettant un intéraction maximale
 * entre l'utilisateur et le programme.
 */
inline bool RzxApplication::hasMainWindow() const
{
	return mainui != NULL;
}

#endif
