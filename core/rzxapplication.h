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
#include <QPointer>
#include <QList>

#include <RzxBaseLoader>
#include <RzxModule>
#include <RzxProperty>

class RzxComputer;

/**
@author Florent Bruneau
*/

///Classe principale de l'application
/** Le but de cette classe est de r�unir tous les composants...
 */
class Q_DECL_EXPORT RzxApplication:public QApplication, public RzxBaseLoader<RzxModule>
{
	Q_OBJECT
	Q_PROPERTY(bool initialised READ isInitialised)
	
	QPointer<RzxProperty> properties;

	QList<RzxModule*> hiders;
	RzxModule *mainui;
	RzxModule *chatProto;
	RzxModule *chatui;
	RzxModule *propertiesProto;
	RzxModule *propertiesUi;

	bool wellInit;
	static Rzx::Version m_version;

	public:
		RzxApplication(int argc, char **argv);
		~RzxApplication();
		bool isInitialised() const;
		bool hasHider() const;
		bool hasMainWindow() const;
		static Rzx::Version version();
		
		static RzxApplication *instance();
		static QWidget *mainWindow();
		static QList<RzxModule*> modulesList();

		static RzxModule *chatModule();
		static RzxModule *propertiesModule();
		static RzxModule *chatUiModule();
		
	protected:
		bool loadCore();
		
		virtual void loadBuiltins();
		virtual bool installModule(RzxModule*);
		virtual void linkModules();

	public slots:
		void saveSettings();
		void preferences();

		void relayProperties(RzxComputer*);

	protected slots:
		void toggleResponder();
		void activateResponder();
		void deactivateResponder();

	signals:
		///Simple relais pour RzxModule::haveProperties
		void haveProperties(RzxComputer*, bool *);
};

///Retourne la version de qRezix
inline Rzx::Version RzxApplication::version()
{
	return m_version;
}

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

///Indique si l'application b�n�ficie d'une trayicon
/** La trayicon a un statut particulier car elle permet � l'application
 * d'avoir une int�raction 'discr�te'...
 */
inline bool RzxApplication::hasHider() const
{
	return hiders.count();
}

///Indique si l'application a une fen�tre principale
/** La fen�tre principale est une fen�tre permettant un int�raction maximale
 * entre l'utilisateur et le programme.
 */
inline bool RzxApplication::hasMainWindow() const
{
	return mainui != NULL;
}

///Retourne le module de chat
/** Le module retourn� ici est un module � utiliser par d�faut pour le chat si
 * l'ordinateur utilise un protocole qui ne g�re pas le chat.
 */
inline RzxModule *RzxApplication::chatModule()
{
	return instance()->chatProto;
}

///Retourne le module des propri�t�s
/** Le module retourn� ici est un module � utiliser par d�faut pour le check des
 * propri�t�s de l'utilisateur distant si le protocole r�seau via lequel il est connect�
 * ne le permet pas.
 */
inline RzxModule *RzxApplication::propertiesModule()
{
	return instance()->propertiesProto;
}

///Retourne le module de l'interface utilisateur pour le chat
inline RzxModule *RzxApplication::chatUiModule()
{
	return instance()->chatui;
}

#endif
