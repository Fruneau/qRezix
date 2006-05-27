/***************************************************************************
                     rzxabstractconfig.cpp  -  description
                             -------------------
    begin                : Fri Aug 12 2005
    copyright            : (C) 2005 Florent Bruneau
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
#include <QWidget>
#include <QDockWidget>
#include <QSplitter>
#include <QDesktopWidget>

#include <RzxAbstractConfig>
#include <RzxBaseModule>
#include <RzxApplication>

///Construction d'un RzxAbstractConfig
/** Cette construction initialise le QSettings pour lui donner la configuration
 * nécessaire pour le stockage cohérent des informations entre les modules.
 *
 * Ce constructeur teste également la version du module.
 */
RzxAbstractConfig::RzxAbstractConfig(RzxBaseModule *m_module)
	:QSettings("BR", "qRezix"), module(m_module)
{
	if(module)
	{
		beginGroup(module->name());
		setValue("version", module->versionString());
	}
	else
	{
		beginGroup("general");
		setValue("version", Rzx::versionToString(RzxApplication::version()));
	}
}

///Destruction du RzxAbstractConfig
RzxAbstractConfig::~RzxAbstractConfig()
{
}

///Enregistre la taille de la fenêtre...
/** L'enregistrement de la fenêtre consiste en :
 * - sa taille
 * - sa position
 * - son état
 */
void RzxAbstractConfig::saveWidget(const QString& name, QWidget *widget)
{
	beginGroup(name);

	//On enregistre taille et position
	setValue("size", widget->size());
	setValue("pos", widget->pos());
	setValue("maximized", (bool)(widget->windowState() & Qt::WindowMaximized));
	setValue("minimized", (bool)(widget->windowState() & Qt::WindowMinimized));
	setValue("visible", widget->isVisible());

	//On enregistre l'équilibre des splitters
	QList<QSplitter*> ls = widget->findChildren<QSplitter*>();
	int i = 0;
	foreach(QSplitter *splitter, ls)
	{
		beginGroup(QString("splitter%1").arg(i++));
		QList<int> sizes = splitter->sizes();
		QList<QVariant> variants;
		int j = 0;
		bool save = true;
		foreach(int size, sizes)
		{
			const QWidget *widget = splitter->widget(j++);
			if(!widget || widget->isHidden())
			{
				save = false;
				break;
			}
			variants << size;
		}
		if(save)
			setValue("sizes", variants);
		endGroup();
	}

	endGroup();
}

///Restore l'état de la fenêtre
void RzxAbstractConfig::restoreWidget(const QString& name, QWidget *widget, const QPoint& pos, const QSize& size, bool def)
{
	beginGroup(name);

	//On rétabli la taille de la fenêtre
	QSize s = def?size:value("size", size).toSize();
	widget->resize(s);

	//Remise en place de la fenêtre selon les paramètres enregistrés
	QPoint p = def?pos:value("pos", pos).toPoint();
	//On évite de mettre la fenêtre en position inacessible
	if(p.x() > QApplication::desktop()->width())
		p.setX(QApplication::desktop()->width() - s.width());
	if(p.x() < 0)
		p.setX(0);
	if(p.y() > QApplication::desktop()->height())
		p.setY(QApplication::desktop()->height() - s.height());
#ifndef Q_OS_MAC
	if(p.y() < 0)
		p.setY(0);
#else
	if(p.y() < 20) //La barre de menu de Mac OS occupe le haut de l'écran
		p.setY(20);
#endif
	widget->move(p);

	//Remise en état de la fenêtre au niveau visibilité
	const bool visible = value("visible", false).toBool();
	if(value("maximized", false).toBool())
	{
		widget->setWindowState(widget->windowState() | Qt::WindowMaximized);
		if(visible) widget->showMaximized();
	}
	else if(value("minimized", false).toBool())
	{
		widget->setWindowState(widget->windowState() | Qt::WindowMinimized);
		if(visible) widget->showMinimized();
	}
	else if(visible)
		widget->show();

	if(!visible)
		widget->hide();

	//On rétabli l'équilibre des splitters
	if(!def)
	{
		QList<QSplitter*> ls = widget->findChildren<QSplitter*>();
		int i = 0;
		foreach(QSplitter *splitter, ls)
		{
			beginGroup(QString("splitter%1").arg(i++));
			if(!value("sizes").isNull())
			{
				QList<QVariant> variants = value("sizes").toList();
				QList<int> sizes;
				foreach(QVariant variant, variants)
					sizes << variant.toInt();
				splitter->setSizes(sizes);
			}
			endGroup();
		}
	}

	endGroup();
}
