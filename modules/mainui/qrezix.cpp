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
#include <QMenuBar>
#include <QString>
#include <QPixmap>
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
#include <RzxStyle>
#include <RzxInfoMessage>

#include "qrezix.h"
#include "ui_rzxstatus.h"

#include "rzxmainuiconfig.h"
#include "rzxquit.h"
#include "rzxrezalmodel.h"
#include "rzxusergroup.h"

#ifdef RZX_RZLVIEW_BUILTIN
#	include "../../rezals/view/rzxrezalview.h"
#endif
#ifdef RZX_RZLDETAIL_BUILTIN
#	include "../../rezals/detail/rzxrezaldetail.h"
#endif
#ifdef RZX_RZLINDEX_BUILTIN
#	include "../../rezals/index/rzxrezalindex.h"
#endif
#ifdef RZX_RZLMAP_BUILTIN
#	include "../../rezals/map/rzxrezalmap.h"
#endif
#ifdef RZX_RZLBOB_BUILTIN
#	include "../../rezals/bob/rzxbob.h"
#endif

RZX_GLOBAL_INIT(QRezix)

///Construction de la fen�tre principale
QRezix::QRezix(QWidget *parent)
	: QMainWindow(parent, Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowContextHelpButtonHint | Qt::WindowContextHelpButtonHint),
	RzxBaseLoader<RzxRezal>("rezals", "rezal*", "getRezal", "getRezalName", "getRezalVersion", "getRezalDescription", "getRezalIcon")
{
	object = this;
	statusFlag = false;
	wellInit = false;
	alreadyOpened=false;
	closing = false;
	central = NULL;
	leSearch = NULL;
	bar = NULL;

	//Construction de l'interface
	RzxStyle::useStyleOnWindow(this);
	statusui = new Ui::RzxStatus();
	QWidget *widget = new QWidget;
	statusui->setupUi(widget);
	statusBar()->addWidget(widget, 1);
	setWindowTitle("qRezix v" + Rzx::versionToString(RzxApplication::version(), Rzx::ShortVersion));
#if QT_VERSION >= 0x040300
        setUnifiedTitleAndToolBarOnMac(true);
#endif

	setSettings(RzxMainUIConfig::global());
	updateLayout();
	loadModules();

	//Chargement des actions
	buildActions();
	
#ifdef Q_OS_MAC
	//Mise en place du menu pour Mac OS...
	popup = NULL;
	setMenu();

	new QShortcut(QKeySequence("Ctrl+M"), this, SLOT(showMinimized()));
#endif

	//Construction des barres d'outils
	bar = addToolBar(tr("Main"));
	bar->setObjectName("Main Toolbar");
	bar->setMovable(false);
	buildToolbar(RzxMainUIConfig::useSearch());

	//Pour bien restaurer l'�tat de la bar d'outil...
	restoreState(RzxMainUIConfig::restoreState());
	RzxMainUIConfig::restoreMainWidget(this);
	buildModuleMenus();

	RzxIconCollection::connect(this, SLOT(changeTheme()));
	
	RzxConnectionLister *lister = RzxConnectionLister::global();
	connect(lister, SIGNAL(status(const QString&,bool)), this, SLOT(status(const QString&, bool)));
	connect(lister, SIGNAL(countChange(const QString&)), statusui->lblCount, SLOT(setText(const QString&)));
	if(RzxComputer::localhost()->ip().isNull())
		connect(lister, SIGNAL(receiveAddress(const RzxHostAddress&)), this, SLOT(setSelection(const RzxHostAddress&)));
	else
		setSelection(RzxComputer::localhost()->ip());
	
	connect(RzxConfig::global(), SIGNAL(iconFormatChange()), this, SLOT(menuFormatChange()));

	//Raccourcis claviers particuliers
	menuFormatChange();
	status("", RzxConnectionLister::global()->isConnected());

	wellInit = TRUE;
}

///Chargement des rezals
void QRezix::loadBuiltins()
{
#ifdef RZX_RZLVIEW_BUILTIN
	if(addBuiltin(RzxThemedIcon("rzlitem"), "List", RzxApplication::version(), "Historical way to display computers"))
		installModule(new RzxRezalView());
#endif
#ifdef RZX_RZLDETAIL_BUILTIN
	if(addBuiltin(RzxThemedIcon("rzldetail"), "Details", RzxApplication::version(), "Detail of an item"))
		installModule(new RzxRezalDetail());
#endif
#ifdef RZX_RZLINDEX_BUILTIN
	if(addBuiltin(RzxThemedIcon("rzlindex"), "Index", RzxApplication::version(), "Index view for easy navigation"))
		installModule(new RzxRezalIndex());
#endif
#ifdef RZX_RZLMAP_BUILTIN
	if(addBuiltin(RzxThemedIcon("rzlmap"), "Platal", RzxApplication::version(), "Show an interactive map of the campus"))
		installModule(new RzxRezalMap());
#endif
#ifdef RZX_RZLBOB_BUILTIN
	if(addBuiltin(RzxThemedIcon("bob"), "Bob", RzxApplication::version(), "Show the state of B�bar"))
		installModule(new RzxBob);
#endif
}

///Chargement des rezals
bool QRezix::installModule(RzxRezal *rezal)
{
	if(RzxBaseLoader<RzxRezal>::installModule(rezal))
	{
		RzxMainUIConfig *conf = RzxMainUIConfig::global();
		conf->beginGroup(rezal->name());

		const bool isFloating = conf->value("isFloating", rezal->floating()).toBool();
		const Qt::DockWidgetArea area = (Qt::DockWidgetArea)conf->value("area", rezal->area()).toInt();
		const bool isVisible = conf->value("isVisible", true).toBool();
		const QPoint point = conf->value("position", QPoint()).toPoint();
		QDockWidget *dock = NULL;

		if(rezal->type() & RzxRezal::TYP_CENTRAL)
			centralisable << rezal;

		if(rezal->type() & RzxRezal::TYP_TOOL)
			inToolBar << rezal;
			
		if(rezal->type() & RzxRezal::TYP_DOCKABLE)
		{
			dock = new QDockWidget(rezal->name());
			dock->installEventFilter(this);
			dock->setParent(this);
			dock->setWindowIcon(rezal->icon());
			dock->setWidget(rezal->widget());
			dock->setFeatures(rezal->features());
			dock->setAllowedAreas(rezal->allowedAreas());
			dock->setFloating(isFloating);
			rezal->setDockWidget(dock);
			if(!isFloating)
			{
				DockPosition pos;
				pos.dock = dock;
				pos.visible = isVisible;
				pos.point = (area == Qt::TopDockWidgetArea || area == Qt::BottomDockWidgetArea) ? point.x() : point.y();
				docks[area] += pos;
			}
		}

		if((rezal->type() & RzxRezal::TYP_INDEX))
			indexes << rezal;
		conf->endGroup();

		//Apr�s endGroup, car contient un changement de r�pertoire...
		if(dock && isFloating)
		{
			conf->restoreWidget(rezal->name(), dock, dock->pos(), dock->size());
			addDockWidget(area, dock);
		}
		if(dock && !isVisible)
			dock->hide();
		return true;
	}
	return false;
}

///Cr�e les liens entres les r�zals
void QRezix::linkModules()
{
	setCentralRezal();
	sel = new QItemSelectionModel(RzxRezalModel::global());
	foreach(RzxRezal *rezal, moduleList())
	{
		if((rezal->type() & RzxRezal::TYP_INDEXED) && indexes.count())
		{
			foreach(RzxRezal *index, indexes)
			{
				connect(index->widget(), SIGNAL(clicked(const QModelIndex&)), rezal->widget(), SLOT(setRootIndex(const QModelIndex&)));
				connect(index->widget(), SIGNAL(activated(const QModelIndex&)), rezal->widget(), SLOT(setRootIndex(const QModelIndex&)));
			}
		}
		QAbstractItemView* widget = rezal->widget();
		if(widget)
			widget->setSelectionModel(sel);
	}

	//Mise en place des dock widget
	foreach(Qt::DockWidgetArea area, docks.keys())
	{
		QList<DockPosition> dockWidget = docks[area];
		qSort(dockWidget.begin(), dockWidget.end(), sortDockPosition);
		foreach(DockPosition dock, dockWidget)
		{
			if(dock.dock)
				addDockWidget(area, dock.dock);
		}
	}
	docks.clear();

#ifdef Q_OS_MAC
	connect(sel, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
			this, SLOT(setMenu(const QModelIndex&, const QModelIndex&)));
#endif
}

///Recr�e les liens entre rezals
void QRezix::relinkModules(RzxRezal *rezal, RzxRezal *)
{
	buildModuleMenus();
	if(closing) return;

	if(!rezal) return;

	if((rezal->type() & RzxRezal::TYP_INDEXED) && indexes.count())
	{
		foreach(RzxRezal *index, indexes)
		{
			connect(index->widget(), SIGNAL(clicked(const QModelIndex&)), rezal->widget(), SLOT(setRootIndex(const QModelIndex&)));
			connect(index->widget(), SIGNAL(activated(const QModelIndex&)), rezal->widget(), SLOT(setRootIndex(const QModelIndex&)));
		}
	}
	QAbstractItemView* widget = rezal->widget();
	if(widget)
		widget->setSelectionModel(sel);

	//Mise en place des dock widget
	foreach(Qt::DockWidgetArea area, docks.keys())
	{
		QList<DockPosition> dockWidget = docks[area];
		qSort(dockWidget.begin(), dockWidget.end(), sortDockPosition);
		foreach(DockPosition dock, dockWidget)
		{
			if(dock.dock)
				addDockWidget(area, dock.dock);
		}
	}
	docks.clear();
}

///D�charge le module indiqu�
void QRezix::unloadModule(RzxRezal *rezal)
{
	if(!rezal) return;

	saveState(rezal);
	QDockWidget *dock = rezal->dockWidget();

	if(central == rezal) central = NULL;
	indexes.removeAll(rezal);
	centralisable.removeAll(rezal);
	inToolBar.removeAll(rezal);
	QList<QAction*> actions = choseCentral.keys(rezal);
	foreach(QAction *action, actions)
		choseCentral.remove(action);

	RzxBaseLoader<RzxRezal>::unloadModule(rezal);

	if(dock)
		delete dock;
}

///D�fini la s�lection initiale
void QRezix::setSelection(const RzxHostAddress& ip)
{
	QModelIndex index;
	const RzxMainUIConfig::Tab tab = (RzxMainUIConfig::Tab)RzxMainUIConfig::defaultTab();
	switch(tab)
	{
		case RzxMainUIConfig::Favorites:
			index = RzxRezalModel::global()->favoriteIndex[0];
			break;

		case RzxMainUIConfig::Promo:
			switch(RzxComputer::localhost()->promo())
			{
				case Rzx::PROMAL_UNK: case Rzx::PROMAL_ORANGE:
					index = RzxRezalModel::global()->oranjeIndex[0];
					break;
				case Rzx::PROMAL_ROUJE:
					index = RzxRezalModel::global()->roujeIndex[0];
					break;
				case Rzx::PROMAL_JONE:
					index = RzxRezalModel::global()->joneIndex[0];
					break;
			}
			break;

		case RzxMainUIConfig::Subnet:
		{
			int i = RzxConfig::rezal(ip);
			if(i != -1)
			{
				index = RzxRezalModel::global()->rezalIndex[i][0];
				break;
			}
		}

		case RzxMainUIConfig::Everybody:
			index = RzxRezalModel::global()->everybodyGroup[0];
			break;

		default:
			const int groupId = RzxUserGroup::groupId(RzxMainUIConfig::defaultTabGroup());
			if(groupId != -1)
				index = RzxRezalModel::global()->userIndexes[groupId][0];
			break;
	}
	sel->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
	disconnect(RzxConnectionLister::global(), SIGNAL(receiveAddress(const RzxHostAddress&)),
		this, SLOT(setSelection(const RzxHostAddress&)));
}

///Change la fen�tre centrale
/** Si aucune n'est sp�cifi�, la fen�tre sera choisie par le programme
 */
void QRezix::setCentralRezal(RzxRezal *rezal)
{
	if(central)
	{
		new RzxInfoMessage(RzxMainUIConfig::global(),
			"changeCentral",
			RzxThemedIcon(Rzx::ICON_SYSTRAYHERE),
			tr("The central widget has been set to %1. You should reload the module in order this changement to take effect. You can do this via this menu.").arg(rezal->name()),
			this);
		if(rezal->type() & RzxRezal::TYP_CENTRAL)
			RzxMainUIConfig::setCentralRezal(rezal->name());
		buildModuleMenus();
		return;
	}

	if(!rezal)
	{
		foreach(RzxRezal *rez, centralisable)
		{
			if(rez->name() == RzxMainUIConfig::centralRezal())
			{
				rezal = rez;
				break;
			}
		}
	}
	if(!rezal)
	{
		foreach(RzxRezal *rez, centralisable)
		{
			if(rez->name() == DEFAULT_REZAL)
			{
				rezal = rez;
				break;
			}
		}
	}
	if(!rezal && centralisable.size())
		rezal = centralisable[0];

	if(!rezal) return;

	if(rezal->dockWidget())
	{
		rezal->widget()->setParent(this);
		delete rezal->dockWidget();
		rezal->setDockWidget(NULL);
	}
	setCentralWidget(rezal->widget());
	central = rezal;
	buildModuleMenus();
}

///Change la fen�tre centrale
/** Surcharge qui identifie la fen�tre � son indexe dans la liste
 * des rezals centralisables
 */
void QRezix::setCentralRezal(int i)
{
	setCentralRezal(centralisable[i]);
}

///Change la fen�tre centrale
/** Surcharge qui identifie la fen�tre � l'action qui a �t� d�clench�e
 */
void QRezix::setCentralRezal(QAction *action)
{
	if(!choseCentral[action]) return;
	setCentralRezal(choseCentral[action]);
}

///Retourne la liste des noms des centralisables
QStringList QRezix::centralRezals() const
{
	QStringList list;
	for(int i = 0 ; i < centralisable.size() ; i++)
		list << centralisable[i]->name();
	return list;
}

///Retourne le nom du rezal central actuel
QString QRezix::centralRezal() const
{
	if(central)
		return central->name();
	return QString();
}

///Construction des actions
/** Les actions d�finissent les appels de base des menus/barre d'outils */
void QRezix::buildActions()
{
	pluginsAction = new QAction(tr("Modules"), this);
	menuView.setTitle(tr("View"));
	menuCentral.setTitle(tr("Central"));
	buildModuleMenus();
	connect(&menuCentral, SIGNAL(triggered(QAction*)), this, SLOT(setCentralRezal(QAction*)));
	menuPlugins.addMenu(&menuView);
	menuPlugins.addMenu(&menuCentral);
	pluginsAction->setMenu(&menuPlugins);
	pluginsAction->setShortcut(Qt::Key_F5); //c'est associ� � updateLayout ~= refresh
	connect(pluginsAction, SIGNAL(triggered()), this, SLOT(updateLayout()));

	searchModeAction = new QAction(tr("Advanced search"), this);
	searchModeAction->setCheckable(true);
	searchModeAction->setChecked(RzxMainUIConfig::searchMode() == RzxRezalSearch::Full);
	connect(searchModeAction, SIGNAL(toggled(bool)), this, SLOT(searchModeToggled(bool)));
}

///energistre l'�tat de la fen�tre et quitte....
QRezix::~QRezix()
{
	saveState();
	closing = true;
	closeModules();
	RzxMainUIConfig::saveState(QMainWindow::saveState());
	RZX_GLOBAL_CLOSE
}

///Sauvegarde l'�tat de la fen�tre et des modules
void QRezix::saveState()
{
	QList<RzxRezal*> rezals = moduleList();
	foreach(RzxRezal *rezal, rezals)
		saveState(rezal);
	RzxMainUIConfig::saveMainWidget(this);
}

///Sauvegarder l'�tat d'un rezal particulier
void QRezix::saveState(RzxRezal *rezal)
{
	if(closing) return;

	RzxMainUIConfig *conf = RzxMainUIConfig::global();
	conf->beginGroup(rezal->name());
		
	QDockWidget *dock = rezal->dockWidget();
	if(dock)
	{
		conf->setValue("isFloating", dock->isFloating());
		conf->setValue("area", dockWidgetArea(dock));
		conf->setValue("isVisible", dock->isVisible());
		conf->setValue("position", dock->pos());
	}
	conf->endGroup();
	if(dock && dock->isFloating())
		conf->saveWidget(rezal->name(), dock);
}

#ifdef Q_OS_MAC

///Construit le menu de qRezix
/** Cette fonctionnalit� n'est disponible que sous Mac OS et a pour but de permettre
 * l'utilisation de qRezix via le menu de Mac OS
 */
void QRezix::setMenu(const QModelIndex& current, const QModelIndex&)
{	
	if(!popup)
		popup = new RzxRezalPopup(current, RzxApplication::menuBar());
	else
		popup->change(current);
	RzxApplication::instance()->refreshMenu();
}	
#endif

///Construit les menus des modules
/** Permet de mettre � jour les menus en fonction des param�tres actuels
 */
void QRezix::buildModuleMenus()
{
	//Liste des modules centraux
	menuCentral.clear();
	choseCentral.clear();
	restartAction = NULL;
	for(int i = 0 ; i < centralisable.size() ; i++)
	{
		RzxRezal *rezal = centralisable[i];
		if(!rezal) continue;
		QAction *action = menuCentral.addAction(rezal->icon(), rezal->name());
		choseCentral.insert(action, rezal);
		action->setCheckable(true);
		action->setChecked(RzxMainUIConfig::centralRezal() == rezal->name());
	}
	if(central && central->name() != RzxMainUIConfig::centralRezal())
	{
		menuCentral.addSeparator();
		restartAction = menuCentral.addAction(RzxIconCollection::getIcon(Rzx::ICON_RELOAD), tr("You should restart qRezix"),
			this, SIGNAL(wantReload()));
		choseCentral.insert(restartAction, NULL);
	}
	
	//Liste des modules
	menuView.clear();
	foreach(RzxRezal *rezal, moduleList())
	{
		if(!rezal) continue;

		//Apr�s endGroup, car contient un changement de r�pertoire...
		QDockWidget *dock = rezal->dockWidget();
		QAbstractItemView *widget = rezal->widget();
		
		QAction *action;
		if (dock || widget)
		{
			//Ajoute une entr�e dans le menu que si le rezl a une fen�tre
			action = menuView.addAction(rezal->icon(), rezal->name());
			action->setCheckable(true);
			if(dock)
			{
				action->setChecked(dock->isVisibleTo(this));
				connect(action, SIGNAL(toggled(bool)), dock, SLOT(setVisible(bool)));
			}
			else
			{
				action->setChecked(rezal->widget()->isVisibleTo(this));
				connect(action, SIGNAL(toggled(bool)), rezal->widget(), SLOT(setVisible(bool)));
			}
		}
	}
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
		
	qDebug("Connection status: %s (%s)",
               connected ? "connected" : "disconnected",
               msg.toAscii().constData());
}

///Met � jour l'affichage des Rezals
void QRezix::updateLayout()
{
	setCorner(Qt::BottomLeftCorner, (Qt::DockWidgetArea)RzxMainUIConfig::bottomLeftCorner());
	setCorner(Qt::TopLeftCorner, (Qt::DockWidgetArea)RzxMainUIConfig::topLeftCorner());
	setCorner(Qt::BottomRightCorner, (Qt::DockWidgetArea)RzxMainUIConfig::bottomRightCorner());
	setCorner(Qt::TopRightCorner, (Qt::DockWidgetArea)RzxMainUIConfig::topRightCorner());

	foreach(RzxRezal *rezal, moduleList())
		rezal->updateLayout();
}

///Gestion de la fermeture de la fen�tre
void QRezix::closeEvent(QCloseEvent * e)
{
	//pour �viter de fermer rezix par m�garde, on affiche un boite de dialogue laissant le choix
	//de fermer qrezix, de minimiser la fen�tre principale --> trayicon, ou d'annuler l'action
	int i;
	if(RzxMainUIConfig::showQuit())
	{
		RzxQuit quitDialog(this);
		i = quitDialog.exec();
	}
	else
		i = RzxMainUIConfig::quitMode();
	if(i != RzxQuit::Quit)
	{
		if(i == RzxQuit::Minimize)
		{
#ifdef Q_OS_MAC
			saveState();
#else
			showMinimized();
#endif //Q_OS_MAC
			if(RzxApplication::instance()->hasHider())
				hide();
		}
#ifdef WIN32 //c'est tr�s tr�s tr�s tr�s tr�s tr�s moche, mais g pas trouv� d'autre mani�re de le faire
		 //c'est pas ma fautre � moi si windows se comporte comme de la merde
		QEvent ev(QEvent::WindowDeactivate); 
		event(&ev);
#endif //WIN32
		e -> ignore();
		return;
	}

	//On enregistre l'�tat avant que le destructeur soit lanc� pour �viter que l'affichage des fen�tre soit modifi�
	//Il semble en tout cas que ce soit le cas sous OS X
	saveState();
	closing = true;
	emit wantQuit();
}

///Intercepte les changements de visibilit� des QDockWidget
/** Pour mettre � jour le menu des modules
 */
bool QRezix::eventFilter(QObject *obj, QEvent *e)
{
	if(qobject_cast<QDockWidget*>(obj))
	{
		if( e->type() == QEvent::HideToParent || e->type() == QEvent::ShowToParent)
			buildModuleMenus();
	}
	return QMainWindow::eventFilter(obj, e);
}

///Gestion de l'affichage de la fen�tre...
/** Converti minimiser en cacher si un hider existe
 */
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

	return QMainWindow::event(e);
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
void QRezix::buildToolbar(bool showSearch)
{
	bar->clear();
	if(leSearch)
		delete leSearch;
	
	bar->addAction(RzxApplication::prefAction());
	bar->addAction(pluginsAction);
	bar->addSeparator();
	foreach(RzxRezal *rez, inToolBar)
		bar->addAction(rez->toolButton());
	
	QWidget *spacer = new QWidget();
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	spacerAction = bar->addWidget(spacer);
	bar->addAction(RzxApplication::awayAction());
	
	if(showSearch)
	{
		//Construction de la ligne d'�dtion des recherche
		leSearch = new QLineEdit();
		leSearch->setMinimumSize(50, 22);
		leSearch->setMaximumSize(150, 22);
		connect(leSearch, SIGNAL(textEdited(const QString&)), this, SIGNAL(searchPatternChanged(const QString&)));
	
		bar->addSeparator();
		bar->addWidget(leSearch);
		bar->addAction(searchModeAction);
		menuFormatChange();
		leSearch->setVisible(true);
	}
}

///Change le pattern de la recherche
void QRezix::setSearchPattern(const QString& pattern)
{
	if(leSearch)
		leSearch->setText(pattern);
}

///D�fini le mode de recherche
void QRezix::searchModeToggled(bool toggle)
{
	RzxRezalSearch::Mode mode = toggle ? RzxRezalSearch::Full : RzxRezalSearch::Lite;
	RzxMainUIConfig::setSearchMode(mode);
	emit searchModeChanged(mode);
}

/// Changement du th�me d'icone
/** Cette m�thode met � jour les ic�nes de l'interface principale (menu), mais aussi celles des listes de connect�s */
void QRezix::changeTheme()
{
	pluginsAction->setIcon(RzxIconCollection::getIcon(Rzx::ICON_PLUGIN));
	menuCentral.setIcon(RzxIconCollection::getIcon(Rzx::ICON_PLUGIN));
	menuView.setIcon(RzxIconCollection::getIcon(Rzx::ICON_LAYOUT));
	if(restartAction)
		restartAction->setIcon(RzxIconCollection::getIcon(Rzx::ICON_RELOAD));
	searchModeAction->setIcon(RzxIconCollection::getPixmap(Rzx::ICON_SEARCH));
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
				int dim = (icons == 2)?32:22;
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


///Auxiliaire pour trier les DockPosition
bool sortDockPosition(const QRezix::DockPosition& a, const QRezix::DockPosition& b)
{
	return a.point < b.point;
}
