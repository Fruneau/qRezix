/***************************************************************************
                          rzxfilesocket  -  description
                             -------------------
    begin                : Mon Jan 30 2006
    copyright            : (C) 2006 by Guillaume Bandet
    email                : guillaume.bandet@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QRegExp>

#include <RzxConfig>
#include <RzxComputer>
#include <RzxApplication>
#include <RzxMessageBox>
#include <QFileDialog>
#include <QTime>

#include "rzxchatsocket.h"

#include "rzxchat.h"
#include "rzxfilelistener.h"
#include "rzxchatlister.h"
#include "rzxchatconfig.h"

#include <QDebug>
QString RzxFileSocket::fileFormat[] = {
	"^CANCEL\r\n",
	"^CHECK\r\n",
	"^OK\r\n",
	"^NOK\r\n",
	"^PROPOSE ([^|]+)\\|(\\d+)",  //nom taille
	"^ACCEPT\r\n",
	"^REJECT\r\n",
	"^SEND (\\d+)\r\n",
	"^SENDOK\r\n",
	"^END\r\n",
	0
};



///Construction d'un socket brut
RzxFileSocket::RzxFileSocket()
	:QTcpSocket(), host(), file(0),chatWindow(0), octetsEcrits(0), modeBinaire(false)
{
	connect(this, SIGNAL(disconnected()), this, SLOT(fileConnexionClosed()));
	connect(this, SIGNAL(readyRead()), this, SLOT(readSocket()));
	connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(fileConnexionError(QAbstractSocket::SocketError)));
	connect(this, SIGNAL(connected()), this, SLOT(fileConnexionEtablished()));
	connect(&timeOut, SIGNAL(timeout()), this, SLOT(fileConnexionTimeout()));
	connect(&checkTimeOut, SIGNAL(timeout()), this, SLOT(checkTimeout()));
	fileState = STATE_NONE;
}

///Construction d'un socket de transfert de fichier sans liaison
RzxFileSocket::RzxFileSocket(RzxComputer *c)
	:QTcpSocket(), host(c->ip()), file(0),chatWindow(0), octetsEcrits(0), modeBinaire(false)
{
	connect(this, SIGNAL(disconnected()), this, SLOT(fileConnexionClosed()));
	connect(this, SIGNAL(readyRead()), this, SLOT(readSocket()));
	connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(fileConnexionError(QAbstractSocket::SocketError)));
	connect(this, SIGNAL(connected()), this, SLOT(fileConnexionEtablished()));
	connect(&timeOut, SIGNAL(timeout()), this, SLOT(fileConnexionTimeout()));
	connect(&checkTimeOut, SIGNAL(timeout()), this, SLOT(checkTimeout()));
	fileState = STATE_NONE;
	connectToHost();
	sockId = RzxChatLister::global()->fileListener()->attach(this);
}

///Destruction d'un socket de transfert de fichier
RzxFileSocket::~RzxFileSocket()
{ }

///Fermeture du socket
void RzxFileSocket::close()
{
	QTcpSocket::close();
	if(widget && chatWindow)
	{
		chatWindow->removeWidget(widget);
		widget->deleteLater();
	}
	if(file)
	{
		file->close();
		delete file;
		file =0;
	}
	RzxChatLister::global()->fileListener()->detach(this);
	deleteLater();
}

///Connexion à l'hôte
void RzxFileSocket::connectToHost()
{
	QTcpSocket::connectToHost(host.toString(), RzxChatConfig::filePort());
	timeOut.start(10*1000); //descend le timeout de connexion à 10s
}

///Installation d'un socket
void RzxFileSocket::setSocketDescriptor(int socket)
{
	QTcpSocket::setSocketDescriptor(socket);
	host = peerAddress();
	sockId = RzxChatLister::global()->fileListener()->attach(this);
}

///Parser des messages
/** va trier les différents messages du protocole.*/
void RzxFileSocket::parse(const QString& msg)
{
	QRegExp cmd;
	int typeMsg = -1;
	int i =0;
	while(typeMsg == -1 && !fileFormat[i].isNull())
	{
		cmd.setPattern(fileFormat[i]);
		if(cmd.indexIn(msg) != -1) 
			typeMsg = i;
		i++;
	}
	switch (typeMsg)
	{
	case FILE_CANCEL:
		host.computer()->receiveChat(Rzx::File , "Envoi annulé");
		close();
		break;

	case FILE_CHECK:
		if(!envoi && fileState == STATE_NONE)
		{	
			if(!RzxChatConfig::refuseFile())
			{
				sendOk();
				fileState = STATE_CHECKED;
			}
			else
			{
				sendNok();
				close();
			}
		}
		break;

	case FILE_OK:
		if(envoi && fileState == STATE_CHECKING)
		{
			checkTimeOut.stop();
			fileState = STATE_PROPOSING;
			host.computer()->receiveChat(Rzx::File , tr("Sending %1, waiting for acceptation...").arg(nom));
			sendPropose(nom.split("/").last(), taille);
		}
		break;

	case FILE_NOK:
		if(envoi && fileState == STATE_CHECKING)
		{
			checkTimeOut.stop();
			host.computer()->receiveChat(Rzx::File , tr("Your friend refused the file."));
			close();
		}
		break;

	case FILE_PROPOSE:
		if(!envoi && fileState == STATE_CHECKED)
		{
			nom = cmd.cap(1);
			taille = cmd.cap(2).toULongLong();
			host.computer()->receiveChat(Rzx::File , tr("Your friend sends you the file <a href=\"RzxTransfer://%1\">%2</a> (%3 bytes), do you want to download it?").arg(sockId).arg(nom).arg(taille));
			chatWindow = RzxChatLister::global()->receiveFile(host);
			//creation du widget
			widget = new RzxFileWidget(0, nom, 0, false);
			chatWindow->addWidget(widget);	//ajoute le widget à la fenetre de chat

			//connexions
			connect(widget,SIGNAL(cancelClicked()),this,SLOT(btnCancel()));
			connect(widget,SIGNAL(acceptClicked()),this,SLOT(btnAccept()));
			connect(widget,SIGNAL(rejectClicked()),this,SLOT(btnReject()));
		}
		break;

	case FILE_ACCEPT:
		if(envoi && fileState == STATE_PROPOSING)
		{
			sendBinary();
		}
		break;

	case FILE_REJECT:
		if(envoi && fileState == STATE_PROPOSING)
		{
			host.computer()->receiveChat(Rzx::File , tr("Your friend has refused your file"));
			close();
		}
		break;

	case FILE_SEND:
		if(!envoi && fileState == STATE_RECEIVING)
		{
			octetsALire = cmd.cap(1).toUInt();
			if(!octetsALire)
				return;
			modeBinaire = true; 
		}
		break;

	case FILE_SENDOK:
		if(envoi && fileState == STATE_SENDING)
		{
			sendBinary();
		}
		break;

	case FILE_END:
		file->close();
		if(octetsEcrits == taille)
		{
			qDebug() << "Le transfert de " << nom << "a reussi";
			host.computer()->receiveChat(Rzx::File , tr("File transfer finished!"));
		}
		else
		{
			qDebug() << "Le fichier recu" << nom << "a la taille" << octetsEcrits << "au lieu de" << taille;
			host.computer()->receiveChat(Rzx::File , tr("An error occured in the transfer, the file size mismatch the original one."));
		}
		close();
		break;
	default:
		qDebug() << "Erreur du transfert de fichier: Cas par défaut atteint avec le message :" << msg;
	}	
}

/*Les méthodes qui suivent servent à l'émission des différents messages*/

void RzxFileSocket::sendCheck()
{
	checkTimeOut.start(3000);	//time out de 3 secondes pour vérifier que le client supporte le transfert de fichier
	send("CHECK\r\n");
}

void RzxFileSocket::sendCancel()
{
	send("CANCEL\r\n");
}

void RzxFileSocket::sendOk()
{	
	send("OK\r\n");
}

void RzxFileSocket::sendNok()
{
	send("NOK\r\n");
}

void RzxFileSocket::sendBinaryOk()
{
	send("SENDOK\r\n");
}


void RzxFileSocket::sendPropose(QString nom, quint64 taille)
{
	QString str = "PROPOSE ";
	str += nom;
	str += QString("|%1").arg(taille); 
	send(str+"\r\n");
}

void RzxFileSocket::sendAccept()
{
	send("ACCEPT\r\n");
}

void RzxFileSocket::sendReject()
{
	send("REJECT\r\n");
}

void RzxFileSocket::sendBinary()
{
	if(fileState != STATE_SENDING)
	{
		if(!file->open(QIODevice::ReadOnly))
		{
			host.computer()->receiveChat(Rzx::File , "Ouverture impossible");
			sendCancel();
			close();
			return;
		}
		fileState = STATE_SENDING;
		tempsTransfert.start();
	}
	if(file->bytesAvailable()>0)
	{
		QByteArray data = file->read(2048);
		send(QString("SEND %1\r\n").arg(data.size()));
		write(data);
		octetsEcrits += data.size();
		if (taille != 0)
			widget->setValue(100 * octetsEcrits/taille);
		if(octetsEcrits)
		{
			int msEcoulees = tempsTransfert.elapsed();
			int msRestant = (int)(((double)(taille - octetsEcrits) /(double)octetsEcrits) * msEcoulees);
			QTime tZero(0,0);
			QString format = tr("'remains: '");
			if(msRestant >= 3600000)
				format+= "hh'h'm";
			format+= "m'm'ss's'";
			widget->setInfo(tZero.addMSecs(msRestant).toString(format));
		}
	}
	else
        sendEnd();
}

void RzxFileSocket::sendEnd()
{
	send("END\r\n");
	close();
}

///Emission d'un message vers un autre client
/** Envoie d'un message QUI DOIT AVOIR ETE FORMATE AUPARAVANT par le socket défini.*/
//// reimplementer tout ca
void RzxFileSocket::send(const QString& msg)
{
	QString message = tmpMsg + msg;
	switch(state())
	{
		case QAbstractSocket::ConnectedState:
			tmpMsg ="";
			if(write(message.toLatin1()) == -1)
			{
				qDebug("Impossible d'émettre les données vers ");
				host.computer()->receiveChat(Rzx::InfoMessage, tr("Unable to send data... writeBlock returns -1"));
			}
			else
				flush();
			return;
			
		case QAbstractSocket::UnconnectedState:
			connectToHost();	
		default:
			tmpMsg += msg;
			return;
	}
}

///Lecture d'un message en attente sur le socket sock
/** Cette méthode lit un message envoyé sur un socket particulier et balance directement vers le parser pour que le message soit interprété*/
void RzxFileSocket::readSocket()
{
	while(bytesAvailable())
	{
		if (modeBinaire)
		{
			QByteArray data = read(octetsALire);
			octetsALire -= data.size();
			if(octetsALire <=0)
			{
				modeBinaire = false;
				sendBinaryOk();
			}
			file->write(data);
			octetsEcrits += data.size();
			if (taille!=0)
				widget->setValue(100 * octetsEcrits/taille);

			if(octetsEcrits)
			{
				int msEcoulees = tempsTransfert.elapsed();
				int msRestant = (int)(((double)(taille - octetsEcrits) /(double)octetsEcrits) * msEcoulees);
				QTime tZero(0,0);
				QString format = tr("'remains: '");
				if(msRestant >= 3600000)
					format+= "hh'h'm";
				format+= "m'm'ss's'";
				widget->setInfo(tZero.addMSecs(msRestant).toString(format));
			}
		}
		else
		{
			QString msg;
			if(canReadLine())
			{
				msg = readLine();
				if(msg.contains("\r\n"))
					parse(msg);
			}
		}
	}
}

/** Emission de la demande de controle lorsque la connexion est établie */
void RzxFileSocket::fileConnexionEtablished()
{
	timeOut.stop();
	host = peerAddress();
	send(""); // assure que les messages envoyé avant la connexion n'attendent pas trop
	qDebug("Socket ouvert vers %s...", host.toString().toAscii().constData());
}

/**La connexion a été fermée on l'indique à l'utilisateur */
void RzxFileSocket::fileConnexionClosed()
{
	host.computer()->receiveChat(Rzx::InfoMessage, tr("Connexion closed."));
	qDebug("File connection with %s closed by peer", host.toString().toAscii().constData());
	close();
}

/** Gestion des erreurs lors de la connexion et de la communication chaque erreur donne lieu a une mise en garde de l'utilisateur*/
void RzxFileSocket::fileConnexionError(QAbstractSocket::SocketError error)
{
	switch(error)
	{
		case ConnectionRefusedError:
			host.computer()->receiveChat(Rzx::InfoMessage, tr("File connection error: connection refused"));
			qDebug("File connection has been refused by the client");
			close();
			break;
		case HostNotFoundError:
			host.computer()->receiveChat(Rzx::InfoMessage, tr("File connection error: host not found"));
			qDebug("Can't find client");
			close();
			break;
		case QAbstractSocket::SocketTimeoutError:
			host.computer()->receiveChat(Rzx::InfoMessage, tr("File connection error: timeout"));
			qDebug("File connection timeout");
			close();
			break;
		case QAbstractSocket::RemoteHostClosedError:
			break;
		default:
			host.computer()->receiveChat(Rzx::InfoMessage, tr("File connection error: data corruption"));
			qDebug("Error while reading datas %d", error);
			break;
	}
	if(timeOut.isActive()) timeOut.stop();
}

///Cas où la connexion n'a pas pu être établie dans les délais
void RzxFileSocket::fileConnexionTimeout()
{
	fileConnexionError(SocketTimeoutError);
}

void RzxFileSocket::checkTimeout()
{
	host.computer()->receiveChat(Rzx::File, tr("Your correspondant's client doesn't seem to support file transfer."));
	sendCancel();
	close();
}

void RzxFileSocket::sendFile(QString filename)
{
	nom = filename;
	file = new QFile(filename);
	taille = file->size();
	qDebug() << nom << "  " << taille;
	envoi = true;
	fileState = STATE_CHECKING;
	sendCheck();
}

void RzxFileSocket::btnAccept()
{

	QString filename = QFileDialog::getSaveFileName(0, tr("Path for the file"), nom);  //A remettre plus tard
	if(filename == "")
		return;
	file = new QFile(filename);
	if(!file)
	{
		qDebug() << "QFile impossible a créer";
		RzxMessageBox::warning(0, tr("File creation impossible"), tr("The creation of the file %1 is impossible, check the rights or the free space.").arg(filename));
		return;
	}
	if(!file->open(QIODevice::WriteOnly))
	{
		qDebug() << "Impossible de créer le fichier.";
		RzxMessageBox::warning(0, tr("File creation impossible"), tr("The creation of the file %1 is impossible, check the rights or the free space.").arg(filename));
		return;
	}
	//au cas où il y a eu renommage
	nom = filename.split("/").last();
	widget->setTitle(nom);

	widget->setModeCancel();
	fileState = STATE_RECEIVING;
	tempsTransfert.start();
	sendAccept();
}

void RzxFileSocket::btnReject()
{
	host.computer()->receiveChat(Rzx::File , tr("You have refused the file"));
	sendReject();
	close();
}

void RzxFileSocket::btnCancel()
{
	host.computer()->receiveChat(Rzx::File, tr("You have cancelled the tranfer."));
	sendCancel();
	close();
}
