/***************************************************************************
                          rzxchat.cpp  -  description
                             -------------------
    begin                : Sat Jan 26 2002
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
#include <qtextedit.h>
#include <qtextview.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qaccel.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <qwidget.h>
#include <qwidgetlist.h>
#include <qapplication.h>
#include <qfile.h>
#include <qdir.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qsocket.h>
#include <qiconset.h>
#include <qpoint.h>
#include <qsound.h>
#ifndef WIN32
#include <qprocess.h>
#endif

#include "rzxchat.h"

#include "rzxconfig.h"
#include "rzxcomputer.h"
#include "rzxpluginloader.h"

//On crée la fenêtre soit avec un socket d'une connection déjà établie
RzxChat::RzxChat(RzxChatSocket* sock)
{
	setSocket(sock);
	init();
}

//Soit sans socket, celui-ci sera initialisé de par la suite
RzxChat::RzxChat(const RzxHostAddress& peerAddress)
{
	peer = peerAddress;
	socket = NULL;
	init();
}

void RzxChat::init()
{
	hist = prop = NULL;
	
	//
	QAccel * accel = new QAccel(btnSend);
	accel -> insertItem(CTRL + Key_Return, 100);
	accel -> connectItem(100, btnSend, SIGNAL(clicked()));
	accel -> insertItem(CTRL + Key_Enter, 101);
	accel -> connectItem(101, btnSend, SIGNAL(clicked()));
	btnSound -> setAccel(Key_F1);
	accel -> insertItem(Key_Escape, 102);
	accel -> connectItem(102, btnClose, SIGNAL(clicked()));

	//on ajoute l'icone du son
	changeTheme();
	changeIconFormat();

	//gestion touches haut et bas
	curLine = history = 0;

	defFont = new QFont("Terminal", 11);
	//chargement des fontes
	cbFontSelect->insertStringList(RzxConfig::globalConfig()->getFontList());
	connect(cbFontSelect, SIGNAL(activated(int)), this, SLOT(fontChanged(int)));
	fontChanged(0);
	connect(btnBold, SIGNAL(toggled(bool)), edMsg, SLOT(setBold(bool)));
	connect(btnItalic, SIGNAL(toggled(bool)), edMsg, SLOT(setItalic(bool)));
	connect(btnUnderline, SIGNAL(toggled(bool)), edMsg, SLOT(setUnderline(bool)));
	connect(cbSize, SIGNAL(activated(int)), this, SLOT(sizeChanged(int)));
	txtHistory -> setTextFormat(Qt::RichText);
	txtHistory -> setReadOnly(true);
	// on rajoute le bouton maximiser

#ifdef WIN32
  timer = new QTimer( this );
  connect( timer, SIGNAL(timeout()),this, SLOT(messageReceived()) );
#endif

	connect(btnSend, SIGNAL(clicked()), this, SLOT(btnSendClicked()));
	connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(btnHistorique, SIGNAL(toggled(bool)), this, SLOT(btnHistoriqueClicked(bool)));
	connect(btnProperties, SIGNAL(toggled(bool)), this, SLOT(btnPropertiesClicked(bool)));
	connect(btnPlugins, SIGNAL(clicked()), this, SLOT(pluginsMenu()));
	connect(edMsg, SIGNAL(enterPressed()), this, SLOT(onReturnPressed()));
	connect(edMsg, SIGNAL(arrowPressed(bool)), this, SLOT(onArrowPressed(bool)));
	connect(edMsg, SIGNAL(textWritten()), this, SLOT(onTextChanged()));
	connect(cbSendHTML, SIGNAL(toggled(bool)), this, SLOT(activateFormat(bool)));
	activateFormat(false);
	edMsg -> setText("");
}

RzxChat::~RzxChat(){
	QString temp = textHistorique;

	QString filename = RzxConfig::historique(peer.toRezix(), hostname);
	if (filename.isNull()) return;
	
	QFile file(filename);		
	file.open(IO_ReadWrite |IO_Append);
	file.writeBlock(temp, temp.length());
	file.close();
	if(socket)
	{
		socket->close();
		qDebug("Connection with " + hostname + " has been closed by killing the chat window");
	}
}


/****************************************************
* RzxPopup
****************************************************/
RzxPopup::RzxPopup(QWidget *parent, const char *name)
	:QFrame(parent, name, WDestructiveClose | WStyle_Customize | WType_TopLevel)
{ }

RzxPopup::~RzxPopup()
{
	emit aboutToQuit();
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

RzxTextEdit::RzxTextEdit(QWidget *parent, const char*name)
		: QTextEdit(parent, name) {
	setTextFormat(Qt::RichText);
}

RzxTextEdit::~RzxTextEdit() {
}

void RzxTextEdit::keyPressEvent(QKeyEvent *e) {
	QKeyEvent * eMapped=e;
	bool down=false;
	switch(eMapped->key()) {
	case Qt::Key_Enter: case Qt::Key_Return:
		if(!(eMapped->state() & Qt::ShiftButton)) {
			emit enterPressed();
			break;
		}
		eMapped =new QKeyEvent(QEvent::KeyRelease, Qt::Key_Enter, e->ascii(), e->state());
		QTextEdit::keyPressEvent(eMapped);
		break;
	case Qt::Key_Down: 
		down=true;
	case Qt::Key_Up:
		if((eMapped->state() & Qt::ShiftButton) || (eMapped->state() & Qt::ControlButton)) {
			//charger la ligne qui va bien dans l'historique
			emit arrowPressed(down);
			break;
		}
		eMapped =new QKeyEvent(QEvent::KeyRelease, e->key(), e->ascii(), e->state());
		
	default:
		QTextEdit::keyPressEvent(eMapped);
		emit textWritten();
	}
}

void RzxChat::messageReceived(){
#ifdef WIN32
  bool fireEnabled=TRUE;
  QWidgetList  *list = QApplication::topLevelWidgets();
  QWidgetListIt it( *list );          // iterate over the widgets
  QWidget * w;
  while ( (w=it.current()) != 0 ) {   // for each widget...
    if(w->isActiveWindow()) {fireEnabled=FALSE;break;}
    ++it;
  }
  delete list;                        // delete the list, not the widgets

	if(fireEnabled) setActiveWindow();
#endif
}

/******************************
* CHAT
*/

RzxChatSocket *RzxChat::getSocket()
{
	return socket;
}

void RzxChat::setSocket(RzxChatSocket* sock)
{
	if(socket != NULL && socket->state() == QSocket::Connected && sock->socket() != socket->socket())
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
	}
}


void RzxChat::setHostname(const QString& name)
{
#ifdef WIN32
	setCaption( tr( "Chat" ) + " - " + name + " [Qt]" );
#else
	setCaption( tr( "Chat" ) + " - " + name );
#endif
	hostname=name;
}

void RzxChat::append(const QString& color, const QString& host, const QString& msg) {
	QTime cur = QTime::currentTime();
	QDate date = QDate::currentDate();
	QString tmp, tmpH, head="", tmpD;
	tmpD = date.toString("ddd d MMMM yy");
	head.sprintf("<i>%2i:%.2i:%.2i", 
			cur.hour(),
			cur.minute(),
			cur.second());
	tmp.sprintf("<i>");
	if(msg.left(4)=="/me "){
		 //Action
		if(host.length()<3) tmp = ("<font color=\"purple\">" + tmp + " * %1 %2</i></font><br>")
					.arg(RzxConfig::globalConfig()->localHost()->getName()).arg(msg.mid(4));
		else tmp = ("<font color=\"purple\">" + tmp + " * %1 %2</i></font><br>")
					.arg(host.mid(0, host.length()-2)).arg(msg.mid(4));
		tmpD = QString("<font color=\"purple\"><i>%1 - %2</i></font>").arg(tmpD, head);
		tmpH = ("<font color=\"purple\">"+head+"</font>");
		 
	}
	else{
		tmp = ("<font color=\"%1\">" + tmp + " %2</i> %3</font><br>")
					.arg(color).arg(host).arg(msg);
		tmpD = QString("<font color=\"%1\"><i>%2 - %3</i></font>").arg(color).arg(tmpD, head);
		tmpH = ("<font color=\"%1\">"+head+"</font>").arg(color);
	}
	if(RzxConfig::globalConfig()->printTime())
		txtHistory -> setText(txtHistory -> text() + tmpH + tmp);
	else
		txtHistory -> setText(txtHistory -> text() + tmp);
	textHistorique = textHistorique + tmpD + tmp;
	txtHistory -> ensureVisible(0, txtHistory -> contentsHeight());
	edMsg->setFocus();
}

/** Affiche un message reçu, et emet un son s'il faut */
void RzxChat::receive(const QString& msg)
{
	QString message = msg;
	RzxPlugInLoader::global()->chatChanged(edMsg);
	RzxPlugInLoader::global()->chatReceived(&message);
	if(RzxConfig::beep()&&/*!edMsg->hasFocus()&&*/!btnSound->isOn()) {
#ifdef WIN32
		QString file = RzxConfig::beepSound();
		if( !file.isEmpty() && QFile( file ).exists() )
			QSound::play( file );
        else
            QApplication::beep();
#else
		QString cmd = RzxConfig::beepCmd(), file = RzxConfig::beepSound();
		if (!cmd.isEmpty() && !file.isEmpty()) {
			QProcess process;
			process.addArgument(cmd);
			process.addArgument(file);
			process.start();
		}
#endif
	
	}
		
	append("blue", hostname + "> ", message);
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

	QString header = "*** ";
	if(withHostname) header += hostname + " ";
	append("gray", header, msg);
}

///Réception d'un message pong
void RzxChat::pong(int ms)
{
	notify(tr(QString(tr("Pong received within %1 msecs")).arg(ms)));
}

/** utilisé pour tronquer la chaine et enlever le retour chariot quand
l'utilisateur utilise Return ou Enter pour envoyer son texte */
void RzxChat::onReturnPressed() {
	int length=edMsg->text().length();
	if(length==0) {  //vide + /n
		edMsg->setText("");
		return;
	}
	//edMsg->setText(edMsg->text().left(length-1));
	btnSendClicked();
}

/* utilisé pour permettre à l'utilisateur de faire du multilignes en 
utilisant Shift+ Enter comme retour chariot */
/*void RzxChat::onShiftReturnPressed() {
	edMsg->setText(edMsg->text()+"\n");
	edMsg->moveCursor(QTextEdit::MoveDown, false);
}*/

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

void RzxChat::onTextChanged() {
	if(!history) {
		history = new ListText(edMsg->text(), 0);
		curLine = history;
		return;
	}
	history -> texte = edMsg->text();

}

void RzxChat::fontChanged(int index) {
	QString family = cbFontSelect->text(index);
	btnBold->setEnabled(RzxConfig::globalConfig()->isBoldSupported(family));
	btnItalic->setEnabled(RzxConfig::globalConfig()->isItalicSupported(family));
	QValueList<int> pSize = RzxConfig::globalConfig()->getSizes(family);
	
	cbSize->clear();
	for (QValueList<int>::Iterator points = pSize.begin(); points != pSize.end(); ++points )
        	cbSize->insertItem(QString::number(*points));
		
	edMsg -> setFamily(family);
}

void RzxChat::sizeChanged(int index) {
	QString size = cbSize -> text(index);
	bool ok;
	int point = size.toInt(&ok, 10);
	if(!ok)
		return;
	edMsg -> setPointSize(point);
}

void RzxChat::activateFormat(bool on) {
	cbFontSelect->setEnabled(on);
	cbColorSelect->setEnabled(on);
	cbSize->setEnabled(on);
	btnBold->setEnabled(on);
	btnUnderline->setEnabled(on);
	btnItalic->setEnabled(on);
	if(!on) {
#ifdef WIN32
//parce que Terminal n'existe pas sous Win !
		edMsg -> setFamily("Arial");
		edMsg -> setPointSize(8);
#else
		edMsg -> setFamily("Terminal");
		edMsg -> setPointSize(11);
#endif
		edMsg -> setBold(false);
		edMsg -> setItalic(false);
		edMsg -> setUnderline(false);
		edMsg -> setTextFormat(Qt::PlainText);
		edMsg -> setText(edMsg->text());
	}
	else
		edMsg -> setTextFormat(Qt::RichText);
}

/** No descriptions */
void RzxChat::btnSendClicked()
{
	//Pour que les plug-ins qui en on besoin modifie le texte de chat
	RzxPlugInLoader::global()->chatSending();
	
	//traitement du message
	QString msg = edMsg -> text();
	if(msg.isEmpty()) return;

	history = new ListText(msg, history);
	curLine = history;

	bool format = cbSendHTML->isChecked();
	if(!format && msg.left(6)=="<html>")
	{
		cbSendHTML->setChecked(true);
	}
	else if(!format && (msg == "/ping" || msg.left(6) == "/ping "))
	{
		getValidSocket()->sendPing();
		edMsg->setText("");
		notify(tr("Ping emitted"));
		return;
	}

	// Conversion du texte en HTML si necessaire
	if(!cbSendHTML->isChecked())
	{
		static const QString htmlflag("/<html>");
		msg.replace(htmlflag, "<html>");

		static const QString ampersand("&");
		msg.replace(ampersand, "&amp;");

		static const QString tag("<");
		msg.replace(tag, "&lt;");

		static const QString returns_cr("\n");
		msg.replace(returns_cr, "<br>");

		// Pour que les espaces soient bien transmis
		static const QString tag_space("> ");
		msg.replace(tag_space, ">&nbsp;");

		static const QString dblspace("  ");
		msg.replace(dblspace, " &nbsp;");
	}
	else {
		QString msgB(msg);
		msgB.remove("<p>");
		msgB.remove("</p>");
		msgB.remove("<html>");
		msgB.remove("</html>");
		//int endHeaderPos = msgB.find("</head>");
		//msgB.remove(0, endHeaderPos+7);
		msg=msgB;
	}
	

	QString dispMsg = msg;
	RzxPlugInLoader::global()->chatEmitted(&dispMsg);
	append("red", "> ", dispMsg);
	sendChat(msg);	//passage par la sous-couche de gestion du socket avant d'émettre
	edMsg -> setText("");

	if(!format && cbSendHTML->isChecked())
	{
		cbSendHTML->setChecked(false);
	}
}

void RzxChat::btnHistoriqueClicked(bool on){
	if(!on)
	{
		if(hist)
			hist->close();
		hist = NULL;
		return;
	}

	if(hist) return;
	btnProperties->setOn(false);
	
	QString temp = textHistorique;

	QString filename = RzxConfig::historique(peer.toRezix(), hostname);
	if (filename.isNull()) return;
	
	QFile file(filename);		
	file.open(IO_ReadWrite |IO_Append);
	file.writeBlock(temp, temp.length());
	file.close();
	QPoint *pos = new QPoint(btnHistorique->mapToGlobal(btnHistorique->rect().bottomLeft()));
	hist = RzxChatSocket::showHistorique( peer.toRezix(), hostname, false, this, pos);
	hist->show();
}

void RzxChat::btnPropertiesClicked(bool on)
{
	if(!on)
	{
		if(prop)
			prop->close();
		prop = NULL;
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
	prop = socket->showProperties(peer, msg, false, this, pos);
	prop->show();
}

#ifdef WIN32
void RzxChat::showEvent ( QShowEvent * e){
	timer->stop();
}
#endif

/// Gestion de la connexion avec l'autre client
/** Cette méthode permet de gérer les deux cas :
 *		- soit la connexion est déjà établie, on utilise le socket déjà en place
 *		- soit il n'y a pas connexion, dans ce cas, on ouvre la connexion l'émission du message se fera dès que celle-ci sera prête
 */
void RzxChat::sendChat(const QString& msg)
{
	getValidSocket()->sendChat(msg);
}

/** No descriptions */
void RzxChat::closeEvent(QCloseEvent * e)
{
	if(hist)
		hist->close();
	if(prop)
		prop->close();
	hist = prop = NULL;
	RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_CHAT, NULL);
	e -> accept();
	emit closed(peer);
}

bool RzxChat::event(QEvent *e)
{
/*	if(e->type() == QEvent::WindowDeactivate)
	{
		if(hist) ((RzxPopup*)hist)->forceVisible(false);
		if(prop) ((RzxPopup*)prop)->forceVisible(false);
	}*/
	if(e->type() == QEvent::WindowActivate)
	{
#ifndef WIN32
		if(hist) hist->raise();
		if(prop) prop->raise();
#endif
		RzxPlugInLoader::global()->chatChanged(edMsg);
	}
#ifdef WIN32
	if(isActiveWindow())
		timer->stop();
#endif

	return RzxChatUI::event(e);
}

///Changement du thème d'icône
void RzxChat::changeTheme()
{
	QIconSet sound, pi, hist, send, prop, close;
	int icons = RzxConfig::menuIconSize();
	int texts = RzxConfig::menuTextPosition();
	
	if(icons || !texts)
	{
		pi.setPixmap(*RzxConfig::themedIcon("plugin"), QIconSet::Automatic);
		hist.setPixmap(*RzxConfig::themedIcon("historique"), QIconSet::Automatic);
		send.setPixmap(*RzxConfig::themedIcon("send"), QIconSet::Automatic);
		prop.setPixmap(*RzxConfig::themedIcon("prop"), QIconSet::Automatic);
	}
	sound.setPixmap(*RzxConfig::soundIcon(false), QIconSet::Automatic, QIconSet::Normal, QIconSet::Off);
	sound.setPixmap(*RzxConfig::soundIcon(true), QIconSet::Automatic, QIconSet::Normal, QIconSet::On);
	close.setPixmap(*RzxConfig::themedIcon("cancel"), QIconSet::Automatic);
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
				QIconSet empty;
				btnPlugins->setIconSet(empty);
				btnHistorique->setIconSet(empty);
				btnSend->setIconSet(empty);
				btnProperties->setIconSet(empty);
			}
			break;
		
		case 1: //petites icônes
		case 2: //grandes icones
			{
				if(btnPlugins->iconSet().isNull()) changeTheme(); //pour recharcher les icônes s'il y a besoin
				btnPlugins->setUsesBigPixmap(false);
				btnHistorique->setUsesBigPixmap(false);
				btnSend->setUsesBigPixmap(false);
				btnProperties->setUsesBigPixmap(false);
			}
			break;
	}
	
	//Mise à jour de la position du texte
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
}

/// Affichage du menu plug-ins lors d'un clic sur le bouton
/** Les actions sont gérées directement par le plug-in s'il a bien été programmé */
void RzxChat::pluginsMenu()
{
	menuPlugins.clear();
	RzxPlugInLoader::global()->menuChat(menuPlugins);
	if(!menuPlugins.count())
		menuPlugins.insertItem("<none>");
	menuPlugins.popup(btnPlugins->mapToGlobal(btnPlugins->rect().bottomLeft()));
}
