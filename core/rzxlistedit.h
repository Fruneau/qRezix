/***************************************************************************
            rzxlistedit  -  fenêtre d'édition de computer list
                             -------------------
    begin                : Thu Dec 15 2005
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
#ifndef RZXLISTEDIT_H
#define RZXLISTEDIT_H

#include <QWidget>

/**
 @author Florent Bruneau
 */

class RzxComputerList;
class RzxComputer;
class RzxHostAddress;
namespace Ui { class RzxListEdit; }

///Fenêtre d'édition des listes d'ordinateurs
/** Cette classe permet l'édition simple des RzxComputerList de façon
 * totalement transparente pour l'utilisateur qui n'a à choisir que des
 * informations choisies pour être lisibles...
 *
 * Cette objet ce présente sous la forme de :
 * 	- une ligne d'édition et un bouton ajouter
 * 	- une liste d'élément et un bouton supprimer
 */
class RzxListEdit: public QWidget
{
	Q_OBJECT

	Ui::RzxListEdit *ui;
	RzxComputerList *list;

	public:
		RzxListEdit(QWidget *parent = NULL);
		~RzxListEdit();

	public slots:
		void setList(RzxComputerList *);
		void changeTheme();
		void refresh();

	protected slots:
		void add();
		void add(const QString&);
		void add(const RzxHostAddress&);
		void remove();
		void tryRemove();
		void enterPressed(bool&);
		void edited(const QString&);
		void selectionChanged();
		void refresh(RzxComputer*);
		void connectComputer(RzxComputer*);
		void lightRefresh();
};

#endif
