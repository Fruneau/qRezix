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

#include "rzxrezal.h"
#include "rzxitem.h"
#include "rzxconfig.h"
#include "rzxchat.h"
#include "rzxitem.h"
#include "rzxclientlistener.h"
#include "rzxmessagebox.h"
#include "q.xpm"


#ifdef WIN32
#include <windows.h>
#include <tchar.h>
#include <malloc.h>
#include <stdlib.h>
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

	QScrollView::ScrollBarMode mode = RzxConfig::autoColonne() ? QScrollView::AlwaysOff : QScrollView::Auto;
	setHScrollBarMode(mode);

	setAllColumnsShowFocus(true);
	
	server = RzxServerListener::object();
	client = RzxClientListener::object();

	connect(server, SIGNAL(sysmsg(const QString&)), this, SLOT(sysmsg(const QString&)));
	connect(server, SIGNAL(fatal(const QString&)), this, SLOT(fatal(const QString&)));
	
	connect(server, SIGNAL(login(const QString&)), this, SLOT(login(const QString&)));
	connect(server, SIGNAL(logout(const RzxHostAddress&)), this, SLOT(logout(const RzxHostAddress&)));
	connect(server, SIGNAL(rcvIcon(QImage*,const RzxHostAddress&)), this, SLOT(recvIcon(QImage*,const RzxHostAddress&)));
	connect(server, SIGNAL(disconnected()), this, SLOT(serverDisconnected()));

	connect(this, SIGNAL(needIcon(const RzxHostAddress&)), server, SLOT(getIcon(const RzxHostAddress&)));

	connect(server, SIGNAL(status(const QString&, bool)), this, SIGNAL(status(const QString&, bool)));

	connect(server, SIGNAL(connected()), this, SLOT(serverConnected()));

	connect(this, SIGNAL(doubleClicked(QListViewItem *)),
		this, SLOT(onListDblClicked(QListViewItem *)));

	// On est obligé d'utiliser ce signal pour savoir dans quelle colonne le
	// double-clic suivant a lieu
	connect(this, SIGNAL(pressed(QListViewItem *, const QPoint &, int)),
		this, SLOT(onListClicked(QListViewItem *, const QPoint &, int)));

	lastColumnClicked = ColIcone;
			
	// CHAT
	connect(client, SIGNAL(chat(const RzxHostAddress&, const QString&)),
		this, SLOT(chat(const RzxHostAddress&, const QString&)));

	// RECEPTION DES PROPRIETES D'UN ORDINATEUR
	connect(client, SIGNAL(propAnswer(const RzxHostAddress&, const QString&)), this, SLOT(showProperties(const RzxHostAddress&, const QString&)));
	connect(client, SIGNAL(propQuery(const RzxHostAddress&)), RzxClientListener::object(), SLOT(sendProperties(const RzxHostAddress&)));
	connect(client, SIGNAL(propertiesSent(const RzxHostAddress&)), this, SLOT(warnProperties(const RzxHostAddress&)));
	
	// FERMETURE DU SOCKET
	connect(server, SIGNAL(disconnected()), this, SIGNAL(socketClosed()));

	// GESTION DU MENU CONTEXTUEL
	connect(this,SIGNAL(rightButtonPressed(QListViewItem *,const QPoint &,int )),this,SLOT(creePopUpMenu(QListViewItem *,const QPoint &,int )));
	connect(this, SIGNAL(spacePressed(QListViewItem *)), this, SLOT(chatCreate()));
}

RzxRezal::~RzxRezal(){
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
		
		RzxItem::ListViewItem* item=(RzxItem::ListViewItem*) ordinateurSelect;
		int serveurs=item->servers;
		popup.clear();
		
		popup.insertItem(tr("begin &Chat"),this,SLOT(chatCreate()));
		if(serveurs & 1) popup.insertItem(tr("Samba connect"),this,SLOT(samba()));
		if((serveurs>>1) & 1) popup.insertItem(tr("FTP connect"), this, SLOT(ftp()));
		if((serveurs>>3) & 1) popup.insertItem(tr("browse Web"), this, SLOT(http()));
		if((serveurs>>4) & 1)	popup.insertItem(tr("read News"), this, SLOT(news()));
		popup.insertSeparator();
		popup.insertItem(tr("History"),this,SLOT(historique()));
		popup.insertItem(tr("Properties"),this,SLOT(proprietes()));
		popup.insertSeparator();
		if(RzxConfig::globalConfig()->favorites->find(ordinateurSelect->text(1)))
			popup.insertItem(tr("Remove from favorites"),this,SLOT(removeFromFavorites()));
		else
			popup.insertItem(tr("Add to favorites"),this,SLOT(addToFavorites()));
		popup.popup(pos);
	}
}

void RzxRezal::proprietes(const RzxHostAddress& peer){
	client -> sendPropQuery( peer );
}

void RzxRezal::proprietes(){
	RzxItem::ListViewItem* item=(RzxItem::ListViewItem*) currentItem();
	client -> sendPropQuery(item->ip);
}

void RzxRezal::historique(){
	RzxItem::ListViewItem * item=(RzxItem::ListViewItem*) currentItem();
	QString hostname = iplist.find(item -> ip.toString()) -> getName();
	showHistorique( item -> ip.toRezix(), hostname );
}

void RzxRezal::showHistorique( unsigned long ip, QString hostname ){
	// chargement de l'historique
	QString filename = RzxConfig::historique(ip, hostname);
	if (filename.isNull()) {
		emit status(tr("No history file for user %1").arg(hostname), false);
		return;
	}
	
	QString text;
	QFile file(filename);
	if(!file.exists()){
	  emit status(tr("No history file for user %1").arg(hostname), false);
	  return;
	} 
	file.open(IO_ReadOnly);	
	QTextStream stream(&file);
	while(!stream.eof()) {
		text += stream.readLine();
	}
	file.close();
	
	// construction de la boite de dialogue
	QDialog* histoDialog = new QDialog(this->parentWidget(), "Histo", false, WDestructiveClose);
	QPixmap iconeProg((const char **)q);
	iconeProg.setMask(iconeProg.createHeuristicMask() );		
	histoDialog->setIcon(iconeProg);
	
	QGridLayout * qHistoLayout = new QGridLayout(histoDialog);
	qHistoLayout->setSpacing(0);
	qHistoLayout->setMargin(6);
	
#ifdef WIN32
	histoDialog->setCaption( tr( "History" ) +"[Qt]");
#else
	histoDialog->setCaption( tr( "History" ) );
#endif

	// creation de la liste des proprietes et ajout au layout
	QTextView* histoView = new QTextView(histoDialog, "HistoView");
	qHistoLayout->addWidget((QWidget*)histoView, 0, 0);
	
	histoView -> setText(text);
	histoDialog->resize(300, 300);
	histoDialog->show();
}

// affichage des proprietes d'un ordinateur
void RzxRezal::showProperties(const RzxHostAddress&, const QString& msg)
{
	// creation de la boite de dialogue (non modale, elle se detruit automatiquement grace a WDestructiveClose)
	QDialog* propertiesDialog = new QDialog(this->parentWidget(), "ClientProp", false, WDestructiveClose);
	propertiesDialog->resize(300, 300);

		QPixmap iconeProg((const char **)q);
		iconeProg.setMask(iconeProg.createHeuristicMask() );
		propertiesDialog->setIcon(iconeProg);


	// Layout, pour le resize libre
    QGridLayout * qPropertiesLayout = new QGridLayout(propertiesDialog);
    qPropertiesLayout->setSpacing(0);
    qPropertiesLayout->setMargin(6);

#ifdef WIN32
	propertiesDialog->setCaption( tr( "Computer properties" ) +"[Qt]");
#else
	propertiesDialog->setCaption( tr( "Computer properties" ) );
#endif

	// creation de la liste des proprietes et ajout au layout
	QListView* PropList = new QListView(propertiesDialog, "PropView");
	qPropertiesLayout->addWidget((QWidget*)PropList, 0, 0);
	
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
	for (QStringList::Iterator itItem = props.begin(); itItem != props.end(); itItem++)
	{
		QStringList::Iterator itLabel = itItem++;
		if((*itLabel).length()==0) break;
		if(*itItem) { // si la chaine est vide, on prend pas.
			vi = new QListViewItem(PropList, vi, (*itLabel), (*itItem));
			propCount++;
		}
	}
	propertiesDialog->show();
	
	// Fit de la fenetre, on ne le fait pas si il n'y a pas d'accents, sinon ca plante
	if (propCount > 0)
	{
		int width=PropList->columnWidth(0)+PropList->columnWidth(1)+4+12;
		int height=(PropList->childCount()+1)*(*PropList->firstChild()).height()+8+12;
		propertiesDialog->resize(width,height);
	}
}

void RzxRezal::removeFromFavorites(){
	QString temp=currentItem()->text(1);
	RzxConfig::globalConfig()->favorites->remove(temp);
	sort();
	RzxConfig::globalConfig()->writeFavorites();
}

void RzxRezal::addToFavorites(){
	QString temp=currentItem()->text(1);
	RzxConfig::globalConfig()->favorites->insert(temp,new QString("1"));
	sort();
	RzxConfig::globalConfig()->writeFavorites();
}

void RzxRezal::ftp(){
	RzxItem::ListViewItem* item=(RzxItem::ListViewItem*) currentItem();
//	int serveurs=item->servers;
	QString tempPath = RzxConfig::globalConfig()->FTPPath();
	QString tempip = (item->ip).toString();
	QString ip=tempip;
	tempip="ftp://"+tempip;

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

		WinExec(cmd.latin1(), 1);
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

		WinExec(cmd.latin1(), 1);

	}

	// bulletproof FTP
/*	else if (iFtpClient == 3)
	{
		RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\BPFTP\\Bullet Proof FTP\\Options"), 0, KEY_ALL_ACCESS, &hKey);
		RegSetValueEx(hKey, TEXT("FirewallEnabled"), 0, REG_SZ, LPBYTE(& strRegValue), 2 * sizeof(TCHAR));
		RegCloseKey(hKey);
	}*/

	else{ // client FTP standard
		QString cmd="explorer " + tempip;
		WinExec(cmd.latin1(), 1);
	}
#else
	QString cmd = "cd "+tempPath+"; "+RzxConfig::globalConfig()->ftpCmd()+" "+tempip+" &";
	if(RzxConfig::globalConfig()->ftpCmd() != "gftp")
	#ifdef WITH_KDE
		cmd="konsole -e "+cmd;
	#else
		cmd="xterm -e "+cmd;
	#endif
	system(cmd.latin1());
#endif
}

void RzxRezal::samba(){
	RzxItem::ListViewItem* item=(RzxItem::ListViewItem*) currentItem();

	QString tempip = (item -> ip).toString();
	QString tempPath = RzxConfig::globalConfig()->FTPPath();

#ifdef WIN32
	int serveurs=item->servers;
	QString cmd = "explorer \\\\" + (item -> ip).toString();
	WinExec(cmd.latin1(), 1);
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

void RzxRezal::http(){
	RzxItem::ListViewItem* item=(RzxItem::ListViewItem*) currentItem();
	QString tempip = "http://" + (item -> ip).toString();

#ifdef WIN32
    if( RzxConfig::globalConfig()->httpCmd().compare("standard") == 0)
    {
        ShellExecute( NULL, NULL, tempip, NULL, NULL, SW_SHOW );
    }
    else
    {
    	int serveurs=item->servers;
    	QString cmd;
    	HKEY hKey;
    	RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software"), 0, KEY_ALL_ACCESS, &hKey);
		if(!RzxConfig::globalConfig()->httpCmd().compare("opera") &&
		    !RegOpenKeyEx(hKey, TEXT("Opera Software"), 0, KEY_ALL_ACCESS, &hKey)){
		
			unsigned char buffer[MAX_PATH];
			unsigned long KeyType = 0;
			unsigned long KeySize = sizeof(TCHAR) * MAX_PATH;
			RegQueryValueEx(hKey, "Last CommandLine", 0, &KeyType, buffer, &KeySize);
			RegCloseKey(hKey);
			QString temp=(char *)buffer;
			cmd = temp + tempip;
	    }
	    else
			cmd = "explorer " + tempip;

	    WinExec(cmd.latin1(),1);
    }
#else
	QString cmd=RzxConfig::globalConfig()->httpCmd()+" " +tempip +" &";
	system(cmd.latin1());
#endif
}

void RzxRezal::news(){
	RzxItem::ListViewItem* item=(RzxItem::ListViewItem*) currentItem();
	QString tempip = "news://" + (item -> ip).toString();
#ifdef WIN32
	int serveurs=item->servers;
	if(!RzxConfig::globalConfig()->newsCmd().compare("standard")){
		QString cmd = "explorer " + tempip;
		WinExec(cmd.latin1(),1);
	}
#else
	if(!RzxConfig::globalConfig()->newsCmd().compare("knode")){
		QString cmd = "knode " + tempip;
		system(cmd.latin1());
	}
#endif
}

void RzxRezal::afficheColonnes(){
	int colonnesAffichees=RzxConfig::colonnes();
  int i;

  for(i =0; i<columns(); i++){
		if((colonnesAffichees>>i) & 1){
    		if(i==0) {
				setColumnWidth(i,(RzxConfig::globalConfig()->computerIconSize()+1)*32+4);
			} else if(i==1) {
				setColumnWidthMode(i,QListView::Maximum);
			} else if(i==2) {
  				if(columnWidth(i)==0) setColumnWidth(i,200);
     		} else {
      		setColumnWidth(i,40);
      	}
		} else {
			setColumnWidthMode(i,Manual);
   		setColumnWidth(i,0);
		}
  	}
	triggerUpdate();
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
  	if(width()>(somme+20))
    	setColumnWidth(colrem, width()-somme-20);
		else
    	setColumnWidthMode(colrem,Maximum);
	}
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
    item = new RzxItem(newComputer, this);
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
		chatWithLogin->info(tr("RECONNECTED"));

  connect(newComputer, SIGNAL(isUpdated()), item, SLOT(update()));
  emit countChange(tr("%1 clients connected").arg(iplist.count()));
  item -> update();

  if( alreadyHere ) sort(); // Retrie la liste pour les éventuelles modifs
  
  afficheColonnes();
  if(RzxConfig::globalConfig()->autoColonne())
  	adapteColonnes();
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
		chatWithLogin->info(tr("DISCONNECTED"));
}

/*************************
* GESTION DU CHAT
*/

void RzxRezal::chat(const RzxHostAddress& peer, const QString& msg) {
	RzxClientListener * dcc = RzxClientListener::object();
	if(RzxConfig::autoResponder()) {
		dcc -> sendResponder(peer, RzxConfig::autoResponderMsg());
		emit status(tr("%1 is now marked as away").arg(peer.toString()), false);
	}
	else {
		RzxChat * chatWindow = chatCreate(peer);
		if (!chatWindow) return;
		chatWindow -> receive(msg);
	}
}

RzxChat * RzxRezal::chatCreate() {
	RzxItem::ListViewItem* item=(RzxItem::ListViewItem*) currentItem();
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
		connect(object, SIGNAL(send(const RzxHostAddress&, const QString&)), 
	    	client, SLOT(sendChat(const RzxHostAddress&, const QString&)));
		connect(object, SIGNAL(showHistorique(unsigned long, QString)), this, SLOT(showHistorique(unsigned long, QString)));
		connect(object, SIGNAL(askProperties(const RzxHostAddress&)), this, SLOT(proprietes(const RzxHostAddress&)));
		chats.insert(peer.toString(), object);
	}
	
	object -> show();

	return object;
}

/** No descriptions */
void RzxRezal::chatDelete(const RzxHostAddress& peerAddress){        
	// Auto-Delete = true, le chat est supprimé automatiquement. Qt rules !!!
  	chats.remove(peerAddress.toString());
}

void RzxRezal::warnProperties(const RzxHostAddress& peer) {
	if(RzxConfig::globalConfig()->warnCheckingProperties()==0)
		return;
	
	RzxChat * object = chats.find(peer.toString());
	RzxComputer * computer = iplist.find(peer.toString());
	if (!computer)
		return;
	if (!object) {
		sysmsg(tr("Properties sent to ")+computer->getName()+" ("+peer.toString()+")");
		return;
	}
	object->append("gray", computer->getName(), tr("has checked your properties"));
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
	RzxItem::ListViewItem * item = static_cast<RzxItem::ListViewItem*> (sender);
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
			RzxItem::ListViewItem * listItem = (RzxItem::ListViewItem *) sender;
			chatCreate(listItem -> ip);
		}
	}
}

/** Memorise la colonne cliquee */
void RzxRezal::onListClicked( QListViewItem * item, const QPoint & pnt, int c ){
    lastColumnClicked = (NumColonne) c;
}

/** No descriptions */
void RzxRezal::serverDisconnected(){        
        setEnabled(false);
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
        server -> close();
}

void RzxRezal::resizeEvent(QResizeEvent * e) {
	QListView::resizeEvent(e);
	if (RzxConfig::autoColonne()) adapteColonnes();
}

void RzxRezal::keyPressEvent(QKeyEvent *e) {
	//QListView::keyPressEvent(e);
	QString s=e->text();
	if(s.isNull() == TRUE)
		return;
	QChar c=s.at(0);
	if(!c.isLetter()) {
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
	RzxItem::ListViewItem* item=static_cast<RzxItem::ListViewItem*> (currentItem());
	RzxHostAddress ipSaved=item->ip; //y a des chances que item soit ! null :)
	while(true) {
		item=static_cast<RzxItem::ListViewItem*>(item->itemBelow());
		if(!item)
			item=static_cast<RzxItem::ListViewItem*>(firstChild()); //au bout on revient au début
		if(item->ip == ipSaved)
			return; //on a fait le tour, personne avec cette lettre
		RzxComputer * comp=iplist.find((item->ip).toString());
		if(comp->getName().lower().at(0)==c) {
			if(RzxConfig::favoritesMode()==0 || !RzxConfig::globalConfig()->favorites->find(item->text(1))) {
				setCurrentItem(item);
				ensureItemVisible(item);
				return;
			}
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
		RzxItem::ListViewItem * item = static_cast<RzxItem::ListViewItem *> (it.current());
		item -> getItem() -> update();
	}
}

void RzxRezal::languageChanged(){
	qDebug("RzxRezal::languageChanged()");
	for (int i = 0; i < numColonnes; i++) {
		setColumnText(i, tr(colNames[i]));
	}
}

