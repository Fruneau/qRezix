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
#include "ui_rzxpropertyui.h"

#define NB_COL 10

/**
  *@author Benoit Casoetto
  */

///Fen�tre de pr�f�rences
/** Cette fen�tre regroupe les pr�f�rences globales de qRezix ainsi que celles
 * de tous les modules. Elle est g�n�r�e � partir des donn�es fournies par les modules
 * et fournit un menu arborescent qui simplifie l'utilisation de la fen�tre et
 * la compr�hension de l'organisation du programme.
 */
class RzxProperty : public QDialog, public Ui::frmPref
{
	Q_OBJECT
	RZX_GLOBAL(RzxProperty)

	QTreeWidgetItem *generalItem;
	QTreeWidgetItem *networkItem ;
	QTreeWidgetItem *confItem;

public: 
	RzxProperty(QWidget *parent = NULL);
	~RzxProperty();

	static QString browse(const QString& name, const QString& title, const QString& glob);

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


protected: // Protected methods
	virtual void changeEvent(QEvent*);

private:
	void initLangCombo();
	void initThemeCombo();
	void initStyleCombo();

	bool updateLocalHost();
	QPixmap localhostIcon;

	template<class T>
	void buildModules(const QList<T*>&, QTreeWidget* = NULL, QTreeWidgetItem* = NULL);
	template<class T>
	void initModules(const QList<T*>&, bool def = false);
	template<class T>
	void updateModules(const QList<T*>&);
	template<class T>
	void changeThemeModules(const QList<T*>&, QTreeWidget *, QTreeWidgetItem*);
	template<class T>
	void closeModules(const QList<T*>&);

	QTreeWidgetItem* createPage(QWidget*, const QString&, const QIcon&, QTreeWidgetItem*);

protected slots: // Protected slots
	void launchDirSelectDialog();
	void chooseIcon();
	void lockCmbMenuText(int index);
	void changePage(QTreeWidgetItem*, QTreeWidgetItem*);
};

///Inclu les modules dans la fen�tre de propri�t�s
template<class T>
void RzxProperty::buildModules(const QList<T*>& modules, QTreeWidget *view, QTreeWidgetItem *parent)
{
	foreach(T *module, modules)
	{
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
				item = createPage(NULL, module->name(), module->icon(), parent);
				for(int i = 0 ; i < props.size(); i++)
					createPage(props[i], names[i], module->icon(), item);
			}
		}

		if(view)
		{
			QTreeWidgetItem *item = new QTreeWidgetItem(view);
			item->setIcon(0, module->icon());
			item->setText(0, module->name());
			item->setText(1, module->versionString());
			item->setText(2, module->description());
		}

		buildModules<RzxBaseModule>(children, NULL, item);
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

///Met � jour les donn�es des modules
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

///Change le th�me des modules
template<class T>
void RzxProperty::changeThemeModules(const QList<T*>& modules, QTreeWidget *view, QTreeWidgetItem *parent)
{
	int i = 0;
	foreach(T *module, modules)
	{
		if(view)
			view->topLevelItem(i)->setIcon(0, module->icon());
		QList<QWidget*> props = module->propWidgets();
		QTreeWidgetItem *item = parent->child(i++);

		QList<RzxBaseModule*> children = module->childModules();

		if(props.size() >= 1 || children.size())
		{
			item->setIcon(0, module->icon());
			if(props.size() > 1)
			{
				for(int k = 0 ; k < item->childCount(); k++)
					item->child(k)->setIcon(0, module->icon());
			}
		}

		changeThemeModules<RzxBaseModule>(children, NULL, item);
	}
}

///Ferme les modules
template<class T>
void RzxProperty::closeModules(const QList<T*>& modules)
{
	foreach(T *module, modules)
	{
		if(module)
			module->propClose();

		QList<RzxBaseModule*> children = module->childModules();
		closeModules<RzxBaseModule>(children);
	}
}


#endif
