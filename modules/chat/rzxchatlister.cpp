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
#include <QComboBox>
#include <QTime>

#include <RzxGlobal>
#include <RzxConfig>
#include <RzxIconCollection>
#include <RzxMessageBox>
#include <RzxConnectionLister>
#include <RzxApplication>
#include <RzxProperty>
#include <RzxComputer>


#include "rzxchatlister.h"

///Exporte le module
RZX_MODULE_EXPORT(RzxChatLister)

#include "rzxchat.h"
#include "rzxclientlistener.h"
#include "ui_rzxchatpropui.h"
#include "rzxchatconfig.h"


RZX_CONFIG_INIT(RzxChatConfig)
RZX_GLOBAL_INIT(RzxChatLister)

RzxChatLister::RzxChatLister()
	:RzxModule("Chat 1.7.0-svn", QT_TR_NOOP("qRezix graphical chat interface"))
{
	beginLoading();
	propWidget = NULL;
	ui = NULL;
	wellInit = false;
	object = this;
	setType(MOD_CHATGUI);
	setType(MOD_CHAT);
	setType(MOD_PROPERTIES);
	setType(MOD_PROPGUI);
	setIcon(Rzx::ICON_CHAT);

	new RzxChatConfig(this);
	RzxChatConfig::global()->loadFontList();

	client = RzxClientListener::global();
	connect(client, SIGNAL(propertiesSent(RzxComputer*)), this, SLOT(warnProperties(RzxComputer*)));
	connect(client, SIGNAL(haveProperties(RzxComputer*)), this, SIGNAL(haveProperties(RzxComputer*)));

	RzxConnectionLister *lister = RzxConnectionLister::global();
	connect(lister, SIGNAL(login(RzxComputer* )), this, SLOT(login(RzxComputer* )));
	connect(lister, SIGNAL(update(RzxComputer* )), this, SLOT(login(RzxComputer* )));
	connect(lister, SIGNAL(logout(RzxComputer* )), this, SLOT(logout(RzxComputer* )));

	if(!client->listen(RzxChatConfig::chatPort()) )
	{
		RzxMessageBox::warning( (QWidget *) parent(), "qRezix",
			tr("Cannot create peer to peer socket !\n\nChat and properties browsing are disabled") );
	}
	else
	{
		RzxComputer::localhost()->addCapabilities(Rzx::CAP_CHAT);
		wellInit = true;
	}
	
	//Recherche des thèmes de smileys installés
	qDebug("Searching smileys themes...");
	QList<QDir> path = RzxConfig::dirList(RzxConfig::AllDirsExceptTemp, "smileys");

	foreach(QDir dir, path)
	{
		QStringList subDirs = dir.entryList(QDir::Dirs, QDir::Name | QDir::IgnoreCase);
		foreach(QString subDir, subDirs)
		{
			//on utilise .keys().contain() car value[] fait un insert dans le QHash
			//ce qui tendrait donc à rajouter des clés parasites dans la liste
			if(!smileyDir.keys().contains(subDir))
			{
				QDir *theme = new QDir(dir);
				theme->cd(subDir);
				if(theme->exists("theme"))
				{
					qDebug() << "*" << subDir << "in" << theme->path();
					smileyDir.insert(subDir, theme);
				}
				else
					delete theme;
			}
		}
	}
	loadSmileys();
	endLoading();
}

RzxChatLister::~RzxChatLister()
{
	beginClosing();
	closeChats();
	client->deleteLater();
	delete RzxChatConfig::global();
	endClosing();
	RZX_GLOBAL_CLOSE
}

/** Sert aussi au raffraichissement des données*/
void RzxChatLister::login(RzxComputer *computer)
{
	RzxChat *chat = getChatByIP(computer->ip());
	if(chat)
	{
		//Indication au chat de la reconnexion
		chat->setComputer(computer);
		if (!computer)
			chat->info( tr("reconnected") );
		if(chatByLogin[computer->name()] != chat)
		{
			chatByLogin.remove(chatByLogin.key(chat));
			chatByLogin.insert(computer->name(), chat);
		}
	}
}

///Enregistre la déconnexion d'un client
void RzxChatLister::logout(RzxComputer *computer)
{
	RzxChat *chat = getChatByIP(computer->ip());
	if ( chat )
		chat->info( tr("disconnected") );
}

///Lancement du chat
void RzxChatLister::chat(RzxComputer *computer)
{
	createChat(computer);
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
		chat = new RzxChat(computer);

		connect(chat, SIGNAL(send(const QString&)), this, SIGNAL(wantDeactivateResponder()));
		connect( chat, SIGNAL( closed(RzxComputer*) ), this, SLOT( deleteChat(RzxComputer*) ) );
		connect( RzxIconCollection::global(), SIGNAL(themeChanged(const QString& )), chat, SLOT( changeTheme() ) );
		connect( RzxConfig::global(), SIGNAL( iconFormatChange() ), chat, SLOT( changeIconFormat() ) );
		chatByIP.insert(peer, chat);
		chatByLogin.insert(computer->name(), chat);
		chat->show();
	}
	return chat;
}

///Fermeture du chat (si il existe) associé au login
void RzxChatLister::closeChat( const QString& login )
{
	RzxChat *chat = getChatByName(login);
	if(chat)
		chat->close();
}

/** No descriptions */
void RzxChatLister::deleteChat(RzxComputer *c)
{
	if(!c) return;

	RzxChat *chat = chatByIP.take(c->ip());
	chatByLogin.remove(c->name());
	delete chat;
}

///Réception d'un chat...
void RzxChatLister::receiveChatMessage(RzxComputer *computer, Rzx::ChatMessageType type, const QString& msg)
{
	if(!computer) return;

	RzxChat *chat = getChatByIP(computer->ip());
	if(!chat && type == Rzx::Chat)
		chat = createChat(computer);
	if(!chat) return;
	chat->receiveChatMessage(type, msg);
}

///Envoie d'un chat
void RzxChatLister::sendChatMessage(RzxComputer *computer, Rzx::ChatMessageType type, const QString& msg)
{
	client->sendChatMessage(computer, type, msg);
}

///Fermeture des chats en cours
/** Cette méthode à pour but de fermer tous les chats en cours pour libérer en particulier le port 5050. Normalement l'appel de cette méthode à la fermeture de qRezix doit corriger le problème de réouverture de l'écoute qui intervient à certains moments lors du démarrage de qRezix */
void RzxChatLister::closeChats()
{
	foreach(RzxChat *chat, chatByIP.values())
		chat->close();
}

///Demande l'affichage de l'historique
void RzxChatLister::history(RzxComputer *c)
{
	historique(c);
}

///Demande le check des proiétés de \a peer
void RzxChatLister::properties(RzxComputer *computer)
{
	client->checkProperty(computer->ip());
}

///Indique que les propriétés ont été checkées par \a peer
void RzxChatLister::warnProperties(RzxComputer *computer)
{
	if(!computer) return;

	RzxChat *chat = getChatByIP(computer->ip());
	QTime cur = QTime::currentTime();
	QString heure;
	heure.sprintf( "%2i:%.2i:%.2i",
	               cur.hour(),
	               cur.minute(),
	               cur.second() );

	if(!chat)
	{
		if(!RzxChatConfig::warnWhenChecked())
			return ;
		RzxMessageBox::information(NULL, tr("Properties sent to %1").arg(computer->name()),
			tr("name : <i>%1</i><br>"
				"address : <i>%2</i><br>"
				"client : <i>%3</i><br>"
				"time : <i>%4</i>")
				.arg(computer->name()).arg(computer->ip().toString()).arg(computer->client()).arg(heure));
		return ;
	}
	chat->notify( tr( "has checked your properties" ), true );
}

///Demande l'affichage des propriétés
void RzxChatLister::showProperties(RzxComputer *c)
{
	if(!c) return;

	RzxChat *chat = getChatByIP(c->ip());
	if(!chat)
		showProperties(c->ip(), RzxConfig::cache(c->ip()));
	else
		chat->receiveProperties(RzxConfig::cache(c->ip()));
}

///Affichage des proprietes d'un ordinateur
QWidget *RzxChatLister::showProperties(RzxComputer *computer, const QString& msg, bool withFrame, QWidget *parent, QAbstractButton *button)
{
	if(!computer)
		return NULL;

	QWidget *propertiesDialog;

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
		propertiesDialog = new RzxChatPopup(button, parent?parent:RzxApplication::mainWindow());
		((QFrame*)propertiesDialog)->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
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
	return propertiesDialog;
}

///Affichage de la fenêtre de favoris
QWidget *RzxChatLister::historique(RzxComputer *computer, bool withFrame, QWidget *parent, QAbstractButton *button)
{
	const RzxHostAddress &ip = computer->ip();
	const QString &hostname = computer->name();

	// chargement de l'historique
	QString filename = RzxChatConfig::historique(ip.toRezix(), hostname);
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
		histoDialog = new RzxChatPopup(button, parent?parent:RzxApplication::mainWindow());
		((QFrame*)histoDialog)->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
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

/** \reimp */
QList<QWidget*> RzxChatLister::propWidgets()
{
	if(!ui)
		ui = new Ui::RzxChatPropUI;
	if(!propWidget)
	{
		propWidget = new QWidget;
		ui->setupUi(propWidget);
		connect( ui->btnBeepBrowse, SIGNAL( clicked() ), this, SLOT( chooseBeep() ) );
	}
	return QList<QWidget*>() << propWidget;
}

/** \reimp */
QStringList RzxChatLister::propWidgetsName()
{
	return QStringList() << name();
}

/** \reimp */
void RzxChatLister::propInit(bool def)
{
	ui->chkBeep->setChecked(RzxChatConfig::beep(def));
	ui->beepSound->setText(RzxChatConfig::beepSound(def));
	ui->cbPropertiesWarning->setChecked(RzxChatConfig::warnWhenChecked(def));
	ui->cbPrintTime->setChecked(RzxChatConfig::printTime(def));
	ui->chat_port->setValue(RzxChatConfig::chatPort(def));
	ui->smileyCombo->addItems(smileyDir.keys());
	ui->smileyCombo->setCurrentIndex(ui->smileyCombo->findText(RzxChatConfig::smileyTheme()));
}

/** \reimp */
void RzxChatLister::propUpdate()
{
	if(!ui) return;

	RzxChatConfig::setBeep(ui->chkBeep->isChecked());
	RzxChatConfig::setBeepSound(ui->beepSound->text());
	RzxChatConfig::setWarnWhenChecked(ui->cbPropertiesWarning->isChecked());
	RzxChatConfig::setPrintTime(ui->cbPrintTime->isChecked());
	RzxChatConfig::setChatPort(ui->chat_port->value());
	RzxChatConfig::setSmileyTheme(ui->smileyCombo->currentText());
	loadSmileys();
}

/** \reimp */
void RzxChatLister::propClose()
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

///Choix du son pour le beep
void RzxChatLister::chooseBeep()
{
	QString file = RzxProperty::browse(tr("All files"), tr("Sound file selection"), "*");
	if (file.isEmpty()) return;

	ui->beepSound->setText(file);
}

/// Chargement des correspondances motif/smiley
void RzxChatLister::loadSmileys(){
	smileys.clear();
	// chargement de la config
	QDir *dir = smileyDir[RzxChatConfig::smileyTheme()];
	if (dir){
		QString text;
		QFile file(dir->absolutePath()+"/theme");
		if(file.exists()){
			file.open(QIODevice::ReadOnly);
			QTextStream stream(&file);
			stream.setCodec("UTF-8");
			while(!stream.atEnd()) {
				text = stream.readLine();
				QStringList list = text.split("###");
				if(list.count() == 2){
					QStringList rep = list[0].split("$$");
					for (int i = 0; i < rep.size(); ++i) {
						QStringList item;
						item << rep[i].trimmed();
						item << list[1];
						smileys << item;
					}
				}
			}
			file.close();
		}
	}
}
