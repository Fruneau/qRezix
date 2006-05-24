/***************************************************************************
                               rzxtraywindow.h
         Gestion des fenêtres popups de notification de la trayicon
                             -------------------
    begin                : Tue Nov 16 2004
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
#ifndef RZXTRAYWINDOW_H
#define RZXTRAYWINDOW_H

#include <QFrame>
#include <QTimer>
#include <QPointer>

#include <RzxComputer>

class QMouseEvent;

///Gestion de la fenêtre qui s'affiche lorsque les favoris changent d'état
/** Construit, affiche et détruit la fenêtre.
 *
 * Dans l'état actuel la fenêtre apparaît en haut à gauche pour une durée de 5s
 * et est constituée des éléments suivant :
 * 	- un fond dont la couleur correspond à l'action (ces couleurs sont définies
 * dans les données de configuration de RzxConfig)
 * 	- l'icône de la personne dont l'état à changé
 * 	- le nom de la personne concerné
 * 	- un texte décrivant le nouvel état
 */
class RzxTrayWindow: public QFrame
{
	Q_OBJECT
	
	public:
		enum Theme {
			None = 0,
			Nice = 1,
			Old = 2,
			Growl = 3
		};
		Q_ENUMS(Theme)
		
		enum Type {
			ConnectionState = 1,
			Chat = 2
		};
		Q_ENUMS(Type);
	
	private:
		int time;
		QPointer<RzxComputer> computer;
		QPointer<QWidget> active;

		Type type;
	
		QString chatText;
		
		void init(Theme);
	
	public:

		RzxTrayWindow(Theme, RzxComputer *computer, unsigned int time = 4);
		RzxTrayWindow(Theme, RzxComputer *computer, const QString& text, unsigned int time = 4);
		~RzxTrayWindow();

	protected:
		void niceTheme();
		void niceThemeFactory(const QString& title, const QString& text, const QPixmap& icon);
		void oldTheme();
		void oldThemeFactory(const QString& title, const QString& text, const QPixmap& icon, const QColor& bg);
		
#ifdef Q_OS_MAC
		void growlNotif();
		void growlNotifFactory(const QString& title, const QString& text, const QPixmap& icon, const QString& type);
#endif

	protected slots:
		void giveFocus();
		void tryToShow();
		virtual void mousePressEvent(QMouseEvent*);
		virtual bool event(QEvent *);
};

#endif
