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
	setValue("size", widget->size());
	setValue("pos", widget->pos());
	setValue("maximized", (bool)(widget->windowState() & Qt::WindowMaximized));
	setValue("minimized", (bool)(widget->windowState() & Qt::WindowMinimized));
	setValue("visible", widget->isVisible());

	QList<QSplitter*> ls = widget->findChildren<QSplitter*>();
	int i = 0;
	foreach(QSplitter *splitter, ls)
	{
		beginGroup(QString("splitter%1").arg(i++));
		QList<int> sizes = splitter->sizes();
		QList<QVariant> variants;
		foreach(int size, sizes)
			variants << size;
		setValue("sizes", variants);
		endGroup();
	}

	endGroup();
}

///Restore l'état de la fenêtre
void RzxAbstractConfig::restoreWidget(const QString& name, QWidget *widget, const QPoint& pos, const QSize& size, bool def)
{
	beginGroup(name);

	QSize s = def?size:value("size", size).toSize();
	widget->resize(s);

	QPoint p = def?pos:value("pos", pos).toPoint();
	if(p.x() > QApplication::desktop()->width() || p.y() > QApplication::desktop()->height())
		p = QPoint(QApplication::desktop()->width() - s.width(), QApplication::desktop()->height() - s.height());
	widget->move(p);

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
