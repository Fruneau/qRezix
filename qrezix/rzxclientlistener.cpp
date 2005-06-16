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

#include <QSocketNotifier>
#include <QRegExp>
#include <QTcpSocket>
#include <QWidget>
#include <QFrame>
#include <QImage>
#include <QBitmap>
#include <QLabel>
#include <QMessageBox>
#include <QLayout>
#include <QApplication>
#include <QDir>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTextStream>
#include <QPixmap>
#include <QGridLayout>
#include <QTextCursor>

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
	:QTcpSocket(), host()
{
	alone = false;
	chatWindow = NULL;
	connect(this, SIGNAL(disconnected()), this, SLOT(chatConnexionClosed()));
	connect(this, SIGNAL(readyRead()), this, SLOT(readSocket()));
	connect(this, SIGNAL(error(SocketError)), this, SLOT(chatConnexionError(SocketError)));
	connect(this, SIGNAL(connected()), this, SLOT(chatConnexionEtablished()));
	connect(&timeOut, SIGNAL(timeout()), this, SLOT(chatConnexionTimeout()));
	RzxClientListener::global()->attach(this);
}

///Construction d'un socket de chat li� � une fen�tre
RzxChatSocket::RzxChatSocket(const RzxHostAddress& s_host, RzxChat *parent)
	:QTcpSocket(), host(s_host)
{
	chatWindow = parent;
	alone = false;
	connect(this, SIGNAL(disconnected()), this, SLOT(chatConnexionClosed()));
	connect(this, SIGNAL(readyRead()), this, SLOT(readSocket()));
	connect(this, SIGNAL(error(SocketError)), this, SLOT(chatConnexionError(SocketError)));
	connect(this, SIGNAL(connected()), this, SLOT(chatConnexionEtablished()));
	connect(&timeOut, SIGNAL(timeout()), this, SLOT(chatConnexionTimeout()));
	RzxClientListener::global()->attach(this);
}

///Construction d'un socket de chat sans liaison
RzxChatSocket::RzxChatSocket(const RzxHostAddress& s_host, bool s_alone)
	:QTcpSocket(), host(s_host)
{
	chatWindow = NULL;
	alone = s_alone;
	connect(this, SIGNAL(disconnected()), this, SLOT(chatConnexionClosed()));
	connect(this, SIGNAL(readyRead()), this, SLOT(readSocket()));
	connect(this, SIGNAL(error(SocketError)), this, SLOT(chatConnexionError(SocketError)));
	connect(this, SIGNAL(connected()), this, SLOT(chatConnexionEtablished()));
	connect(&timeOut, SIGNAL(timeout()), this, SLOT(chatConnexionTimeout()));
	if(alone)
		connectToHost();
	RzxClientListener::global()->attach(this);
}

///Destruction d'un socket de chat
RzxChatSocket::~RzxChatSocket()
{
}

///Fermeture du socket
void RzxChatSocket::close()
{
	QTcpSocket::close();
	if(chatWindow)
		chatWindow->setSocket(NULL);
	chatWindow = NULL;
	deleteLater();
}

///Liaison du socket � un chat
void RzxChatSocket::setParent(RzxChat *parent)
{
	chatWindow = parent;
}

///Connexion � l'h�te
void RzxChatSocket::connectToHost()
{
	QTcpSocket::connectToHost(host.toString(), RzxConfig::chatPort());
	timeOut.start(10*1000); //descend le timeout de connexion � 10s
}

///Installation d'un socket
void RzxChatSocket::setSocketDescriptor(int socket)
{
	QTcpSocket::setSocketDescriptor(socket);
	host = peerAddress();
}

////Parser des messages
/** C'est cette m�thode qui va vraiment faire le tri entre un chat et une demande de propri�t�. Lorsqu'un chat est envoy�, le message est �mis vers rzxrezal qui alors redonne le message � la bonne fen�tre de chat si elle existe, ou la cr�e dans le cas contraire */
int RzxChatSocket::parse(const QString& msg)
{
	int i = 0;
	QRegExp cmd;
	int fin = msg.find("\r\n");     //recherche de la fin du message
	if(fin == -1) return -1;
	
	while(!DCCFormat[i].isNull())
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

/*Les m�thodes qui suivent servent � l'�mission des diff�rents messages*/
///Envoi d'une demande de propri�t�
void RzxChatSocket::sendPropQuery() {
	send("PROPQUERY \r\n\0");
}

///Envoi d'une requ�te ping
void RzxChatSocket::sendPing()
{
	pongTime = QTime::currentTime();
	send("PING \r\n");
}

///Envoi d'une r�ponse pong
void RzxChatSocket::sendPong()
{
	send("PONG \r\n");
}

///Envoi de l'�tat de la frappe
void RzxChatSocket::sendTyping(bool state)
{
	QString msg = "TYPING ";
	msg += (state?"1":"0");
	send(msg + "\r\n");
}

///Formatage des propri�t�s de l'utilisateur
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
/** Composition du message de chat envoy� par l'utilisateur, pour qu'il soit envoy� au correspondant
 * <br>Voir aussi : \ref sendDccChat()
 */
void RzxChatSocket::sendChat(const QString& msg)
{
	emit chatSent();
	sendDccChat(msg);
	sendTyping(false);
}

///Emission du message du r�pondeur automatique
/** Composition du message de chat indiquant que l'utilisateur est sur r�pondeur, pour qu'il soit envoy� au correspondant
 * <br>Voir aussi : \ref sendDccChat()
 */
void RzxChatSocket::sendResponder(const QString& msg)
{
	sendDccChat(msg);
}

///Envoi d'un message de chat
/** Met en forme un message de chat quelconque pour qu'il soit envoy�. Cette m�thode ajoute la part li� au protocole au message.
 * <br>Il faut utiliser \ref sendChat() pour envoyer un chat.
 */
void RzxChatSocket::sendDccChat(const QString& msg) {
//	if( !valid ) return;

	send(QString("CHAT " + msg + "\r\n\0"));
}

///Emission d'un message vers un autre client
/** Envoie d'un message QUI DOIT AVOIR ETE FORMATE AUPARAVANT par le socket d�fini.*/
void RzxChatSocket::send(const QString& msg)
{
	switch(state())
	{
		case Connected:
			if(writeBlock(msg.latin1(), (msg.length())) == -1)
			{
				qDebug("Impossible d'�mettre les donn�es vers ");
				emit info(tr("Unable to send data... writeBlock returns -1"));
			}
			else
			{
				flush();
				qDebug("Message envoy� : " + msg.left(msg.length()-2).replace('%', "%%"));
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
/** Cette m�thode lit un message envoy� sur un socket particulier et balance directement vers le parser pour que le message soit interpr�t�*/
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

/** Emission du message lorsque la connexion est �tablie */
void RzxChatSocket::chatConnexionEtablished()
{
	qDebug("Socket ouvert vers " + host.toString() + "... envoi du message");
	if(alone)
		sendPropQuery();
	else if(!tmpChat.isNull())
	{
		send(tmpChat);
		tmpChat = QString::null;
	}
	timeOut.stop();
}

/**La connexion a �t� ferm�e (sans doute par fermeture de la fen�tre de chat) on l'indique � l'utilisateur */
void RzxChatSocket::chatConnexionClosed()
{
	if(!alone) emit info(tr("ends the chat"));
	qDebug("Connection with " + host.toString() + " closed by peer");
	close();
}

/** Gestion des erreurs lors de la connexion et de la communication chaque erreur donne lieu a une mise en garde de l'utilisateur*/
void RzxChatSocket::chatConnexionError(SocketError error)
{
	switch(error)
	{
		case ConnectionRefusedError:
			emit info(tr("can't be contact, check his firewall... CONNECTION ERROR"));
			qDebug("Connexion has been refused by the client");
			close();
			break;
		case HostNotFoundError:
			emit info(tr("can't be found... CONNECTION ERROR"));
			qDebug("Can't find client");
			close();
			break;
		case QAbstractSocket::RemoteHostClosedError:
			break;
//		case SocketRead:
		default:
			emit info(tr("has sent datas which can't be read... CONNECTION ERROR"));
			qDebug("Error while reading datas " + QString::number(error));
			break;
	}
	if(timeOut.isActive()) timeOut.stop();
}

//Cas o� la connexion n'a pas pu �tre �tablie dans les d�lais
void RzxChatSocket::chatConnexionTimeout()
{
	chatConnexionError(SocketTimeoutError);
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
		propertiesDialog = new QDialog(parent?parent:QRezix::global(), Qt::Tool | Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
		propertiesDialog->setAttribute(Qt::WA_DeleteOnClose);
		propertiesDialog->resize(300, 320);

		QPixmap iconeProg((const char **)q);
		iconeProg.setMask(iconeProg.createHeuristicMask() );
		propertiesDialog->setIcon(iconeProg);

		propertiesDialog->setWindowTitle( tr( "%1's properties" ).arg(computer->getName()) );
	}
	else
	{
		propertiesDialog = new RzxPopup(parent?parent:QRezix::global());
		((QFrame*)propertiesDialog)->setFrameShape(QFrame::PopupPanel);
		((QFrame*)propertiesDialog)->setFrameShadow(QFrame::Raised);
		if(pos) propertiesDialog->move(*pos);
	}

	// Layout, pour le resize libre
	QGridLayout * qPropertiesLayout = new QGridLayout(propertiesDialog);
	qPropertiesLayout->setSpacing(0);
	qPropertiesLayout->setMargin(withFrame?6:0);
 
	// creation de la liste des proprietes et ajout au layout
	QTreeWidget* propList = new QTreeWidget();
	QLabel *clientLabel = new QLabel(tr("xNet client : %1").arg(computer->getClient()));
	qPropertiesLayout->addWidget(propList, 0, 0);
	qPropertiesLayout->addWidget(clientLabel, 300, 0);
 
	propList->setPaletteBackgroundColor(QColor(255,255,255));
	propList->resize(300, 300);
	propList->setColumnCount(2);
	propList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	propList->setSortingEnabled(false);
	
	// Cr�ation des en-t�tes de colonnes
	QTreeWidgetItem *item = new QTreeWidgetItem();
	item->setText(0, tr("Property"));
	item->setText(1, tr("Value"));
	propList->setHeaderItem(item);

	// Remplissage
	QStringList props = QStringList::split('|', msg, true);
	int propCount = 0;
	item = NULL;
	for(int i = 0 ; i < props.size() - 1 ; i+=2)
	{
		item = new QTreeWidgetItem(propList, item);
		item->setText(0, props[i]);
		item->setText(1, props[i+1]);
		propCount++;
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
//	int width=PropList->columnWidth(0)+PropList->columnWidth(1)+4+12;
	int height=(propCount+3)*20; //+20 pour le client xnet, et puis headers e un peu de marge
	propertiesDialog->resize(200,height);
	RzxConfig::addCache(peer,msg);
	return propertiesDialog;
}

QWidget *RzxChatSocket::showHistorique( unsigned long ip, const QString& hostname, bool withFrame, QWidget *parent, QPoint *pos ){
	// chargement de l'historique
	QString filename = RzxConfig::historique(ip, hostname);
	if (filename.isNull())
		return NULL;
 
	QString text;
	QFile file(filename);
	if(!file.exists())
		return NULL;
	
	file.open(QIODevice::ReadOnly); 
	QTextStream stream(&file);
	while(!stream.atEnd()) {
		text += stream.readLine();
	}
	file.close();
 
	// construction de la boite de dialogue
	QWidget *histoDialog;
	if(withFrame)
	{
		histoDialog = new QDialog(parent?parent:QRezix::global(), Qt::Tool | Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
		histoDialog->setAttribute(Qt::WA_DeleteOnClose);
		QPixmap iconeProg((const char **)q);
		iconeProg.setMask(iconeProg.createHeuristicMask() );  
		histoDialog->setIcon(iconeProg);

		histoDialog->setWindowTitle( tr( "History - %1" ).arg(hostname) );
	}
	else
	{
		histoDialog = new RzxPopup(parent?parent:QRezix::global());
		((QFrame*)histoDialog)->setFrameShape(QFrame::PopupPanel);
		((QFrame*)histoDialog)->setFrameShadow(QFrame::Raised);
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
	
	histoDialog->resize(300, 300);
	histoView -> setPaletteBackgroundColor(QColor(255,255,255));
	histoView -> setHtml(text);
        histoView->textCursor().movePosition(QTextCursor::End);
        histoView->ensureCursorVisible();
	
	histoDialog->raise();
	if(withFrame)
		histoDialog->show();
	return histoDialog;
}


RzxClientListener * RzxClientListener::object = 0;

RzxClientListener::RzxClientListener()
{
	if(!object) object = this;
	connect(this, SIGNAL(newConnection()), this, SLOT(socketRead()));
}

///Connexion d'un RzxChatSocket au reste du programme
void RzxClientListener::attach(RzxChatSocket *sock)
{
	connect(sock, SIGNAL(propertiesSent(const RzxHostAddress& )), this, SIGNAL(propertiesSent(const RzxHostAddress& )));
	connect(sock, SIGNAL(chatSent()), this, SIGNAL(chatSent()));
}

///R�ception d'une connexion entrante
/**Ce slot est appel� d�s que l'�coute enregistre une demande d'�criture sur le port tcp 5050
 *		-# On r�cup�re le socket de la connexion avec le client
 *		-# On analyse les donn�es envoy�es jusqu'� obtenir un message 'compr�hensible'
 *		-# On dispatch alors ce message sur les diff�rentes possibilit�s (pour l'instant chat ou prop)
 */
void RzxClientListener::socketRead() {
	//R�cup�ration de la connexion
	// On v�rifie au passage que la connexion est valide
	QHostAddress host;
	QTcpSocket *rawSocket = nextPendingConnection();
	if(rawSocket == NULL) {
		qDebug("There is no socket to be connected");
		return;
	}
	host = rawSocket->peerAddress();
        if(!RzxConfig::globalConfig()->isBan(host.toString())) 
                qDebug("Accept connexion to client " + host.toString());
        else {
                qDebug("Message from client "+ host.toString()+ " has been ignored");
		rawSocket->abort();
		delete rawSocket;
		return;
	}
						
	//Construction du chat d'accueil
	RzxChatSocket *sock;
	sock = new RzxChatSocket();
	sock->setSocket(rawSocket->socketDescriptor());
}

///Demande des propri�t�s de mani�re ind�pendante
/** Cr�e un socket pour la demande des propri�t�s � l'utilisateur en face */
void RzxClientListener::checkProperty(const RzxHostAddress& host)
{
	connect(new RzxChatSocket(host, true), SIGNAL(info(const QString&)), this, SLOT(info(const QString&)));
}

///Permet l'affichage des messages d'erreur des socket con�us uniquement pour le check de propri�t�s
void RzxClientListener::info(const QString& msg)
{
	RzxMessageBox::information(QRezix::global(), tr("Connection error"), tr("An error occured while checking properties :\n") + msg);
}
