/***************************************************************************
                          rzxclientlistener.cpp  -  description
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

#include <qsocketnotifier.h>
#include <qregexp.h>
#include <qsocket.h>
#include <qwidget.h>
#include <qframe.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qdir.h>
#include <qtextview.h>
#include <qtextedit.h>
#include <qtextstream.h>
#include <qlistview.h>
#include <qframe.h>

#include "rzxclientlistener.h"

#include "rzxconnectionlister.h"
#include "rzxmessagebox.h"
#include "rzxcomputer.h"
#include "rzxconfig.h"
#include "rzxchat.h"
#include "qrezix.h"
#include "q.xpm"

//attention a toujours avoir DCCFormat[DCC_message] = messageFormat
QString RzxChatSocket::DCCFormat[] = {
	"(^PROPQUERY )\r\n",
	"(^PROPANSWER )(.*)\r\n",
	"(^CHAT )(.*)\r\n",
	"(^PING )\r\n",
	"(^PONG )\r\n",
	"(^TYPING )(0|1)\r\n",
	0
};

///Construction d'un socket brute
RzxChatSocket::RzxChatSocket()
	:QSocket(), host()
{
	alone = false;
	chatWindow = NULL;
	connect(this, SIGNAL(connectionClosed()), this, SLOT(chatConnexionClosed()));
	connect(this, SIGNAL(readyRead()), this, SLOT(readSocket()));
	connect(this, SIGNAL(error(int)), this, SLOT(chatConnexionError(int)));
	connect(this, SIGNAL(connected()), this, SLOT(chatConnexionEtablished()));
	connect(&timeOut, SIGNAL(timeout()), this, SLOT(chatConnexionTimeout()));
	RzxClientListener::object()->attach(this);
}

///Construction d'un socket de chat lié à une fenêtre
RzxChatSocket::RzxChatSocket(const RzxHostAddress& s_host, RzxChat *parent)
	:QSocket(), host(s_host)
{
	chatWindow = parent;
	alone = false;
	connect(this, SIGNAL(connectionClosed()), this, SLOT(chatConnexionClosed()));
	connect(this, SIGNAL(readyRead()), this, SLOT(readSocket()));
	connect(this, SIGNAL(error(int)), this, SLOT(chatConnexionError(int)));
	connect(this, SIGNAL(connected()), this, SLOT(chatConnexionEtablished()));
	connect(&timeOut, SIGNAL(timeout()), this, SLOT(chatConnexionTimeout()));
	RzxClientListener::object()->attach(this);
}

///Construction d'un socket de chat sans liaison
RzxChatSocket::RzxChatSocket(const RzxHostAddress& s_host, bool s_alone)
	:QSocket(), host(s_host)
{
	chatWindow = NULL;
	alone = s_alone;
	connect(this, SIGNAL(connectionClosed()), this, SLOT(chatConnexionClosed()));
	connect(this, SIGNAL(readyRead()), this, SLOT(readSocket()));
	connect(this, SIGNAL(error(int)), this, SLOT(chatConnexionError(int)));
	connect(this, SIGNAL(connected()), this, SLOT(chatConnexionEtablished()));
	connect(&timeOut, SIGNAL(timeout()), this, SLOT(chatConnexionTimeout()));
	if(alone)
		connectToHost();
	RzxClientListener::object()->attach(this);
}

///Destruction d'un socket de chat
RzxChatSocket::~RzxChatSocket()
{
}

///Fermeture du socket
void RzxChatSocket::close()
{
	QSocket::close();
	if(chatWindow)
		chatWindow->setSocket(NULL);
	chatWindow = NULL;
	deleteLater();
}

///Liaison du socket à un chat
void RzxChatSocket::setParent(RzxChat *parent)
{
	chatWindow = parent;
}

///Connexion à l'hôte
void RzxChatSocket::connectToHost()
{
	QSocket::connectToHost(host.toString(), RzxConfig::chatPort());
	timeOut.start(10*1000); //descend le timeout de connexion à 10s
}

///Installation d'un socket
void RzxChatSocket::setSocket(int socket)
{
	QSocket::setSocket(socket);
	host = peerAddress();
}

////Parser des messages
/** C'est cette méthode qui va vraiment faire le tri entre un chat et une demande de propriété. Lorsqu'un chat est envoyé, le message est émis vers rzxrezal qui alors redonne le message à la bonne fenêtre de chat si elle existe, ou la crée dans le cas contraire */
int RzxChatSocket::parse(const QString& msg)
{
	int i = 0;
	QRegExp cmd;
	int fin = msg.find("\r\n");     //recherche de la fin du message
	if(fin == -1) return -1;
	
	while(DCCFormat[i])
	{
		cmd.setPattern(DCCFormat[i]);
		if(cmd.search(msg, 0) >= 0) 
		{
			switch(i) {
				case DCC_PROPQUERY:
					qDebug("Parsing PROPQUERY");
					sendProperties();
					return DCC_PROPQUERY;
					break;
				case DCC_PROPANSWER:
					qDebug("Parsing PROPANSWER: " + cmd.cap(2).replace('%',"%%"));
					if(cmd.cap(2).isEmpty())					// si il n'y a pas les donnees necessaires 
					{
						emit notify(tr("has send empty properties"));
						return DCC_PROPANSWER;		// ou que l'on n'a rien demande on s'arrete
					}
					if(!chatWindow)
						showProperties(host, cmd.cap(2));
					else
						chatWindow->receiveProperties(cmd.cap(2));
					if(alone)
						close();
					return DCC_PROPANSWER;
					break;
				case DCC_CHAT:
					qDebug("Parsing CHAT : " + cmd.cap(2).replace('%',"%%"));
					if(RzxConfig::autoResponder())
						sendResponder(RzxConfig::autoResponderMsg());
					if(RzxConfig::autoResponder() == RzxComputer::REP_REFUSE)
					{
						if(!chatWindow) deleteLater();
						return DCC_CHAT;
					}
					if(!chatWindow)
					{
						chatWindow = RzxConnectionLister::global()->chatCreate(this->peerAddress());
						if(!chatWindow) return DCC_CHAT;
						chatWindow->setSocket(this);
					}
					emit chat(cmd.cap(2));
					emit typing(false);
					return DCC_CHAT;
					break;
				case DCC_PING:
					sendPong();
					qDebug("Parsing PING");
					emit notify(tr("Ping received"));
					return DCC_PING;
					break;
				case DCC_PONG:
					emit pongReceived(pongTime.msecsTo(QTime::currentTime()));
					qDebug("Parsing PONG");
					return DCC_PONG;
					break;
				case DCC_TYPING:
					emit typing(cmd.cap(2)=="1");
					qDebug("Parsing TYPING");
					return DCC_TYPING;
					break;
			};
		}
		i++;
	}
	return -1;
}

/*Les méthodes qui suivent servent à l'émission des différents messages*/
///Envoi d'une demande de propriété
void RzxChatSocket::sendPropQuery() {
	send("PROPQUERY \r\n\0");
}

///Envoi d'une requête ping
void RzxChatSocket::sendPing()
{
	pongTime = QTime::currentTime();
	send("PING \r\n");
}

///Envoi d'une réponse pong
void RzxChatSocket::sendPong()
{
	send("PONG \r\n");
}

///Envoi de l'état de la frappe
void RzxChatSocket::sendTyping(bool state)
{
	QString msg = "TYPING ";
	msg += (state?"1":"0");
	send(msg + "\r\n");
}

///Formatage des propriétés de l'utilisateur
void RzxChatSocket::sendProperties()
{
	RzxHostAddress peer = peerAddress();
	
	QStringList strList;
	strList << tr("Surname") << RzxConfig::propName();
	strList << tr("First name") << RzxConfig::propLastName();
	strList << tr("Nick") << RzxConfig::propSurname();
	strList << tr("Phone") << RzxConfig::propTel();
	strList << tr("E-Mail") << RzxConfig::propMail();
	strList << tr("Web") << RzxConfig::propWebPage();
	strList << tr("Room") << RzxConfig::propCasert();
	strList << tr("Sport") << RzxConfig::propSport();
	strList << tr("Promo") << RzxConfig::propPromo();

	QString msg = strList.join("|");
	send("PROPANSWER " + msg + "\r\n\0");
	emit propertiesSent(peer);
}

///Emission d'un message de chat
/** Composition du message de chat envoyé par l'utilisateur, pour qu'il soit envoyé au correspondant
 * <br>Voir aussi : \ref sendDccChat()
 */
void RzxChatSocket::sendChat(const QString& msg)
{
	emit chatSent();
	sendDccChat(msg);
	sendTyping(false);
}

///Emission du message du répondeur automatique
/** Composition du message de chat indiquant que l'utilisateur est sur répondeur, pour qu'il soit envoyé au correspondant
 * <br>Voir aussi : \ref sendDccChat()
 */
void RzxChatSocket::sendResponder(const QString& msg)
{
	sendDccChat(msg);
}

///Envoi d'un message de chat
/** Met en forme un message de chat quelconque pour qu'il soit envoyé. Cette méthode ajoute la part lié au protocole au message.
 * <br>Il faut utiliser \ref sendChat() pour envoyer un chat.
 */
void RzxChatSocket::sendDccChat(const QString& msg) {
//	if( !valid ) return;

	send(QString("CHAT " + msg + "\r\n\0"));
}

///Emission d'un message vers un autre client
/** Envoie d'un message QUI DOIT AVOIR ETE FORMATE AUPARAVANT par le socket défini.*/
void RzxChatSocket::send(const QString& msg)
{
	switch(state())
	{
		case Connected:
			if(writeBlock(msg.latin1(), (msg.length())) == -1)
			{
				qDebug("Impossible d'émettre les données vers ");
				emit info(tr("Unable to send data... writeBlock returns -1"));
			}
			else
			{
				flush();
				qDebug("Message envoyé : " + msg.left(msg.length()-2).replace('%', "%%"));
			}
			return;
			
		case Idle:
			connectToHost();
		default:
			tmpChat += msg;
			return;
	}
}

///Lecture d'un message en attente sur le socket sock
/** Cette méthode lit un message envoyé sur un socket particulier et balance directement vers le parser pour que le message soit interprété*/
int RzxChatSocket::readSocket()
{
	QString msg;
	int p = -1;

	if(!canReadLine()) return -1;

	while(canReadLine())
	{
		msg = readLine();
		if(msg.find("\r\n") != -1)
		{
			if(p == -1) p = parse(msg);
		}
	}
	return p;
}

/** Emission du message lorsque la connexion est établie */
void RzxChatSocket::chatConnexionEtablished()
{
	qDebug("Socket ouvert vers " + host.toString() + "... envoi du message");
	if(alone)
		sendPropQuery();
	else if(tmpChat)
	{
		send(tmpChat);
		tmpChat = QString::null;
	}
	timeOut.stop();
}

/**La connexion a été fermée (sans doute par fermeture de la fenêtre de chat) on l'indique à l'utilisateur */
void RzxChatSocket::chatConnexionClosed()
{
	emit info(tr("ends the chat"));
	qDebug("Connection with " + host.toString() + " closed by peer");
	close();
}

/** Gestion des erreurs lors de la connexion et de la communication chaque erreur donne lieu a une mise en garde de l'utilisateur*/
void RzxChatSocket::chatConnexionError(int Error)
{
	switch(Error)
	{
		case ErrConnectionRefused:
			emit info(tr("can't be contact, check his firewall... CONNECTION ERROR"));
			qDebug("Connexion has been refused by the client");
			close();
			break;
		case ErrHostNotFound:
			emit info(tr("can't be found... CONNECTION ERROR"));
			qDebug("Can't find client");
			close();
			break;
		case ErrSocketRead:
			emit info(tr("has sent datas which can't be read... CONNECTION ERROR"));
			qDebug("Error while reading datas");
			break;
	}
	if(timeOut.isActive()) timeOut.stop();
}

//Cas où la connexion n'a pas pu être établie dans les délais
void RzxChatSocket::chatConnexionTimeout()
{
	chatConnexionError(ErrConnectionRefused);
}

// affichage des proprietes d'un ordinateur
QWidget *RzxChatSocket::showProperties(const RzxHostAddress& peer, const QString& msg, bool withFrame, QWidget *parent, QPoint *pos )
{
	QWidget *propertiesDialog;
	RzxComputer *computer = RzxConnectionLister::global()->iplist.find(peer.toString());

	if(!computer)
		return NULL;

	if(withFrame)
	{
		// creation de la boite de dialogue (non modale, elle se detruit automatiquement grace a WDestructiveClose)
		propertiesDialog = new QDialog(parent?parent:QRezix::global(), "ClientProp", false, WDestructiveClose | WStyle_Customize | WStyle_Tool | WStyle_Title | WStyle_SysMenu);
		propertiesDialog->resize(300, 320);

		QPixmap iconeProg((const char **)q);
		iconeProg.setMask(iconeProg.createHeuristicMask() );
		propertiesDialog->setIcon(iconeProg);

	#ifdef WIN32
		propertiesDialog->setCaption( tr( "%1's properties" ).arg(computer->getName()) +" [Qt]");
	#else
		propertiesDialog->setCaption( tr( "%1's properties" ).arg(computer->getName()) );
	#endif
	}
	else
	{
		propertiesDialog = new RzxPopup(parent?parent:QRezix::global(), "ClientProp");
		((QFrame*)propertiesDialog) -> setFrameShape(QFrame::PopupPanel);
		((QFrame*)propertiesDialog) -> setFrameShadow(QFrame::Raised);
		if(pos) propertiesDialog->move(*pos);
	}

	// Layout, pour le resize libre
	QGridLayout * qPropertiesLayout = new QGridLayout(propertiesDialog);
	qPropertiesLayout->setSpacing(0);
	qPropertiesLayout->setMargin(withFrame?6:0);
 
	// creation de la liste des proprietes et ajout au layout
	QListView* PropList = new QListView(propertiesDialog, "PropView");
	QLabel *clientLabel = new QLabel(tr("xNet client : %1").arg(computer->getClient()), propertiesDialog, "ClientLabel");
	qPropertiesLayout->addWidget((QWidget*)PropList, 0, 0);
	qPropertiesLayout->addWidget((QWidget*)clientLabel, 300, 0);
 
	PropList->resize(300, 300);
	PropList->addColumn(tr("Property"), -1);
	PropList->addColumn(tr("Value"), -1);
	QScrollView::ScrollBarMode mode = QScrollView::AlwaysOff;
	PropList -> setHScrollBarMode(mode);
 
	QStringList props = QStringList::split('|', msg, true);
	// ajout des proprietes de la liste, sans tri.
	PropList->setSorting(-1,FALSE);
	QListViewItem* vi = NULL;
	int propCount = 0;
	
	for(QStringList::Iterator itItem = props.begin(); itItem != props.end(); itItem++)
	{
		QStringList::Iterator itLabel = itItem++;
		if(itItem == props.end()) break;
		if((*itLabel).length() && (*itItem).length())
		{
			vi = new QListViewItem(PropList, vi, (*itLabel), (*itItem));
			propCount++;
		}
	}
	if(!propCount)
	{
		propertiesDialog->deleteLater();
		return NULL;
	}

	propertiesDialog->raise();
	if(withFrame)
		propertiesDialog->show();
 
	// Fit de la fenetre, on ne le fait pas si il n'y a pas d'accents, sinon ca plante
	int width=PropList->columnWidth(0)+PropList->columnWidth(1)+4+12;
	int height=(PropList->childCount()+1)*(*PropList->firstChild()).height()+8+12+20; //+20 pour le client xnet
	propertiesDialog->resize(width,height);
	return propertiesDialog;
}

QWidget *RzxChatSocket::showHistorique( unsigned long ip, QString hostname, bool withFrame, QWidget *parent, QPoint *pos ){
	// chargement de l'historique
	QString filename = RzxConfig::historique(ip, hostname);
	if (filename.isNull())
		return NULL;
 
	QString text;
	QFile file(filename);
	if(!file.exists())
		return NULL;
	
	file.open(IO_ReadOnly); 
	QTextStream stream(&file);
	while(!stream.eof()) {
		text += stream.readLine();
	}
	file.close();
 
	// construction de la boite de dialogue
	QWidget *histoDialog;
	if(withFrame)
	{
		histoDialog = new QDialog(parent?parent:QRezix::global(), "Histo", false, WDestructiveClose | WStyle_Customize | Qt::WStyle_Tool | WStyle_Title | WStyle_SysMenu);
		QPixmap iconeProg((const char **)q);
		iconeProg.setMask(iconeProg.createHeuristicMask() );  
		histoDialog->setIcon(iconeProg);

	#ifdef WIN32
		histoDialog->setCaption( tr( "History - %1" ).arg(hostname) +" - [Qt]");
	#else
		histoDialog->setCaption( tr( "History - %1" ).arg(hostname) );
	#endif
	}
	else
	{
		histoDialog = new RzxPopup(parent?parent:QRezix::global(), "Histo");
		((QFrame*)histoDialog) -> setFrameShape(QFrame::PopupPanel);
		((QFrame*)histoDialog) -> setFrameShadow(QFrame::Raised);
		if(pos) 
		{
			QPoint ul = *pos;
			//if(ul + 300 > 
			histoDialog->move(ul);
		}
	}
	QGridLayout * qHistoLayout = new QGridLayout(histoDialog);
	qHistoLayout->setSpacing(0);
	qHistoLayout->setMargin(withFrame?6:0);


	// creation de la liste des proprietes et ajout au layout
	QTextView* histoView = new QTextView(histoDialog, "HistoView");
	qHistoLayout->addWidget((QWidget*)histoView, 0, 0);
	
	histoDialog->resize(300, 300);
	histoView -> setText(text);
	histoView -> scrollToBottom();
	
	histoDialog->raise();
	if(withFrame)
		histoDialog->show();
	return histoDialog;
}


RzxClientListener * RzxClientListener::globalObject = 0;
RzxClientListener* RzxClientListener::object() {
	if (!globalObject)
		globalObject = new RzxClientListener;
		
	return globalObject;
}


bool RzxClientListener::isValid( void ) const { return valid; }


RzxClientListener::RzxClientListener()
	: QObject(0, "Client"), listenSocket(QSocketDevice::Stream)
{
	valid = false;
	notify = NULL;
}

///Ouverture de l'écoute du port 5050
/** Ouverture du port tcp 5050 (par défaut) pour une écoute*/
bool RzxClientListener::listenOnPort(Q_UINT32 port) {
	valid = false;
	if( !listenSocket.isValid() ){
		qDebug("tcp socket not valid");
		return false;
	}

	if( !listenSocket.bind(QHostAddress(), port) ){
		qDebug("Could not bind to socket");
		return false;
	}
		
	listenSocket.setBlocking(false);
	listenSocket.setAddressReusable(false);
	if( !listenSocket.listen(50) ) //bon 50 pourquoi pas...
	{
		qDebug("Could not listen on listenSocket");
		return false;
	}

	valid = true;
	
	notify = new QSocketNotifier(listenSocket.socket(), QSocketNotifier::Read, this);
	notify -> setEnabled(true);
	connect(notify, SIGNAL(activated(int)), SLOT(socketRead(int)));
	return true;
}

RzxClientListener::~RzxClientListener(){
	if(notify)
		delete notify;
}

void RzxClientListener::close()
{
	listenSocket.close();
}

///Connexion d'un RzxChatSocket au reste du programme
void RzxClientListener::attach(RzxChatSocket *sock)
{
	connect(sock, SIGNAL(propertiesSent(const RzxHostAddress& )), this, SIGNAL(propertiesSent(const RzxHostAddress& )));
	connect(sock, SIGNAL(chatSent()), this, SIGNAL(chatSent()));
}

///Réception d'une connexion entrante
/**Ce slot est appelé dès que l'écoute enregistre une demande d'écriture sur le port tcp 5050
 *		-# On récupère le socket de la connexion avec le client
 *		-# On analyse les données envoyées jusqu'à obtenir un message 'compréhensible'
 *		-# On dispatch alors ce message sur les différentes possibilités (pour l'instant chat ou prop)
 */
void RzxClientListener::socketRead(int socket){
	QHostAddress host;
	RzxChatSocket *sock;
	
	// On sait jamais
	if( socket != listenSocket.socket() ) {
		qDebug("assertion socket!=listenSocket.socket() failed!");
		return;
	}

	sock = new RzxChatSocket();
	sock->setSocket(listenSocket.accept());
	host = sock->peerAddress();
	if(!RzxConfig::globalConfig()->ignoreList->find(host.toString())) 
		qDebug("Accept connexion to client " + host.toString());
	else {
		qDebug("Message from client "+ host.toString()+ " has been ignored");
		delete sock;
	}
}

///Demande des propriétés de manière indépendante
/** Crée un socket pour la demande des propriétés à l'utilisateur en face */
void RzxClientListener::checkProperty(const RzxHostAddress& host)
{
	connect(new RzxChatSocket(host, true), SIGNAL(info(const QString&)), this, SLOT(info(const QString&)));
}

///Permet l'affichage des messages d'erreur des socket conçus uniquement pour le check de propriétés
void RzxClientListener::info(const QString& msg)
{
	RzxMessageBox::information(NULL, tr("Connection error"), tr("An error occured while checking properties :\n") + msg);
}
