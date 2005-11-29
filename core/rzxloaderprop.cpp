/***************************************************************************
                         rzxloaderprop  -  description
                            -------------------
   begin                : Mon Nov 29 2005
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
#include <RzxIconCollection>

#include <RzxLoaderProp>

///Construction de la fen�tre � partir de l'Ui
RzxBaseLoaderProp::RzxBaseLoaderProp(QWidget *parent)
	:QWidget(parent)
{
	setupUi(this);
	RzxIconCollection::connect(this, SLOT(changeTheme()));
	connect(btnLoad, SIGNAL(clicked()), this, SLOT(load()));
	connect(btnReload, SIGNAL(clicked()), this, SLOT(reload()));
	connect(treeModules, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
		this, SLOT(itemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
}

///Fermeture de la fen�tre
RzxBaseLoaderProp::~RzxBaseLoaderProp()
{
}

///Initialise la fen�tre
void RzxBaseLoaderProp::init()
{
	changeTheme();
}
