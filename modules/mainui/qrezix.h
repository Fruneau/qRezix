/***************************************************************************
                          qrezix.h  -  description
                             -------------------
    begin                : lun jan 28 16:27:20 CET 2002
    copyright            : (C) 2002 by Sylvain Joyeux
    email                : sylvain.joyeux@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QREZIX_H
#define QREZIX_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QMainWindow>
#include <QModelIndex>
#include <QPointer>
#include <QLineEdit>

#include <RzxBaseLoader>

#include "rzxrezal.h"
#include "rzxrezalpopup.h"
#include "rzxrezalsearch.h"
#include "rzxmainuiglobal.h"

class RzxComputer;
class QEvent;
class QCloseEvent;
class QAction;
class QLabel;
class RzxRezalPopup;
namespace Ui { class RzxStatus; };

///Fenêtre principale
/** Cette classe qui était la classe principale du programme dans les
 * version antérieure est désormais la classe qui gère l'interface
 * principale du programme.
 *
 * C'est en fait une QMainWindow avec des fonctionnalité de charge de
 * plug-in via l'héritage de \ref RzxBaseLoader. Elle innonve dans cette
 * version par sa structure modulaire qui permet à l'utilisateur de choisir
 * à tout moment la méthode d'affichage des \ref RzxComputer. Les différents
 * modules d'affichage ont différentes type qui permet à QRezix de les ordonner
 * correctement.
 *
 * La création d'un nouveau module d'affichage passe par la dérivation de
 * la classe \ref RzxRezal qui dérive elle même de \ref RzxBaseModule. Cette
 * dérivation permet une bonne intégration à l'interface de qRezix et en particulier
 * à la fenêtre de préférence.
 */
class RZX_MAINUI_EXPORT QRezix : public QMainWindow, public RzxBaseLoader<RzxRezal>
{
	Q_OBJECT
	Q_PROPERTY(bool initialised READ isInitialised)
	RZX_GLOBAL(QRezix)

	///Enregistre les informations nécessaire pour order des dockWidgets avant de les afficher
	struct DockPosition
	{
		//QPointer car le dockWidget peut être supprimé (ex: si la fenêtre devient centralWidget)
		QPointer<QDockWidget> dock;
		bool visible;
		int point;
	};

	friend bool sortDockPosition(const DockPosition&, const DockPosition&);

	RzxRezal *central;
	QList<RzxRezal*> indexes;
	QList<RzxRezal*> centralisable;
	QHash<Qt::DockWidgetArea, QList<DockPosition> > docks;
	QHash<QAction*, RzxRezal*> choseCentral;
	QItemSelectionModel *sel;

	QToolBar *bar;
	QPointer<QLineEdit> leSearch;
	QMenu menuPlugins;
	QMenu menuView;
	QMenu menuCentral;
	bool statusFlag;
	bool statusMax;
	bool alreadyOpened;
	bool wellInit;
	bool closing;
	
	QAction *pluginsAction;
	QAction *prefAction;
	QAction *awayAction;
	QAction *restartAction;
	QAction *searchModeAction;
	QAction *spacerAction;

	Ui::RzxStatus *statusui;

	void buildActions();

	QRezix(QWidget* parent = NULL);

public:
	~QRezix();
	bool isInitialised() const;
	QStringList centralRezals() const;
	QString centralRezal() const;
	virtual bool eventFilter(QObject *, QEvent*);

signals:
	void wantQuit();
	void wantPreferences();
	void wantToggleResponder();
	void wantReload();
	void searchPatternChanged(const QString&);
	void searchModeChanged(RzxRezalSearch::Mode);

protected:
	void saveState();
	void saveState(RzxRezal*);
	virtual void closeEvent(QCloseEvent * e);
	virtual bool event(QEvent * e);

	virtual void loadBuiltins();
	virtual bool installModule(RzxRezal*);
	virtual void linkModules();
	virtual void relinkModules(RzxRezal* = NULL, RzxRezal* = NULL);
	virtual void unloadModule(RzxRezal*);

	void buildModuleMenus();

public slots: // Public slots
	void status(const QString& msg, bool fatal);
	void toggleAutoResponder();
	void activateAutoResponder( bool state );
	void changeTheme();
	void menuFormatChange();
	void buildToolbar(bool);
	void setSearchPattern(const QString&);
	void searchModeToggled(bool);
	void toggleVisible();
	void updateLayout();
	void setCentralRezal(int);

protected slots:
	void setCentralRezal(RzxRezal *rezal = NULL);
	void setCentralRezal(QAction *);

	void setSelection(const RzxHostAddress&);
	
#ifdef Q_OS_MAC
	private:
		QPointer<RzxRezalPopup> popup;

	protected slots:
		void setMenu(const QModelIndex& = QModelIndex(), const QModelIndex& = QModelIndex());
#endif
};

bool sortDockPosition(const QRezix::DockPosition& a, const QRezix::DockPosition& b);

///Indique si l'objet est bien initialisé
inline bool QRezix::isInitialised() const
{
	return wellInit;
}

#endif
