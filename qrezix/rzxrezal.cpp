/***************************************************************************
                          rzxrezal.cpp  -  description
                             -------------------
    begin                : Thu Jan 24 2002
    copyright            : (C) 2002 by Sylvain Joyeux
    email                : sylvain.joyeux@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is  software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the  Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qimage.h>
#include <qbitmap.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qdir.h>
#include <qtextview.h>
#include <qtextedit.h>
#include <qtextstream.h>
#include <qsocket.h>
#include <qframe.h>

#include "rzxrezal.h"
#include "rzxitem.h"
#include "rzxconfig.h"
#include "rzxchat.h"
#include "rzxitem.h"
#include "rzxclientlistener.h"
#include "rzxmessagebox.h"
#include "rzxpluginloader.h"
#include "q.xpm"


#ifdef WIN32
	#include <windows.h>
	#include <tchar.h>
	#include <malloc.h>
	#include <stdlib.h>
	#ifdef UNICODE
		#define RzxShellExecute(a, b, str, c, d, e) \
			ShellExecute( a, b, (LPCWSTR)(str.unicode()), c, d, e )
	#else
		#define RzxShellExecute(a, b, str, c, d, e) \
			ShellExecute( a, b, (LPCSTR)(str.latin1()), c, d, e )
	#endif
	#define RzxWinExec(str, a) \
		WinExec((LPCSTR)(str.latin1()), a)
#else
	#include <stdlib.h>
#endif


const char * RzxRezal::colNames[RzxRezal::numColonnes] =
		{ QT_TR_NOOP("Icon"), QT_TR_NOOP("Computer name"), QT_TR_NOOP("Comment"),
			QT_TR_NOOP("Samba"), QT_TR_NOOP("FTP"), 
			/*QT_TR_NOOP("Hotline"),*/ QT_TR_NOOP("Web"), 
			QT_TR_NOOP("News"), QT_TR_NOOP("OS"), 
			QT_TR_NOOP("Gateway"), QT_TR_NOOP("Promo") };


RzxRezal::RzxRezal(QWidget * parent, const char * name) : QListView(parent, name), iplist(USER_HASH_TABLE_LENGTH){
	iplist.setAutoDelete(true);

	// Comme ça les chats sont détruits lorsque RzxRezal est détruit.
	chats.setAutoDelete(true);

	int i;
	for (i = 0; i < numColonnes; i++) {
		addColumn(tr(colNames[i]));
		setColumnWidthMode(i, Manual);
		if (i > ColRemarque) setColumnAlignment(i, Qt::AlignCenter);
	}
	setColumnAlignment(ColNom, Qt::AlignCenter);
	setColumnAlignment(ColRemarque, Qt::AlignVCenter);

	setHScrollBarMode(QScrollView::Auto);

	setAllColumnsShowFocus(true);
 
	server = RzxServerListener::object();
	client = RzxClientListener::object();

	connect(server, SIGNAL(login(const QString&)), this, SLOT(login(const QString&)));
	connect(server, SIGNAL(logout(const RzxHostAddress&)), this, SLOT(logout(const RzxHostAddress&)));
	connect(server, SIGNAL(rcvIcon(QImage*,const RzxHostAddress&)), this, SLOT(recvIcon(QImage*,const RzxHostAddress&)));
	connect(server, SIGNAL(disconnected()), this, SLOT(serverDisconnected()));

	connect(server, SIGNAL(status(const QString&, bool)), this, SIGNAL(status(const QString&, bool)));

	connect(server, SIGNAL(connected()), this, SLOT(serverConnected()));

	connect(this, SIGNAL(doubleClicked(QListViewItem *)),
		this, SLOT(onListDblClicked(QListViewItem *)));

	// On est obligé d'utiliser ce signal pour savoir dans quelle colonne le
	// double-clic suivant a lieu
	connect(this, SIGNAL(pressed(QListViewItem *, const QPoint &, int)),
		this, SLOT(onListClicked(QListViewItem *, const QPoint &, int)));

	lastColumnClicked = ColIcone;
  
	// FERMETURE DU SOCKET
	connect(server, SIGNAL(disconnected()), this, SIGNAL(socketClosed()));

	// GESTION DU MENU CONTEXTUEL
	connect(this,SIGNAL(rightButtonPressed(QListViewItem *,const QPoint &,int )),this,SLOT(creePopUpMenu(QListViewItem *,const QPoint &,int )));
	connect(this, SIGNAL(spacePressed(QListViewItem *)), this, SLOT(chatCreate()));
}

RzxRezal::~RzxRezal(){
}

void RzxRezal::showNotFavorites(bool val)
{
	dispNotFavorites = val;
}

RzxPopupMenu::RzxPopupMenu(QWidget * parent, const char * name) : QPopupMenu(parent, name) {
}

void RzxPopupMenu::keyPressEvent(QKeyEvent *e) {
	if(e->key()!=Qt::Key_Left) {
		QPopupMenu::keyPressEvent(e);
		return;
	}
	close();
} 

void RzxRezal::creePopUpMenu(QListViewItem *ordinateurSelect,const QPoint & pos, int){
	if(ordinateurSelect){
  
		RzxItem* item=(RzxItem*) ordinateurSelect;
		int serveurs=item->servers;
		popup.clear();
  
		popup.insertItem(*RzxConfig::themedIcon("chat"), tr("begin &Chat"),this,SLOT(chatCreate()));
		if(serveurs & 1) popup.insertItem(*RzxConfig::themedIcon("samba"), tr("Samba connect"),this,SLOT(samba()));
		if((serveurs>>1) & 1) popup.insertItem(*RzxConfig::themedIcon("ftp"), tr("FTP connect"), this, SLOT(ftp()));
		if((serveurs>>3) & 1) popup.insertItem(*RzxConfig::themedIcon("http"), tr("browse Web"), this, SLOT(http()));
		if((serveurs>>4) & 1) popup.insertItem(*RzxConfig::themedIcon("news"), tr("read News"), this, SLOT(news()));
		popup.insertSeparator();
		popup.insertItem(*RzxConfig::themedIcon("historique"), tr("History"),this,SLOT(historique()));
		popup.insertItem(*RzxConfig::themedIcon("prop"), tr("Properties"),this,SLOT(proprietes()));
		popup.insertSeparator();
		if(RzxConfig::globalConfig()->favorites->find(ordinateurSelect->text(1)))
			popup.insertItem(*RzxConfig::themedIcon("not_favorite"), tr("Remove from favorites"),this,SLOT(removeFromFavorites()));
		else
			popup.insertItem(*RzxConfig::themedIcon("favorite"), tr("Add to favorites"),this,SLOT(addToFavorites()));
		popup.insertSeparator();
		popup.insertItem(*RzxConfig::themedIcon("cancel"), tr("Cancel"), &popup, SLOT(hide()));
		RzxPlugInLoader::global()->menuItem(popup);
		popup.popup(pos);
	}
}

void RzxRezal::proprietes(const RzxHostAddress& peer)
{
	RzxChat * object = chats.find(peer.toString());
	RzxComputer * computer = iplist.find(peer.toString());
	if (!computer)
		return;
	if (!object)
		client->checkProperty(peer);
	else
	{
		if(object->getSocket())
			object->getSocket()->sendPropQuery();
		else
			client->checkProperty(peer);
	}
}

void RzxRezal::proprietes(){
	RzxItem* item=(RzxItem*) currentItem();
	proprietes(item->ip);
}

void RzxRezal::historique(){
	RzxItem * item=(RzxItem*) currentItem();
	QString hostname = iplist.find(item -> ip.toString()) -> getName();
	if(!RzxChatSocket::showHistorique( item -> ip.toRezix(), hostname))
		emit status(tr("No history file for user %1").arg(hostname), false);
}

void RzxRezal::removeFromFavorites(){
	QString temp=currentItem()->text(1);
	RzxConfig::globalConfig()->favorites->remove(temp);
	RzxConfig::globalConfig()->writeFavorites();
	emit favoriteChanged();
}

void RzxRezal::addToFavorites(){
	QString temp=currentItem()->text(1);
	RzxConfig::globalConfig()->favorites->insert(temp,new QString("1"));
	RzxConfig::globalConfig()->writeFavorites();
	emit favoriteChanged();
}

// lance le client ftp
void RzxRezal::ftp(const QString& login){
	qDebug(login);
	int offset = login.find("/");
	QString path;
	if(offset == -1) path = "";
	else path = login.mid(offset+1);
	
	QString m_login;
	m_login = login.left(offset);
	
	RzxItem *item;
	if(!login)
		item=(RzxItem*) currentItem();
	else
		item=(RzxItem*) findItem(m_login, ColNom, ExactMatch);
	if(!item) return;
	// int serveurs=item->servers;
	QString tempPath = RzxConfig::globalConfig()->FTPPath();
	QString tempip = (item->ip).toString();
	QString ip=tempip;
	tempip="ftp://"+tempip+"/"+path;
	qDebug(tempip);

#ifdef WIN32
	int iRegValue = 0;
	TCHAR strRegValue[] = TEXT("0");
	HKEY hKey;

	QString sFtpClient=RzxConfig::globalConfig()->ftpCmd();

	// leechftp :
	if( (!sFtpClient.compare("LeechFTP")) &&
		!RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\LeechFTP"), 0, KEY_ALL_ACCESS, &hKey) )
	{
		RegSetValueEx(hKey, TEXT("ProxyMode"), 0, REG_DWORD, LPBYTE(& iRegValue), 4);
		RegSetValueEx(hKey, TEXT("LocalDir"),0,REG_SZ,
			(unsigned char*)(QDir::convertSeparators(RzxConfig::globalConfig()->FTPPath()).latin1()),
			RzxConfig::globalConfig()->FTPPath().length() * sizeof(unsigned char));

		unsigned char buffer[MAX_PATH];
		unsigned long KeyType = 0;
		unsigned long KeySize = sizeof(TCHAR) * MAX_PATH;
		RegQueryValueEx(hKey, TEXT("AppDir"), 0, &KeyType, buffer, &KeySize);
		RegCloseKey(hKey);
		QString temp=(char*)buffer;
		QString cmd=temp+"leechftp.exe " + tempip;

		RzxWinExec(cmd, 1);
	}
	// smartftp :
	else if( (!sFtpClient.compare("SmartFTP")) &&
		!RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\SmartFTP"), 0, KEY_ALL_ACCESS, &hKey))
	{
		RegCloseKey(hKey);
		HKEY hKey2;
		RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\SmartFTP\\ProxySettings"), 0, KEY_ALL_ACCESS, &hKey2);
		unsigned char buffer[MAX_PATH];
		unsigned char * pointer;
		unsigned long KeyType = 0;
		unsigned long KeySize = sizeof(TCHAR) * MAX_PATH;
  
		if ( QApplication::winVersion() & Qt::WV_NT_based ){
			unsigned long size;
			RegQueryInfoKey(hKey,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,&size,NULL,NULL);
			size+=ip.length() * sizeof(unsigned char);
			unsigned char *buffer2;
			buffer2 = (unsigned char *)malloc(size);
			strcpy((char*)buffer2,ip.latin1());
			pointer=buffer2;
			buffer2[ip.length()]=';';
			pointer+=ip.length()+1;
			RegQueryValueEx(hKey2, TEXT("Proxy Exceptions"), 0, &KeyType, pointer, &size);
			QString test((char*)pointer);
   
			if(!test.contains(ip+";")){
				RegSetValueEx(hKey2,TEXT("Proxy Exceptions"),0,REG_SZ,(const unsigned char*)buffer2,KeySize+ip.length()+1);
			}
			free(buffer2);
		}
		else RegSetValueEx(hKey, TEXT("Proxy Type"), 0, REG_DWORD, LPBYTE(& iRegValue), 4);
		RegCloseKey(hKey2);

		RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\SmartFTP\\Network"), 0, KEY_ALL_ACCESS, &hKey2);
		RegSetValueEx(hKey2,TEXT("Default Path"),0,REG_SZ,
				(unsigned char*)(QDir::convertSeparators(RzxConfig::globalConfig()->FTPPath()).latin1()),
				RzxConfig::globalConfig()->FTPPath().length() * sizeof(unsigned char));
		RegCloseKey(hKey2);
  
		RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\SmartFTP"), 0, KEY_ALL_ACCESS, &hKey);
		RegQueryValueEx(hKey, TEXT("Install Directory"), 0, &KeyType, buffer, &KeySize);
		QString temp=(char*)buffer;
		RegCloseKey(hKey);
		QString cmd=temp+"smartftp.exe " + tempip;

		RzxWinExec(cmd, 1);

	}

	// bulletproof FTP
/* else if (iFtpClient == 3)
	{
		RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\BPFTP\\Bullet Proof FTP\\Options"), 0, KEY_ALL_ACCESS, &hKey);
		RegSetValueEx(hKey, TEXT("FirewallEnabled"), 0, REG_SZ, LPBYTE(& strRegValue), 2 * sizeof(TCHAR));
		RegCloseKey(hKey);
	}*/

	else{ // client FTP standard
		QString cmd="explorer " + tempip;
		RzxWinExec(cmd, 1);
	}
#else
	QString cmd = "cd "+tempPath+"; "+RzxConfig::globalConfig()->ftpCmd()+" "+tempip;
	if(RzxConfig::globalConfig()->ftpCmd() == "lftp")
	// on lance le client dans un terminal
	#ifdef WITH_KDE
		cmd = "konsole -e \"" + cmd + "\" &";
	#else
		cmd = "xterm -e \"" + cmd + "\" &";
	#endif
	else
	//client graphique
		cmd = cmd + " &";

	system(cmd.latin1());
#endif
}

void RzxRezal::samba(const QString& login){
	RzxItem *item;
	if(!login)
		item=(RzxItem*) currentItem();
	else
		item=(RzxItem*) findItem(login, ColNom, ExactMatch);
	if(!item) return;

	QString tempip = (item -> ip).toString();
	QString tempPath = RzxConfig::globalConfig()->FTPPath();

#ifdef WIN32
	int serveurs=item->servers;
	QString cmd = "explorer \\\\" + (item -> ip).toString();
	RzxWinExec(cmd, 1);
#else
	QString cmd = "cd "+tempPath+"; konqueror smb://" + (item ->ip).toString() + "/" +" &";
	system(cmd.latin1());
#endif
}

/*void RzxRezal::fermetureLeechFTP(){
	int iRegValue = 2;
	HKEY hKey;

	RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\LeechFTP"), 0, KEY_ALL_ACCESS, &hKey);
	RegSetValueEx(hKey, TEXT("ProxyMode"), 0, REG_DWORD, LPBYTE(& iRegValue), 4);
	RegCloseKey(hKey);
}*/

// lance le client http
void RzxRezal::http(const QString& login)
{
	RzxItem *item;
	if(!login)
		item=(RzxItem*) currentItem();
	else
		item=(RzxItem*) findItem(login, ColNom, ExactMatch);
	if(!item) return;
	QString tempip = "http://" + (item -> ip).toString();
	QString cmd=RzxConfig::globalConfig()->httpCmd();

#ifdef WIN32
	if( cmd == "standard" )
	RzxShellExecute( NULL, NULL, tempip, NULL, NULL, SW_SHOW );
	else
	{
		int serveurs = item->servers;
		QString cmd;
		HKEY hKey;
		RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software"), 0, KEY_ALL_ACCESS, &hKey);

		if ( cmd == "Opera" && !RegOpenKeyEx(hKey, TEXT("Opera Software"), 0, KEY_ALL_ACCESS, &hKey) ) {
  
			unsigned char buffer[MAX_PATH];
			unsigned long KeyType = 0;
			unsigned long KeySize = sizeof(TCHAR) * MAX_PATH;
			RegQueryValueEx(hKey, TEXT("Last CommandLine"), 0, &KeyType, buffer, &KeySize);
			RegCloseKey(hKey);
			QString temp=(char *)buffer;
			cmd = temp + tempip;
		}
		else
		cmd = "explorer " + tempip;

 
		RzxWinExec(cmd, 1);
	}
#else
	cmd = cmd + " " + tempip + " &";
	system(cmd.latin1());
#endif
}

// lance le client news
void RzxRezal::news(const QString& login){
	RzxItem *item;
	if(!login)
		item=(RzxItem*) currentItem();
	else
		item=(RzxItem*) findItem(login, ColNom, ExactMatch);
	if(!item) return;
	QString tempip = "news://" + (item -> ip).toString();
	QString cmd = RzxConfig::globalConfig()->newsCmd();

#ifdef WIN32
	int serveurs=item->servers;
	if( cmd == "standard" )
		cmd = "explorer " + tempip;
	else
		cmd = cmd + " " + tempip;

	RzxWinExec(cmd,1);
#else
	cmd = cmd + " " + tempip + " &";
	system(cmd.latin1());
#endif
}

void RzxRezal::afficheColonnes(){
	int colonnesAffichees=RzxConfig::colonnes();
	int i;

	for(i =0; i<columns(); i++){
		setColumnWidthMode(i,Manual);
		setColumnWidth(i,0);
		if((colonnesAffichees>>i) & 1){
			if(i==0) {
				setColumnWidth(i,(RzxConfig::globalConfig()->computerIconSize()+1)*32+4);
			} else if(i==1) {
				adjustColumn (i); setColumnWidthMode(i,QListView::Maximum);
			} else if(i==2) {
				//if(columnWidth(i)==0) setColumnWidth(i,200);
			} else {
				setColumnWidth(i,40);
			}
		}
	}
		adapteColonnes();
}

void RzxRezal::adapteColonnes(){
	int colonnesAffichees=RzxConfig::colonnes();
	int somme=0;
	int i;
	const int colrem=2;

	for(i =0;i<columns();i++){
		if(i!=colrem)
			somme+=columnWidth(i);
	}
  
	if(((colonnesAffichees >> colrem) & 1)) {
		if(width()>(somme+110))
			setColumnWidth(colrem, width()-somme-10);
		else
			setColumnWidth(colrem,100);
	}
	triggerUpdate(); 
}

void RzxRezal::initConnection() {
	server -> setupConnection();
	if( ! client -> listenOnPort(RzxConfig::chatPort()) )
		RzxMessageBox::warning( (QWidget *) parent(), "qRezix",
		tr("Cannot create peer to peer socket !\n\nChat and properties browsing are disabled") );
}

/** No descriptions */
void RzxRezal::recvIcon(QImage* icon, const RzxHostAddress& ip){
	RzxComputer * object = iplist.find(ip.toString());
	if (object) {
		icon -> save(RzxConfig::computerIconsDir().absFilePath(object -> getFilename()), "PNG");
		QPixmap pix;
		pix.convertFromImage(*icon);
		object -> setIcon(pix);
	}
}

/** No descriptions */
void RzxRezal::login(const QString& ordi){
	RzxComputer * newComputer = new RzxComputer;
	connect(newComputer, SIGNAL(needIcon(const RzxHostAddress&)), this, SIGNAL(needIcon(const RzxHostAddress&)));
	if (newComputer -> parse(ordi)) {
		delete newComputer;
		return;
	}
  
	// Recherche si cet ordinateur était déjà présent
	QString tempIP = newComputer -> getIP().toString();
	RzxComputer * object = iplist.find(tempIP);
	RzxItem * item;
	bool alreadyHere = object != NULL;
	// non ==> nouvel item
	if (!alreadyHere) {
		item = new RzxItem(newComputer, this, dispNotFavorites);
	}
	else {
		item = (RzxItem *) object -> child(0, "RzxItem");
		if (!item) return;

		object -> removeChild(item);
		iplist.remove(newComputer -> getIP().toString());

		newComputer -> insertChild(item);
	}

	iplist.insert(newComputer -> getIP().toString(), newComputer);
	RzxChat * chatWithLogin=chats.find(newComputer -> getIP().toString());
	// informe de la reconnexion si c'est pas juste un refresh
	if( !object && chatWithLogin )
	chatWithLogin->info(tr("reconnected"));

	connect(newComputer, SIGNAL(isUpdated()), item, SLOT(update()));
	connect(this, SIGNAL(favoriteChanged()), item, SLOT(update()));
	emit countChange(tr("%1 clients connected").arg(iplist.count()));
	item -> update();

	if( alreadyHere ) sort(); // Retrie la liste pour les éventuelles modifs
  
	afficheColonnes();
	
 
}

/** No descriptions */
void RzxRezal::logout(const RzxHostAddress& ip){
        QString key = ip.toString();
        RzxComputer * object = iplist.find(key);
        if (object) {
                iplist.remove(key);
                emit countChange(tr("%1 clients connected").arg(iplist.count()));
        }
        
RzxChat * chatWithLogin=chats.find(ip.toString());
	if(chatWithLogin)
		chatWithLogin->info(tr("disconnected"));
}

///Retourne la liste des IP des gens connectés
/** Permet pour les plug-ins d'obtenir facilement la liste les ip connectées */
QStringList RzxRezal::getIpList()
{
	QDictIterator<RzxComputer> it(iplist);
	QStringList ips;
	for( ; it.current() ; ++it)
	{
		ips << (it.currentKey());
	}
	return ips;
}

/*************************
* GESTION DU CHAT
*/

void RzxRezal::chat(QSocket* socket, const QString& msg) {
	RzxHostAddress peer = socket->peerAddress();
	if(RzxConfig::autoResponder()) {
		((RzxChatSocket*) socket) -> sendResponder(RzxConfig::autoResponderMsg());
		emit status(tr("%1 is now marked as away").arg(peer.toString()), false);
	}
	else {
		RzxChat * chatWindow = chatCreate(peer);
		if (!chatWindow) return;
		chatWindow->setSocket((RzxChatSocket*)socket);      //on change le socket si nécessaire
		chatWindow -> receive(msg);
	}
}

RzxChat * RzxRezal::chatCreate(const QString& login)
{
	RzxItem *item;
	if(!login)
		item=(RzxItem*) currentItem();
	else
		item=(RzxItem*) findItem(login, ColNom, ExactMatch);
	if(!item) return NULL;
	RzxHostAddress tempip = (item->ip);
	return chatCreate(tempip);
}

RzxChat * RzxRezal::chatCreate(const RzxHostAddress& peer) {
	RzxChat * object = chats.find(peer.toString());
	if (!object) {
		RzxComputer * computer = iplist.find(peer.toString());
		if (!computer) {
			qWarning(tr("Received a chat request from %1").arg(peer.toString()));
			qWarning(tr("%1 is NOT logged").arg(peer.toString()));
			return 0;
		}
		/*  if(computer->getName() == RzxConfig::localHost()->getName())
		{
			RzxMessageBox::information(NULL, tr("Can't open chat"), tr("Hey, it's you %1... you can't have a chat with yourself.\n If you really want to chat to yourself, you just have to find a mirror...").arg(computer->getName()));
			return NULL;
		}*/
		object = new RzxChat(peer);

		QPixmap iconeProg((const char **)q);
		iconeProg.setMask(iconeProg.createHeuristicMask() );
		object->setIcon(iconeProg);

#ifdef WIN32
		object->setCaption(tr("Chat")+" - " + computer->getName() +" [Qt]");
#else
		object->setCaption(tr("Chat")+" - " + computer->getName());
#endif
		object->setHostname(computer->getName());

		object->edMsg->setFocus();

		connect(object, SIGNAL(closed(const RzxHostAddress&)), this, SLOT(chatDelete(const RzxHostAddress&)));
//		connect(object, SIGNAL(showHistorique(unsigned long, QString, bool, QWidget*, QPoint*)),
//			this, SLOT(showHistorique(unsigned long, QString, bool, QWidget*, QPoint*)));
//		connect(object, SIGNAL(showProperties(const RzxHostAddress&, const QString&, bool, QWidget*, QPoint*)),
//			this, SLOT(showProperties(const RzxHostAddress&, const QString&, bool, QWidget*, QPoint* )));
		connect(RzxConfig::globalConfig(), SIGNAL(themeChanged()), object, SLOT(changeTheme()));
		connect(RzxConfig::globalConfig(), SIGNAL(iconFormatChange()), object, SLOT(changeIconFormat()));
		chats.insert(peer.toString(), object);
	}
	object -> show();

	return object;
}

///Fermeture du chat (si il existe) associé au login
void RzxRezal::closeChat(const QString& login)
{
	RzxItem *item=(RzxItem*) findItem(login, ColNom, ExactMatch);
	if(!item) return;
	RzxChat *chat = chats.find(item->ip.toString());
	chat->close();
}


/** No descriptions */
void RzxRezal::chatDelete(const RzxHostAddress& peerAddress){        
	// Auto-Delete = true, le chat est supprimé automatiquement. Qt rules !!!
	chats.remove(peerAddress.toString());
}

///Fermeture des chats en cours
/** Cette méthode à pour but de fermer tous les chats en cours pour libérer en particulier le port 5050. Normalement l'appel de cette méthode à la fermeture de qRezix doit corriger le problème de réouverture de l'écoute qui intervient à certains moments lors du démarrage de qRezix */
void RzxRezal::closeChats()
{
	QDictIterator<RzxChat> it(chats);
	for( ; it.current() ; ++it)
	{
		it.current()->close();
	}
}


void RzxRezal::warnProperties(const RzxHostAddress& peer) {
	RzxChat * object = chats.find(peer.toString());
	RzxComputer * computer = iplist.find(peer.toString());
	if (!computer)
		return;
	QTime cur = QTime::currentTime();
        QString heure;
        heure.sprintf("<i>%2i:%.2i:%.2i",
                                cur.hour(),
                                cur.minute(),
                                cur.second());
    
	if (!object) {
		if(RzxConfig::globalConfig()->warnCheckingProperties()==0)
			return;
		sysmsg(tr("Properties sent to %2 (%3) at %1").arg(heure).arg(computer->getName()).arg(peer.toString()));
		return;
	}
	object->notify(tr("has checked your properties"), true);
}

/** No descriptions */
void RzxRezal::sysmsg(const QString& msg){
	// Boîte de dialogue non modale, pour que les comms continuent.
	RzxMessageBox::information( (QWidget *) parent(), tr("XNet Server message:"), msg );
}
/** No descriptions */
void RzxRezal::fatal(const QString& msg){
	// Boîte de dialogue modale
	RzxMessageBox::critical( (QWidget *) parent(), tr("Error")+" - "+tr("XNet Server message"), msg, true );
}

/** CRASSOU AU POSSIBLE
* TODO: gerer ca autrement */
void RzxRezal::onListDblClicked(QListViewItem * sender){
	RzxItem * item = static_cast<RzxItem*> (sender);
	int serveurs=item->servers;

	bool gere = false;

	// Quelle colonne a été cliquée ?
	switch( lastColumnClicked )
	{
	case ColSamba:
        	if( serveurs & 0x01 ) { gere = true; samba(); }
		break;

	case ColFTP:
        	if( serveurs & 0x02 ) { gere = true; ftp(); }
		break;

	case ColHTTP:
        	if( serveurs & 0x08 ) { gere = true; http(); }
		break;

	case ColNews:
        	if( serveurs & 0x10 ) { gere = true; news(); }
		break;

	default: ;
	}

	if( !gere ) // par défaut -> chat ou ftp.
	{
		if(RzxConfig::globalConfig()->doubleClicRole() && (serveurs & RzxComputer::SERVER_FTP)){
			ftp();
		}
		else {/*chat*/
			RzxItem * listItem = (RzxItem *) sender;
			chatCreate(listItem -> ip);
		}
	}
}

/** Memorise la colonne cliquee */
void RzxRezal::onListClicked( QListViewItem * item, const QPoint & pnt, int c ){
	lastColumnClicked = (NumColonne) c;
}

/** No descriptions */
void RzxRezal::serverDisconnected()
{        
}

/** No descriptions */
void RzxRezal::serverConnected(){
        setEnabled(true);
        iplist.clear();        
}

/** No descriptions */
bool RzxRezal::isSocketClosed() const{
        return server -> isSocketClosed();
}

/** No descriptions */
void RzxRezal::closeSocket(){
        disconnect(server, SIGNAL(disconnected()), this, SLOT(serverDisconnected()));
        client -> close();        
        chats.clear();
        if(server) server -> close();
}

void RzxRezal::resizeEvent(QResizeEvent * e) {
	QListView::resizeEvent(e);
	adapteColonnes();
}

void RzxRezal::keyPressEvent(QKeyEvent *e) {
	//QListView::keyPressEvent(e);
	QString s=e->text();

#ifndef WIN32
	if(s.isNull() == TRUE)
		return;
	QChar c=s.at(0);
	if(!c.isLetter()) {
#else
	QChar c;
	if(s.isNull() != TRUE)
		c = s.at(0);
	if(s.isNull() == TRUE || !c.isLetter()) {
#endif
		if(e->key()==Qt::Key_Right) {
			QRect r(itemRect( currentItem())); //r != 0 car the currentItem IS visible
			QPoint qp=r.center();
			qp.setY(qp.y()+r.height());
			creePopUpMenu(currentItem(), qp,0);
			return;
		}
		QListView::keyPressEvent(e); //on laisse Qt gérer
		return;
	}
	RzxItem* item=static_cast<RzxItem*> (currentItem());
	RzxHostAddress ipSaved=item->ip; //y a des chances que item soit ! null :)
	while(true) {
		item=static_cast<RzxItem*>(item->itemBelow());
		if(!item)
			item=static_cast<RzxItem*>(firstChild()); //au bout on revient au début
		if(item->ip == ipSaved)
			return; //on a fait le tour, personne avec cette lettre
		RzxComputer * comp=iplist.find((item->ip).toString());
		if(comp->getName().lower().at(0)==c) {
			setCurrentItem(item);
			ensureItemVisible(item);
			return;
		}
	}
	//ICI JAI LAISSE DS TRUCS EN PLAN
	//
	//
	// OUI ICI OUI !!!!!!!!!!!!!!!!!!
	//
	//
	//BON CA DEVRAIT SUFFIRE LA  C ASSEZ VISIBLE
}

/** No descriptions */
void RzxRezal::redrawAllIcons(){
	if (!RzxConfig::computerIconSize())
		setColumnWidth(0, 32);
 
	for (QListViewItemIterator it(this); it.current(); ++it) {
		RzxItem * item = static_cast<RzxItem *> (it.current());
		item -> update();
	}
}

void RzxRezal::languageChanged(){
	qDebug("RzxRezal::languageChanged()");
	for (int i = 0; i < numColonnes; i++) {
		setColumnText(i, tr(colNames[i]));
	}
}

