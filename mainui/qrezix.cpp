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
#include <QToolButton>
#include <QLineEdit>
#include <QToolBox>
#include <QMenuBar>
#include <QMenu>
#include <QRect>
#include <QPoint>
#include <QString>
#include <QFile>
#include <QIcon>
#include <QPixmap>
#include <QBitmap>
#include <QApplication>
#include <QProcess>
#include <QSound>
#include <QCloseEvent>
#include <QEvent>
#include <QShortcut>
#include <QSizePolicy>
#include <QToolBar>
#include <QAction>

#include <RzxApplication>
#include <RzxGlobal>
#include <RzxConfig>
#include <RzxIconCollection>
#include <RzxPlugInLoader>
#include <RzxConnectionLister>
#include <RzxComputer>

#include "qrezix.h"

#include "rzxquit.h"
#include "rzxrezalmodel.h"
#include "rzxrezalview.h"

QRezix *QRezix::object = NULL;

QRezix::QRezix(QWidget *parent)
 : QMainWindow(parent, Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowContextHelpButtonHint | Qt::WindowContextHelpButtonHint), accel(0)
{
	object = this;
	setupUi(this);
	statusFlag = false;
	wellInit = false;
	alreadyOpened=false;

	///Chargement des plug-ins
	buildActions();
	
#ifdef Q_OS_MAC
	QMenuBar *menu = new QMenuBar();
	QMenu *popup = menu->addMenu("Tools");
	popup->addAction("Preferences", this, SLOT(boitePreferences()));
#endif

	//Construction de la ligne d'édtion des recherche
	leSearch = new QLineEdit();
	leSearch->setMinimumSize(50, 22);
	leSearch->setMaximumSize(150, 22);
	connect(leSearch, SIGNAL(returnPressed()), this, SLOT(launchSearch()));
//	connect(leSearch, SIGNAL(textChanged(const QString&)), rezal, SLOT(setFilter(const QString&)));
//	connect(leSearch, SIGNAL(textChanged(const QString&)), rezalFavorites, SLOT(setFilter(const QString&)));
	
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
	
	rezal->setRootIndex(RzxRezalModel::global()->everybodyGroup);
	rezalFavorites->setRootIndex(RzxRezalModel::global()->favoriteIndex);

	connect(RzxComputer::localhost(), SIGNAL(stateChanged(RzxComputer*)), this, SLOT(toggleAutoResponder()));
	connect(RzxIconCollection::global(), SIGNAL(themeChanged(const QString& )), this, SLOT(changeTheme()));
	QString windowSize = RzxConfig::readWindowSize();
#ifndef Q_OS_MAC
	if(windowSize.left(1)=="1")
		statusMax = true;
	else
#endif
	{
		statusMax = false;
		int height=windowSize.mid(1,4).toInt();
		int width =windowSize.mid(5,4).toInt();
		resize(QSize(width,height));
		move(RzxConfig::readWindowPosition()); //QPoint(2,24));
	}
	
	// Préparation de l'insterface
	activateAutoResponder( RzxConfig::autoResponder() != 0 );

	connect(rezal, SIGNAL(searchPatternChanged(const QString&)), leSearch, SLOT(setText(const QString &)));
	connect(rezalFavorites, SIGNAL(searchPatternChanged(const QString&)), leSearch, SLOT(setText(const QString &)));

	RzxConnectionLister *lister = RzxConnectionLister::global();
	connect(lister, SIGNAL(status(const QString&,bool)), this, SLOT(status(const QString&, bool)));
	connect(lister, SIGNAL(countChange(const QString&)), lblCount, SLOT(setText(const QString&)));
	
	connect(RzxConfig::global(), SIGNAL(iconFormatChange()), this, SLOT(menuFormatChange()));

	tbRezalContainer->setCurrentIndex(RzxConfig::defaultTab());
	showSearch(RzxConfig::global()->useSearch());
	
	//Raccourcis claviers particuliers
	accel = new QShortcut(Qt::Key_Tab + Qt::SHIFT, this, SLOT(switchTab()));
	menuFormatChange();

	wellInit = TRUE;
}

///Construction des actions
/** Les actions définissent les appels de base des menus/barre d'outils */
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
	connect(columnsAction, SIGNAL(triggered()), rezal, SLOT(afficheColonnes()));
	connect(columnsAction, SIGNAL(triggered()), rezalFavorites, SLOT(afficheColonnes()));
	
	searchAction = new QAction(tr("Search"), this);
	searchAction->setCheckable(true);
/*	connect(searchAction, SIGNAL(toggled(bool)), rezal, SLOT(activeFilter(bool)));
	connect(searchAction, SIGNAL(toggled(bool)), rezalFavorites, SLOT(activeFilter(bool)));*/

	awayAction = new QAction(tr("Away"), this);
	awayAction->setCheckable(true);	
	connect(awayAction, SIGNAL(triggered()), this, SIGNAL(wantToggleResponder()));
}

///energistre l'état de la fenêtre et quitte....
QRezix::~QRezix()
{
	QSize s = size();       // store size
	
	//Pas très beau, mais c'est juste pour voir si ça améliore le rétablissement de l'état de la fenêtre sur mac
	QString height = QString("%1").arg(s.height(), 4, 10).replace(' ', '0');
	QString width =  QString("%1").arg(s.width(), 4, 10).replace(' ', '0');
	QString windowSize;
#ifndef Q_OS_MAC
	if(isMaximized())
		windowSize = "100000000";
	else
#endif
		windowSize="0"+height+width;
	RzxConfig::global()->writeWindowSize(windowSize);
	RzxConfig::global()->writeWindowPosition(pos());
	if(accel)
	{
		delete accel;
		accel = NULL;
	}
}

void QRezix::status(const QString& msg, bool fatal)
{
	lblStatus -> setText(msg);
	statusFlag = !fatal;

	if(statusFlag)
		lblStatusIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_ON));
	else
		lblStatusIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_OFF));
		
	qDebug(("Connection status : " + QString(fatal?"disconnected ":"connected ") + "(" + msg + ")").toAscii().constData());
}

void QRezix::closeEvent(QCloseEvent * e){
	//pour éviter de fermer rezix par mégarde, on affiche un boite de dialogue laissant le choix
	//de fermer qrezix, de minimiser la fenêtre principale --> trayicon, ou d'annuler l'action
	if(!isHidden() && !isMinimized())
	{
		int i;
		if(RzxConfig::showQuit())
		{
			RzxQuit quitDialog(this);
			i = quitDialog.exec();
		}
		else
			i = RzxConfig::quitMode();
		if(i != RzxQuit::selectQuit)
		{
			if(i == RzxQuit::selectMinimize)
				showMinimized();
#ifdef WIN32 //c'est très très très très très très moche, mais g pas trouvé d'autre manière de le faire
			 //c'est pas ma fautre à moi si windows se comporte comme de la merde
			QEvent ev(QEvent::WindowDeactivate); 
			event(&ev);
#endif
			e -> ignore();
			return;
		}
	}

	emit wantQuit();
}

bool QRezix::event(QEvent * e){
	if(e->type()==QEvent::WindowDeactivate)
	{
		if(isMinimized() && RzxConfig::global()->useSystray() && RzxApplication::instance()->hasHider())
			hide();
		return true;
	}
	else if( e->type() == QEvent::Resize && alreadyOpened && !isMinimized())
		statusMax = isMaximized();

	return QWidget::event(e);
}

void QRezix::switchTab()
{
	tbRezalContainer->setCurrentIndex(1 - tbRezalContainer->currentIndex());
}

void QRezix::delayedInit() {
	RzxConnectionLister::global()->initConnection();
}

void QRezix::socketClosed(){
	close();
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

///Lancement d'une recherche sur le pseudo dans la liste des personnes connectées
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

void QRezix::toggleVisible()
{
	if(isVisible())
		hide();
	else{
		bool saveStatusMax = statusMax;
		showNormal();	//pour forcer l'affichage de la fenêtre ==> modifie statusMax
		if(saveStatusMax)
		{
			showMaximized();
			statusMax = saveStatusMax;
		}
		show();
		activateWindow();
		raise();
		alreadyOpened=true;
		rezal->afficheColonnes();
		rezalFavorites->afficheColonnes();
	}
}

///Montre/cache la parte de recherche
void QRezix::showSearch(bool state)
{
	leSearch->setVisible(state);
	searchAction->setVisible(state);
}

///Pour prendre en compte le changement de traduction...
void QRezix::changeEvent(QEvent *e)
{
	QMainWindow::changeEvent(e);
	if(e->type() == QEvent::LanguageChange)
	{
		retranslateUi(this);
		setWindowTitle("qRezix v" + RZX_VERSION);

		//Parce que Qt oublie de traduire les deux noms
		//Alors faut le faire à la main, mais franchement, c du foutage de gueule
		//à mon avis ça leur prendrait 5 minutes chez trolltech pour corriger le pb
		tbRezalContainer->setItemText(0, tr("Favorites"));
		tbRezalContainer->setItemText(1, tr("Everybody"));
	}
}
 
/// Changement du thème d'icone
/** Cette méthode met à jour les icônes de l'interface principale (menu), mais aussi celles des listes de connectés */
void QRezix::changeTheme()
{
	pluginsAction->setIcon(RzxIconCollection::getIcon(Rzx::ICON_PLUGIN));
	awayAction->setIcon(RzxIconCollection::getResponderIcon());
	columnsAction->setIcon(RzxIconCollection::getIcon(Rzx::ICON_COLUMN));
	prefAction->setIcon(RzxIconCollection::getIcon(Rzx::ICON_PREFERENCES));
	searchAction->setIcon(RzxIconCollection::getIcon(Rzx::ICON_SEARCH));
	lblCountIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_NOTFAVORITE)
		.scaled(16,16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	tbRezalContainer->setItemIcon(1,RzxIconCollection::getPixmap(Rzx::ICON_NOTFAVORITE));
	tbRezalContainer->setItemIcon(0,RzxIconCollection::getPixmap(Rzx::ICON_FAVORITE));
	if(statusFlag)
		lblStatusIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_ON));
	else
		lblStatusIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_OFF));
}

///Changement de format des boutons de la barre d'outils
/** Change la position du texte, ou la taille des icônes de la barre d'outil */
void QRezix::menuFormatChange()
{
	int icons = RzxConfig::menuIconSize();
	int texts = RzxConfig::menuTextPosition();

	//On transforme le cas 'pas d'icônes et pas de texte' en 'petites icônes et pas de texte'
	if(!texts && !icons) icons = 1;

	//Si on a pas d'icône, on met le texte sur le côté... pour éviter un bug d'affichage
	if(!icons) texts = 1;
	Qt::ToolButtonStyle style = Qt::ToolButtonIconOnly;
	if(icons && !texts) style = Qt::ToolButtonIconOnly;
	else if(!icons && texts) style = Qt::ToolButtonTextOnly;
	else if(icons && texts == 1) style = Qt::ToolButtonTextBesideIcon;
	else if(icons && texts == 2) style = Qt::ToolButtonTextUnderIcon;	
	setToolButtonStyle(style);
	
	//Mise à jour de la taille des icônes
	switch(icons)
	{
		case 0: //pas d'icône
			{
				QIcon empty;
				QPixmap emptyIcon;
				tbRezalContainer->setItemIcon(0,empty);
				tbRezalContainer->setItemIcon(1,empty);
				lblStatusIcon->hide();
				lblCountIcon->hide();
			}
			break;
		case 1: //petites icônes
		case 2: //grandes icones
			{
				if(pluginsAction->icon().isNull()) changeTheme();
				int dim = (icons == 2)?32:16;
				QSize size = QSize(dim,dim);
				setIconSize(size);
				lblStatusIcon->show();
				lblCountIcon->show();
			}
			break;
	}
	
	//Mise à jour de la position du texte
	if(texts)
	{
		lblStatus->show();
		lblCountIcon->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	}
	else
	{
		lblStatus->hide();
		lblCountIcon->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	}
}

/// Affichage du menu plug-ins lors d'un clic sur le bouton
/** Les actions sont gérées directement par le plug-in s'il a bien été programmé */
void QRezix::pluginsMenu()
{
	menuPlugins.clear();
	RzxPlugInLoader::global()->menuAction(menuPlugins);
	if(!menuPlugins.actions().count())
		menuPlugins.addAction("<none>");
//	menuPlugins.popup(btnPlugins->mapToGlobal(btnPlugins->rect().topRight()));
}
