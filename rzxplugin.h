/***************************************************************************
                         rzxplugin.h  -  description
                             -------------------
    begin                : Thu Jul 19 2004
    copyright            : (C) 2004 by Florent Bruneau
    email                : fruneau@melix.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef RZXPLUGIN_H
#define RZXPLUGIN_H

#include <qstring.h>
#include <qstringlist.h>
#include <qpopupmenu.h>
#include <qobject.h>
#include <qsettings.h>
#include <qvariant.h>


/**
@author Florent Bruneau
*/

class RzxPlugIn : public QObject
{
	Q_OBJECT

	QString name;
	QString description;

	public:
		//Donn�es pour les requ�tes de donn�es
		enum Data
		{
			DATA_NONE = 0,
			DATA_SERVERFTP = 1,	//bool
			DATA_SERVERHTTP = 2,	//bool
			DATA_SERVERNEWS = 3,	//bool
			DATA_SERVERSMB = 4,	//bool
			DATA_DNSNAME = 5,		//QString
			DATA_NAME = 6,			//QString
			DATA_FIRSTNAME = 7,	//QString
			DATA_SURNAME = 8,		//QString
			DATA_IP = 9				//QString
		};

	protected:
		bool prop;

		QPopupMenu *tray;
		QPopupMenu *item;
		QStringList *action;

		/*Les fonctions qui suivents fournissent au plug-in
		le mat�riel pour enregistrer et lire des donn�es de
		configuration de mani�re organis�e*/
		QSettings *settings;

		void initSettings(const QString& path);
		QString readEntry(const QString& name, const QString& def);
		bool writeEntry(const QString& name, const QString& value);

	private:
		RzxPlugIn();
		
	public:
		RzxPlugIn(const QString& nm, const QString& desc);
		virtual ~RzxPlugIn();

		//Il est du devoir du concepteur de plug-in
		//de mettre � jour les champs name, description et prop
		//pour que ces valeurs fonction renvoient des donn�es sens�es
		QString getName();
		QString getDescription();
		bool hasProp();

		/* Partie abstraite devant �tre r�impl�ment�e dans chaque
		classe fille qui d�finira un plug-in */
		virtual QPopupMenu *getTrayMenu() = 0;
		virtual QPopupMenu *getItemMenu() = 0;
		virtual QStringList *getActionMenu() = 0;

		virtual void init(const QString& path, const QString& ip) = 0;
		virtual void stop() = 0;

		virtual void properties() = 0;

		//slots et signaux g�n�rique assurant les relations entre le plug-in lui m�me
		//et l'interface de qrezix
		//ce sont les seuls qui puissent �tre g�r� par l'interface de rezix
	public slots:
		virtual void getData(Data data, const QVariant *value) = 0;

	protected slots:
		void sender(const QString& msg);
		void querySender(Data data);

	signals:
		void send(const QString& msg);
		void queryData(RzxPlugIn::Data data, RzxPlugIn *plugin);
};

#endif
