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
/** Le but de cette classe est de réunir tous les composants...
 */
class RZX_CORE_EXPORT RzxApplication:public QApplication, public RzxBaseLoader<RzxModule>
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
		static void displayHelp();
		static Rzx::Version version();
		
		static RzxApplication *instance();
		static QWidget *mainWindow();
		static QList<RzxModule*> modulesList();

		static RzxModule *chatModule();
		static RzxModule *propertiesModule();
		static RzxModule *chatUiModule();
		
	protected:
		bool loadCore();
		void installHider(RzxModule*);
		
		virtual void loadBuiltins();
		virtual bool installModule(RzxModule*);
		virtual void linkModules();
		virtual void relinkModules(RzxModule* = NULL, RzxModule* = NULL);
		virtual void unloadModule(RzxModule*);

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


#endif
