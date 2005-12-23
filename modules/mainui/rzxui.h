/***************************************************************************
                                 rzxui.cpp
          Interface du module pour l'linterface principale de qRezix
                             -------------------
    begin                : Sat Aug 6 2005
    copyright            : (C) 2004 by Florent Bruneau
    email                : fruneau@melix.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RZXUI_H
#define RZXUI_H

#undef RZX_BUILTIN
#undef RZX_PLUGIN
#ifdef RZX_MAINUI_BUILTIN
#	define RZX_BUILTIN
#else
#	define RZX_PLUGIN
#endif

#include <RzxModule>

#include "rzxmainuiglobal.h"

/**
 @author Florent Bruneau
 */

class QRezix;
namespace Ui { class RzxMainuiProp; }

///Interface de module pour l'interface principale de qRezix
/** Cette classe existe parce que le double héritage par QObjet
 * pose quelques problème...
 */
class RZX_MAINUI_EXPORT RzxUi:public RzxModule
{
	Q_OBJECT

	QRezix *qrezix;

	Ui::RzxMainuiProp *ui;
	QWidget *propWidget;

	public:
		RzxUi();
		~RzxUi();

		virtual bool isInitialised() const;

	public slots:
		virtual void show();
		virtual void hide();
		virtual void toggleVisible();

	public:
		virtual QList<RzxBaseModule*> childModules() const;
		virtual QWidget *mainWindow() const;
		virtual QList<QWidget*> propWidgets();
		virtual QStringList propWidgetsName();

	public slots:
		virtual void propInit(bool def = false);
		virtual void propUpdate();
		virtual void propClose();
		virtual void setTreeItem(QTreeWidgetItem *item);
		void reload();
};

#endif
