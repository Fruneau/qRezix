/*
* trayicon.cpp - trayicon class
* Florent Bruneau, Copyright (C) 2004-2005 Binet Réseau
* Ported to Qt 4 by Guillaume Bandet, 2007
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/
#define RZX_MODULE_NAME "Systray"
#define RZX_MODULE_DESCRIPTION "Systray and Dock integration"
#define RZX_MODULE_ICON RzxThemedIcon("trayicon")

#include <QToolTip>
#include <QMenu>
#include <QPainter>

#include <RzxConfig>
#include <RzxComputer>
#include <RzxIconCollection>
#include <RzxConnectionLister>
#include <RzxApplication>
#include <RzxTranslator>

#include "rzxtrayicon.h"
#include "rzxtrayconfig.h"

#ifdef Q_WS_MAC
void qt_mac_set_dock_menu( QMenu *menu );
#endif

///Exporte le module
RZX_MODULE_EXPORT(RzxTrayIcon)

///Initialisation de la config
RZX_CONFIG_INIT(RzxTrayConfig)

/*!
  Creates a RzxTrayIcon object displaying \a icon and \a tooltip, and opening
  \a popup when clicked with the right mousebutton. \a parent and \a name are
  propagated to the QObject constructor. The icon is initially invisible.
 
  \sa show
*/
RzxTrayIcon::RzxTrayIcon()
		: RzxModule(RZX_MODULE_NAME, QT_TRANSLATE_NOOP("RzxBaseModule", "Systray and Dock integration"), RZX_MODULE_VERSION)
{
	beginLoading();
	setType(MOD_GUI);
#ifndef Q_WS_MAC
	setType(MOD_HIDE);
#endif
	setIcon(RZX_MODULE_ICON);
	tray = new QSystemTrayIcon();
	new RzxTrayConfig(this);
#ifndef Q_OS_MAC
	ui = NULL;
	propWidget = NULL;
	RzxTranslator::connect(this, SLOT(translate()));
#endif
	buildMenu();
	connect(RzxComputer::localhost(), SIGNAL(stateChanged(RzxComputer*)), this, SLOT(changeTrayIcon()));
	connect(RzxConnectionLister::global(), SIGNAL(countChange(const QString& )), this, SLOT(setToolTip(const QString& )));
	connect(tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));
	RzxIconCollection::connect(this, SLOT(changeTrayIcon()));
	changeTrayIcon();
	tray->show();
	endLoading();
}

/*!
  Removes the icon from the system tray and frees all allocated resources.
*/
RzxTrayIcon::~RzxTrayIcon()
{
	beginClosing();
	delete tray;
	delete RzxTrayConfig::global();
	endClosing();
}

/** \reimp */
bool RzxTrayIcon::isInitialised() const
{
	return true;
}

///Mise à jour de l'icône de qRezix
void RzxTrayIcon::changeTrayIcon()
{
	// Change l'icone dans la tray
	QPixmap trayIcon;
	if(RzxComputer::localhost()->state() == Rzx::STATE_DISCONNECTED)
		trayIcon = RzxIconCollection::getPixmap(Rzx::ICON_SYSTRAYDISCON);
	else if(!RzxConfig::autoResponder())
		trayIcon = RzxIconCollection::getPixmap(Rzx::ICON_SYSTRAYHERE);
	else
		trayIcon = RzxIconCollection::getPixmap(Rzx::ICON_SYSTRAYAWAY);
	if(trayIcon.isNull())
		trayIcon = RzxIconCollection::qRezixIcon();
#ifdef Q_WS_MAC
	buildMenu();
#else
	changePropTheme();
#endif
	setTrayIcon(trayIcon);
}

void RzxTrayIcon::setTrayIcon( const QPixmap &icon )
{
	QPixmap pix = QPixmap(icon.size());
	if(!RzxTrayConfig::transparent())
		pix.fill(QColor(RzxTrayConfig::backgroundColor()));
	else
		pix.fill(QColor(0, 0, 0 ,0));
	QPainter pixPainter(&pix);
	pixPainter.setRenderHint(QPainter::SmoothPixmapTransform);
	pixPainter.drawPixmap(0,0, icon);
	tray->setIcon(QIcon(pix));
}

QPixmap RzxTrayIcon::trayIcon() const
{
	return tray->icon().pixmap(32,32);
}

void RzxTrayIcon::setToolTip( const QString &tooltip )
{
	tray->setToolTip(tooltip);
}

QString RzxTrayIcon::toolTip() const
{
	return tray->toolTip();
}

///Contruit le menu contextuel
void RzxTrayIcon::buildMenu()
{
	if ( pop.actions().count() )
		pop.clear();

#ifndef Q_OS_MAC
	QList<RzxComputer*> list;
	QList<RzxComputer*> favList;
	RzxTrayConfig::QuickActions actions = (RzxTrayConfig::QuickAction)RzxTrayConfig::quickActions();

#define newMenu(mname, micon, text, filter, slot, favoris) \
	{ \
		mname.clear(); \
		mname.setTitle(text); \
		mname.setIcon(RzxIconCollection::getIcon(micon)); \
		list = RzxConnectionLister::global()->computerList(filter); \
		qSort(list.begin(), list.end(), computerLessThan); \
		favList.clear(); \
		if (favoris) \
			foreach(RzxComputer *computer, list) \
				if(testComputerFavorite(computer)) \
					favList << computer; \
		subMenu(mname, micon, slot, list, favList); \
	}

	if(actions & RzxTrayConfig::Chat)
		newMenu(chat, Rzx::ICON_CHAT, tr("Chat..."),testComputerChat, SLOT(chat()), (actions & RzxTrayConfig::ChatFav) );
	if(actions & RzxTrayConfig::Ftp)
		newMenu(ftp, Rzx::ICON_FTP, tr("Open FTP..."), testComputerFtp, SLOT(ftp()), (actions & RzxTrayConfig::ChatFav) );
	if(actions & RzxTrayConfig::Http)
		newMenu(http, Rzx::ICON_HTTP, tr("Open Web Page..."), testComputerHttp, SLOT(http()), 0);
	if(actions & RzxTrayConfig::Samba)
		newMenu(samba, Rzx::ICON_SAMBA, tr("Open Samba..."), testComputerSamba, SLOT(samba()), 0);
	if(actions & RzxTrayConfig::News)
		newMenu(news, Rzx::ICON_NEWS, tr("Read News..."), testComputerNews, SLOT(news()), 0);
	if(actions & RzxTrayConfig::Mail)
		newMenu(mail, Rzx::ICON_MAIL, tr("Send a Mail..."), testComputerMail, SLOT(mail()), 0);

#undef newMenu

	if(pop.actions().count())
		pop.addSeparator();
#endif //!Q_OS_MAC

	pop.addAction(RzxApplication::prefAction());
#define newItem(name, trad, receiver, slot) pop.addAction(RzxIconCollection::getIcon(name), trad, receiver, slot)
	if (RzxConfig::autoResponder())
		newItem(Rzx::ICON_HERE, tr( "&I'm back !" ), this, SIGNAL( wantToggleResponder() ) );
	else
		newItem(Rzx::ICON_AWAY, tr( "I'm &away !" ), this, SIGNAL( wantToggleResponder() ) );
#undef newItem
	
#ifndef Q_OS_MAC
	pop.addAction(RzxApplication::quitAction());
#else
	qt_mac_set_dock_menu( &pop );
#endif
}

///Fonction auxiliaire pour faire les titres dynamiques des sous-menus
QString RzxTrayIcon::titleFromSplit(int j, QString splitPoints)
{
	QString titre;
	// Faut penser à une façon plus sympa de renvoyer cette erreur (qui ne doit jamais arriver...)
	if ( (j+1) >= splitPoints.size() )
	{
		titre = "Sub";
		return titre;
	}

	// Le premier sous-menu est spécial, il doit commencer par la lettre du premier
	// Pour l'instant, vue la façon dont le découpage est fait,
	// il n'y a pas de possibilité qu'il comporte une seule lettre
	if ( j == 0 )
	{
		titre = "  -  ";
		titre[0] = splitPoints.at(0).toUpper();
		titre[4] = splitPoints.at(1).toUpper();
	}
	else {
		// Cas d'une seule lettre pour le sous-menu
		if ( (splitPoints.at(j).toAscii()+1) == splitPoints.at(j+1).toAscii() )
			titre = QString(QChar(splitPoints.at(j).toAscii() + 1).toUpper());

		else {
			titre = "  -  ";
			titre[0] = QChar(splitPoints.at(j).toAscii() + 1).toUpper();
			titre[4] = splitPoints.at(j+1).toUpper();
		}
	}

	return titre;
}

///Fonction auxiliaire pour calculer les points de separation des sous-menus
int RzxTrayIcon::calculerSplit(QString& splitPoints, QList<RzxComputer*> list)
{
	// Changer ça (dans les prefs / calculer avec QtApplication::desktop()->size() )
	int tailleMenu = 50;

	// Le menu est petit, rien à faire
	if (list.count() <= tailleMenu) return 0;

	splitPoints[0] = list[0]->name().at(0).toLower();
	int j = 1;
	QChar c;
	int i = 1;
	int i2 = 1;
	for(; i < list.count(); i++, i2++)
	{
		c = list[i-1]->name().at(0).toLower();
		if ( c != list[i]->name().at(0).toLower() )
		{
			splitPoints[j] = c;
			if (i2 > tailleMenu)
			{
				j++;
				i2 = 0;
			}
		}
	}
	splitPoints[j] = list[i-1]->name().at(0).toLower();

	return 1;
}

///Fonction qui cree les menus du TrayIcon
void RzxTrayIcon::subMenu(QMenu& mname, Rzx::Icon micon, const char * slot, QList<RzxComputer*> list, QList<RzxComputer*> fav)
{
	if(list.count())
	{
		QString splitPoints;
		QMenu * mnsub;
		bool makeSubMenu = calculerSplit(splitPoints, list);
		if ( makeSubMenu || fav.count() )
		{
			// Creation du sous-menu des favoris
			if (fav.count())
			{
				mnsub = new QMenu(tr("Favorites"), &mname);
				mnsub->setIcon(RzxIconCollection::getIcon(Rzx::ICON_FAVORITE));
				mname.addMenu(mnsub);

				qSort(fav.begin(), fav.end(), computerLessThan);
				for(int i = 0; i < fav.count(); i++)
					mnsub->addAction(fav[i]->icon(), fav[i]->name(), fav[i], slot);
			}

			// Necessaire pour eviter de tester contre splitPoints.at(1) si splitPoints est vide
			if ( !makeSubMenu )
			{
				mnsub = new QMenu(tr("Everybody"), &mname);
				mnsub->setIcon(RzxIconCollection::getIcon(micon));
				mname.addMenu(mnsub);
				for(int i = 0 ; i < list.count() ; i++)
					mnsub->addAction(list[i]->icon(), list[i]->name(), list[i], slot);
			}
			else
			{
				mnsub = new QMenu(titleFromSplit(0,splitPoints), &mname);
				mnsub->setIcon(RzxIconCollection::getIcon(micon));
				mname.addMenu(mnsub);
				int j = 1;
				for(int i = 0 ; i < list.count() ; i++)
				{
					if( list[i]->name().at(0).toLower() > splitPoints.at(j) )
					{
						mnsub = new QMenu(titleFromSplit(j,splitPoints), &mname);
						mnsub->setIcon(RzxIconCollection::getIcon(micon));
						mname.addMenu(mnsub);
						j++;
					}
					mnsub->addAction(list[i]->icon(), list[i]->name(), list[i], slot);
				}
			}
		 }
		else {
			for(int i = 0 ; i < list.count() ; i++)
				mname.addAction(list[i]->icon(), list[i]->name(), list[i], slot);
		}
		pop.addMenu(&mname);
	}
}


#ifndef Q_OS_MAC
#include "ui_rzxtrayprop.h"

/** \reimp */
QList<QWidget*> RzxTrayIcon::propWidgets()
{
	if(!ui)
		ui = new Ui::RzxTrayProp;
	if(!propWidget)
	{
		propWidget = new QWidget;
		ui->setupUi(propWidget);
#ifndef Q_WS_X11
		ui->groupSize->hide();
		ui->groupBackground->hide();
#endif
		connect(ui->btnColor, SIGNAL(clicked()), this, SLOT(selectColor()));
	}
	return QList<QWidget*>() << propWidget;
}

/** \reimp */
QStringList RzxTrayIcon::propWidgetsName()
{
	return QStringList() << name();
}

///Change le thème d'icône dans le fenêtre de préférences
void RzxTrayIcon::changePropTheme()
{
	if(!ui) return;

	ui->cbQuickChat->setIcon(RzxIconCollection::getIcon(Rzx::ICON_CHAT));
	ui->cbQuickFtp->setIcon(RzxIconCollection::getIcon(Rzx::ICON_FTP));
	ui->cbQuickHttp->setIcon(RzxIconCollection::getIcon(Rzx::ICON_HTTP));
	ui->cbQuickSamba->setIcon(RzxIconCollection::getIcon(Rzx::ICON_SAMBA));
	ui->cbQuickNews->setIcon(RzxIconCollection::getIcon(Rzx::ICON_NEWS));
	ui->cbQuickMail->setIcon(RzxIconCollection::getIcon(Rzx::ICON_MAIL));
	ui->cbQuickFav->setIcon(RzxIconCollection::getIcon(Rzx::ICON_FAVORITE));
}

/** \reimp */
void RzxTrayIcon::propInit(bool def)
{
	RzxTrayConfig::QuickActions actions = (RzxTrayConfig::QuickAction)RzxTrayConfig::quickActions();
	ui->cbQuickChat->setChecked(actions & RzxTrayConfig::Chat);
	ui->cbQuickFtp->setChecked(actions & RzxTrayConfig::Ftp);
	ui->cbQuickHttp->setChecked(actions & RzxTrayConfig::Http);
	ui->cbQuickSamba->setChecked(actions & RzxTrayConfig::Samba);
	ui->cbQuickNews->setChecked(actions & RzxTrayConfig::News);
	ui->cbQuickMail->setChecked(actions & RzxTrayConfig::Mail);
	ui->cbQuickFav->setChecked(actions & RzxTrayConfig::ChatFav);

	ui->cbTransparent->setChecked(RzxTrayConfig::transparent());
	bg = RzxTrayConfig::backgroundColor();
	updateBackground();
	
	changePropTheme();
}

/** \reimp */
void RzxTrayIcon::propUpdate()
{
	if(!ui) return;

	RzxTrayConfig::QuickActions actions;
	if(ui->cbQuickChat->isChecked()) actions |= RzxTrayConfig::Chat;
	if(ui->cbQuickFtp->isChecked()) actions |= RzxTrayConfig::Ftp;
	if(ui->cbQuickHttp->isChecked()) actions |= RzxTrayConfig::Http;
	if(ui->cbQuickSamba->isChecked()) actions |= RzxTrayConfig::Samba;
	if(ui->cbQuickNews->isChecked()) actions |= RzxTrayConfig::News;
	if(ui->cbQuickMail->isChecked()) actions |= RzxTrayConfig::Mail;
	if(ui->cbQuickFav->isChecked()) actions |= RzxTrayConfig::ChatFav;
	RzxTrayConfig::setQuickActions(actions);

	RzxTrayConfig::setTransparent(ui->cbTransparent->isChecked());
	RzxTrayConfig::setBackgroundColor(bg.rgb());
}

/** \reimp */
void RzxTrayIcon::propClose()
{
	if(propWidget)
	{
		delete propWidget;
		propWidget = NULL;
	}
	if(ui)
	{
		delete ui;
		ui = NULL;
	}
}

///
void RzxTrayIcon::trayActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch(reason)
	{
	case QSystemTrayIcon::Context:
		buildMenu();
		tray->setContextMenu(&pop);
		break;
	case QSystemTrayIcon::Trigger:
	case QSystemTrayIcon::DoubleClick:
		emit wantToggleVisible();
		break;
	default:
		break;
	}
}

///Mise à jour de la traduction
void RzxTrayIcon::translate()
{
	if(ui)
		ui->retranslateUi(propWidget);
}

///Affichage de la couleur de fond
void RzxTrayIcon::updateBackground()
{
	QPixmap pix(16, 16);
	pix.fill(bg);
	ui->lblColor->setPixmap(pix);
}

#include <QColorDialog>

///Affichage d'un dialogue demandant de choisir la couleur
void RzxTrayIcon::selectColor()
{
	QWidget *widget = propWidget->window();
	bg = QColorDialog::getColor(bg, widget);
	updateBackground();
	if(widget)
		widget->raise();
}

#endif
