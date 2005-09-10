/***************************************************************************
                          qrezix.cpp  -  description
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
#include <QLabel>
#include <QLineEdit>
#include <QToolBox>
#include <QMenuBar>
#include <QMenu>
#include <QRect>
#include <QPoint>
#include <QString>
#include <QIcon>
#include <QPixmap>
#include <QBitmap>
#include <QApplication>
#include <QCloseEvent>
#include <QEvent>
#include <QShortcut>
#include <QSizePolicy>
#include <QToolBar>
#include <QAction>
#include <QDockWidget>
#include <QStatusBar>

#include <RzxApplication>
#include <RzxGlobal>
#include <RzxConfig>
#include <RzxIconCollection>
#include <RzxConnectionLister>
#include <RzxComputer>

#include "qrezix.h"

#include "rzxquit.h"
#include "rzxrezalmodel.h"
#include "rzxrezalview.h"
#include "rzxmainuiconfig.h"
#include "rzxrezaldetail.h"
#include "rzxrezalindex.h"

RZX_GLOBAL_INIT(QRezix)

///Construction de la fen�tre principale
QRezix::QRezix(QWidget *parent)
	: QMainWindow(parent, Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowContextHelpButtonHint | Qt::WindowContextHelpButtonHint)
{
	object = this;
	statusFlag = false;
	wellInit = false;
	alreadyOpened=false;
	central = NULL;
	index = NULL;

	RzxConfig::useStyleOnWindow(this);
	statusui = new Ui::RzxStatusUI();
	QWidget *widget = new QWidget;
	statusui->setupUi(widget);
	statusBar()->addWidget(widget, 1);

	loadModules("rezals", "rezal*", "getRezal");
	setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
	setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);

	RzxMainUIConfig::restoreMainWidget(this);

	///Chargement des plug-ins
	buildActions();
	
#ifdef Q_OS_MAC
	QMenuBar *menu = new QMenuBar();
	QMenu *popup = menu->addMenu("Tools");
	popup->addAction("Preferences", this, SIGNAL(wantPreferences()));
#endif

	//Construction de la ligne d'�dtion des recherche
	leSearch = new QLineEdit();
	leSearch->setMinimumSize(50, 22);
	leSearch->setMaximumSize(150, 22);
	connect(leSearch, SIGNAL(returnPressed()), this, SLOT(launchSearch()));
	
	//Construction des barres d'outils
	QToolBar *bar = addToolBar("Main");
	bar->addAction(pluginsAction);
	bar->addSeparator();
	bar->addWidget(leSearch);
	bar->addAction(searchAction);
	bar->setMovable(false);

	QLabel *spacer = new QLabel(); //CRAAAAAAAAAAAAAAAAAAAADE
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	bar->addWidget(spacer);
	bar->addAction(awayAction);
	bar->addAction(columnsAction);
	bar->addAction(prefAction);
	bar->setMovable(false);


	connect(RzxComputer::localhost(), SIGNAL(stateChanged(RzxComputer*)), this, SLOT(toggleAutoResponder()));
	connect(RzxIconCollection::global(), SIGNAL(themeChanged(const QString& )), this, SLOT(changeTheme()));
	
	// Pr�paration de l'insterface
	activateAutoResponder( RzxConfig::autoResponder() != 0 );

	RzxConnectionLister *lister = RzxConnectionLister::global();
	connect(lister, SIGNAL(status(const QString&,bool)), this, SLOT(status(const QString&, bool)));
	connect(lister, SIGNAL(countChange(const QString&)), statusui->lblCount, SLOT(setText(const QString&)));
	
	connect(RzxConfig::global(), SIGNAL(iconFormatChange()), this, SLOT(menuFormatChange()));

	showSearch(RzxMainUIConfig::useSearch());
	
	//Raccourcis claviers particuliers
	menuFormatChange();

	wellInit = TRUE;
}

///Chargement des rezals
void QRezix::loadBuiltins()
{
	installModule(new RzxRezalView());
	installModule(new RzxRezalDetail());
	installModule(new RzxRezalIndex());
}

///Chargement des rezals
bool QRezix::installModule(RzxRezal *rezal)
{
	if(RzxBaseLoader<RzxRezal>::installModule(rezal))
	{
		RzxMainUIConfig *conf = RzxMainUIConfig::global();
		conf->beginGroup(rezal->name());

		bool isCentral = conf->value("isCentral", (bool)(rezal->type() & RzxRezal::TYP_CENTRAL)).toBool();
		bool isFloating = conf->value("isFloating", rezal->floating()).toBool();
		Qt::DockWidgetArea area = (Qt::DockWidgetArea)conf->value("area", rezal->area()).toInt();
		bool isVisible = conf->value("isVisible", true).toBool();
		QDockWidget *dock = NULL;

		if(!central && isCentral)
		{
			setCentralWidget(rezal->widget());
			central = rezal;
		}
		else if(rezal->type() & RzxRezal::TYP_DOCKABLE)
		{
			dock = new QDockWidget(rezal->name());
			if(isFloating)
				dock->setParent(this);
			dock->setWidget(rezal->widget());
			dock->setFeatures(rezal->features());
			dock->setAllowedAreas(rezal->allowedAreas());
			dock->setFloating(isFloating);
			rezal->setDockWidget(dock);
			if(!isFloating)
				addDockWidget(area, dock);
		}

		if((rezal->type() & RzxRezal::TYP_INDEX) && !index)
			index = rezal;
		conf->endGroup();

		//Apr�s endGroup, car contient un changement de r�pertoire...
		if(dock && isFloating)
			conf->restoreWidget(rezal->name(), dock, dock->pos(), dock->size());
		if(dock && !isVisible)
			dock->hide();
		return true;
	}
	return false;
}

///Cr�e les liens entres les r�zals
void QRezix::linkModules()
{
	foreach(RzxRezal *rezal, moduleList())
	{
		if((rezal->type() & RzxRezal::TYP_INDEXED) && index)
		{
			rezal->widget()->setRootIndex(RzxRezalModel::global()->everybodyGroup);
			connect(index->widget(), SIGNAL(clicked(const QModelIndex&)), rezal->widget(), SLOT(setRootIndex(const QModelIndex&)));
			connect(index->widget(), SIGNAL(activated(const QModelIndex&)), rezal->widget(), SLOT(setRootIndex(const QModelIndex&)));
		}
		rezal->widget()->setSelectionModel(moduleList()[0]->widget()->selectionModel());
	}
}

///Construction des actions
/** Les actions d�finissent les appels de base des menus/barre d'outils */
void QRezix::buildActions()
{
	pluginsAction = new QAction(tr("Plug-ins"), this);
	//pluginsAction->setCheckable(true);
	pluginsMenu();
	pluginsAction->setMenu(&menuPlugins);
//	connect(pluginsAction, SIGNAL(toggled(bool)), this, SLOT(pluginsMenu(bool)));
//	connect(&menuPlugins, SIGNAL(aboutToHide()), pluginsAction, SLOT(toggle()));

	prefAction = new QAction(tr("Preferences"), this);
	connect(prefAction, SIGNAL(triggered()), this, SIGNAL(wantPreferences()));
	
	columnsAction = new QAction(tr("Adjust columns"), this);
	connect(columnsAction, SIGNAL(triggered()), this, SLOT(updateLayout()));
	
	searchAction = new QAction(tr("Search"), this);
	searchAction->setCheckable(true);
/*	connect(searchAction, SIGNAL(toggled(bool)), rezal, SLOT(activeFilter(bool)));
	connect(searchAction, SIGNAL(toggled(bool)), rezalFavorites, SLOT(activeFilter(bool)));*/

	awayAction = new QAction(tr("Away"), this);
	awayAction->setCheckable(true);	
	connect(awayAction, SIGNAL(triggered()), this, SIGNAL(wantToggleResponder()));
}

///energistre l'�tat de la fen�tre et quitte....
QRezix::~QRezix()
{
	closeModules();
	RZX_GLOBAL_CLOSE
}

///Sauvegarde l'�tat de la fen�tre et des modules
void QRezix::saveState()
{
	RzxMainUIConfig *conf = RzxMainUIConfig::global();
	QList<RzxRezal*> rezals = moduleList();
	foreach(RzxRezal *rezal, rezals)
	{
		conf->beginGroup(rezal->name());
		
		conf->setValue("isCentral", rezal == central);
		QDockWidget *dock = rezal->dockWidget();
		if(dock)
		{
			conf->setValue("isFloating", dock->isFloating());
			conf->setValue("area", dockWidgetArea(dock));
			conf->setValue("isVisible", dock->isVisible());
		}
		conf->endGroup();
		if(dock && dock->isFloating())
			conf->saveWidget(rezal->name(), dock);
	}
	RzxMainUIConfig::saveMainWidget(this);
}

///Change l'information d'�tat
void QRezix::status(const QString& msg, bool connected)
{
	statusui->lblStatus -> setText(msg);
	statusFlag = connected;

	if(statusFlag)
		statusui->lblStatusIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_ON));
	else
		statusui->lblStatusIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_OFF));
		
	qDebug(("Connection status : " + QString(connected?"connected ":"disconnected ") + "(" + msg + ")").toAscii().constData());
}

///Met � jour l'affichage des Rezals
void QRezix::updateLayout()
{
	foreach(RzxRezal *rezal, moduleList())
		rezal->updateLayout();
}

void QRezix::closeEvent(QCloseEvent * e){
	//pour �viter de fermer rezix par m�garde, on affiche un boite de dialogue laissant le choix
	//de fermer qrezix, de minimiser la fen�tre principale --> trayicon, ou d'annuler l'action
	if(!isHidden() && !isMinimized())
	{
		int i;
		if(RzxMainUIConfig::showQuit())
		{
			RzxQuit quitDialog(this);
			i = quitDialog.exec();
		}
		else
			i = RzxMainUIConfig::quitMode();
		if(i != RzxQuit::selectQuit)
		{
			if(i == RzxQuit::selectMinimize)
				showMinimized();
#ifdef WIN32 //c'est tr�s tr�s tr�s tr�s tr�s tr�s moche, mais g pas trouv� d'autre mani�re de le faire
			 //c'est pas ma fautre � moi si windows se comporte comme de la merde
			QEvent ev(QEvent::WindowDeactivate); 
			event(&ev);
#endif
			e -> ignore();
			return;
		}
	}

	//On enregistre l'�tat avant que le destructeur soit lanc� pour �viter que l'affichage des fen�tre soit modifi�
	//Il semble en tout cas que ce soit le cas sous OS X
	saveState();
	emit wantQuit();
}

bool QRezix::event(QEvent * e)
{
	if(e->type()==QEvent::WindowDeactivate)
	{
		if(isMinimized() && RzxApplication::instance()->hasHider())
			hide();
		return true;
	}
	else if( e->type() == QEvent::Resize && alreadyOpened && !isMinimized())
		statusMax = isMaximized();

	return QWidget::event(e);
}

void QRezix::toggleAutoResponder()
{
	activateAutoResponder( RzxComputer::localhost()->isOnResponder());
}

void QRezix::activateAutoResponder( bool state )
{
	if(!state == awayAction->isChecked())
	{
		awayAction->setChecked(state);
		return;
	}
}

///Lancement d'une recherche sur le pseudo dans la liste des personnes connect�es
void QRezix::launchSearch()
{
	if(searchAction->isChecked())
	{
		searchAction->setChecked(false);
		return;
	}
	if(!leSearch->text().length()) searchAction->setChecked(false);
	else searchAction->setChecked(true);
}

///Change l'�tat d'affichage de la fen�tre...
void QRezix::toggleVisible()
{
	if(isVisible())
		hide();
	else{
		bool saveStatusMax = statusMax;
		showNormal();	//pour forcer l'affichage de la fen�tre ==> modifie statusMax
		if(saveStatusMax)
		{
			showMaximized();
			statusMax = saveStatusMax;
		}
		show();
		activateWindow();
		raise();
		alreadyOpened=true;
		updateLayout();
	}
}

///Montre/cache la parte de recherche
void QRezix::showSearch(bool state)
{
	leSearch->setVisible(state);
	searchAction->setVisible(state);
}

///Change le pattern de la recherche
void QRezix::setSearchPattern(const QString& pattern)
{
	leSearch->setText(pattern);
}

///Pour prendre en compte le changement de traduction...
void QRezix::changeEvent(QEvent *e)
{
	QMainWindow::changeEvent(e);
	if(e->type() == QEvent::LanguageChange)
		setWindowTitle("qRezix v" + RZX_VERSION);
}
 
/// Changement du th�me d'icone
/** Cette m�thode met � jour les ic�nes de l'interface principale (menu), mais aussi celles des listes de connect�s */
void QRezix::changeTheme()
{
	pluginsAction->setIcon(RzxIconCollection::getIcon(Rzx::ICON_PLUGIN));
	awayAction->setIcon(RzxIconCollection::getResponderIcon());
	columnsAction->setIcon(RzxIconCollection::getIcon(Rzx::ICON_COLUMN));
	prefAction->setIcon(RzxIconCollection::getIcon(Rzx::ICON_PREFERENCES));
	searchAction->setIcon(RzxIconCollection::getIcon(Rzx::ICON_SEARCH));
	statusui->lblCountIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_NOTFAVORITE)
		.scaled(16,16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	if(statusFlag)
		statusui->lblStatusIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_ON));
	else
		statusui->lblStatusIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_OFF));
}

///Changement de format des boutons de la barre d'outils
/** Change la position du texte, ou la taille des ic�nes de la barre d'outil */
void QRezix::menuFormatChange()
{
	int icons = RzxConfig::menuIconSize();
	int texts = RzxConfig::menuTextPosition();

	//On transforme le cas 'pas d'ic�nes et pas de texte' en 'petites ic�nes et pas de texte'
	if(!texts && !icons) icons = 1;

	//Si on a pas d'ic�ne, on met le texte sur le c�t�... pour �viter un bug d'affichage
	if(!icons) texts = 1;
	Qt::ToolButtonStyle style = Qt::ToolButtonIconOnly;
	if(icons && !texts) style = Qt::ToolButtonIconOnly;
	else if(!icons && texts) style = Qt::ToolButtonTextOnly;
	else if(icons && texts == 1) style = Qt::ToolButtonTextBesideIcon;
	else if(icons && texts == 2) style = Qt::ToolButtonTextUnderIcon;	
	setToolButtonStyle(style);
	
	//Mise � jour de la taille des ic�nes
	switch(icons)
	{
		case 0: //pas d'ic�ne
			{
				QIcon empty;
				QPixmap emptyIcon;
				statusui->lblStatusIcon->hide();
				statusui->lblCountIcon->hide();
			}
			break;
		case 1: //petites ic�nes
		case 2: //grandes icones
			{
				if(pluginsAction->icon().isNull()) changeTheme();
				int dim = (icons == 2)?32:16;
				QSize size = QSize(dim,dim);
				setIconSize(size);
				statusui->lblStatusIcon->show();
				statusui->lblCountIcon->show();
			}
			break;
	}
	
	//Mise � jour de la position du texte
	if(texts)
	{
		statusui->lblStatus->show();
		statusui->lblCountIcon->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	}
	else
	{
		statusui->lblStatus->hide();
		statusui->lblCountIcon->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	}
}

/// Affichage du menu plug-ins lors d'un clic sur le bouton
/** Les actions sont g�r�es directement par le plug-in s'il a bien �t� programm� */
void QRezix::pluginsMenu()
{
	menuPlugins.clear();
	if(!menuPlugins.actions().count())
		menuPlugins.addAction("<none>");
//	menuPlugins.popup(btnPlugins->mapToGlobal(btnPlugins->rect().topRight()));
}
