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
#include <QColor>

#include <RzxIconCollection>

#include <RzxBaseLoader>

#include "ui_rzxloader.h"

class RzxProperty;
class QTreeWidgetItem;

///Classe abstraite affichant les outils facilitant le chargement/d�chargement d'objets
/** Cette classe se charge par elle m�me de se connecter correctement au RzxProperty
 * mail il faut lui faire remonter l'information du QTreeWidgetItem vi setPropertyParent.
 * Pour ceci, et si cette classe est utilis�e dans un module, il faut utiliser la m�thode
 * \ref RzxBaseModule::setTreeItem.
 *
 * Une classe d�riv�e devra simplement d�finir des actions pour :
 * 	- le chargement de l'objet s�lectionn� : \ref load
 * 	- le rechargement de l'objet s�lectionn� : \ref reload
 * 	- le changement de s�lection : \ref itemChanged
 * 	- le changement de th�me d'ic�ne : \ref changeTheme
 *
 * Une impl�mentation par d�faut est disponible via \ref RzxLoaderProp
 */
class RZX_CORE_EXPORT RzxBaseLoaderProp:public QWidget, protected Ui::RzxLoader
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

		void emitNotifyLoad(const QString&);
		void emitNotifyUnload(const QString&);

	protected slots:
		virtual void load() = 0;
		virtual void reload() = 0;
		virtual void itemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous) = 0;

		virtual void changeTheme() = 0;

	signals:
		void notifyLoad(const QString&, QTreeWidgetItem*);
		void notifyUnload(const QString&, QTreeWidgetItem*);
};

///Impl�mentation du RzxBaseLoaderProp adapt� au RzxBaseLoader-RzxBaseModule
/** Cette impl�mentation est con�ue pour s'articul�e autour du coupe
 * \ref RzxBaseLoader - \ref RzxBaseModule et leur classes d�riv�es, et donc � la
 * structure de module int�gr�e � qRezix.
 *
 * Il suffit ensuite d'instancier l'objet via un typedef
 * typedef RzxLoaderProp<MonTypeDeModule> RzxMonTypeLoaderProp;
 *
 * RzxMonTypeLoaderProp est d�s lors utilisable dans QDesigner (enfin dans une
 * ui... en le faisant h�riter de QWidget... enfin pour le faire il faut 
 * faire d�river de QFrame, puis modifier le .ui � la main pour rendre l'h�ritage
 * depuis QWidget et retirer le propri�t�s issues de QFrame).
 */
template <class T>
class RzxLoaderProp:public RzxBaseLoaderProp
{
	RzxBaseLoader<T> *loader;
	T *module;
	bool unloadable;
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

///D�finition du loader associ�
/** Et remplissage de la fen�tre avec les informations adapt�es
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
	unloadable = false;
	QStringList modules = loader->modules.keys();
	changeTheme();
	treeModules->clear();
	foreach(const QString &name, modules)
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
		{
			item->setIcon(0, loader->icons[name]);
			item->setText(0, name);
			item->setText(1, Rzx::versionToString(loader->versions[name]));
			if(loader->descriptions[name])
				item->setText(2, QApplication::translate("RzxBaseModule", loader->descriptions[name]));
			else
				item->setText(2, loader->files[name]);
			item->setTextColor(0, Qt::darkGray);
			item->setTextColor(1, Qt::darkGray);
			item->setTextColor(2, Qt::darkGray);
		}
	}
}

///Prise en compte du changement d'objet s�lectionn�
template <class T>
void RzxLoaderProp<T>::itemChanged(QTreeWidgetItem * current, QTreeWidgetItem *)
{
	if(!current)
		name = QString();
	else
		name = current->text(0);

	if(name.isEmpty())
	{
		module = NULL;
		btnLoad->setEnabled(false);
		btnReload->setEnabled(false);
		return;
	}

	module = loader->modules[name];
	unloadable = module || loader->toBeReloaded.contains(name);
	if(!unloadable)
	{
		btnReload->setEnabled(false);
		btnLoad->setEnabled(true);
		btnLoad->setText(QApplication::translate("RzxLoader", "Load"));
		btnLoad->setIcon(RzxIconCollection::getIcon(Rzx::ICON_LOAD));
	}
	else
	{
		btnReload->setEnabled(module);
		btnLoad->setEnabled(true);
		btnLoad->setText(QApplication::translate("RzxLoader", "Unload"));
		btnLoad->setIcon(RzxIconCollection::getIcon(Rzx::ICON_UNLOAD));
	}
	if(loader->files[name].isNull())
		lblReboot->setText("<i>" + QApplication::translate("RzxLoader", "(Re)Loading this module requires to restart qRezix") + "</i>");
	else
		lblReboot->clear();
}

///Demande le (d�)chargement du module
template <class T>
void RzxLoaderProp<T>::load()
{
	QTreeWidgetItem *item = treeModules->currentItem();
	if(module)
	{
		emitNotifyUnload(module->name());
		loader->unloadModule(module);
		if(item)
		{
			item->setTextColor(0, Qt::darkGray);
			item->setTextColor(1, Qt::darkGray);
			item->setTextColor(2, Qt::darkGray);
		}
	}
	else if(!name.isNull())
	{
		loader->loadModule(name);
		emitNotifyLoad(name);
		T *module = loader->modules[name];
		if(module && item)
		{
			item->setIcon(0, module->icon());
			item->setText(0, module->name());
			item->setText(1, module->versionString());
			item->setText(2, module->description());
			item->setTextColor(0, Qt::black);
			item->setTextColor(1, Qt::black);
			item->setTextColor(2, Qt::black);
		}
	}
	itemChanged(treeModules->currentItem());
}

///Demande le rechargement du module
template <class T>
void RzxLoaderProp<T>::reload()
{
	if(!name.isNull())
	{
		load();
		load();
	}
	itemChanged(treeModules->currentItem());
}

///Change le th�me d'ic�ne...
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
	btnLoad->setIcon(RzxIconCollection::getIcon(unloadable?Rzx::ICON_UNLOAD:Rzx::ICON_LOAD));
}

#endif
