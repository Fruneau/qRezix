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
#include <qdns.h>
#include <qtextedit.h>
#include <qtextview.h>
#include <qpushbutton.h>
#include <qaccel.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <qwidget.h>
#include <qwidgetlist.h>
#include <qapplication.h>
#include <qfile.h>
#include <qdir.h>
#include <qregexp.h>
#include <qcheckbox.h>
#include <qcombobox.h>

#include <qsound.h>
#ifndef WIN32
#include <qprocess.h>
#endif

#include "rzxchat.h"
#include "rzxconfig.h"
#include "rzxcomputer.h"

RzxChat::RzxChat(const RzxHostAddress& peerAddress)
{
	QAccel * accel = new QAccel(btnSend);
	accel -> insertItem(CTRL + Key_Return, 100);
	accel -> connectItem(100, btnSend, SIGNAL(clicked()));
	accel -> insertItem(CTRL + Key_Enter, 101);
	accel -> connectItem(101, btnSend, SIGNAL(clicked()));
//	accel -> insertItem(SHIFT + Key_Enter, 103);
//	accel -> connectItem(103, this, SLOT(onShiftReturnPressed()));
	//accel -> insertItem(SHIFT + Key_Return, 104);
	//accel -> connectItem(104, this, SLOT(onShiftReturnPressed()));
	btnSound -> setAccel(Key_F1);
	accel -> insertItem(Key_Escape, 102);
	accel -> connectItem(102, btnClose, SIGNAL(clicked()));
	
	//on ajoute l'icone du son
	btnSound -> setPixmap(*RzxConfig::soundIcon(false)); //true par défaut
	
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
	peer = peerAddress;

//	setCaption("Chat - " + peer.toString());

#ifdef WIN32
  timer = new QTimer( this );
  connect( timer, SIGNAL(timeout()),this, SLOT(messageReceived()) );
#endif

	connect(btnSend, SIGNAL(clicked()), this, SLOT(btnSendClicked()));
	connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(btnHistorique, SIGNAL(clicked()), this, SLOT(btnHistoriqueClicked()));
	connect(btnProperties, SIGNAL(clicked()), this, SLOT(btnPropertiesClicked()));
	connect(edMsg, SIGNAL(enterPressed()), this, SLOT(onReturnPressed()));
	connect(edMsg, SIGNAL(arrowPressed(bool)), this, SLOT(onArrowPressed(bool)));
	connect(edMsg, SIGNAL(textWritten()), this, SLOT(onTextChanged()));
	connect(btnSound, SIGNAL(toggled(bool)), this, SLOT(soundToggled(bool)));
	connect(cbSendHTML, SIGNAL(toggled(bool)), this, SLOT(activateFormat(bool)));
	activateFormat(false);
}

RzxChat::~RzxChat(){
	QString temp = textHistorique;

	QString filename = RzxConfig::historique(peer.toRezix(), hostname);
	if (filename.isNull()) return;
	
	QFile file(filename);		
	file.open(IO_ReadWrite |IO_Append);
	file.writeBlock(temp, temp.length());
	file.close();
}

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

void RzxChat::setHostname(const QString& name){
	hostname=name;
}

void RzxChat::append(const QString& color, const QString& host, const QString& msg) {
	QTime cur = QTime::currentTime();
	QString tmp, tmpH, head="";
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
		tmpH = ("<font color=\"purple\">"+head+"</font>");
		 
	}
	else{
		tmp = ("<font color=\"%1\">" + tmp + " %2</i> %3</font><br>")
					.arg(color).arg(host).arg(msg);
		tmpH = ("<font color=\"%1\">"+head+"</font>").arg(color);
	}
	if(RzxConfig::globalConfig()->printTime())
		txtHistory -> setText(txtHistory -> text() + tmpH + tmp);
	else
		 txtHistory -> setText(txtHistory -> text() + tmp);
	textHistorique = textHistorique + tmpH + tmp;
	txtHistory -> ensureVisible(0, txtHistory -> contentsHeight());
	edMsg->setFocus();
}

/** Affiche un message reçu, et emet un son s'il faut */
void RzxChat::receive(const QString& msg){
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
		
	append("blue", hostname + "> ", msg);
#ifdef WIN32
  if(!timer->isActive()) timer->start( 1000, FALSE );
#endif
}


/** Affiche une info de status (deconnexion, reconnexion) */
void RzxChat::info(const QString& msg){
    append( "darkgreen", hostname + " ", msg );
}

/* utilisé pour tronquer la chaine et enlever le retour chariot quand
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
		edMsg -> setPointSize(11);
		edMsg -> setBold(false);
		edMsg -> setItalic(false);
		edMsg -> setUnderline(false);
		edMsg -> setTextFormat(Qt::PlainText);
	}
	else
		edMsg -> setTextFormat(Qt::RichText);
}
#else
		edMsg -> setFamily("Terminal");
		edMsg -> setPointSize(11);
		edMsg -> setBold(false);
		edMsg -> setItalic(false);
		edMsg -> setUnderline(false);
		edMsg -> setTextFormat(Qt::PlainText);
	}
	else
		edMsg -> setTextFormat(Qt::RichText);
}
#endif

/** No descriptions */
void RzxChat::btnSendClicked(){
	QString msg = edMsg -> text();
	if(msg.isEmpty()) return;

	history = new ListText(msg, history);
	curLine = history;
		
	// Conversion du texte en HTML si necessaire
	if( ! cbSendHTML->isChecked() )
	{
		static const QRegExp ampersand("&");
		msg.replace(ampersand, "&amp;");

		static const QRegExp tag("<");
		msg.replace(tag, "&lt;");

        static const QRegExp returns_cr("\n");
		msg.replace(returns_cr, "<br>");

		// Pour que les espaces soient bien transmis
		static const QRegExp tag_space("> ");
		msg.replace(tag_space, ">&nbsp;");

		static const QRegExp dblspace("  ");
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
	
	append("red", "> ", msg);
	emit send(peer, msg);
	qDebug("Message envoyé");
	qDebug(msg);
	edMsg -> setText("");
}

void RzxChat::btnHistoriqueClicked(){
	QString temp = textHistorique;

	QString filename = RzxConfig::historique(peer.toRezix(), hostname);
	if (filename.isNull()) return;
	
	QFile file(filename);		
	file.open(IO_ReadWrite |IO_Append);
	file.writeBlock(temp, temp.length());
	file.close();
	emit showHistorique( peer.toRezix(), hostname );
}


void RzxChat::btnPropertiesClicked(){
	emit askProperties( peer );
}

void RzxChat::soundToggled(bool state) {
	btnSound->setPixmap(*RzxConfig::soundIcon(state));
}

#ifdef WIN32
void RzxChat::showEvent ( QShowEvent * e){
	timer->stop();
}

bool RzxChat::event ( QEvent * e){
	if(isActiveWindow())
		timer->stop();
	return QWidget::event(e);
}
#endif

/** No descriptions */
void RzxChat::closeEvent(QCloseEvent * e){
	 e -> accept();
	 emit closed(peer);
}
