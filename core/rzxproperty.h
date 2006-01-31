/***************************************************************************
                          qrezix.h  -  description
                             -------------------
    copyright            : (C) 2002 by Benoit Casoetto
    email                : benoit.casoetto@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RZXPROPERTY_H
#define RZXPROPERTY_H

#include <QDialog>
#include <QListWidget>
#include <QTreeWidget>
#include <QStackedWidget>

#include <RzxGlobal>
#include <RzxBaseModule>
#include "ui_rzxproperty.h"

/**
  *@author Benoit Casoetto
  */

///Fenêtre de préférences
/** Cette fenêtre regroupe les préférences globales de qRezix ainsi que celles
 * de tous les modules. Elle est générée à partir des données fournies par les modules
 * et fournit un menu arborescent qui simplifie l'utilisation de la fenêtre et
 * la compréhension de l'organisation du programme.
 */
class RZX_CORE_EXPORT RzxProperty : public QDialog, private Ui::RzxProperty
{
	Q_OBJECT

	QTreeWidgetItem *generalItem;
	QTreeWidgetItem *favoritesItem;
	QTreeWidgetItem *layoutItem;
	QTreeWidgetItem *modulesItem;
	QTreeWidgetItem *networkItem ;

	///Indexes des différentes pages prédéfinies
	enum Pages {
		UserInfo = 0,
		Favorites = 1,
		Layout = 2,
		Modules = 3,
		Network = 4,
		Blank = 5
	};

	///Stocke les données liées à un sport
	struct Sport {
		const QString icon;
		const char* name;
		const int id;
	};
	static const Sport sports[];
	friend bool sportLessThan(const Sport*, const Sport*);

public: 
	RzxProperty(QWidget *parent = NULL);
	~RzxProperty();

	static QString browse(const QString& name, const QString& title, const QString& glob, QWidget *parent = NULL);

	QString infoNeeded();
	int infoCompleteMessage();

signals:
	void end();
	
public slots: // Public slots
	/** Met a jour la base globale de configuration a partir des modifications de l'utilisateur */
	bool miseAJour();
	void oK();
	void annuler();
	void initDlg(bool def = false);
	void aboutQt();
	void changeTheme();

	void addModule(const QString&, QTreeWidgetItem*);
	void deleteModule(const QString&, QTreeWidgetItem*);

	virtual void setVisible(bool);

protected: // Protected methods
	virtual void changeEvent(QEvent*);

private:
	void initLangCombo();
	void initThemeCombo();
	void initStyleCombo();
	void initSportCombo();
	void initPromoCombo();

	void updateLocalHost();
	QPixmap localhostIcon;

	template<class T>
	void buildModules(const QList<T*>&, QTreeWidgetItem* = NULL);
	template<class T>
	void initModules(const QList<T*>&, bool def = false);
	template<class T>
	void updateModules(const QList<T*>&);
	template<class T>
	void changeThemeModules(const QList<T*>&, QTreeWidgetItem*);
	template<class T>
	void closeModules(const QList<T*>&);

	void deletePage(QTreeWidgetItem*);
	void rebuildIndexes(const int, QTreeWidgetItem* = NULL);

	QTreeWidgetItem* createPage(QWidget*, const QString&, const QIcon&, QTreeWidgetItem*);

protected slots: // Protected slots
	void launchDirSelectDialog();
	void chooseIcon();
	void lockCmbMenuText(int index);
	void changePage(QTreeWidgetItem*, QTreeWidgetItem*);

	void changeTheme(const QString& text);
	void changeLanguage(const QString& language);
	void fillThemeView();
	void fillCacheList();
	void fillCacheView();
	void sendMailToCache();
	void deleteCache();
	void tryDeleteCache();

	void emitEnterPressed();

signals:
	///Ce signal est émis lors de la pression de Del ou Backspace
	/** Permet de centraliser la gestion de l'appuie de ces deux touches
	 * et que TOUTES les widgets concernés puisse le recevoir et l'analyser
	 */
	void deleteWanted();

	///Ce signal est émis lorsque la touche Enter est pressée...
	/** Cela permet de partager l'information avec toutes les sous-fenêtres
	 * qui peuvent nécessiter la pression de Enter avant valider le contenu
	 * de la fenêtre.
	 *
	 * Si used = true, quelqu'un a déjà utilisé l'information
	 */
	void enterPressed(bool &used);
};

///Inclu les modules dans la fenêtre de propriétés
template<class T>
void RzxProperty::buildModules(const QList<T*>& modules, QTreeWidgetItem *parent)
{
	foreach(T *module, modules)
	{
		if(!module)
			continue;
		QList<QWidget*> props = module->propWidgets();
		QStringList names = module->propWidgetsName();
		QTreeWidgetItem *item = parent;

		QList<RzxBaseModule*> children = module->childModules();

		if(props.size() >= 1 || children.size())
		{
			if(props.size() == 1 || (!props.size() && children.size()))
				item = createPage(props[0], names[0], module->icon(), parent);
			else
			{
				item = createPage(props[0], module->name(), module->icon(), parent);
				for(int i = 1 ; i < props.size(); i++)
					createPage(props[i], names[i], module->icon(), item);
			}
			module->setTreeItem(item);
		}

		buildModules<RzxBaseModule>(children, item);
	}
}

///Initialise les modules
template<class T>
void RzxProperty::initModules(const QList<T*>& modules, bool def)
{
	foreach(T *module, modules)
	{
		module->propInit(def);

		QList<RzxBaseModule*> children = module->childModules();
		initModules<RzxBaseModule>(children, def);
	}
}

///Met à jour les données des modules
template<class T>
void RzxProperty::updateModules(const QList<T*>& modules)
{
	foreach(T *module, modules)
	{
		module->propUpdate();

		QList<RzxBaseModule*> children = module->childModules();
		updateModules<RzxBaseModule>(children);
	}
}

///Change le thème des modules
template<class T>
void RzxProperty::changeThemeModules(const QList<T*>& modules, QTreeWidgetItem *parent)
{
	int i = 0;
	foreach(T *module, modules)
	{
		const QList<QWidget*> props = module->propWidgets();
		QTreeWidgetItem *item = parent->child(i++);

		QList<RzxBaseModule*> children = module->childModules();

		if(item && (props.size() >= 1 || children.size()))
		{
			item->setIcon(0, module->icon());
			if(props.size() > 1)
			{
				for(int k = 0 ; k < props.size() -1 ; k++)
					item->child(k)->setIcon(0, module->icon());
			}
		}

		changeThemeModules<RzxBaseModule>(children, item);
	}
}

///Ferme les modules
template<class T>
void RzxProperty::closeModules(const QList<T*>& modules)
{
	prefStack->setCurrentIndex(0);
	foreach(T *module, modules)
	{
		if(module)
			module->propClose();

		QList<RzxBaseModule*> children = module->childModules();
		closeModules<RzxBaseModule>(children);
	}
}

#endif
