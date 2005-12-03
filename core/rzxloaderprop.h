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
#include <RzxIconCollection>

#include <RzxBaseLoader>

#include "ui_rzxloaderui.h"

class RzxProperty;
class QTreeWidgetItem;

///Classe abstraite affichant les outils facilitant le chargement/déchargement d'objets
class RzxBaseLoaderProp:public QWidget, protected Ui::RzxLoaderUI
{
	Q_OBJECT
	QTreeWidgetItem *parentItem;

	public:
		RzxBaseLoaderProp(QWidget *parent = NULL);
		~RzxBaseLoaderProp();

		virtual void init();
		void connectToPropertyWindow(RzxProperty * = NULL) const;

	public slots:
		void setPropertyParent(QTreeWidgetItem * = NULL);

	protected slots:
		virtual void load() = 0;
		virtual void reload() = 0;
		virtual void itemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous) = 0;

		virtual void changeTheme() = 0;

		void emitNotifyLoad(const QString&);
		void emitNotifyUnload(const QString&);

	signals:
		void notifyLoad(const QString&, QTreeWidgetItem*);
		void notifyUnload(const QString&, QTreeWidgetItem*);
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

///Construction de l'sobjet
template <class T>
RzxLoaderProp<T>::RzxLoaderProp(QWidget *parent)
	:RzxBaseLoaderProp(parent)
{
	module = NULL;
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
	changeTheme();
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
		btnLoad->setIcon(RzxIconCollection::getIcon(Rzx::ICON_LOAD));
	}
	else
	{
		btnReload->setEnabled(true);
		btnLoad->setEnabled(true);
		btnLoad->setText(tr("Unload"));
		btnLoad->setIcon(RzxIconCollection::getIcon(Rzx::ICON_UNLOAD));
	}
}

///Demande le (dé)chargement du module
template <class T>
void RzxLoaderProp<T>::load()
{
	if(module)
	{
		emitNotifyUnload(module->name());
		loader->unloadModule(module);
	}
	else if(!name.isNull())
	{
		loader->loadModule(name);
		emitNotifyLoad(name);
	}
	itemChanged(treeModules->currentItem());
}

///Demande le rechargement du module
template <class T>
void RzxLoaderProp<T>::reload()
{
	if(!name.isNull())
	{
		emitNotifyUnload(name);
		loader->reloadModule(module->name());
		emitNotifyLoad(name);
	}
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
	btnReload->setIcon(RzxIconCollection::getIcon(Rzx::ICON_RELOAD));
	btnLoad->setIcon(RzxIconCollection::getIcon(module?Rzx::ICON_UNLOAD:Rzx::ICON_LOAD));
}

#endif
