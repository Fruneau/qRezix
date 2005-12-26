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

//Gestion des int�ractions avec la fen�tre
#include <QShortcut>
#include <QShowEvent>
#include <QCloseEvent>
#include <QMoveEvent>

//Pour la construction de la fen�tre
#include <QIcon>
#include <QSplitter>
#include <QPushButton>
#include <QToolButton>
#include <QCheckBox>
#include <QComboBox>

//Pour l'�dition et le parcours de texte
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

///Constructin d'une fen�tre de chat associ�e � la machine indiqu�e
RzxChat::RzxChat(RzxComputer *c)
	:QWidget(NULL, Qt::WindowContextHelpButtonHint), lastIP(0)
{
	init();
	setComputer(c);
}

///Initialisation de la fen�tre de chat
/** L'initialisation de la fen�tre � proprement dit ne g�re pas la partie m_socket */
void RzxChat::init()
{
	setAttribute(Qt::WA_DeleteOnClose);
	RzxStyle::useStyleOnWindow(this);

	/**** Construction de l'UI ****/
	/* Splitter avec 2 partie dont une est l'ui */
	//Partie 1 : L'afficheur de discussion
	txtHistory = new QTextEdit();
	txtHistory->setReadOnly(true);
	txtHistory->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	QWidget *historyContainer = new QWidget();
	QGridLayout *glayout = new QGridLayout(historyContainer);
	glayout->addWidget(txtHistory);
	historyContainer->setLayout(glayout);
	//Partie 2 : L'�diteur qui est d�fini dans l'ui RzxChatUI
	editor = new QWidget();
	ui = new Ui::RzxChat();
	ui->setupUi(editor);
	ui->edMsg->setChat(this);
	//Construction du splitter
	splitter = new QSplitter(Qt::Vertical);
	splitter->addWidget(historyContainer);
	splitter->addWidget(editor);
	splitter->setSizes(QList<int>() << 120 <<	100);
	setBaseSize(width(), 230);
	
	//Pour que le splitter soit bien redimensionn� avec la fen�tre
	glayout = new QGridLayout(this);
	setLayout(glayout);
	glayout->setMargin(0);
	glayout->setSpacing(0);
	glayout->addWidget(splitter);
	ui->edMsg->setFocus();

	//Restoration des dimensions
	RzxChatConfig::restoreChatWidget(this);

	/* D�finition des raccourcis claviers */
	new QShortcut(Qt::CTRL + Qt::Key_Return, ui->btnSend, SIGNAL(clicked()));
	new QShortcut(Qt::CTRL + Qt::Key_Enter, ui->btnSend, SIGNAL(clicked()));
	new QShortcut(Qt::Key_Escape, ui->btnClose, SIGNAL(clicked()));
	new QShortcut(Qt::Key_F1, ui->btnSound, SLOT(toggle()));

	/* Construction du texte et des ic�nes des boutons */
	changeTheme();
	changeSmileyTheme();
	changeIconFormat();

	/**** Pr�paration des donn�es ****/
	curColor = Qt::black;

	//chargement des fontes
	ui->cbFontSelect->insertItems(0, RzxChatConfig::getFontList());
	ui->cbFontSelect->setCurrentIndex(ui->cbFontSelect->findText(ui->edMsg->font()));
	
	//chargement de la liste des couleurs
	ui->cbColorSelect->addItem(tr("Custom colours...")); //tjs 0
	for(int i=0; i<16; ++i)
		addColor(preDefinedColors[i]);
	ui->cbColorSelect->setCurrentIndex(1); //black par d�faut

	//gestion du formatiage du texte
	connect(ui->btnBold, SIGNAL(toggled(bool)), ui->edMsg, SLOT(setBold(bool)));
	connect(ui->btnItalic, SIGNAL(toggled(bool)), ui->edMsg, SLOT(setItalic(bool)));
	connect(ui->btnUnderline, SIGNAL(toggled(bool)), ui->edMsg, SLOT(setUnderline(bool)));
	connect(ui->cbColorSelect, SIGNAL(activated(int)), this, SLOT(setColor(int)));
	connect(ui->cbFontSelect, SIGNAL(activated(int)), this, SLOT(setFont(int)));
	connect(ui->cbSize, SIGNAL(activated(int)), this, SLOT(setSize(int)));
	connect(ui->cbSendHTML, SIGNAL(toggled(bool)), this, SLOT(setHtml(bool)));

	/** Connexions **/
#ifdef WIN32
	timer = new QTimer( this );
	connect( timer, SIGNAL(timeout()),this, SLOT(messageReceived()) );
#endif
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
	
#ifdef WIN32
	if(timer) delete timer;
#endif
	delete ui;	
}

///D�fini la machine associ�e � la fen�tre de chat
void RzxChat::setComputer(RzxComputer* c)
{
	m_computer = c;
	if(c)
	{
		lastIP = c->ip().toRezix();
		lastName = c->name();
	}
	updateTitle();
}

///R�ception d'un message pong
void RzxChat::pong(int ms)
{
	notify(QString(tr("Pong received within %1 msecs")).arg(ms));
}


///Changement de l'�tat de l'autre utilisateur
void RzxChat::peerTypingStateChanged(bool state)
{
	peerTyping=state;
	updateTitle();
}

///Changement du titre de la fen�tre
/** Le titre est de la forme :
 * Chat - remoteHostName( - Is typing a message)?( - \d+ unread)
 */ 
void RzxChat::updateTitle()
{
	QString title = tr("Chat") + " - " + name();
	
	if(peerTyping && isActiveWindow()) title += " - " + tr("Is typing a message");
	if(unread) title += " - " + QString::number(unread) + " " + tr("unread");
	setWindowTitle(title);
}

/*********************** Gestion des actions ********************************/
///Valide le texte
/** utilis� pour tronquer la chaine et enlever le retour chariot quand
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

///On ajoute une nouvelle couleur dans la liste des couleurs pr�d�finies
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

///Changement de la police de caract�re
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

///Activation/D�sactivation du formatage HTML du texte
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
}


/******************** Editiion du texte *******************/
///En cas d'�dition du texte
void RzxChat::onTextChanged()
{
	if(!typing && ui->edMsg->toPlainText().length())
	{
		typing = true;
		if(computer())
			computer()->sendChat(Rzx::Typing);
		//On ne cr�e pas de m_socket pour envoyer typing
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

///Ajoute les en-t�tes qui vont bien pour l'affichage
void RzxChat::append(const QString& color, const QString& host, const QString& argMsg)
{
	QDateTime date = QDateTime::currentDateTime();
	QString tmp, tmpH, head="", tmpD;
	tmpD = date.toString("ddd d MMMM yy");
	head = date.toString("<i>hh:mm:ss") + "&nbsp;";
	tmp.sprintf("<i>");
	
	//Nettoyage du html du message
	QString msg(argMsg);
	msg.remove(QRegExp("<head>.*</head>")).remove("<html>")
		.remove(QRegExp("<body[^<>]*>")).remove("</html>").remove("</body>");

	//Distinction du /me et d'un message normal
	QRegExp action("^(\\s*<[^<>]+>)*/me(<[^<>]+>|\\s)(.*)");
	if(!action.indexIn(msg))
	{
		QString entete = action.cap(1);
		QString entext = action.cap(2);
		QString pieddp = action.cap(3);
		if(host == ">&nbsp;") tmp = ("<font color=\"purple\">" + tmp + " * %1%2%3%4</i></font><br>")
					.arg(entete).arg(RzxComputer::localhost()->name()).arg(entext).arg(pieddp);
		else tmp = ("<font color=\"purple\">" + tmp + " * %1%2%3%4</i></font><br>")
					.arg(entete).arg(host.mid(0, host.length()-7)).arg(entext).arg(pieddp);
		tmpD = QString("<font color=\"purple\"><i>%1 - %2</i></font>").arg(tmpD, head);
		tmpH = ("<font color=\"purple\">"+head+"</font>");
		 
	}
	else {
		tmp = ("<font color=\"%1\">" + tmp + " %2</i> %3</font><br>")
					.arg(color).arg(host).arg(msg);
		tmpD = QString("<font color=\"%1\"><i>%2 - %3</i></font>").arg(color).arg(tmpD, head);
		tmpH = ("<font color=\"%1\">"+head+"</font>").arg(color);
	}

	// Enregistrement des logs...
	textHistorique = textHistorique + tmpD + tmp;

	// Gestion des smileys
	RzxSmileys::replace(tmp);
	
	if(RzxChatConfig::printTime())
		txtHistory->append(tmpH + tmp);
	else
		txtHistory->append(tmp);
	txtHistory->textCursor().movePosition(QTextCursor::End);
	txtHistory->ensureCursorVisible();
	ui->edMsg->setFocus();
}

/** Affiche un message re�u, et emet un son s'il faut */
void RzxChat::receive(const QString& msg)
{
	QString message = msg;
	if(RzxChatConfig::beep() && !ui->btnSound->isChecked())
		RzxSound::play(RzxChatConfig::beepSound());	
	
	if(RzxConfig::autoResponder())
		append("darkgray", name() + ">&nbsp;", message);
	else
		append("blue", name() + ">&nbsp;", message);
	if(!isActiveWindow())
	{
		unread++;
		updateTitle();
	}
#ifdef WIN32
  if(!timer->isActive()) timer->start( 1000);
#endif
}


/// Affiche une info de status (deconnexion, reconnexion)
void RzxChat::info(const QString& msg)
{
	append( "darkgreen", name() + " ", msg );
}

/// Affiche un message de notification (envoie de prop, ping, pong...)
void RzxChat::notify(const QString& msg, bool withHostname)
{
	if(!RzxChatConfig::warnWhenChecked())
		return;

	QString header = "***&nbsp;";
	if(withHostname) header += name() + "&nbsp;";
	append("gray", header, msg);
}

///Reception d'un message ou autre
void RzxChat::receiveChatMessage(Rzx::ChatMessageType type, const QString& msg)
{
	switch(type)
	{
		case Rzx::Responder: case Rzx::Chat: receive(msg); break;
		case Rzx::Ping: notify(tr("Ping request received")); break;
		case Rzx::Pong: notify(tr("Pong answer received")); break;
		case Rzx::Typing:  peerTypingStateChanged(true); break;
		case Rzx::StopTyping:  peerTypingStateChanged(false); break;
		case Rzx::InfoMessage: info(msg); break;
		case Rzx::Closed: info(tr("Chat closed")); break;
	}
}

///Envoie le contenu de la fen�tre d'�dition
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
	
	//Apr�s l'analyse du texte, on s�lectionne ce qui va bien
	QString msg;
	if(format)
		msg = htmlMsg;
	else
		msg = rawMsg;
		
	QString dispMsg = msg;
	append("red", ">&nbsp;", dispMsg);
	sendChat(msg);	//passage par la sous-couche de gestion du m_socket avant d'�mettre

	ui->edMsg->validate();
}

/********* Gestion des propri�t�s et de l'historique *********/
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

	//Affiche la fen�tre
	hist = (RzxChatPopup*)RzxChatLister::global()->historique(RzxHostAddress::fromRezix(lastIP), false, this, ui->btnHistorique);
	hist->show();
}

///L'utilisateur demande les propri�t�s, on lance le check.
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
	computer()->checkProperties();
}

///Demande l'affichage des propri�t�s
void RzxChat::receiveProperties(const QString& msg)
{
	prop = (RzxChatPopup*)RzxChatLister::global()->showProperties(computer()->ip(), msg, false, this, ui->btnProperties);
	if(prop.isNull())
	{
		ui->btnProperties->setChecked(false);
		return;
	}
	prop->show();
}

///D�placement des popups avec la fen�tre principale
void RzxChat::moveEvent(QMoveEvent *)
{
	if(!hist.isNull())
		hist->move();
	if(!prop.isNull())
		prop->move();
	if(!smileyUi.isNull())
		smileyUi->move();
}

/// Gestion de la connexion avec l'autre client
/** Cette m�thode permet de g�rer les deux cas :
 *		- soit la connexion est d�j� �tablie, on utilise le m_socket d�j� en place
 *		- soit il n'y a pas connexion, dans ce cas, on ouvre la connexion l'�mission du message se fera d�s que celle-ci sera pr�te
 */
void RzxChat::sendChat(const QString& msg)
{
	if(computer())
		computer()->sendChat(Rzx::Chat, msg);
}

/******************* Gestion des �v�nements ***************************/
#ifdef WIN32
void RzxChat::showEvent ( QShowEvent *){
	timer->stop();
}
#endif

/// Ex�cution � la fermeture
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

///Pour r�cup�rer quelques �v�nements (genre activation de la fen�tre)
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
#ifdef WIN32
	if(isActiveWindow())
		timer->stop();
#endif

	return QWidget::event(e);
}


/********************** Gestion de l'apparence ****************************/
///Changement du th�me d'ic�ne
void RzxChat::changeTheme()
{
	ui->btnSound->setIcon(RzxIconCollection::getSoundIcon());
	ui->btnHistorique->setIcon(RzxIconCollection::getIcon(Rzx::ICON_HISTORIQUE));
	ui->btnSend->setIcon(RzxIconCollection::getIcon(Rzx::ICON_SEND));
	ui->btnProperties->setIcon(RzxIconCollection::getIcon(Rzx::ICON_PROPRIETES));
	ui->btnClose->setIcon(RzxIconCollection::getIcon(Rzx::ICON_CANCEL));
}

///Changement du th�me de smileys
void RzxChat::changeSmileyTheme()
{
	ui->btnSmiley->setIcon(RzxSmileys::pixmap(":-)"));
	ui->btnSmiley->setEnabled(RzxSmileys::isValid());
}

///Changement du format d'affichage des ic�nes
/** Contrairement au menu de la fen�tre principale, les ic�nes sont ici toujours petites et le texte toujours � droite. Les seules possibilit�s sont de masquer ou d'afficher le texte et les ic�nes */
void RzxChat::changeIconFormat()
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
	else if(icons && texts) style = Qt::ToolButtonTextBesideIcon;

	QList<QToolButton*> toolButtons = editor->findChildren<QToolButton*>();
	foreach(QToolButton *button, toolButtons)
		button->setToolButtonStyle(style);
	
	if(ui->btnSound->icon().isNull()) changeTheme(); //pour recharcher les ic�nes s'il y a besoin
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
