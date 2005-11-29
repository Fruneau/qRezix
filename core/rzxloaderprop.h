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
#ifndef RZXLOADERPROP_H
#define RZXLOADERPROP_H

/**
 @author Florent Bruneau
 */

#include <QWidget>

#include <RzxBaseLoader>

#include "ui_rzxloaderui.h"

///Classe abstraite affichant les outils facilitant le chargement/déchargement d'objets
class RzxBaseLoaderProp:public QWidget, protected Ui::RzxLoaderUI
{
	Q_OBJECT

	public:
		RzxBaseLoaderProp(QWidget *parent = NULL);
		~RzxBaseLoaderProp();

		virtual void init();

	protected slots:
		virtual void load() = 0;
		virtual void reload() = 0;
		virtual void itemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous) = 0;

		virtual void changeTheme() = 0;
};

template <class T>
class RzxLoaderProp:public RzxBaseLoaderProp
{
	RzxBaseLoader<T> *loader;
	T *module;
	QString name;	

	public:
		RzxLoaderProp(QWidget *parent = NULL);
		~RzxLoaderProp();

		void setLoader(RzxBaseLoader<T> *);
		virtual void init();

	protected:
		virtual void load();
		virtual void reload();
		virtual void itemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous = NULL);

		virtual void changeTheme();
};

///Construction de l'objet
template <class T>
RzxLoaderProp<T>::RzxLoaderProp(QWidget *parent)
	:RzxBaseLoaderProp(parent)
{
}

///Destruction de l'object
template <class T>
RzxLoaderProp<T>::~RzxLoaderProp()
{
}

///Définition du loader associé
/** Et remplissage de la fenêtre avec les informations adaptées
 */
template <class T>
void RzxLoaderProp<T>::setLoader(RzxBaseLoader<T> *m_loader)
{
	loader = m_loader;
	init();
}

///Initialise le contenu de l'objet
template <class T>
void RzxLoaderProp<T>::init()
{
	QStringList modules = loader->modules.keys();
	foreach(QString name, modules)
	{
		QTreeWidgetItem *item = new QTreeWidgetItem(treeModules);
		T *module = loader->modules[name];
		if(module)
		{
			item->setIcon(0, module->icon());
			item->setText(0, module->name());
			item->setText(1, module->versionString());
			item->setText(2, module->description());
		}
		else
			item->setText(0, name);
	}
}

///Prise en compte du changement d'objet sélectionné
template <class T>
void RzxLoaderProp<T>::itemChanged(QTreeWidgetItem * current, QTreeWidgetItem *)
{
	if(!current)
	{
		name = QString();
		module = NULL;
		btnLoad->setEnabled(false);
		btnReload->setEnabled(false);
		return;
	}

	name = current->text(0);
	module = loader->modules[name];
	if(!module)
	{
		btnReload->setEnabled(false);
		btnLoad->setEnabled(true);
		btnLoad->setText(tr("Load"));
	}
	else
	{
		btnReload->setEnabled(true);
		btnLoad->setEnabled(true);
		btnLoad->setText(tr("Unload"));
	}
}

///Demande le (dé)chargement du module
template <class T>
void RzxLoaderProp<T>::load()
{
	if(module)
		loader->unloadModule(module);
	else if(!name.isNull())
		loader->loadModule(name);
	itemChanged(treeModules->currentItem());
}

///Demande le rechargement du module
template <class T>
void RzxLoaderProp<T>::reload()
{
	if(!name.isNull())
		loader->reloadModule(module->name());
	itemChanged(treeModules->currentItem());
}

///Change le thème d'icône...
template <class T>
void RzxLoaderProp<T>::changeTheme()
{
	for(int i = 0 ; i < treeModules->topLevelItemCount() ; i++)
	{
		QTreeWidgetItem *item = treeModules->topLevelItem(i);
		T *module = loader->modules[item->text(0)];
		if(module)
			item->setIcon(0, module->icon());
	}
}

#endif
