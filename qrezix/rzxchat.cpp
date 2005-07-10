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
#include <QPushButton>
#include <QToolButton>
#include <QShortcut>
#include <QDateTime>
#include <QTimer>
#include <QWidget>
#include <QApplication>
#include <QFile>
#include <QCheckBox>
#include <QComboBox>
#include <QIcon>
#include <QPoint>
#include <QSound>
#include <QColor>
#include <QColorDialog>
#include <QRegExp>
#include <QPixmap>
#include <QSplitter>
#include <QFrame>
#include <QList>
#include <QShowEvent>
#include <QCloseEvent>
#include <QMoveEvent>
#include <QKeyEvent>
#include <QEvent>

#if !defined(WIN32) && !defined(Q_OS_MAC)
#include <QProcess>
#endif

#include "rzxchat.h"

#include "qrezix.h"
#include "rzxconfig.h"
#include "rzxcomputer.h"
#include "rzxpluginloader.h"

/****************************************************
* RzxPopup
****************************************************/
RzxPopup::RzxPopup(QWidget *parent)
#ifdef Q_OS_MAC
	:QFrame(parent, Qt::Drawer)
#else
	:QFrame(parent, Qt::WType_TopLevel)
#endif
{
	setAttribute(Qt::WA_DeleteOnClose);
}

/***************************************************
* RzxChat::ListText
***************************************************/
RzxChat::ListText::ListText(QString t, ListText * pN) {
	texte = t;
	pNext = pN;
	pPrevious = 0;
	if(pNext != 0)
		pNext -> pPrevious = this;
}

RzxChat::ListText::~ListText() {
	delete pNext;
}

/******************************
* RzxChat
******************************/
const QColor RzxChat::preDefinedColors[16] = {Qt::black, Qt::red, Qt::darkRed,
		Qt::green, Qt::darkGreen, Qt::blue, Qt::darkBlue, Qt::cyan, Qt::darkCyan,
		Qt::magenta, Qt::darkMagenta, Qt::yellow, Qt::darkYellow, Qt::gray,
		Qt::darkGray, Qt::lightGray};

/*************** Création/Destruction de la fenêtre *****************/
//On crée la fenêtre soit avec un socket d'une connection déjà établie
RzxChat::RzxChat(RzxChatSocket* sock)
	:QWidget(NULL, Qt::WindowContextHelpButtonHint), RzxChatUI()
{
	setSocket(sock);
	init();
}

//Soit sans socket, celui-ci sera initialisé de par la suite
RzxChat::RzxChat(const RzxHostAddress& peerAddress)
	:QWidget(NULL, Qt::WindowContextHelpButtonHint), RzxChatUI()
{
	peer = peerAddress;
	socket = NULL;
	init();
}

///Initialisation de la fenêtre de chat
/** L'initialisation de la fenêtre à proprement dit ne gère pas la partie socket */
void RzxChat::init()
{
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
	//Partie 2 : L'éditeur qui est défini dans l'ui RzxChatUI
	editor = new QWidget();
	setupUi(editor);
	edMsg->setChat(this);
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
    edMsg->setFocus();

	/* Définition des raccourcis claviers */
	new QShortcut(Qt::CTRL + Qt::Key_Return, btnSend, SIGNAL(clicked()));
	new QShortcut(Qt::CTRL + Qt::Key_Enter, btnSend, SIGNAL(clicked()));
	new QShortcut(Qt::Key_Escape, btnClose, SIGNAL(clicked()));
	new QShortcut(Qt::Key_F1, btnSound, SLOT(toggle()));

	/* Construction du texte et des icônes des boutons */
	setIcon(QRezix::qRezixIcon());
	changeTheme();
	changeIconFormat();


	/**** Préparation des données ****/
	//gestion touches haut et bas
	curLine = history = 0;
	curColor = Qt::black;

	//chargement des fontes
	defFont = new QFont("Terminal", 11);
	cbFontSelect->insertStringList(RzxConfig::globalConfig()->getFontList());
	
	//chargement de la liste des couleurs
	cbColorSelect->insertItem(tr ("Custom colours...")); //tjs 0
	for(int i=0; i<16; ++i)
		addColor(preDefinedColors[i]);
	cbColorSelect->setCurrentItem(1); //black par défaut

	//gestion du formatiage du texte
	connect(btnBold, SIGNAL(toggled(bool)), edMsg, SLOT(setBold(bool)));
	connect(btnItalic, SIGNAL(toggled(bool)), edMsg, SLOT(setFontItalic(bool)));
	connect(btnUnderline, SIGNAL(toggled(bool)), edMsg, SLOT(setFontUnderline(bool)));
	connect(cbColorSelect, SIGNAL(activated(int)), this, SLOT(on_cbColorSelect_activated(int)));
	connect(cbFontSelect, SIGNAL(activated(int)), this, SLOT(on_cbFontSelect_activated(int)));
	connect(cbSize, SIGNAL(activated(int)), this, SLOT(on_cbSize_activated(int)));
	connect(cbSendHTML, SIGNAL(toggled(bool)), this, SLOT(on_cbSendHTML_toggled(bool)));

	/** Connexions **/
#ifdef WIN32
	timer = new QTimer( this );
	connect( timer, SIGNAL(timeout()),this, SLOT(messageReceived()) );
#endif
	connect(edMsg, SIGNAL(enterPressed()), this, SLOT(onReturnPressed()));
	connect(edMsg, SIGNAL(arrowPressed(bool)), this, SLOT(onArrowPressed(bool)));
	connect(edMsg, SIGNAL(textWritten()), this, SLOT(onTextChanged()));
	connect(btnHistorique, SIGNAL(toggled(bool)), this, SLOT(on_btnHistorique_toggled(bool)));
	connect(btnProperties, SIGNAL(toggled(bool)), this, SLOT(on_btnProperties_toggled(bool)));
	connect(btnPlugins, SIGNAL(toggled(int)), this, SLOT(on_btnPlugins_toggled(int)));
	connect(btnSend, SIGNAL(clicked()), this, SLOT(on_btnSend_clicked()));
	connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));

	on_cbFontSelect_activated(0);
	on_cbSendHTML_toggled(false);	
	edMsg->setPlainText("");
	typing = peerTyping = false;
	unread = 0;
}

///Bye bye
RzxChat::~RzxChat(){
	QString temp = textHistorique;

	QString filename = RzxConfig::historique(peer.toRezix(), hostname);
	if (filename.isNull()) return;
	
	QFile file(filename);		
	file.open(QIODevice::ReadWrite |QIODevice::Append);
	file.writeBlock(temp, temp.length());
	file.close();
	
#ifdef WIN32
	if(timer) delete timer;
#endif
	
	if(defFont) delete defFont;
	if(socket)
	{
		socket->close();
		qDebug("Connection with " + hostname + " has been closed by killing the chat window");
	}
}

/********************* Gestion de la connexion au réseau ***********************/
// Inline définies dans rzxchat.h :
//	 - RzxChatSocket *RzxChatSocket::getSocket();
//	 - RzxChatSocket *RzxChatSocket::getValidSocket();

///Installation/Remplacement du socket de chat
void RzxChat::setSocket(RzxChatSocket* sock)
{
	if(socket != NULL && socket->isConnected() && *sock != *socket)
	{
		qDebug("Un nouveau socket différent a été proposé à " + hostname);
		socket->close();
	}
	
	socket = sock;

	if(socket)
	{
		socket->setParent(this);
		connect(this, SIGNAL(send(const QString& )), socket, SLOT(sendChat(const QString& )));
		connect(socket, SIGNAL(chat(const QString& )), this, SLOT(receive(const QString& )));
		connect(socket, SIGNAL(info(const QString& )), this, SLOT(info(const QString& )));
		connect(socket, SIGNAL(notify(const QString&, bool)), this, SLOT(notify(const QString&, bool )));
		connect(socket, SIGNAL(pongReceived(int )), this, SLOT(pong(int)));
		connect(&typingTimer, SIGNAL(timeout()), socket, SLOT(sendTyping()));
		connect(socket, SIGNAL(typing(bool)), this, SLOT(peerTypingStateChanged(bool)));
	}
}

///Réception d'un message pong
void RzxChat::pong(int ms)
{
	notify(tr(QString(tr("Pong received within %1 msecs")).arg(ms)));
}


///Changement de l'état de l'autre utilisateur
void RzxChat::peerTypingStateChanged(bool state)
{
	peerTyping=state;
	updateTitle();
}


///Changement du nom de la machine avec laquelle on discute
void RzxChat::setHostname(const QString& name)
{
	hostname=name;
	updateTitle();
}

///Changement du titre de la fenêtre
/** Le titre est de la forme :
 * Chat - remoteHostName( - Is typing a message)?( - \d+ unread)
 */ 
void RzxChat::updateTitle()
{
	QString title = tr("Chat") + " - " + hostname;
	
	if(peerTyping && isActiveWindow()) title += " - " + tr("Is typing a message");
	if(unread) title += " - " + QString::number(unread) + " " + tr("unread");
	setWindowTitle(title);
}

/*********************** Gestion des actions ********************************/
///Valide le texte
/** utilisé pour tronquer la chaine et enlever le retour chariot quand
l'utilisateur utilise Return ou Enter pour envoyer son texte */
void RzxChat::onReturnPressed() {
	int length=edMsg->text().length();
	if(length==0) {  //vide + /n
		edMsg->setText("");
		return;
	}
	//edMsg->setText(edMsg->text().left(length-1));
	on_btnSend_clicked();
}

///Pacours de l'historique
void RzxChat::onArrowPressed(bool down) {
	if(history==0)
		return;
	ListText * newCur=0;
	if(down)
		newCur = curLine->pPrevious;
	else
		newCur = curLine->pNext;
	if(!newCur)
		newCur = history;
	edMsg->setText(newCur->texte);
	curLine = newCur;
}

///On ajoute une nouvelle couleur dans la liste des couleurs prédéfinies
void RzxChat::addColor(QColor color) {
	QPixmap p = QPixmap(50, 15);
	p.fill(color);
	cbColorSelect->insertItem(p);
}

///Changement de la couleur du texte
void RzxChat::on_cbColorSelect_activated(int index) {
	QColor c;
	if(index==0)
		c=QColorDialog::getColor(curColor, this, "colorSelector");
	else {
		if(index <=16)
			c=preDefinedColors[index-1];
		else
			c=QColorDialog::customColor(index - 17);
	}
	curColor = c.isValid() ? c : curColor;
	qDebug("Color changed");
	edMsg->setTextColor(curColor);
}

///Changement de la police de caractère
void RzxChat::on_cbFontSelect_activated(int index) {
	QString family = cbFontSelect->text(index);
	btnBold->setEnabled(RzxConfig::globalConfig()->isBoldSupported(family));
	btnItalic->setEnabled(RzxConfig::globalConfig()->isItalicSupported(family));
	QList<int> pSize = RzxConfig::globalConfig()->getSizes(family);

	QString size = cbSize->currentText();
	cbSize->clear();
	QListIterator<int> points(pSize);
	while(points.hasNext())
	{
		QString newItem = QString::number(points.next());
		cbSize->insertItem(newItem);
		if(newItem == size)
			cbSize->setCurrentText(size);
	}
	edMsg->setFontFamily(family);
}

///Changement de la taille du texte
void RzxChat::on_cbSize_activated(int index) {
	QString size = cbSize->text(index);
	bool ok;
	int point = size.toInt(&ok, 10);
	if(!ok)
		return;
	edMsg->setFontPointSize(point);
}

///Activation/Désactivation du formatage HTML du texte
void RzxChat::on_cbSendHTML_toggled(bool on) {
	cbFontSelect->setEnabled(on);
	cbColorSelect->setEnabled(on);
	cbSize->setEnabled(on);
	btnBold->setEnabled(on);
	btnUnderline->setEnabled(on);
	btnItalic->setEnabled(on);
	if(!on) {
#ifdef WIN32
//parce que Terminal n'existe pas sous Win !
		edMsg->setFontFamily("Arial");
		edMsg->setFontPointSize(8);
#else
		edMsg->setFontFamily("Terminal");
		edMsg->setFontPointSize(11);
#endif
		edMsg->setBold(false);
		edMsg->setFontItalic(false);
		edMsg->setFontUnderline(false);
		edMsg->setPlainText(edMsg->toPlainText());
	}
}


/******************** Editiion du texte *******************/
void RzxChat::onTextChanged()
{
	if(!history) {
		history = new ListText(edMsg->text(), 0);
		curLine = history;
		return;
	}
	history -> texte = edMsg->text();
	if(!typing && edMsg->text().length())
	{
		typing = true;
		//On ne crée pas de socket pour envoyer typing
		if(socket)
			socket->sendTyping(true);
		typingTimer.start(10*1000, true);
	}
	if(typing && !edMsg->text().length())
	{
		typing = false;
		if(socket)
			socket->sendTyping(false);
		typingTimer.stop();
	}
}

///Ajoute les en-têtes qui vont bien pour l'affichage
void RzxChat::append(const QString& color, const QString& host, const QString& argMsg) {
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
	if(!action.search(msg)) {
		QString entete = action.cap(1);
		QString entext = action.cap(2);
		QString pieddp = action.cap(3);
		if(host == ">&nbsp;") tmp = ("<font color=\"purple\">" + tmp + " * %1%2%3%4</i></font><br>")
					.arg(entete).arg(RzxConfig::globalConfig()->localHost()->getName()).arg(entext).arg(pieddp);
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
	if(RzxConfig::globalConfig()->printTime())
		txtHistory->append(tmpH + tmp);
	else
		txtHistory->append(tmp);
	textHistorique = textHistorique + tmpD + tmp;
	txtHistory->textCursor().movePosition(QTextCursor::End);
	txtHistory->ensureCursorVisible();
	edMsg->setFocus();
}

/** Affiche un message reçu, et emet un son s'il faut */
void RzxChat::receive(const QString& msg)
{
	QString message = msg;
	RzxPlugInLoader::global()->chatChanged(edMsg);
	RzxPlugInLoader::global()->chatReceived(&message);
	if(RzxConfig::beep() && !btnSound->isChecked()) {
#if defined(WIN32) || defined(Q_OS_MAC)
		QString file = RzxConfig::beepSound();
		if( !file.isEmpty() && QFile( file ).exists() )
			QSound::play( file );
		else
			QApplication::beep();
#else
		QString cmd = RzxConfig::beepCmd(), file = RzxConfig::beepSound();
		if (!cmd.isEmpty() && !file.isEmpty()) {
			QProcess process;
			process.start(cmd, QStringList(file));
		}
#endif
	
	}
	
	if(RzxConfig::autoResponder())
		append("darkgray", hostname + ">&nbsp;", message);
	else
		append("blue", hostname + ">&nbsp;", message);
	if(!isActiveWindow())
	{
		unread++;
		updateTitle();
	}
#ifdef WIN32
  if(!timer->isActive()) timer->start( 1000, FALSE );
#endif
}


/** Affiche une info de status (deconnexion, reconnexion) */
void RzxChat::info(const QString& msg){
	append( "darkgreen", hostname + " ", msg );
}

/// Affiche un message de notification (envoie de prop, ping, pong...)
void RzxChat::notify(const QString& msg, bool withHostname)
{
	if(RzxConfig::globalConfig()->warnCheckingProperties()==0)
		return;

	QString header = "***&nbsp;";
	if(withHostname) header += hostname + "&nbsp;";
	append("gray", header, msg);
}


/** No descriptions */
void RzxChat::on_btnSend_clicked()
{
	//Pour que les plug-ins qui en on besoin modifie le texte de chat
	RzxPlugInLoader::global()->chatSending();
	bool format = cbSendHTML->isChecked();

	typingTimer.stop();
	typing = false;
	
	//traitement du message
	QString htmlMsg = edMsg->toHtml();
	QString rawMsg = edMsg->toPlainText();
	if(rawMsg.isEmpty()) return;

	history = new ListText(htmlMsg, history);
	curLine = history;

	if(rawMsg == "/ping" || rawMsg.left(6) == "/ping ")
	{
		getValidSocket()->sendPing();
		edMsg->setText("");
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
		msg = htmlMsg;
	else
		msg = rawMsg;
		
	QString dispMsg = msg;
	RzxPlugInLoader::global()->chatEmitted(&dispMsg);
	append("red", ">&nbsp;", dispMsg);
	sendChat(msg);	//passage par la sous-couche de gestion du socket avant d'émettre
	edMsg -> setText("");
	
/*	if(cbSendHTML->isChecked())
	{
		fontChanged(cbFontSelect->currentItem());
		sizeChanged(cbSize->currentItem());
		colorClicked(cbColorSelect->currentItem());
		edMsg->setBold(btnBold->isOn());
		edMsg->setItalic(btnItalic->isOn());
		edMsg->setUnderline(btnUnderline->isOn());
	}*/
}

/********* Gestion des propriétés et de l'historique *********/
///L'utilisateur demande l'historique.
void RzxChat::on_btnHistorique_toggled(bool on) {
	if(!on)
	{
		if(!hist.isNull())
			hist->close();
		return;
	}

	if(hist) return;
	btnProperties->setOn(false);
	
	QString temp = textHistorique;

	QString filename = RzxConfig::historique(peer.toRezix(), hostname);
	if (filename.isNull()) return;
	
	QFile file(filename);		
	file.open(QIODevice::ReadWrite |QIODevice::Append);
	file.writeBlock(temp, temp.length());
	file.close();
	QPoint *pos = new QPoint(btnHistorique->mapToGlobal(btnHistorique->rect().bottomLeft()));
	hist = (RzxPopup*)RzxChatSocket::showHistorique(peer, hostname, false, this, pos);
	delete pos;
	hist->show();
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
	btnHistorique->setOn(false);
	getValidSocket()->sendPropQuery();
}

///Demande l'affichage des propriétés
void RzxChat::receiveProperties(const QString& msg)
{
	QPoint *pos = new QPoint(btnProperties->mapToGlobal(btnProperties->rect().bottomLeft()));
	prop = (RzxPopup*)socket->showProperties(peer, msg, false, this, pos);
	delete pos;
	if(prop.isNull())
	{
		btnProperties->setOn(false);
		return;
	}
	prop->show();
}

void RzxChat::moveEvent(QMoveEvent *)
{
	if(!hist.isNull())
		hist->move(btnHistorique->mapToGlobal(btnHistorique->rect().bottomLeft()));
	if(!prop.isNull())
		prop->move(btnProperties->mapToGlobal(btnProperties->rect().bottomLeft()));
}

/// Gestion de la connexion avec l'autre client
/** Cette méthode permet de gérer les deux cas :
 *		- soit la connexion est déjà établie, on utilise le socket déjà en place
 *		- soit il n'y a pas connexion, dans ce cas, on ouvre la connexion l'émission du message se fera dès que celle-ci sera prête
 */
void RzxChat::sendChat(const QString& msg)
{
	getValidSocket()->sendChat(msg);
}

/******************* Gestion des événements ***************************/
#ifdef WIN32
void RzxChat::showEvent ( QShowEvent * e){
	timer->stop();
}
#endif

/// Exécution à la fermeture
void RzxChat::closeEvent(QCloseEvent * e)
{
	if(!hist.isNull())
		hist->close();
	if(!prop.isNull())
		prop->close();
	RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_CHAT, NULL);
	e -> accept();
	emit closed(peer);
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
#endif
		RzxPlugInLoader::global()->chatChanged(edMsg);
	}
#ifdef WIN32
	if(isActiveWindow())
		timer->stop();
#endif

	return QWidget::event(e);
}


/********************** Gestion de l'apparence ****************************/
///Changement du thème d'icône
void RzxChat::changeTheme()
{
	QIcon sound, pi, hist, send, prop, close;
	int icons = RzxConfig::menuIconSize();
	int texts = RzxConfig::menuTextPosition();
	
	if(icons || !texts)
	{
		pi.addPixmap(RzxConfig::themedIcon("plugin"));
		hist.addPixmap(RzxConfig::themedIcon("historique"));
		send.addPixmap(RzxConfig::themedIcon("send"));
		prop.addPixmap(RzxConfig::themedIcon("prop"));
	}
	sound.addPixmap(RzxConfig::soundIcon(false), QIcon::Normal, QIcon::Off);
	sound.addPixmap(RzxConfig::soundIcon(true), QIcon::Normal, QIcon::On);
	close.addPixmap(RzxConfig::themedIcon("cancel"));
	btnSound->setIconSet(sound);
	btnPlugins->setIconSet(pi);
	btnHistorique->setIconSet(hist);
	btnSend->setIconSet(send);
	btnProperties->setIconSet(prop);
	btnClose->setIconSet(close);
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
	
	//Mise à jour de la taille des icônes
	switch(icons)
	{
		case 0: //pas d'icône
			{
				QIcon empty;
				btnPlugins->setIconSet(empty);
				btnHistorique->setIconSet(empty);
				btnSend->setIconSet(empty);
				btnProperties->setIconSet(empty);
			}
			break;
		
		case 1: //petites icônes
		case 2: //grandes icones
			{
#ifdef Q_OS_MAC
				if(!btnPlugins->iconSet() || btnPlugins->iconSet()->isNull()) changeTheme();
#else
				if(btnPlugins->iconSet().isNull()) changeTheme(); //pour recharcher les icônes s'il y a besoin
				btnPlugins->setUsesBigPixmap(false);
				btnHistorique->setUsesBigPixmap(false);
				btnSend->setUsesBigPixmap(false);
				btnProperties->setUsesBigPixmap(false);
#endif
			}
			break;
	}
	
	//Mise à jour de la position du texte
#ifdef Q_OS_MAC
	if(texts)
	{
		btnPlugins->setText(tr("Plug-ins"));
		btnHistorique->setText(tr("History"));
		btnSend->setText(tr("Send"));
		btnProperties->setText(tr("Properties"));
	}
	else
	{
		btnPlugins->setText("");
		btnHistorique->setText("");
		btnSend->setText("");
		btnProperties->setText("");
	}
#else
	btnPlugins->setUsesTextLabel(texts);
	btnHistorique->setUsesTextLabel(texts);
	btnSend->setUsesTextLabel(texts);
	btnProperties->setUsesTextLabel(texts);
	if(texts)
	{
		btnPlugins->setTextPosition(QToolButton::BesideIcon);
		btnHistorique->setTextPosition(QToolButton::BesideIcon);
		btnSend->setTextPosition(QToolButton::BesideIcon);
		btnProperties->setTextPosition(QToolButton::BesideIcon);
	}
#endif
}

#ifdef Q_OS_MAC
void RzxChat::languageChange()
{
	QWidget::languageChange();
	changeIconFormat();
}
#endif

/// Affichage du menu plug-ins lors d'un clic sur le bouton
/** Les actions sont gérées directement par le plug-in s'il a bien été programmé */
void RzxChat::on_btnPlugins_clicked()
{
	menuPlugins.clear();
	RzxPlugInLoader::global()->menuChat(menuPlugins);
	if(!menuPlugins.count())
		menuPlugins.addAction("<none>");
	menuPlugins.popup(btnPlugins->mapToGlobal(btnPlugins->rect().bottomLeft()));
}


/***************************************************
* RzxTextEdit : fenêtre d'édition avec interceptions
***************************************************/
RzxTextEdit::~RzxTextEdit() {
}

void RzxTextEdit::keyPressEvent(QKeyEvent *e) {
	QKeyEvent * eMapped=e;
//	bool down=false;
//	int para, index, line;
	
	//Saut de ligne - Envoie du message
	switch(eMapped->key()) {
	case Qt::Key_Enter: case Qt::Key_Return:
		if(!(eMapped->state() & Qt::ShiftButton)) {
			emit enterPressed();
			break;
		}
		eMapped =new QKeyEvent(QEvent::KeyRelease, Qt::Key_Enter, e->ascii(), e->state());
		QTextEdit::keyPressEvent(eMapped);
		break;

	//Autocompletion
	case Qt::Key_Tab:
		//Pour que quand on appuie sur tab ça fasse la complétion du nick
		if(!nickAutocompletion())
		{
			QTextEdit::keyPressEvent(eMapped);
			emit textWritten();
		}
		break;
	
	//Parcours de l'historique
/*	case Qt::Key_Down: 
		down=true;
	case Qt::Key_Up:
		//Pour pouvoir éviter d'avoir à appuyer sur Shift ou Ctrl si on est à l'extrémité de la boite
		getCursorPosition(&para, &index);
		line = lineOfChar(para, index);
		for(int i = 0 ; i<para ; i++) line += linesOfParagraph(i);
		
		//Et op, parcours de l'historique si les conditions sont réunies
		if((eMapped->state() & Qt::ShiftButton) || (eMapped->state() & Qt::ControlButton) || (down && (line == lines()-1)) || (!down && !line)) {
			emit arrowPressed(down);
			break;
		}
		eMapped =new QKeyEvent(QEvent::KeyRelease, e->key(), e->ascii(), e->state());
*/	
	//Texte normal
	default:
		QTextEdit::keyPressEvent(eMapped);
		emit textWritten();
	}
}

///Fait une completion automatique du nick au niveau du curseur.
/** La completion n'est réalisée que si aucune sélection n'est actuellement définie */
bool RzxTextEdit::nickAutocompletion()
{
	QTextCursor cursor = textCursor();
	
	//Si y'a une sélection, on zappe
	if(cursor.hasSelection())
		return false;
	
	//On récupère la position du curseur et la paragraphe concerné
	int index = cursor.position();
	index - cursor.block().position();
	if(!index) return false;
	
	QRegExp mask("[^-A-Za-z0-9]([-A-Za-z0-9]+)$");
	QString textPara = cursor.block().text();
	
	//Juste pour se souvenir des pseudos possibles
	QString localName = RzxConfig::globalConfig()->localHost()->getName();
	QString remoteName = chat->getHostName();
	
	for(int i = 1 ; i <= index && (localName.length() > i || remoteName.length() > i) ; i++)
	{
		//Chaine de caractère qui précède le curseur de taille i
		QString nick = textPara.mid(index-i, i);
		
		if(mask.search(nick) != -1 || i == index)
		{
			if(mask.search(nick) != -1) nick = mask.cap(1);
			if(!remoteName.lower().find(nick.lower()) && localName.lower().find(nick.lower()))
			{
				cursor.insertText(remoteName.right(remoteName.length()-nick.length()) + " ");
				return true;
			}
			else if(remoteName.lower().find(nick.lower()) && !localName.lower().find(nick.lower()))
			{
				cursor.insertText(localName.right(localName.length()-nick.length()) + " ");
				return true;
			}
			return false;
		}
	}
	return false;
}
