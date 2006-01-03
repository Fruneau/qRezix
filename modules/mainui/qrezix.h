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

#include <RzxBaseLoader>

#include "rzxrezal.h"
#include "rzxrezalpopup.h"
#include "rzxmainuiglobal.h"

class RzxComputer;
class QEvent;
class QCloseEvent;
class QAction;
class QLineEdit;
class QLabel;
class RzxRezalPopup;
namespace Ui { class RzxStatus; };

///Fen�tre principale
/** Cette classe qui �tait la classe principale du programme dans les
 * version ant�rieure est d�sormais la classe qui g�re l'interface
 * principale du programme.
 *
 * C'est en fait une QMainWindow avec des fonctionnalit� de charge de
 * plug-in via l'h�ritage de \ref RzxBaseLoader. Elle innonve dans cette
 * version par sa structure modulaire qui permet � l'utilisateur de choisir
 * � tout moment la m�thode d'affichage des \ref RzxComputer. Les diff�rents
 * modules d'affichage ont diff�rentes type qui permet � QRezix de les ordonner
 * correctement.
 *
 * La cr�ation d'un nouveau module d'affichage passe par la d�rivation de
 * la classe \ref RzxRezal qui d�rive elle m�me de \ref RzxBaseModule. Cette
 * d�rivation permet une bonne int�gration � l'interface de qRezix et en particulier
 * � la fen�tre de pr�f�rence.
 */
class RZX_MAINUI_EXPORT QRezix : public QMainWindow, public RzxBaseLoader<RzxRezal>
{
	Q_OBJECT
	Q_PROPERTY(bool initialised READ isInitialised)
	RZX_GLOBAL(QRezix)

	RzxRezal *central;
	QList<RzxRezal*> indexes;
	QList<RzxRezal*> centralisable;
	QHash<QAction*, RzxRezal*> choseCentral;
	QItemSelectionModel *sel;

	QToolBar *bar;
	QLineEdit *leSearch;
	QLabel *lblSearch;
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
	QAction *columnsAction;
	QAction *awayAction;
	QAction *restartAction;
	QAction *spacerAction;

	Ui::RzxStatus *statusui;

	void buildActions();

	QRezix(QWidget* parent = NULL);
public:
	~QRezix();
	bool isInitialised() const;
	QStringList centralRezals() const;
	QString centralRezal() const;

signals:
	void wantQuit();
	void wantPreferences();
	void wantToggleResponder();
	void wantReload();
	void searchPatternChanged(const QString&);

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
	void showSearch(bool state);
	void setSearchPattern(const QString&);
	void toggleVisible();
	void updateLayout();
	void setCentralRezal(int);

protected slots:
	void setCentralRezal(RzxRezal *rezal = NULL);
	void setCentralRezal(QAction *);

#ifdef Q_OS_MAC
	private:
		QPointer<RzxRezalPopup> popup;

	protected slots:
		void setMenu(const QModelIndex& = QModelIndex(), const QModelIndex& = QModelIndex());
#endif
};

///Indique si l'objet est bien initialis�
inline bool QRezix::isInitialised() const
{
	return wellInit;
}

#endif
