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
#include <RzxApplication>
#include <RzxProperty>

#include <RzxLoaderProp>

///Construction de la fenêtre à partir de l'Ui
RzxBaseLoaderProp::RzxBaseLoaderProp(QWidget *parent)
	:QWidget(parent)
{
	setupUi(this);
	parentItem = NULL;
	RzxIconCollection::connect(this, SLOT(changeTheme()));
	connect(btnLoad, SIGNAL(clicked()), this, SLOT(load()));
	connect(btnReload, SIGNAL(clicked()), this, SLOT(reload()));
	connect(treeModules, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
		this, SLOT(itemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
}

///Fermeture de la fenêtre
RzxBaseLoaderProp::~RzxBaseLoaderProp()
{
}

///Initialise la fenêtre
void RzxBaseLoaderProp::init()
{
	changeTheme();
}

///Défini l'objet parent auquel se rapport cet afficheur
void RzxBaseLoaderProp::setPropertyParent(QTreeWidgetItem *item)
{
	connectToPropertyWindow();
	parentItem = item;
}

///Connecte la fenêtre au RzxProperty
void RzxBaseLoaderProp::connectToPropertyWindow(RzxProperty *prop) const
{
	if(!prop)
		prop = RzxProperty::global();

	connect(this, SIGNAL(notifyLoad(const QString&, QTreeWidgetItem*)), prop, SLOT(addModule(const QString&, QTreeWidgetItem*)));
	connect(this, SIGNAL(notifyUnload(const QString&, QTreeWidgetItem*)), prop, SLOT(deleteModule(const QString&, QTreeWidgetItem*)));
}

///Emet un signal annonçant le chargement d'un module
void RzxBaseLoaderProp::emitNotifyLoad(const QString& name)
{
	emit notifyLoad(name, parentItem);
}

///Emet un signal annonçant le déchargement d'un module
void RzxBaseLoaderProp::emitNotifyUnload(const QString& name)
{
	emit notifyUnload(name, parentItem);
}
