/***************************************************************************
                          rzxjabberprotocole.cpp  -  description
                             -------------------
    begin                : Fri Sept 25 2005
    copyright            : (C) 2002 by Guillaume Porcher
    email                : pico@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#define RZX_MODULE_NAME "Jabber"
#define RZX_MODULE_VERSIONSTR "0.0.1-svn"

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
#include <RzxConnectionLister>

#include "rzxjabberprotocole.h"
#include "rzxjabberconfig.h"
#include "ui_rzxjabberpropui.h"

using namespace gloox;

RZX_NETWORK_EXPORT(RzxJabberProtocole)
RZX_CONFIG_INIT(RzxJabberConfig)

///Construction... RAS
RzxJabberProtocole::RzxJabberProtocole()
	: RzxNetwork(RZX_MODULE_NAME, QT_TR_NOOP("Native support for the jabber protocole"), RZX_MODULE_VERSION)
{
	setType(RzxNetwork::TYP_CHAT);
	setType(RzxNetwork::TYP_PROPERTIES);
	beginLoading();
	ui = NULL;
	propWidget = NULL;
	
	new RzxJabberConfig(this);
	
	client = new RzxJabberClient(this);
	connect(client, SIGNAL(presence(QString, QString, int)), this, SLOT(presenceRequest(QString, QString, int)));
	connect(client, SIGNAL(msgReceived(QString, QString)), this, SLOT(recvMsg(QString,QString))); 
	connect(client, SIGNAL(connected()), this, SLOT(connection()));
	connect(client, SIGNAL(connected()), this, SLOT(updateLocalhost()));
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
* FENETRE DE PROPRIETES
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
		connect(ui->getButton,SIGNAL(clicked()),this, SLOT(updateLocalhost()));
		connect(ui->setButton,SIGNAL(clicked()),this, SLOT(sendProperties()));
		connect(ui->btnChangePass, SIGNAL(clicked()), this, SLOT(wantChangePass()));
		connect(ui->btnNewAccount, SIGNAL(clicked()), this, SLOT(wantNewAccount()));
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
		client->client()->rosterManager()->subscribe(ui->rosterEdit->text().toUtf8().data()," ",l,"User added");
	}
};

void RzxJabberProtocole::removeRosterItem(){
	if(!ui->rosterEdit->text().isEmpty()){
		client->client()->rosterManager()->unsubscribe(ui->rosterEdit->text().toUtf8().data(),"User removed",true);
	}
};

void RzxJabberProtocole::buildRosterList(){
	// remplissage de la liste des Roster
	if(ui){
		ui->rosterList->clear();
		ui->rosterEdit->setText("");
		std::map<std::string, RosterItem*>::const_iterator i;
		for( i = client->client()->rosterManager()->roster()->begin() ; i !=client->client()->rosterManager()->roster()->end() ; i++){
			ui->rosterList->addItem(QString::fromUtf8(((*i).first).data()));
		}
		ui->rosterList->sortItems();
	}
};


QStringList RzxJabberProtocole::propWidgetsName()
{
	return QStringList() << name();
}


void RzxJabberProtocole::propInit(bool def)
{
	ui->server_name->setText( RzxJabberConfig::serverName() );
	ui->user->setText( RzxJabberConfig::user() );
	ui->pass->setText( RzxJabberConfig::pass() );
	ui->server_port->setValue( RzxJabberConfig::serverPort() );
	ui->reconnection->setValue( RzxJabberConfig::reconnection() / 1000 );
	ui->ping_timeout->setValue( RzxJabberConfig::pingTimeout() / 1000 );
	
	if(isStarted()){
		buildRosterList();
		ui->propName->setText(computerList["myself"]->props()->name);
		ui->propNick->setText(computerList["myself"]->props()->nick);
		ui->propEmail->setText(computerList["myself"]->props()->email);
		ui->propBirthday->setText(computerList["myself"]->props()->birthday);
		ui->propWebsite->setText(computerList["myself"]->props()->website);
		ui->propPhone->setText(computerList["myself"]->props()->phone);
		ui->propStreet->setText(computerList["myself"]->props()->street);
		ui->propPostCode->setText(computerList["myself"]->props()->postCode);
		ui->propCity->setText(computerList["myself"]->props()->city);
		ui->propRegion->setText(computerList["myself"]->props()->region);
		ui->propCountry->setText(computerList["myself"]->props()->country);
		ui->propOrgName->setText(computerList["myself"]->props()->orgName);
		ui->propOrgUnit->setText(computerList["myself"]->props()->orgUnit);
	}
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
	RzxJabberComputer *newComputer = new RzxJabberComputer("myself", "myself", 0);
	connect(newComputer->props(), SIGNAL(receivedProperties(RzxJabberComputer*)), this, SLOT(receivedProperties(RzxJabberComputer*)));
	computerList["myself"] = newComputer;
	client->start();
}

void RzxJabberProtocole::stop() {
	if(client && client->isRunning()){
		client->stop();
		client->wait(1000);
	}
}


void RzxJabberProtocole::presenceRequest(QString jid, QString name, int type) {
	QString localhost = QString::fromUtf8(client->client()->jid().bare().data());
	if(jid == localhost)
		return;
	struct options_t
	{
	unsigned Server :6;
	unsigned SysEx  :3;     //0=Inconnu, 1=Win9X, 2=WinNT, 3=Linux, 4=MacOS, 5=MacOS X, 6=BSD
	unsigned Promo  :2; //0 = Orange, 1=Jne, 2=Rouje (Chica la rouje ! <== bah nan, �la j�e !!!)
	unsigned Repondeur :2; //0=accepter, 1= repondeur, 2=refuser les messages, 3= unused
	// total 13 bits / 32
	unsigned Capabilities :19;
	} opt;
	opt.Capabilities=2; 
	opt.Promo=0;
	opt.SysEx=0;
	opt.Server=0;
	struct version_t
	{
		unsigned FunnyVersion	:14;
		unsigned MinorVersion	:7;
		unsigned MajorVersion	:3;
		unsigned Client		:8;	//1 = ReziX; 2 = XNet, 3 = MacXNet, 4 = CPANet 5 = CocoaXNet 6 = qrezix 7 = mxnet
	} version;
	version.Client = 4;
	version.MajorVersion = 0;
	version.MinorVersion = 0;
	version.FunnyVersion = 1;
	
	RzxJabberComputer *newComputer = new RzxJabberComputer(jid, name, computerList.size());
	if(type > 0 && computerList.contains(newComputer->jid())){
		newComputer = computerList[newComputer->jid()];
		newComputer->nbClients++;
	}
	computerList[newComputer->jid()] = newComputer;
	connect(newComputer->props(), SIGNAL(receivedProperties(RzxJabberComputer*)), this, SLOT(receivedProperties(RzxJabberComputer*)));
	switch(type){
		case 0: // Hors ligne
			newComputer->nbClients--;
			if(newComputer->nbClients==0){
				emit logout(newComputer->ip());
				computerList.remove(newComputer->jid());
				break;
			}
			/// @todo: chercher l'etat des resources restantes
		case 1: // Away
			opt.Repondeur=1;
			emit login( this, 
			RzxHostAddress(newComputer->ip()), //IP
			newComputer->name(), //Nom de la machine 
			*((quint32*) &opt), //Options
			*((quint32*) &version), //Version du client
			0xffffffff, //Hash de l'ic�e
			0, //Flags ?????
			"Client Jabber"); //Remarque
			break;
		case 2: // En ligne
			opt.Repondeur=0;
			emit login(  this, 
			RzxHostAddress(newComputer->ip()), //IP
			newComputer->name(), //Nom de la machine 
			*((quint32*) &opt), //Options
			*((quint32*) &version), //Version du client
			0xffffffff, //Hash de l'ic�e
			0, //Flags ?????
			"Client Jabber"); //Remarque
			break;
		case 3: // Hors ligne
			opt.Repondeur=2;
			emit login(  this, 
				     RzxHostAddress(newComputer->ip()), //IP
				     newComputer->name(), //Nom de la machine 
				     *((quint32*) &opt), //Options
				     *((quint32*) &version), //Version du client
				     0xffffffff, //Hash de l'ic�e
				     0, //Flags ?????
				     "Client Jabber"); //Remarque
			break;
	}
}


void RzxJabberProtocole::sendMsg(QString to, QString msg) {
	Tag *m = new Tag( "message" );
	m->addAttrib( "from", client->client()->jid().full() );
	m->addAttrib( "to", to.toUtf8().data() );
	m->addAttrib( "type", "chat" );
	Tag *b = new Tag( "body", msg.toUtf8().data() );
	m->addChild( b );
	client->send( m );
}

void RzxJabberProtocole::recvMsg(QString to, QString msg) {
	RzxConnectionLister::global()->getComputerByName(to)->receiveChat(Rzx::Chat, msg);
}

void RzxJabberProtocole::sendChatMessage(RzxComputer* comp, Rzx::ChatMessageType type, const QString& msg){
	sendMsg(comp->name(),msg);
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
void RzxJabberProtocole::properties(RzxComputer* comp){
	getProperties(comp->name(), comp->name());
}

void RzxJabberProtocole::getProperties(QString jid, QString comp){
	const std::string id = client->client()->getID();
	Tag *t = new Tag( "iq" );
	t->addAttrib( "type", "get" );
	t->addAttrib( "id", id );
	t->addAttrib( "to", jid.toUtf8().data() );
	Tag *q = new Tag( t, "vcard" );
	q->addAttrib( "xmlns", "vcard-temp" );
	client->client()->trackID( computerList[comp]->props(), id, 0 );
	client->send( t );
};

void RzxJabberProtocole::receivedProperties(RzxJabberComputer* from)
{
	if(from->name()!="myself"){
		RzxConfig::addCache(from->ip(), from->props()->toMsg());
		emit haveProperties(RzxConnectionLister::global()->getComputerByIP(from->ip()));
	}
}

void RzxJabberProtocole::chat(RzxComputer* comp){
};

void RzxJabberProtocole::sendProperties()
{
	computerList["myself"]->props()->name    = ui->propName->text();
	computerList["myself"]->props()->nick    = ui->propNick->text();
	computerList["myself"]->props()->email   = ui->propEmail->text();
	computerList["myself"]->props()->birthday= ui->propBirthday->text();
	computerList["myself"]->props()->website = ui->propWebsite->text();
	computerList["myself"]->props()->phone   = ui->propPhone->text();
	computerList["myself"]->props()->street  = ui->propStreet->text();
	computerList["myself"]->props()->postCode= ui->propPostCode->text();
	computerList["myself"]->props()->city    = ui->propCity->text();
	computerList["myself"]->props()->region  = ui->propRegion->text();
	computerList["myself"]->props()->country = ui->propCountry->text();
	computerList["myself"]->props()->orgName = ui->propOrgName->text();
	computerList["myself"]->props()->orgUnit = ui->propOrgUnit->text();
	const std::string id = client->client()->getID();
	Tag *t = computerList["myself"]->props()->toIq();
	t->addAttrib( "id", id );
	client->send( t );
}

void RzxJabberProtocole::updateLocalhost()
{
	QString jid = QString::fromUtf8(client->client()->jid().bare().data());
	getProperties(jid,"myself");
}

/*
* Changement du mot de passe
*/
void RzxJabberProtocole::wantChangePass()
{
	new RzxChangePass(this, RzxJabberConfig::pass());
}

void RzxJabberProtocole::changePass(const QString& newPass)
{
	if(isStarted()){
		client->changePass(newPass);
	}
}

void RzxJabberProtocole::usePass(const QString& pass)
{
	if(ui)
		ui->pass->setText(pass);
}

void RzxJabberProtocole::wantNewAccount()
{
	if(client && client->isRunning() && ui)
		client->wantNewAccount(ui->user->text(),ui->pass->text(),ui->server_name->text(),ui->server_port->value());
}