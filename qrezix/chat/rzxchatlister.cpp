/***************************************************************************
                          rzxchatlister  -  description
                             -------------------
    begin                : Thu Jul 28 2005
    copyright            : (C) 2005 by Florent Bruneau
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
//Pour les fenêtres historique/proprités
#include <QFrame>
#include <QDialog>
#include <QGridLayout>
//Pour la fenêtre d'historique
#include <QFile>
#include <QTextStream>
#include <QTextEdit>
#include <QTextCursor>
//Pour la fenêtre propriétés
#include <QLabel>
#include <QHeaderView>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "../core/rzxglobal.h"
#include "../core/rzxconfig.h"
#include "../core/rzxiconcollection.h"
#include "../core/rzxmessagebox.h"
#include "../core/rzxconnectionlister.h"
#include "../rzxapplication.h"

#include "rzxchatlister.h"

#include "rzxchat.h"
#include "rzxclientlistener.h"


RzxChatLister *RzxChatLister::object = NULL;

RzxChatLister::RzxChatLister()
	:RzxModule("Chat 1.7.0-svn", QT_TR_NOOP("qRezix graphical chat interface"))
{
	beginLoading();
	wellInit = false;
	object = this;
	setType(MOD_CHATUI);
	client = RzxClientListener::global();
	connect(client, SIGNAL(propertiesSent(const RzxHostAddress&)), this, SLOT(warnProperties(const RzxHostAddress&)));

	RzxConnectionLister *lister = RzxConnectionLister::global();
	connect(lister, SIGNAL(login(RzxComputer* )), this, SLOT(login(RzxComputer* )));
	connect(lister, SIGNAL(update(RzxComputer* )), this, SLOT(login(RzxComputer* )));
	connect(lister, SIGNAL(logout(RzxComputer* )), this, SLOT(logout(RzxComputer* )));

	connect(lister, SIGNAL(wantChat(RzxComputer* )), this, SLOT(createChat(RzxComputer* )));
	connect(lister, SIGNAL(wantHistorique(RzxComputer* )), this, SLOT(historique(RzxComputer* )));
	connect(lister, SIGNAL(wantProperties(RzxComputer* )), this, SLOT(proprietes(RzxComputer* )));

	if(!client->listen(RzxConfig::chatPort()) )
	{
		RzxMessageBox::warning( (QWidget *) parent(), "qRezix",
			tr("Cannot create peer to peer socket !\n\nChat and properties browsing are disabled") );
	}
	else
	{
		RzxComputer::localhost()->addCapabilities(Rzx::CAP_CHAT);
		wellInit = true;
	}
	endLoading();
}

RzxChatLister::~RzxChatLister()
{
	beginClosing();
	closeChats();
	client->deleteLater();
	endClosing();
}

/** Sert aussi au raffraichissement des données*/
void RzxChatLister::login(RzxComputer *computer)
{
	RzxChat *chat = chatByLogin.take(computer->name());
	if(chat)
	{
		//Indication au chat de la reconnexion
		if (!computer)
			chat->info( tr("reconnected") );
		chatByLogin.insert(computer->name(), chat);
		chat->setHostname(computer->name());
	}
}

///Enregistre la déconnexion d'un client
void RzxChatLister::logout(RzxComputer *computer)
{
	RzxChat *chat = getChatByIP(computer->ip());
	if ( chat )
		chat->info( tr("disconnected") );
}

///Création d'une fenêtre de chat associée à l'ordinateur
RzxChat *RzxChatLister::createChat(RzxComputer *computer)
{
	if(!computer)
		return NULL;

	RzxHostAddress peer = computer->ip();
	RzxChat *chat = getChatByIP(peer);
	if(!chat)
	{
		RzxChat *chat = new RzxChat(peer);
		chat->setHostname( computer->name() );

		connect( chat, SIGNAL( closed( const RzxHostAddress& ) ), this, SLOT( deleteChat( const RzxHostAddress& ) ) );
		connect( RzxIconCollection::global(), SIGNAL(themeChanged(const QString& )), chat, SLOT( changeTheme() ) );
		connect( RzxConfig::global(), SIGNAL( iconFormatChange() ), chat, SLOT( changeIconFormat() ) );
		chatByIP.insert(peer, chat);
		chatByLogin.insert(computer->name(), chat);
		chat->show();
	}
	return chat;
}

///Création d'une fenêtre associée à une addresse
RzxChat *RzxChatLister::createChat(const RzxHostAddress& ip)
{
	return createChat(RzxConnectionLister::global()->getComputerByIP(ip));
}

///Fermeture du chat (si il existe) associé au login
void RzxChatLister::closeChat( const QString& login )
{
	RzxChat *chat = getChatByName(login);
	if(chat)
		chat->close();
}

/** No descriptions */
void RzxChatLister::deleteChat( const RzxHostAddress& peerAddress )
{
	RzxChat *chat = chatByIP.take(peerAddress);
	chatByLogin.remove(chat->hostname());
	delete chat;
}

///Fermeture des chats en cours
/** Cette méthode à pour but de fermer tous les chats en cours pour libérer en particulier le port 5050. Normalement l'appel de cette méthode à la fermeture de qRezix doit corriger le problème de réouverture de l'écoute qui intervient à certains moments lors du démarrage de qRezix */
void RzxChatLister::closeChats()
{
	foreach(RzxChat *chat, chatByIP.values())
		chat->close();
}

///Demande le check des proiétés de \a peer
void RzxChatLister::proprietes(RzxComputer *computer)
{
	RzxChat *object = getChatByIP(computer->ip());
	if(!object)
		client->checkProperty(computer->ip());
	else
	{
		if(object->socket())
			object->socket()->sendPropQuery();
		else
			client->checkProperty(computer->ip());
	}
}

///Indique que les propriétés ont été checkées par \a peer
void RzxChatLister::warnProperties( const RzxHostAddress& peer )
{
	RzxChat *chat = getChatByIP(peer);
	RzxComputer *computer = RzxConnectionLister::global()->getComputerByIP(peer);
	if (!computer)
		return ;
	QTime cur = QTime::currentTime();
	QString heure;
	heure.sprintf( "%2i:%.2i:%.2i",
	               cur.hour(),
	               cur.minute(),
	               cur.second() );

	if (!chat)
	{
		if (RzxConfig::global()->warnCheckingProperties()== 0)
			return ;
		RzxMessageBox::information(NULL, tr("Properties sent to %1").arg(computer->name()),
			tr("name : <i>%1</i><br>"
				"address : <i>%2</i><br>"
				"client : <i>%3</i><br>"
				"time : <i>%4</i>")
				.arg(computer->name()).arg(peer.toString()).arg(computer->client()).arg(heure));
		return ;
	}
	chat->notify( tr( "has checked your properties" ), true );
}


///Affichage des proprietes d'un ordinateur
QWidget *RzxChatLister::showProperties(RzxComputer *computer, const QString& msg, bool withFrame, QWidget *parent, QPoint *pos )
{
	QWidget *propertiesDialog;
	if(!computer)
		return NULL;

	if(withFrame)
	{
		// creation de la boite de dialogue (non modale, elle se detruit automatiquement grace a WDestructiveClose)
		propertiesDialog = new QDialog(parent?parent:RzxApplication::mainWindow(), Qt::Tool | Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
		propertiesDialog->setAttribute(Qt::WA_DeleteOnClose);
		propertiesDialog->resize(300, 320);

		propertiesDialog->setWindowTitle( tr( "%1's properties" ).arg(computer->name()) );
	}
	else
	{
		propertiesDialog = new RzxPopup(parent?parent:RzxApplication::mainWindow());
		((QFrame*)propertiesDialog)->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
		if(pos) propertiesDialog->move(*pos);
	}

	// Layout, pour le resize libre
	QGridLayout * qPropertiesLayout = new QGridLayout(propertiesDialog);
	qPropertiesLayout->setSpacing(0);
	qPropertiesLayout->setMargin(withFrame?6:0);
 
	// creation de la liste des proprietes et ajout au layout
	QTreeWidget* propList = new QTreeWidget();
	QLabel *clientLabel = new QLabel(tr("xNet client : %1").arg(computer->client()));
	qPropertiesLayout->addWidget(propList, 0, 0);
	qPropertiesLayout->addWidget(clientLabel, 300, 0);
 
	QPalette palette;
	palette.setColor(propList->backgroundRole(), QColor(255,255,255));
	propList->setPalette(palette);
	propList->resize(300, 300);
	propList->setColumnCount(2);
	propList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	propList->setSortingEnabled(false);
	
	// Création des en-têtes de colonnes
	QTreeWidgetItem *item = new QTreeWidgetItem();
	item->setText(0, tr("Property"));
	item->setText(1, tr("Value"));
	propList->setHeaderItem(item);
	propList->setRootIsDecorated(false);

	// Remplissage
	QStringList props = msg.split('|');
	int propCount = 0;
	item = NULL;
	for(int i = 0 ; i < props.size() - 1 ; i+=2)
	{
		item = new QTreeWidgetItem(propList, item);
		item->setText(0, props[i]);
		item->setText(1, props[i+1]);
		propCount++;
	}
	QHeaderView *header = propList->header();
	header->resizeSection(0, header->sectionSizeHint(0));
	header->resizeSection(1, header->sectionSizeHint(1));
	if(!propCount)
	{
		propertiesDialog->deleteLater();
		return NULL;
	}

	propertiesDialog->raise();
	if(withFrame)
		propertiesDialog->show();
 
	// Fit de la fenetre, on ne le fait pas si il n'y a pas d'accents, sinon ca plante
//	int width=PropList->columnWidth(0)+PropList->columnWidth(1)+4+12;
	int height=(propCount+3)*20; //+20 pour le client xnet, et puis headers e un peu de marge
	propertiesDialog->resize(header->sizeHint().width(), height);
	RzxConfig::addCache(computer->ip(), msg);
	return propertiesDialog;
}

///Idem avec un RzxHostAddress
QWidget *RzxChatLister::showProperties(const RzxHostAddress& peer, const QString& msg, bool withFrame, QWidget *parent, QPoint *pos )
{
	return showProperties(RzxConnectionLister::global()->getComputerByIP(peer), msg, withFrame, parent, pos);
}

///Affichage de la fenêtre de favoris
QWidget *RzxChatLister::historique(RzxComputer *computer, bool withFrame, QWidget *parent, QPoint *pos )
{
	const RzxHostAddress &ip = computer->ip();
	const QString &hostname = computer->name();

	// chargement de l'historique
	QString filename = RzxConfig::historique(ip.toRezix(), hostname);
	if (filename.isNull())
		return NULL;
 
	QString text;
	QFile file(filename);
	if(!file.exists())
		return NULL;
	
	file.open(QIODevice::ReadOnly); 
	QTextStream stream(&file);
	stream.setCodec("UTF-8");
	while(!stream.atEnd()) {
		text += stream.readLine();
	}
	file.close();
 
	// construction de la boite de dialogue
	QWidget *histoDialog;
	if(withFrame)
	{
		histoDialog = new QDialog(parent?parent:RzxApplication::mainWindow(), Qt::Tool | Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
		histoDialog->setAttribute(Qt::WA_DeleteOnClose);

		histoDialog->setWindowTitle( tr( "History - %1" ).arg(hostname) );
	}
	else
	{
		histoDialog = new RzxPopup(parent?parent:RzxApplication::mainWindow());
		((QFrame*)histoDialog)->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
		if(pos) 
		{
			QPoint ul = *pos;
			histoDialog->move(ul);
		}
	}
	QGridLayout * qHistoLayout = new QGridLayout(histoDialog);
	qHistoLayout->setSpacing(0);
	qHistoLayout->setMargin(withFrame?6:0);


	// creation de la liste des proprietes et ajout au layout
	QTextEdit* histoView = new QTextEdit(histoDialog);
	histoView->setReadOnly(true);
	qHistoLayout->addWidget((QWidget*)histoView, 0, 0);
	
	histoDialog->resize(450, 300);
	QPalette palette;
	palette.setColor(histoView->backgroundRole(), QColor(255,255,255));
	histoView->setPalette(palette);
	histoView->setHtml(text);
	histoView->textCursor().movePosition(QTextCursor::End);
	histoView->ensureCursorVisible();
	
	histoDialog->raise();
	if(withFrame)
		histoDialog->show();
	return histoDialog;
}

///Idem avec un RzxHostAddress
QWidget *RzxChatLister::historique(const RzxHostAddress& peer, bool withFrame, QWidget *parent, QPoint *pos )
{
	return historique(RzxConnectionLister::global()->getComputerByIP(peer), withFrame, parent, pos);
}
