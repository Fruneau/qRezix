/***************************************************************************
                        rzxchat.cpp  -  description
                             -------------------
    begin                : Sat Jan 26 2002
    copyright            : (C) 2002 by Sylvain Joyeux
    email                : sylvain.joyeux@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *	 This program is free software; you can redistribute it and/or modify  *
 *	 it under the terms of the GNU General Public License as published by  *
 *	 the Free Software Foundation; either version 2 of the License, or     *
 *	 (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QTimer>
#include <QPoint>
#include <QList>

//Gestion des intéractions avec la fenêtre
#include <QShortcut>
#include <QShowEvent>
#include <QCloseEvent>
#include <QMoveEvent>

//Pour la construction de la fenêtre
#include <QIcon>
#include <QSplitter>
#include <QPushButton>
#include <QToolButton>
#include <QCheckBox>
#include <QComboBox>

//Pour l'édition et le parcours de texte
#include <QDateTime>
#include <QRegExp>
#include <QTextCursor>
//Pour les couleurs du texte
#include <QColor>
#include <QColorDialog>
#include <QPixmap>

//Pour le fichier d'historique
#include <QFile>
#include <QTextStream>


#include <RzxConfig>
#include <RzxStyle>
#include <RzxIconCollection>
#include <RzxSound>

#include "rzxchat.h"

#include "rzxtextedit.h"
#include "rzxchatlister.h"
#include "rzxchatconfig.h"
#include "rzxsmileys.h"
#include "rzxchatbrowser.h"

#ifdef Q_OS_MAC
#	include "ui_rzxchat_mac.h"
#else
#	include "ui_rzxchat.h"
#endif

/******************************
* RzxChat
******************************/
const QColor RzxChat::preDefinedColors[16] = {Qt::black, Qt::red, Qt::darkRed,
		Qt::green, Qt::darkGreen, Qt::blue, Qt::darkBlue, Qt::cyan, Qt::darkCyan,
		Qt::magenta, Qt::darkMagenta, Qt::yellow, Qt::darkYellow, Qt::gray,
		Qt::darkGray, Qt::lightGray};

///Constructin d'une fenêtre de chat associée à la machine indiquée
RzxChat::RzxChat(RzxComputer *c)
	:QWidget(NULL, Qt::WindowContextHelpButtonHint | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint), lastIP(0)
{
	init();
	setComputer(c);
}

///Initialisation de la fenêtre de chat
/** L'initialisation de la fenêtre à proprement dit ne gère pas la partie m_socket */
void RzxChat::init()
{
	setAttribute(Qt::WA_DeleteOnClose);
	setAttribute(Qt::WA_QuitOnClose,false);
	RzxStyle::useStyleOnWindow(this);

	/**** Construction de l'UI ****/
	/* Splitter avec 2 partie dont une est l'ui */
	//Partie 1 : L'afficheur de discussion
	txtHistory = new RzxChatBrowser();
	txtHistory->setReadOnly(true);
	txtHistory->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	QWidget *historyContainer = new QWidget();
	QGridLayout *glayout = new QGridLayout(historyContainer);
	glayout->addWidget(txtHistory);
	historyContainer->setLayout(glayout);
	//Partie 2 : L'éditeur qui est défini dans l'ui RzxChatUI
	editor = new QWidget();
	ui = new Ui::RzxChat();
	ui->setupUi(editor);
	ui->edMsg->setChat(this);
	ui->cbSendHTML->setChecked(false);
	//Construction du splitter
	splitter = new QSplitter(Qt::Vertical);
	splitter->addWidget(historyContainer);
	splitter->addWidget(editor);
	splitter->setSizes(QList<int>() << 120 <<	100);
	setBaseSize(width(), 230);
	
	//Pour que le splitter soit bien redimensionné avec la fenêtre
	glayout = new QGridLayout(this);
	setLayout(glayout);
	glayout->setMargin(0);
	glayout->setSpacing(0);
	glayout->addWidget(splitter);
	ui->edMsg->setFocus();

	//Restoration des dimensions
	RzxChatConfig::restoreChatWidget(this);

	/* Définition des raccourcis claviers */
	new QShortcut(Qt::CTRL + Qt::Key_Return, ui->btnSend, SIGNAL(clicked()));
	new QShortcut(Qt::CTRL + Qt::Key_Enter, ui->btnSend, SIGNAL(clicked()));
	new QShortcut(Qt::Key_Escape, ui->btnClose, SIGNAL(clicked()));
	new QShortcut(Qt::Key_F1, ui->btnSound, SLOT(toggle()));
#ifdef Q_OS_MAC
	new QShortcut(QKeySequence("Ctrl+M"), this, SLOT(showMinimized()));
	new QShortcut(QKeySequence("Ctrl+,"), this, SIGNAL(wantPreferences()));
	connect(this, SIGNAL(wantPreferences()), RzxChatLister::global(), SIGNAL(wantPreferences()));
#endif

	/* Construction du texte et des icônes des boutons */
	changeTheme();
	changeSmileyTheme();
	changeIconFormat();

	/**** Préparation des données ****/
	curColor = Qt::black;

	//chargement des fontes
	ui->cbFontSelect->insertItems(0, RzxChatConfig::getFontList());
	ui->cbFontSelect->setCurrentIndex(ui->cbFontSelect->findText(ui->edMsg->font()));
	
	//chargement de la liste des couleurs
	ui->cbColorSelect->addItem(tr("Custom...")); //tjs 0
	for(int i=0; i<16; ++i)
		addColor(preDefinedColors[i]);
	ui->cbColorSelect->setCurrentIndex(1); //black par défaut

	//Défini l'activation par défaut du bouton du son
	ui->btnSound->setChecked(RzxChatConfig::beep());

	//gestion du formatiage du texte
	connect(ui->btnBold, SIGNAL(toggled(bool)), ui->edMsg, SLOT(setBold(bool)));
	connect(ui->btnItalic, SIGNAL(toggled(bool)), ui->edMsg, SLOT(setItalic(bool)));
	connect(ui->btnUnderline, SIGNAL(toggled(bool)), ui->edMsg, SLOT(setUnderline(bool)));
	connect(ui->cbColorSelect, SIGNAL(activated(int)), this, SLOT(setColor(int)));
	connect(ui->cbFontSelect, SIGNAL(activated(int)), this, SLOT(setFont(int)));
	connect(ui->cbSize, SIGNAL(activated(int)), this, SLOT(setSize(int)));
	connect(ui->cbSendHTML, SIGNAL(toggled(bool)), this, SLOT(setHtml(bool)));

	/** Connexions **/

	connect(ui->edMsg, SIGNAL(enterPressed()), this, SLOT(onReturnPressed()));
	connect(ui->edMsg, SIGNAL(textWritten()), this, SLOT(onTextChanged()));
	connect(ui->btnHistorique, SIGNAL(toggled(bool)), this, SLOT(on_btnHistorique_toggled(bool)));
	connect(ui->btnProperties, SIGNAL(toggled(bool)), this, SLOT(on_btnProperties_toggled(bool)));
	connect(ui->btnSmiley, SIGNAL(toggled(bool)), this, SLOT(on_btnSmiley_toggled(bool)));
	connect(ui->btnSend, SIGNAL(clicked()), this, SLOT(on_btnSend_clicked()));
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	
	RzxIconCollection::connect(this, SLOT(changeTheme()));
	RzxSmileys::connect(this, SLOT(changeSmileyTheme()));

	setFont(0);
	setHtml(false);
	typing = peerTyping = false;
	unread = 0;
}

///Bye bye
RzxChat::~RzxChat()
{
	if(lastIP)
	{
		QString temp = textHistorique;

		QString filename = RzxChatConfig::historique(rezixIP(), name());
		if(filename.isNull()) return;
	
		QFile file(filename);
		file.open(QIODevice::ReadWrite |QIODevice::Append);
		QTextStream stream(&file);
		stream.setCodec("UTF-8");
		stream << temp;
		file.close();
	}
	
	delete ui;	
}

///Défini la machine associée à la fenêtre de chat
void RzxChat::setComputer(RzxComputer* c)
{
	m_computer = c;
	if(c)
	{
		lastIP = c->ip().toRezix();
		lastName = c->name();
		ui->btnProperties->setEnabled(c->canBeChecked());
		if(c->canBeChatted() && chatBuffer.size())
		{
			notify(tr("Sending buffered messages"));
			for(int i = 0 ; i < chatBuffer.size() ; i++)
				sendChat(chatBuffer[i]);
			chatBuffer.clear();
		}
	}
	updateTitle();
}

///Réception d'un message pong
void RzxChat::pong(int ms)
{
	notify(QString(tr("Pong received within %1 msecs")).arg(ms));
}

///Changement de l'état de l'autre utilisateur
void RzxChat::peerTypingStateChanged(bool state)
{
	peerTyping=state;
	updateTitle();
}

///Changement du titre de la fenêtre
/** Le titre est de la forme :
 * Chat - remoteHostName( - Is typing a message)?( - \d+ unread)
 */ 
void RzxChat::updateTitle()
{
	QString title = tr("Chat") + " - " + name();
	
	if(peerTyping && isActiveWindow()) title += " - " + tr("Is typing a message");
	if(unread) title += " - " + QString::number(unread) + " " + tr("unread");
	setWindowTitle(title);

	if(m_computer)
		setWindowIcon(m_computer->icon());
}

/*********************** Gestion des actions ********************************/
///Valide le texte
/** utilisé pour tronquer la chaine et enlever le retour chariot quand
l'utilisateur utilise Return ou Enter pour envoyer son texte */
void RzxChat::onReturnPressed()
{
	int length = ui->edMsg->toPlainText().length();
	if(!length)
	{  //vide + /n
		ui->edMsg->validate(false);
		return;
	}
	//edMsg->setText(edMsg->text().left(length-1));
	on_btnSend_clicked();
}

///On ajoute une nouvelle couleur dans la liste des couleurs prédéfinies
void RzxChat::addColor(QColor color)
{
	QPixmap p = QPixmap(50, 15);
	p.fill(color);
	ui->cbColorSelect->addItem(p, QString());
}

///Changement de la couleur du texte
void RzxChat::setColor(int index)
{
	QColor c;
	if(!index)
		c = QColorDialog::getColor(curColor, this);
	else
	{
		if(index <=16)
			c = preDefinedColors[index-1];
		else
			c = QColorDialog::customColor(index - 17);
	}
	curColor = c.isValid() ? c : curColor;
	ui->edMsg->setColor(curColor);
}

///Changement de la police de caractère
void RzxChat::setFont(int index)
{
	QString family = RzxChatConfig::nearestFont(ui->cbFontSelect->itemText(index));
	ui->btnBold->setEnabled(RzxChatConfig::isBoldSupported(family));
	ui->btnItalic->setEnabled(RzxChatConfig::isItalicSupported(family));
	QList<int> pSize = RzxChatConfig::getSizes(family);

	ui->cbSize->clear();
	foreach(int point, pSize)
	{
		QString newItem = QString::number(point);
		ui->cbSize->addItem(newItem);
	}
	ui->cbSize->setCurrentIndex(ui->cbSize->findText(QString::number(RzxChatConfig::nearestSize(family, ui->edMsg->size()))));
	ui->edMsg->setFont(family);
}

///Changement de la taille du texte
void RzxChat::setSize(int index)
{
	QString size = ui->cbSize->itemText(index);
	bool ok;
	int point = size.toInt(&ok, 10);
	if(!ok)
		return;
	ui->edMsg->setSize(point);
}

///Activation/Désactivation du formatage HTML du texte
void RzxChat::setHtml(bool on)
{
	const QString &font = ui->edMsg->font();
	ui->cbFontSelect->setEnabled(on);
	ui->cbColorSelect->setEnabled(on);
	ui->cbSize->setEnabled(on);
	ui->btnBold->setEnabled(on && RzxChatConfig::isBoldSupported(font));
	ui->btnUnderline->setEnabled(on);
	ui->btnItalic->setEnabled(on && RzxChatConfig::isItalicSupported(font));
	ui->edMsg->useHtml(on);
	//remet les paramètres par défaut aux boutons parce que c'est comme ca qu'il agit
	if (on)
	{
		const int font = ui->cbFontSelect->findText(ui->edMsg->defaultFormat.family);
		int size = ui->cbSize->findText(QString::number(ui->edMsg->defaultFormat.size));
		if(size == -1) size = 0;
		ui->cbFontSelect->setCurrentIndex(font);
		ui->cbSize->setCurrentIndex(size);
		if(font != -1)
			ui->edMsg->setFont(ui->cbFontSelect->currentText());
		setSize(size);
		ui->edMsg->setSize(ui->cbSize->currentText().toInt());
		ui->cbColorSelect->setCurrentIndex(1);

		ui->btnBold->setChecked(false);
		ui->btnItalic->setChecked(false);
		ui->btnUnderline->setChecked(false);
	}
}


/******************** Editiion du texte *******************/
///En cas d'édition du texte
void RzxChat::onTextChanged()
{
	if(!typing && ui->edMsg->toPlainText().length())
	{
		typing = true;
		if(computer())
			computer()->sendChat(Rzx::Typing);
		//On ne crée pas de m_socket pour envoyer typing
		typingTimer.setSingleShot(true);
		typingTimer.start(10*1000);
	}
	if(typing && !ui->edMsg->toPlainText().length())
	{
		typing = false;
		if(computer())
			computer()->sendChat(Rzx::StopTyping);
		typingTimer.stop();
	}
}

///Ajoute les en-têtes qui vont bien pour l'affichage
void RzxChat::append(const QString& color, const QString& host, const QString& argMsg, RzxComputer *computer)
{
	const QDateTime dater = QDateTime::currentDateTime();
	QString time = dater.toString("hh:mm:ss") + "&nbsp;";
	QString date = dater.toString("ddd d MMMM yy") + " " + time;
	QString name, icon, histText;

	//Nettoyage du html du message
	QString msg(argMsg);
	if(msg.indexOf("<html>"))
	{
		msg.replace(QRegExp("&(?!\\w+;)"), "&amp;");
		msg.replace("<", "&lt;");
	}
	msg.remove(QRegExp("<head>.*</head>")).remove("<html>")
		.remove(QRegExp("<body[^<>]*>")).remove("</html>").remove("</body>");

	//Passage des <font > en <span >
	QRegExp fontMask("^(.*)<font([^>]*)>((?:[^<]|<(?!/font>))*)</font>(.*)$");
	while(fontMask.indexIn(msg) != -1)
	{
		RzxTextEdit::Format format = RzxTextEdit::formatFromFont(fontMask.cap(2));
		msg = fontMask.cap(1) + RzxTextEdit::formatStyle(fontMask.cap(3), format, "span") + fontMask.cap(4);
	}

	//Création des liens hypertextes
	static const QRegExp hyperText("("
		"(\\b[a-zA-Z0-9]+://([-a-zA-Z0-9]+(\\:[-a-zA-Z0-9]+)?\\@)?[-a-zA-Z0-9]+"
			"|[-a-zA-Z0-9]+\\.[-a-zA-Z0-9]+)(\\.[-a-zA-Z0-9]+)*" // (protocole://)?hostname
		"(/~?[-%_a-zA-Z0-9.]*)*"	//directory*
		"(\\?[-%_a-zA-Z0-9=+.]+(&[-%_a-zA-Z0-9=+.]+)*)?" //paramètres?
		"(#[-_a-zA-Z0-9]+)?" //position
		"|(\\\\\\\\[-a-zA-Z0-9]+(\\.[-a-zA-Z0-9]+)*(\\\\[-%_a-zA-Z0-9.]*)*)" // samba
		"|\\b(mailto:)?[-_a-zA-Z0-9]+(\\.[-_a-zA-Z0-9]+)*(\\+[-_a-zA-Z0-9.]+)?" // login
		"\\@[-a-zA-Z0-9]+(\\.[-a-zA-Z0-9]+)*\\b" //mail hostname
		")");
	msg.replace(hyperText, "<a href=\"\\1\">\\1</a>");

#define addColor(text, tmpcolor) QString("<font color=\"") + tmpcolor + "\">" + text + "</font>"
	
	//Distinction du /me et d'un message normal
	static QRegExp action("^(\\s*<[^<>]+>)*/me(<[^<>]+>|\\s)(.*)");
	if(computer && !action.indexIn(msg))
	{
		const QString purple("purple");
		const QString entete = action.cap(1);
		const QString entext = action.cap(2);
		const QString pieddp = action.cap(3);
		msg = addColor("<i>* " + entete + computer->name() + entext + pieddp + "</i><br>", purple);
		date = addColor(date, purple);
		time = addColor(time, purple);
		histText = msg;
		computer = NULL;
	}
	else
	{
		date = addColor(date, color);
		time = addColor(time, color);
		//Si computer != NULL, l'en-tête définitive sera composée ensuite
		if(computer)
		{
			msg = addColor("&nbsp; " + msg + "<br>", color);
			if(!computer->isLocalhost()) name = addColor(computer->name(), color);
			histText = addColor("<i>" + name + host + "</i>", color) + msg;
		}
		else if(!host.isEmpty())
			histText = msg = addColor("<i>" + host + "</i>&nbsp;" + msg + "<br>", color);
		else
			histText = msg = addColor(msg + "<br>", color);
	}

	// Enregistrement des logs...
	textHistorique = textHistorique + date + histText;

	// Gestion des smileys
	RzxSmileys::replace(msg);

	//Composition de l'entête du message en fonction des options
	if(computer)
	{
		//Recherche de l'icône à utiliser
		if(RzxChatConfig::printIcon())
		{
			icon = RzxIconCollection::global()->hashedIconFileName(computer->stamp());
			if(icon.isEmpty())
				icon = RzxIconCollection::global()->pixmapFileName(Rzx::toIcon(computer->sysEx()));
			if(!icon.isEmpty())
			{
				const QString size = QString::number(RzxChatConfig::iconSize());
				icon = "<img src=\"" + icon + "\" alt=\"" + computer->name() + "\" width=" + size + " height=" + size + ">";
			}
		}

		//Composition de l'entête
		QString prepend;
		if(RzxChatConfig::printName() || icon.isEmpty())
			prepend = name;
		if(RzxChatConfig::printIcon() && !icon.isEmpty())
			prepend = icon + prepend;
		if(RzxChatConfig::printPrompt() && !host.isEmpty())
			prepend += host;
		msg = addColor("<i>" + prepend + "</i>", color) + msg;
	}

#undef addColor

	//Ajout de l'heure du message si demandé
	if(RzxChatConfig::printTime())
		txtHistory->append(time + msg);
	else
		txtHistory->append(msg);

	QTextCursor cursor = txtHistory->textCursor();
	cursor.movePosition(QTextCursor::End);
	txtHistory->setTextCursor(cursor);
	txtHistory->ensureCursorVisible();
	ui->edMsg->setFocus();
}

/** Affiche un message reçu, et emet un son s'il faut */
void RzxChat::receive(const QString& msg)
{
	if(ui->btnSound->isChecked())
		RzxSound::play(RzxChatConfig::beepSound());	
	
	if(RzxConfig::autoResponder())
		append("darkgray", RzxChatConfig::prompt(), msg, m_computer);
	else
		append("blue", RzxChatConfig::prompt(), msg, m_computer);
	if(!isActiveWindow())
	{
		unread++;
		updateTitle();
	}
}


/// Affiche une info de status (deconnexion, reconnexion)
void RzxChat::info(const QString& msg)
{
	append("darkgreen", "", msg);
}

/// Affiche un message de notification (envoie de prop, ping, pong...)
void RzxChat::notify(const QString& msg, bool withHostname)
{
	if(!RzxChatConfig::warnWhenChecked())
		return;

	QString header = "***";
	if(withHostname) header += "&nbsp;" + name();
	append("gray", header, msg);
}

///Reception d'un message ou autre
void RzxChat::receiveChatMessage(Rzx::ChatMessageType type, const QString& msg)
{
	switch(type)
	{
		case Rzx::Responder: case Rzx::Chat: receive(msg); break;
		case Rzx::Ping: notify(tr("Ping request received")); break;
		case Rzx::Pong: notify(tr("Pong answer received in ") + msg + "ms"); break;
		case Rzx::Typing:  peerTypingStateChanged(true); break;
		case Rzx::StopTyping:  peerTypingStateChanged(false); break;
		case Rzx::InfoMessage: info(msg); break;
		case Rzx::Closed: info(tr("Chat closed")); break;
	}
}

///Envoie le contenu de la fenêtre d'édition
void RzxChat::on_btnSend_clicked()
{
	//Pour que les plug-ins qui en on besoin modifie le texte de chat
	bool format = ui->cbSendHTML->isChecked();

	typingTimer.stop();
	typing = false;

	//traitement du message
	QString htmlMsg = ui->edMsg->toSimpleHtml();
	QString rawMsg = ui->edMsg->toPlainText();
	if(rawMsg.isEmpty()) return;

	//Gestion de l'envoi de ping
	if(rawMsg == "/ping" || rawMsg.left(6) == "/ping ")
	{
		if(computer())
			computer()->sendChat(Rzx::Ping);
		ui->edMsg->validate(false);
		notify(tr("Ping emitted"));
		return;
	}

	// Conversion du texte en HTML si necessaire
	if(!format)
	{
		static const QString htmlflag("/<html>");
		rawMsg.replace(htmlflag, "<html>");
	}
	
	//Après l'analyse du texte, on sélectionne ce qui va bien
	QString msg;
	if(format)
	{
		msg = htmlMsg;
		if(!msg.contains("<html>"))
			msg = "<html>" + msg + "</html>";
	}
	else
		msg = rawMsg;

	append("red", RzxChatConfig::prompt(), msg, RzxComputer::localhost());
	sendChat(msg);	//passage par la sous-couche de gestion du m_socket avant d'émettre

	ui->edMsg->validate();
	if (format)
	{
		initHtmlText();
	}
}

void RzxChat::initHtmlText()
{
	ui->edMsg->setFont(ui->edMsg->currentFormat.family);
	ui->edMsg->setSize(ui->edMsg->currentFormat.size);
	ui->edMsg->setColor(ui->edMsg->currentFormat.color);
	ui->edMsg->setBold(ui->edMsg->currentFormat.bold);
	ui->edMsg->setItalic(ui->edMsg->currentFormat.italic);
	ui->edMsg->setUnderline(ui->edMsg->currentFormat.underlined);
}


/********* Gestion des propriétés et de l'historique *********/
///L'utilisateur demande l'historique.
void RzxChat::on_btnHistorique_toggled(bool on)
{
	if(!on)
	{
		if(!hist.isNull())
			hist->close();
		return;
	}

	if(hist || !lastIP) return;
	ui->btnProperties->setChecked(false);
	
	//Enregistre l'historique actuel
	QString temp = textHistorique;

	QString filename = RzxChatConfig::historique(rezixIP(), name());
	if (filename.isNull()) return;
	
	QFile file(filename);
	file.open(QIODevice::ReadWrite |QIODevice::Append);
	file.write(temp.toUtf8());
	file.close();

	//Vide le buffer...
	textHistorique = "";

	//Affiche la fenêtre
	hist = (RzxChatPopup*)RzxChatLister::global()->historique(RzxHostAddress::fromRezix(lastIP), false, this, ui->btnHistorique);
	if(!hist.isNull())
		hist->show();
	else
		ui->btnHistorique->toggle();
}

///L'utilisateur demande les propriétés, on lance le check.
void RzxChat::on_btnProperties_toggled(bool on)
{
	if(!on)
	{
		if(!prop.isNull())
			prop->close();
		return;
	}
	
	if(prop) return;
	ui->btnHistorique->setChecked(false);
	if(computer())
		computer()->checkProperties();
	else
		receiveProperties();
}

///Demande l'affichage des propriétés
void RzxChat::receiveProperties()
{
	prop = (RzxChatPopup*)RzxChatLister::global()->showPropertiesWindow(computer(), false, this, ui->btnProperties);
	if(prop.isNull())
	{
		ui->btnProperties->setChecked(false);
		return;
	}
	prop->show();
}

///Déplacement des popups avec la fenêtre principale
void RzxChat::moveEvent(QMoveEvent *)
{
	if(!hist.isNull())
		hist->move();
	if(!prop.isNull())
		prop->move();
	if(!smileyUi.isNull())
		smileyUi->move();
}

///Resize de la fenêtre
void RzxChat::resizeEvent(QResizeEvent *)
{
	moveEvent();
}

/// Gestion de la connexion avec l'autre client
/** Cette méthode permet de gérer les deux cas :
 *		- soit la connexion est déjà établie, on utilise le m_socket déjà en place
 *		- soit il n'y a pas connexion, dans ce cas, on ouvre la connexion l'émission du message se fera dès que celle-ci sera prête
 */
void RzxChat::sendChat(const QString& msg)
{
	if(RzxComputer::localhost()->isOnResponder())
		emit wantDeactivateResponder();
	if(computer() && computer()->canBeChatted())
		computer()->sendChat(Rzx::Chat, msg);
	else
	{
		if(!chatBuffer.size())
			notify(tr("Saving messages as long remote host is disconnected and this window stays open"));
		chatBuffer << msg;
	}
}

/******************* Gestion des événements ***************************/

/// Exécution à la fermeture
void RzxChat::closeEvent(QCloseEvent * e)
{
	if(!hist.isNull())
		hist->close();
	if(!prop.isNull())
		prop->close();
	if(!smileyUi.isNull())
		smileyUi->close();
	RzxChatConfig::saveChatWidget(this);
	e -> accept();

	if(computer())
		computer()->sendChat(Rzx::Closed);
	emit closed(computer());
}

///Pour récupérer quelques événements (genre activation de la fenêtre)
bool RzxChat::event(QEvent *e)
{
	if(e->type() == QEvent::WindowActivate)
	{
		if(unread)
		{
			unread = 0;
			updateTitle();
		}
#ifndef WIN32
		if(!hist.isNull()) hist->raise();
		if(!prop.isNull()) prop->raise();
		if(!smileyUi.isNull()) smileyUi->raise();
#endif
	}

	return QWidget::event(e);
}


/********************** Gestion de l'apparence ****************************/
///Changement du thème d'icône
void RzxChat::changeTheme()
{
	ui->btnSound->setIcon(RzxIconCollection::getSoundIcon());
	ui->btnHistorique->setIcon(RzxIconCollection::getIcon(Rzx::ICON_HISTORIQUE));
	ui->btnSend->setIcon(RzxIconCollection::getIcon(Rzx::ICON_SEND));
	ui->btnProperties->setIcon(RzxIconCollection::getIcon(Rzx::ICON_PROPRIETES));
	ui->btnClose->setIcon(RzxIconCollection::getIcon(Rzx::ICON_CANCEL));
}

///Changement du thème de smileys
void RzxChat::changeSmileyTheme()
{
	ui->btnSmiley->setIcon(RzxSmileys::pixmap(":-)"));
	ui->btnSmiley->setEnabled(RzxSmileys::isValid());
}

///Changement du format d'affichage des icônes
/** Contrairement au menu de la fenêtre principale, les icônes sont ici toujours petites et le texte toujours à droite. Les seules possibilités sont de masquer ou d'afficher le texte et les icônes */
void RzxChat::changeIconFormat()
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
	else if(icons && texts) style = Qt::ToolButtonTextBesideIcon;

	QList<QToolButton*> toolButtons = editor->findChildren<QToolButton*>();
	foreach(QToolButton *button, toolButtons)
		button->setToolButtonStyle(style);
	
	if(ui->btnSound->icon().isNull()) changeTheme(); //pour recharcher les icônes s'il y a besoin
}

///Changement de la langue...
void RzxChat::changeEvent(QEvent *e)
{
	if(e->type() == QEvent::LanguageChange)
	{
		QWidget::languageChange();
		ui->retranslateUi(editor);
		changeIconFormat();
	}
}

/// Affichage du menu des smileys lors d'un clic sur le bouton
void RzxChat::on_btnSmiley_toggled(bool on)
{
	if(!on)
	{
		if(!smileyUi.isNull())
			smileyUi->close();
		return;
	}
	smileyUi = new RzxSmileyUi(ui->btnSmiley, this);
	connect(smileyUi, SIGNAL(clickedSmiley(const QString&)), ui->edMsg, SLOT(insertPlainText(const QString&)));
	smileyUi->show();
}
