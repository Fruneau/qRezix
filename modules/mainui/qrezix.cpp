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
#include <QMenuBar>
#include <QString>
#include <QIcon>
#include <QPixmap>
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
#include <RzxStyle>

#include "qrezix.h"

#include "rzxmainuiconfig.h"
#include "rzxquit.h"
#include "rzxrezalmodel.h"

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

	RzxStyle::useStyleOnWindow(this);
	statusui = new Ui::RzxStatusUI();
	QWidget *widget = new QWidget;
	statusui->setupUi(widget);
	statusBar()->addWidget(widget, 1);
	setWindowTitle("qRezix v" + Rzx::versionToString(RzxApplication::version(), Rzx::ShortVersion));

	loadModules("rezals", "rezal*", "getRezal");
	setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
	setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);

	RzxMainUIConfig::restoreMainWidget(this);

	///Chargement des plug-ins
	buildActions();
	
#ifdef Q_OS_MAC
	QMenuBar *menu = menuBar();
	QMenu *tool = menu->addMenu("qRezix");
	tool->addAction("Preferences", this, SIGNAL(wantPreferences()));
	tool->addAction("Quit", this, SIGNAL(wantQuit()));
	popup = NULL;
	setMenu();
#endif

	//Construction de la ligne d'�dtion des recherche
	leSearch = new QLineEdit();
	leSearch->setMinimumSize(50, 22);
	leSearch->setMaximumSize(150, 22);
	connect(leSearch, SIGNAL(textEdited(const QString&)), this, SIGNAL(searchPatternChanged(const QString&)));
	
	lblSearch = new QLabel();
	
	//Construction des barres d'outils
	QToolBar *bar = addToolBar(tr("Main"));
	bar->addAction(pluginsAction);
	bar->addSeparator();
	bar->addWidget(leSearch);
	bar->addWidget(lblSearch);

	QLabel *spacer = new QLabel(); //CRAAAAAAAAAAAAAAAAAAAADE
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	bar->addWidget(spacer);
	bar->addAction(awayAction);
	bar->addAction(columnsAction);
	bar->addAction(prefAction);
	bar->setMovable(false);


	connect(RzxComputer::localhost(), SIGNAL(stateChanged(RzxComputer*)), this, SLOT(toggleAutoResponder()));
	RzxIconCollection::connect(this, SLOT(changeTheme()));
	
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
#ifdef RZX_RZLVIEW_BUILTIN
	installModule(new RzxRezalView());
#endif
#ifdef RZX_RZLDETAIL_BUILTIN
	installModule(new RzxRezalDetail());
#endif
#ifdef RZX_RZLINDEX_BUILTIN
	installModule(new RzxRezalIndex());
#endif
#ifdef RZX_RZLMAP_BUILTIN
	installModule(new RzxRezalMap());
#endif
}

///Chargement des rezals
bool QRezix::installModule(RzxRezal *rezal)
{
	if(RzxBaseLoader<RzxRezal>::installModule(rezal))
	{
		RzxMainUIConfig *conf = RzxMainUIConfig::global();
		conf->beginGroup(rezal->name());

		bool isFloating = conf->value("isFloating", rezal->floating()).toBool();
		Qt::DockWidgetArea area = (Qt::DockWidgetArea)conf->value("area", rezal->area()).toInt();
		bool isVisible = conf->value("isVisible", true).toBool();
		QDockWidget *dock = NULL;

		if(rezal->type() & RzxRezal::TYP_CENTRAL)
			centralisable << rezal;

		if(rezal->type() & RzxRezal::TYP_DOCKABLE)
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

		QAction *action = menuView.addAction(rezal->icon(), rezal->name());
		action->setCheckable(true);
		action->setChecked(isVisible);

		//Apr�s endGroup, car contient un changement de r�pertoire...
		if(dock && isFloating)
			conf->restoreWidget(rezal->name(), dock, dock->pos(), dock->size());
		if(dock && !isVisible)
			dock->hide();
		if(dock)
			connect(action, SIGNAL(toggled(bool)), dock, SLOT(setVisible(bool)));
		else
			connect(action, SIGNAL(toggled(bool)), rezal->widget(), SLOT(setVisible(bool)));
		return true;
	}
	return false;
}

///Cr�e les liens entres les r�zals
void QRezix::linkModules()
{
	setCentralRezal();
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
#ifdef Q_OS_MAC
	connect(moduleList()[0]->widget()->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
			this, SLOT(setMenu(const QModelIndex&, const QModelIndex&)));
#endif
}

///Change la fen�tre centrale
/** Si aucune n'est sp�cifi�, la fen�tre sera choisie par le programme
 */
void QRezix::setCentralRezal(RzxRezal *rezal)
{
	if(central)
	{
		//N�cessite de relancer qRezix pour �tre pris en compte
		//TODO : message d'avertissement...
		if(rezal->type() & RzxRezal::TYP_CENTRAL)
			RzxMainUIConfig::setCentralRezal(rezal->name());
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
	pluginsAction = new QAction(tr("Plug-ins"), this);
	menuView.setTitle(tr("View"));
	for(int i = 0 ; i < centralisable.size() ; i++)
	{
		QAction *action = menuCentral.addAction(centralisable[i]->icon(), centralisable[i]->name());
		choseCentral.insert(action, centralisable[i]);
	}
	menuCentral.setTitle(tr("Central"));
	connect(&menuCentral, SIGNAL(triggered(QAction*)), this, SLOT(setCentralRezal(QAction*)));
	menuPlugins.addMenu(&menuView);
	menuPlugins.addMenu(&menuCentral);
	pluginsAction->setMenu(&menuPlugins);

	prefAction = new QAction(tr("Preferences"), this);
	connect(prefAction, SIGNAL(triggered()), this, SIGNAL(wantPreferences()));
	
	columnsAction = new QAction(tr("Adjust columns"), this);
	connect(columnsAction, SIGNAL(triggered()), this, SLOT(updateLayout()));
	
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

#ifdef Q_OS_MAC
///Construit le menu de qRezix
/** Cette fonctionnalit� n'est disponible que sous Mac OS et a pour but de permettre
 * l'utilisation de qRezix via le menu de Mac OS
 */
void QRezix::setMenu(const QModelIndex& current, const QModelIndex&)
{	
	if(current.isValid() && !RzxRezalModel::global()->isIndex(current))
	{
		if(!popup)
			popup = new RzxRezalPopup(current, menuBar());
		else
			popup->change(current);
	}
}	
#endif

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

	return QMainWindow::event(e);
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
	lblSearch->setVisible(state);
}

///Change le pattern de la recherche
void QRezix::setSearchPattern(const QString& pattern)
{
	leSearch->setText(pattern);
}

/// Changement du th�me d'icone
/** Cette m�thode met � jour les ic�nes de l'interface principale (menu), mais aussi celles des listes de connect�s */
void QRezix::changeTheme()
{
	pluginsAction->setIcon(RzxIconCollection::getIcon(Rzx::ICON_PLUGIN));
	awayAction->setIcon(RzxIconCollection::getResponderIcon());
	columnsAction->setIcon(RzxIconCollection::getIcon(Rzx::ICON_COLUMN));
	prefAction->setIcon(RzxIconCollection::getIcon(Rzx::ICON_PREFERENCES));
	statusui->lblCountIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_NOTFAVORITE)
		.scaled(16,16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	if(RzxConfig::menuIconSize())
		lblSearch->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_SEARCH));
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
	lblSearch->clear();

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
				lblSearch->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_SEARCH));
			}
			break;
	}
	
	//Mise � jour de la position du texte
	if(texts)
	{
		statusui->lblStatus->show();
		statusui->lblCountIcon->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		lblSearch->setText(tr("Search"));
	}
	else
	{
		statusui->lblStatus->hide();
		statusui->lblCountIcon->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	}
}