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
#define RZX_MODULE_NAME "Chat"
#define RZX_MODULE_DESCRIPTION "qRezix graphical chat interface"
#define RZX_MODULE_ICON Rzx::ICON_CHAT

//Pour les fen�tres historique/proprit�s
#include <QFrame>
#include <QDialog>
#include <QGridLayout>
//Pour la fen�tre d'historique
#include <QFile>
#include <QTextStream>
#include <QTextCursor>
//Pour la fen�tre propri�t�s
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
#include <RzxTranslator>


#include "rzxchatlister.h"

///Exporte le module
RZX_MODULE_EXPORT(RzxChatLister)

#include "rzxchat.h"
#include "rzxclientlistener.h"
#include "rzxfilelistener.h"
#include "ui_rzxchatprop.h"
#include "rzxchatconfig.h"
#include "rzxsmileys.h"
#include "rzxchatbrowser.h"


RZX_CONFIG_INIT(RzxChatConfig)
RZX_GLOBAL_INIT(RzxChatLister)

RzxChatLister::RzxChatLister()
	:RzxModule(RZX_MODULE_NAME, QT_TRANSLATE_NOOP("RzxBaseModule", "qRezix graphical chat interface"), RZX_MODULE_VERSION)
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
	setIcon(RZX_MODULE_ICON);

	new RzxChatConfig(this);

	client = new RzxClientListener();;
	fileServer = new RzxFileListener();
	connect(client, SIGNAL(propertiesSent(RzxComputer*)), this, SLOT(warnProperties(RzxComputer*)));
	connect(client, SIGNAL(haveProperties(RzxComputer*)), this, SIGNAL(haveProperties(RzxComputer*)));

	RzxConnectionLister *lister = RzxConnectionLister::global();
	connect(lister, SIGNAL(login(RzxComputer* )), this, SLOT(login(RzxComputer* )));
	connect(lister, SIGNAL(update(RzxComputer* )), this, SLOT(login(RzxComputer* )));
	connect(lister, SIGNAL(logout(RzxComputer* )), this, SLOT(logout(RzxComputer* )));
	RzxIconCollection::connect(this, SLOT(changeTheme()));
	RzxTranslator::connect(this, SLOT(translate()));

	if(!client->listen(RzxChatConfig::chatPort()) )
		RzxMessageBox::warning((QWidget *)parent(), "qRezix",
			tr("Cannot create peer to peer socket!\n\nChat and properties browsing are disabled") );
	else
		RzxComputer::localhost()->addCapabilities(Rzx::CAP_CHAT);
	if(!fileServer->listen(RzxChatConfig::filePort()) )
		RzxMessageBox::warning((QWidget *)parent(), "qRezix",
			tr("Cannot create peer to peer socket!\n\nFile transfer is disabled") );

	wellInit = true;
	
	endLoading();
}

///Fermeture du module
RzxChatLister::~RzxChatLister()
{
	beginClosing();
	closeChats();
	client->deleteLater();
	fileServer->deleteLater();
	RzxComputer::localhost()->addCapabilities(Rzx::CAP_CHAT, false);
	delete RzxChatConfig::global();
	endClosing();
	RZX_GLOBAL_CLOSE
}

///Retourne le serveur TCP
RzxClientListener *RzxChatLister::listener() const
{
	return client;
}

RzxFileListener *RzxChatLister::fileListener() const
{
	return fileServer;
}


/** Sert aussi au raffraichissement des donn�es*/
void RzxChatLister::login(RzxComputer *computer)
{
	RzxChat *chat = getChatByIP(computer->ip());
	if(chat)
	{
		//Indication au chat de la reconnexion
		const RzxComputer *old = chat->computer();
		chat->setComputer(computer);
		if (computer != old && computer->state() != Rzx::STATE_DISCONNECTED)
			chat->info( tr("Reconnected") );
		if(chatByLogin[computer->name()] != chat)
		{
			chatByLogin.remove(chatByLogin.key(chat));
			chatByLogin.insert(computer->name(), chat);
		}
	}
}

///Enregistre la d�connexion d'un client
void RzxChatLister::logout(RzxComputer *computer)
{
	RzxChat *chat = getChatByIP(computer->ip());
	if ( chat )
		chat->info( tr("Disconnected") );
}

///Lancement du chat
void RzxChatLister::chat(RzxComputer *computer)
{
	createChat(computer);
}

///Cr�ation d'une fen�tre de chat associ�e � l'ordinateur
RzxChat *RzxChatLister::createChat(RzxComputer *computer)
{
	if(!computer)
		return NULL;

	RzxHostAddress peer = computer->ip();
	RzxChat *chat = getChatByIP(peer);
	if(!chat)
	{
		chat = new RzxChat(computer);

		connect( chat, SIGNAL( destroyed(QObject*) ), this, SLOT( deleteChat(QObject*) ) );
		connect( chat, SIGNAL(wantDeactivateResponder()), this, SIGNAL(wantDeactivateResponder()));
		connect( RzxIconCollection::global(), SIGNAL(themeChanged(const QString& )), chat, SLOT( changeTheme() ) );
		connect( RzxConfig::global(), SIGNAL( iconFormatChange() ), chat, SLOT( changeIconFormat() ) );
		chatByIP.insert(peer, chat);
		chatByLogin.insert(computer->name(), chat);
		chat->show();
	}
	return chat;
}

///Fermeture du chat (si il existe) associ� au login
void RzxChatLister::closeChat( const QString& login )
{
	RzxChat *chat = getChatByName(login);
	if(chat)
		chat->close();
}

/** No descriptions */
void RzxChatLister::deleteChat(QObject *object)
{
	if(!object) return;

	RzxChat *chat = qobject_cast<RzxChat*>(object);
	RzxComputer *c = NULL;
	if(!chat)
		c = qobject_cast<RzxComputer*>(object);
	else 
		c = chat->computer();

	if(!c) return;
	chatByIP.remove(c->ip());
	chatByLogin.remove(c->name());
}

///R�ception d'un chat...
void RzxChatLister::receiveChatMessage(RzxComputer *computer, Rzx::ChatMessageType type, const QString& msg)
{
	if(!computer) return;

	RzxChat *chat = getChatByIP(computer->ip());
	if(!chat && (type == Rzx::Chat || type == Rzx::File))
		chat = createChat(computer);
	if(!chat) return;
	chat->receiveChatMessage(type, msg);
}

///Reception d'un fichier, permet de lier la fenetre de chat au socket
RzxChat * RzxChatLister::receiveFile(RzxComputer*computer)
{
	if(!computer) return 0;

	RzxChat *chat = getChatByIP(computer->ip());
	if(!chat)
		chat = createChat(computer);
	return chat;
}

///Envoie d'un chat
void RzxChatLister::sendChatMessage(RzxComputer *computer, Rzx::ChatMessageType type, const QString& msg)
{
	client->sendChatMessage(computer, type, msg);
}

///Fermeture des chats en cours
/** Cette m�thode � pour but de fermer tous les chats en cours pour lib�rer en particulier le port 5050. Normalement l'appel de cette m�thode � la fermeture de qRezix doit corriger le probl�me de r�ouverture de l'�coute qui intervient � certains moments lors du d�marrage de qRezix */
void RzxChatLister::closeChats()
{
	foreach(RzxChat *chat, chatByIP.values())
	{
		if(chat)
			chat->close();
	}
}

///Demande l'affichage de l'historique
void RzxChatLister::history(RzxComputer *c)
{
	historique(c);
}

///Demande le check des proi�t�s de \a peer
void RzxChatLister::properties(RzxComputer *computer)
{
	client->checkProperty(computer->ip());
}

///Indique que les propri�t�s ont �t� check�es par \a peer
void RzxChatLister::warnProperties(RzxComputer *computer)
{
	if(!computer || !RzxChatConfig::warnWhenChecked()) return;

	RzxChat *chat = getChatByIP(computer->ip());

	if(chat)
	{
		chat->notify( tr( "has checked your properties" ), true );
		return;
	}

	// On affiche pas simultan�ment deux fen�tres de notification
	// Pour la m�me machine... s�curit� anti-flood
	if(warnWindow[computer->ip()])
		return;

	const QTime cur = QTime::currentTime();
	const QString heure = cur.toString("hh:mm:ss");

	QWidget *warn = RzxMessageBox::information(NULL, tr("Properties sent to %1").arg(computer->name()),
			tr("name : <i>%1</i><br>"
				"address : <i>%2</i><br>"
				"client : <i>%3</i><br>"
				"time : <i>%4</i>")
				.arg(computer->name()).arg(computer->ip().toString()).arg(computer->client()).arg(heure));
	warnWindow.insert(computer->ip(), warn);
}

///Demande l'affichage des propri�t�s
void RzxChatLister::showProperties(RzxComputer *c)
{
	if(!c) return;

	RzxChat *chat = getChatByIP(c->ip());
	if(chat && chat->isActiveWindow())
	{
		chat->receiveProperties();
	}
	else
		showPropertiesWindow(c);
}

///Affichage des proprietes d'un ordinateur
QWidget *RzxChatLister::showPropertiesWindow(RzxComputer *computer, bool withFrame, QWidget *parent, QAbstractButton *button)
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
	
	// Cr�ation des en-t�tes de colonnes
	QTreeWidgetItem *item = new QTreeWidgetItem();
	item->setText(0, tr("Property"));
	item->setText(1, tr("Value"));
	propList->setHeaderItem(item);
	propList->setRootIsDecorated(false);

	// Remplissage
	RzxConfig::fillWithCache(computer->ip(), propList);
	const int propCount = propList->topLevelItemCount();

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
	propertiesDialog->adjustSize();
	return propertiesDialog;
}

///Affichage de la fen�tre de favoris
QWidget *RzxChatLister::historique(RzxComputer *computer, bool withFrame, QWidget *parent, QAbstractButton *button)
{
	if (!computer)
		return NULL;

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

	// Incruste des smileys pour l'affichage
	RzxSmileys::replace(text);
 
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
	RzxChatBrowser* histoView = new RzxChatBrowser(histoDialog);
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
		ui = new Ui::RzxChatProp;
	if(!propWidget)
	{
		propWidget = new QWidget;
		ui->setupUi(propWidget);
		connect(ui->btnBeepBrowse, SIGNAL( clicked() ), this, SLOT( chooseBeep() ) );
		connect(ui->smileyCombo, SIGNAL(activated(const QString&)), this, SLOT(previewSmileys(const QString&)));
		ui->lstSmileys->installEventFilter(this);
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
	if(!ui) return;

	ui->chkBeep->setChecked(RzxChatConfig::beep(def));
	ui->beepSound->setText(RzxChatConfig::beepSound(def));
	ui->cbPropertiesWarning->setChecked(RzxChatConfig::warnWhenChecked(def));
	ui->cbPrintTime->setChecked(RzxChatConfig::printTime(def));
	ui->cbPrintName->setChecked(RzxChatConfig::printName(def));
	ui->cbPrintIcon->setChecked(RzxChatConfig::printIcon(def));
	ui->cbPrintPrompt->setChecked(RzxChatConfig::printPrompt(def));
	ui->iconSize->setValue(RzxChatConfig::iconSize());
	ui->prompt->setText(RzxChatConfig::prompt());
	ui->chat_port->setValue(RzxChatConfig::chatPort(def));
	ui->file_port->setValue(RzxChatConfig::filePort(def));
	ui->cbRefuseFile->setChecked(RzxChatConfig::refuseFile(def));
	ui->cbDisplayIconText->setChecked(RzxChatConfig::displayIconText(def));

	ui->smileyCombo->clear();
	QStringList themes = RzxSmileys::themeList();
	for(int i = 0 ; i < themes.size() ; i++)
		ui->smileyCombo->addItem(RzxSmileys::pixmap(":-)", themes[i]), themes[i]);
	ui->smileyCombo->setCurrentIndex(ui->smileyCombo->findText(RzxSmileys::theme()));
	previewSmileys(RzxSmileys::theme());
	changeTheme();
}

///Affiche l'aper�u du th�me de smileys s�lectionn�
void RzxChatLister::previewSmileys(const QString& theme)
{
	ui->lstSmileys->clear();
	QList<QPixmap> preview = RzxSmileys::preview(theme);
	foreach(const QPixmap &sml, preview)
	{
		QListWidgetItem *item = new QListWidgetItem(ui->lstSmileys);
		item->setIcon(sml);
	}
}

/** \reimp */
void RzxChatLister::propUpdate()
{
	if(!ui) return;

	RzxChatConfig::setBeep(ui->chkBeep->isChecked());
	RzxChatConfig::setBeepSound(ui->beepSound->text());
	RzxChatConfig::setWarnWhenChecked(ui->cbPropertiesWarning->isChecked());
	RzxChatConfig::setPrintTime(ui->cbPrintTime->isChecked());
	RzxChatConfig::setPrintName(ui->cbPrintName->isChecked());
	RzxChatConfig::setPrintIcon(ui->cbPrintIcon->isChecked());
	RzxChatConfig::setPrintPrompt(ui->cbPrintPrompt->isChecked());
	RzxChatConfig::setIconSize(ui->iconSize->value());
	RzxChatConfig::setPrompt(ui->prompt->text());
	RzxChatConfig::setChatPort(ui->chat_port->value());
	RzxChatConfig::setFilePort(ui->file_port->value());
	RzxChatConfig::setRefuseFile(ui->cbRefuseFile->isChecked());
	RzxChatConfig::setDisplayIconText(ui->cbDisplayIconText->isChecked());
	RzxSmileys::setTheme(ui->smileyCombo->currentText());
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

///Change le th�me d'ic�nes
void RzxChatLister::changeTheme()
{
	if(!ui) return;

	ui->btnBeepBrowse->setIcon(RzxIconCollection::getIcon(Rzx::ICON_FILE));
	ui->chkBeep->setIcon(RzxIconCollection::getIcon(Rzx::ICON_SOUNDON));
}

///Pour que la resize de la fen�tre rafraichisse l'aper�u des smileys
bool RzxChatLister::eventFilter(QObject *obj, QEvent *e)
{
	if(obj == ui->lstSmileys && e->type() == QEvent::Resize)
		previewSmileys(ui->smileyCombo->currentText());
	return false;
}

///Mise � jour de la traduction
void RzxChatLister::translate()
{
	if(!ui) return;

	ui->retranslateUi(propWidget);

	int id = ui->smileyCombo->currentIndex();
	ui->smileyCombo->clear();
	QStringList themes = RzxSmileys::themeList();
	for(int i = 0 ; i < themes.size() ; i++)
		ui->smileyCombo->addItem(RzxSmileys::pixmap(":-)", themes[i]), themes[i]);
	ui->smileyCombo->setCurrentIndex(id);
}
