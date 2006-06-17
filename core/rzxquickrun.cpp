/***************************************************************************
                          rzxquickrun  -  description
                             -------------------
    begin                : Sat Jun 17 2006
    copyright            : (C) 2006 by Florent Bruneau
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
#include <RzxApplication>
#include <RzxIconCollection>
#include <RzxConnectionLister>

#include <RzxQuickRun>
#include "ui_rzxquickrun.h"

///Définition des différentes actions...
/** Ces informations sont utilisées pour mettre à jour de façon dynamique la fenêtre
 */
RzxQuickRun::Action RzxQuickRun::actions[] = {
	{ QT_TR_NOOP("Start a chat"), Rzx::ICON_CHAT, true, testComputerChat },
	{ QT_TR_NOOP("Open FTP"), Rzx::ICON_FTP, true, testComputerFtp },
	{ QT_TR_NOOP("Open Web"), Rzx::ICON_HTTP, true, testComputerHttp },
	{ QT_TR_NOOP("Read news"), Rzx::ICON_NEWS, true, testComputerNews },
	{ QT_TR_NOOP("Open Samba"), Rzx::ICON_SAMBA, true, testComputerSamba },
	{ QT_TR_NOOP("Send a mail"), Rzx::ICON_MAIL, true, testComputerMail },
	{ QT_TR_NOOP("View properties"), Rzx::ICON_PROPRIETES, true, testComputerProperties },
	{ QT_TR_NOOP("Read history"), Rzx::ICON_HISTORIQUE, true, testComputerChat },
	{ QT_TR_NOOP("Add to favorites"), Rzx::ICON_FAVORITE, true, testComputerNotFavorite },
	{ QT_TR_NOOP("Remove from favorites"), Rzx::ICON_NOTFAVORITE, true, testComputerFavorite },
	{ QT_TR_NOOP("Add to banlist"), Rzx::ICON_BAN, true, testComputerNotBan },
	{ QT_TR_NOOP("Remove from banlist"), Rzx::ICON_UNBAN, true, testComputerBan },
	{ QT_TR_NOOP("Change away state"), Rzx::ICON_AWAY, false, NULL },
	{ QT_TR_NOOP("Open preferences"), Rzx::ICON_PREFERENCES, false, NULL }
};

///Construit une boîte "Quick Action"
RzxQuickRun::RzxQuickRun(QWidget *parent)
	:QDialog(parent)
{
	setModal(false);
	setAttribute(Qt::WA_DeleteOnClose);

	ui = new Ui::RzxQuickRun();
	ui->setupUi(this);
	ui->btnOK->setIcon(RzxIconCollection::getIcon(Rzx::ICON_OK));
	ui->btnCancel->setIcon(RzxIconCollection::getIcon(Rzx::ICON_CANCEL));
	
	for(int i = 0 ; i < NbActions ; i++)
	{
		const Action &action = actions[i];
		ui->cbAction->addItem(RzxIconCollection::getIcon(action.icon), tr(action.name), action.needComputer);
	}
	actionChanged(0);
	connect(ui->cbAction, SIGNAL(currentIndexChanged(int)), this, SLOT(actionChanged(int)));
	
	raise();
	show();
}

///Applique le changement
void RzxQuickRun::actionChanged(int actionNb)
{
	const Action &action = actions[actionNb];
	ui->lblWho->setEnabled(action.needComputer);
	ui->cbComputer->setEnabled(action.needComputer);
	ui->lblWho->setVisible(action.needComputer);
	ui->cbComputer->setVisible(action.needComputer);
	adjustSize();
	if(action.needComputer)
	{
		ui->cbComputer->clear();
		RzxComputer::testComputer *filter = action.filter;
		QList<RzxComputer*> list;
		if(!filter)
			list = RzxConnectionLister::global()->computerList();
		else
			list = RzxConnectionLister::global()->computerList(filter);
		qSort(list.begin(), list.end(), computerLessThan);
		for(int i = 0 ; i < list.size() ; i++)
		{
			const RzxComputer *computer = list[i];
			ui->cbComputer->addItem(computer->icon(), computer->name(), computer->ip().toString());
		}
	}
}

///Ferme la fenêtre après exécution de la commande
void RzxQuickRun::accept()
{
	Actions action = (Actions)ui->cbAction->currentIndex();
	RzxComputer *computer = NULL;
	if(actions[action].needComputer)
	{
		const int computerIndex = ui->cbComputer->currentIndex();
		computer = RzxHostAddress(ui->cbComputer->itemData(computerIndex).toString()).computer();
		
		if(computer == NULL)
			QDialog::accept();
	}
	
	switch((Actions)ui->cbAction->currentIndex())
	{
		case Chat: computer->chat(); break;
		case Ftp: computer->ftp(); break;
		case Http: computer->http(); break;
		case News: computer->news(); break;
		case Samba: computer->samba(); break;
		case Mail: computer->mail(); break;
		case Properties: computer->checkProperties(); break;
		case History: computer->history(); break;
		case AddFavorite: computer->addToFavorites(); break;
		case RemoveFavorite: computer->removeFromFavorites(); break;
		case AddBan: computer->ban(); break;
		case RemoveBan: computer->unban(); break;
		case Away: RzxComputer::localhost()->setState(!RzxComputer::localhost()->isOnResponder()); break;
		case Preferences: RzxApplication::instance()->preferences(); break;
		default: break;
	}
	QDialog::accept();
}
