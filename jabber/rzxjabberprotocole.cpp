/***************************************************************************
                          rzxprotocole.cpp  -  description
                             -------------------
    begin                : Fri Jan 25 2002
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
#include <QStringList>
#include <QRegExp>
#include <QLineEdit>
#include <QPushButton>
#include <QIcon>
#include <QRegExpValidator>
#include <QPixmap>
#include <QImage>
#include <QDialog>
#include <QListWidget>

#include <RzxComputer>
#include <RzxIconCollection>
#include <RzxWrongPass>
#include <RzxChangePass>

#include "rzxjabberprotocole.h"
#include "rzxjabberconfig.h"
using namespace gloox;

RZX_NETWORK_EXPORT(RzxJabberProtocole)
RZX_CONFIG_INIT(RzxJabberConfig)

///Construction... RAS
RzxJabberProtocole::RzxJabberProtocole()
	: RzxNetwork("Jabber", "Native support for the jabber protocole",0,0,1,"svn")
{
	beginLoading();
	ui = NULL;
	propWidget = NULL;
	
	new RzxJabberConfig(this);
	
	client = new RzxJabberClient(this);
	connect(client, SIGNAL(presence(QString, QString, int)), this, SLOT(presenceRequest(QString, QString, int)));
	connect(client, SIGNAL(msgReceived(QString, QString)), this, SLOT(sendMsg(QString,QString))); /// @todo gérer l'envoi à la fenetre de chat quand ce sera en place)
	connect(client, SIGNAL(connected()), this, SLOT(connection()));
	connect(client, SIGNAL(disconnected()), this, SLOT(deconnection()));
	connect(client, SIGNAL(rosterUpdated()), this, SLOT(buildRosterList()));
	setIcon(RzxThemedIcon(Rzx::ICON_NETWORK));
	endLoading();
}

///Destruction...
RzxJabberProtocole::~RzxJabberProtocole(){
	beginClosing();
	delete RzxJabberConfig::global();
	delete client;
	endClosing();
}

void RzxJabberProtocole::connection(){
	emit connected(this);
	emit status("Connection jabber");
}

void RzxJabberProtocole::deconnection(){
	emit disconnected(this);
	emit status("deconnection jabber");
}

/****************************************************************************
* FEN�RE DE PROPRI��
*/

/** \reimp */
QList<QWidget*> RzxJabberProtocole::propWidgets()
{
	if(!ui)
		ui = new Ui::RzxJabberPropUI;
	if(!propWidget)
	{
		propWidget = new QWidget;
		ui->setupUi(propWidget);
		connect(ui->rosterList,SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)),this, SLOT(changeRosterText(QListWidgetItem *,QListWidgetItem *)));
		connect(ui->rosterAddButton,SIGNAL(clicked()),this, SLOT(addRosterItem()));
		connect(ui->rosterDeleteButton,SIGNAL(clicked()),this, SLOT(removeRosterItem()));
	}
	return QList<QWidget*>() << propWidget;
}

void RzxJabberProtocole::changeRosterText(QListWidgetItem *cur,QListWidgetItem *old){
	if(cur)
		ui->rosterEdit->setText(cur->text());
};

void RzxJabberProtocole::addRosterItem(){
	StringList l;
	if(!ui->rosterEdit->text().isEmpty()){
		client->client()->rosterManager()->subscribe(ui->rosterEdit->text().toStdString()," ",l,"User added");
	}
};

void RzxJabberProtocole::removeRosterItem(){
	if(!ui->rosterEdit->text().isEmpty()){
		client->client()->rosterManager()->unsubscribe(ui->rosterEdit->text().toStdString(),"User removed",true);
	}
};

void RzxJabberProtocole::buildRosterList(){
	// remplissage de la liste des Roster
	ui->rosterList->clear();
	ui->rosterEdit->setText("");
	std::map<std::string, RosterItem*>::const_iterator i;
	for( i = client->client()->rosterManager()->roster()->begin() ; i !=client->client()->rosterManager()->roster()->end() ; i++){
		ui->rosterList->addItem(QString::fromStdString((*i).first));
	}
	ui->rosterList->sortItems();
};

/** \reimp */
QStringList RzxJabberProtocole::propWidgetsName()
{
	return QStringList() << name();
}

/** \reimp */
void RzxJabberProtocole::propInit(bool def)
{
	ui->server_name->setText( RzxJabberConfig::serverName() );
	ui->user->setText( RzxJabberConfig::user() );
	ui->pass->setText( RzxJabberConfig::pass() );
	ui->server_port->setValue( RzxJabberConfig::serverPort() );
	ui->reconnection->setValue( RzxJabberConfig::reconnection() / 1000 );
	ui->ping_timeout->setValue( RzxJabberConfig::pingTimeout() / 1000 );
	
	buildRosterList();
}

/** \reimp */
void RzxJabberProtocole::propUpdate()
{
	if(!ui) return;

	bool restart =  false;

	if(ui->server_name->text() != RzxJabberConfig::serverName() || ui->server_port->value() != RzxJabberConfig::serverPort() || ui->user->text() != RzxJabberConfig::user() || ui->pass->text() != RzxJabberConfig::pass())
	{
		restart=true;
	}

	RzxJabberConfig::setServerName(ui->server_name->text() );
	RzxJabberConfig::setServerPort(ui->server_port->value() );
	RzxJabberConfig::setUser(ui->user->text() );
	RzxJabberConfig::setPass(ui->pass->text() );
	RzxJabberConfig::setReconnection(ui->reconnection->value() * 1000 );
	RzxJabberConfig::setPingTimeout(ui->ping_timeout->value() * 1000 );
	
	if(restart){
		stop();
		start();
	}
}

/** \reimp */
void RzxJabberProtocole::propClose()
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

bool RzxJabberProtocole::isStarted() const
{
	return client->isRunning() && client->isStarted();
}

void RzxJabberProtocole::start() {
	client->start();
}

void RzxJabberProtocole::stop() {
	client->stop();
	client->wait(1000);
}


void RzxJabberProtocole::presenceRequest(QString jid, QString name, int type) {
	struct options_t
	{
	unsigned Server                 :6;
	unsigned SysEx                          :3;     //0=Inconnu, 1=Win9X, 2=WinNT, 3=Linux, 4=MacOS, 5=MacOS X, 6=BSD
	unsigned Promo                          :2; //0 = Orange, 1=Jne, 2=Rouje (Chica la rouje ! <== bah nan, �la j�e !!!)
	unsigned Repondeur              :2; //0=accepter, 1= repondeur, 2=refuser les messages, 3= unused
	// total 13 bits / 32
	unsigned Capabilities   :19;
	};
	options_t opt;

	RzxJabberComputer newComputer(jid, name, computerList.size());

	if(type > 0 && computerList.contains(newComputer.jid())){
		newComputer.setIp(computerList[newComputer.jid()].ip());
		newComputer.nbClients += computerList[newComputer.jid()].nbClients;
	}
	computerList[newComputer.jid()] = newComputer;
	switch(type){
		case 0: // Hors ligne
			newComputer.nbClients--;
			if(newComputer.nbClients==0){
				emit logout(newComputer.ip());
				computerList.remove(newComputer.jid());
				break;
			}
			/// @todo: chercher l'etat des resources restantes
		case 1: // Away
			opt.Repondeur=1;
			emit login( this, 
			RzxHostAddress(newComputer.ip()), //IP
			newComputer.name(), //Nom de la machine 
			*((quint32*) &opt), //Options
			0, //Version du client
			0, //Hash de l'ic�e
			0, //Flags ?????
			"Client Jabber"); //Remarque
			break;
		case 2: // En ligne
			opt.Repondeur=0;
			emit login(  this, 
			RzxHostAddress(newComputer.ip()), //IP
			newComputer.name(), //Nom de la machine 
			*((quint32*) &opt), //Options
			0, //Version du client
			0, //Hash de l'ic�e
			0, //Flags ?????
			"Client Jabber"); //Remarque
			break;
	}
}

void RzxJabberProtocole::sendMsg(QString to, QString msg) {
	Tag *m = new Tag( "message" );
	m->addAttrib( "from", client->client()->jid().full() );
	m->addAttrib( "to", to.toStdString() );
	m->addAttrib( "type", "chat" );
	Tag *b = new Tag( "body", msg.toStdString() );
	m->addChild( b );
	client->send( m );
}

void RzxJabberProtocole::refresh(){
	Tag *m,*b;
	m = new Tag( "presence" );
	switch(RzxComputer::localhost()->state()){
		case Rzx::STATE_AWAY:
			b = new Tag( "show", "away" );
			m->addChild( b );
			break;
		case Rzx::STATE_HERE:
			break;
		case Rzx::STATE_DISCONNECTED:
			m->addAttrib( "type", "unavailable" );
			break;
		case Rzx::STATE_REFUSE:
			b = new Tag( "show", "dnd" );
			m->addChild( b );
			break;
	}
	b = new Tag( "priority", "5" );
	m->addChild( b );
	client->send( m );
}

/** Début d'implémentation de la recherche des propriétés
 *  Il faut gérer les retours.
 * Pour cela, il faudrait faire une classe à part, ou ajouter un truc à la lib
 * La structure à utiliser est définie dans le JEP 0054
 */
// void RzxJabberProtocole::getProps(QString jid){
// 	const std::string id = client->client()->getID();
// 	Tag *t = new Tag( "iq" );
// 	t->addAttrib( "type", "get" );
// 	t->addAttrib( "id", id );
// 	t->addAttrib( "to", jid.toStdString() );
// 	Tag *q = new Tag( t, "vcard" );
// 	q->addAttrib( "xmlns", "vcard-temp" );
// 	client->client()->trackID( client->client()->disco(), id, 0 );
// 	client->send( t );
// };
