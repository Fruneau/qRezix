/***************************************************************************
                          rzxrezal.h  -  description
                             -------------------
    begin                : Thu Jan 24 2002
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

#ifndef RZXREZAL_H
#define RZXREZAL_H

#include <qlistview.h>
#include <qdict.h>
#include <qpopupmenu.h>
#include <qsocket.h>

#include "rzxserverlistener.h"
#include "rzxclientlistener.h"
#include "rzxcomputer.h"
#include "rzxchat.h"

#define USER_HASH_TABLE_LENGTH 1663

/**
  *@author Sylvain Joyeux
  */

#define USER_HASH_TABLE_LENGTH 1663

//pour réimplanter le clavier et la touche droite, ne mérite pas un .h/.cpp pour lui tt seul
class RzxPopupMenu : public QPopupMenu {
	Q_OBJECT
public:
	RzxPopupMenu( QWidget * parent = 0, const char * name = 0 );

protected:
	void keyPressEvent(QKeyEvent *e);
};

class RzxRezal : public QListView  {
	Q_OBJECT
public: 
	RzxRezal(QWidget * parent, const char * name);
	void initConnection();
	~RzxRezal();
	void closeSocket();

	// Numero des colonnes
	enum NumColonne
		{ ColIcone = 0, ColNom = 1, ColRemarque = 2, ColSamba = 3, ColFTP = 4,
		  /*ColHotline = 5,*/ ColHTTP = 5, ColNews = 6, ColOS = 7, ColGateway = 8,
		  ColPromo = 9, numColonnes = 10 };

	static const char * colNames[numColonnes];

protected: // Protected attributes
	QDict<RzxComputer> iplist;
	RzxServerListener * server;
	RzxClientListener * client;
	QDict<RzxChat> chats;
	// Definit necessaire pour le menu contextuel
	RzxPopupMenu popup;

	NumColonne lastColumnClicked;  // Pour savoir sur quelle icone on double-clique


public slots: // Public slots
	void creePopUpMenu(QListViewItem *inutile1,const QPoint & pos,int inutile);
	void proprietes();
	void proprietes( const RzxHostAddress & );
	void samba();
	void ftp();
	void http();
	void news();

	void showHistorique( unsigned long ip, QString hostname );

	void afficheColonnes();
	void adapteColonnes();
	virtual void recvIcon(QImage*Icon, const RzxHostAddress&);
	virtual void login(const QString& ordi);
	virtual void logout(const RzxHostAddress& ip);
	void redrawAllIcons();
	bool isSocketClosed() const;
	void sysmsg(const QString& msg);
	void fatal(const QString& msg);

	void removeFromFavorites();
	void addToFavorites();
	
	void chat(QSocket* socket, const QString& msg);
	void showProperties(const RzxHostAddress& peer, const QString& msg);
	RzxChat * chatCreate(const RzxHostAddress& peer);
	RzxChat * chatCreate();
	void chatDelete(const RzxHostAddress& peerAddress);
	void warnProperties(const RzxHostAddress& peer);
	
	void serverConnected();
	void serverDisconnected();
	void languageChanged();

signals: // Signals
	void needIcon(const RzxHostAddress&);
	void status(const QString& msg, bool fatal);
	void countChange(const QString& newCount);
	void socketClosed();

protected slots: // Protected slots
	void onListDblClicked(QListViewItem * sender);
	void onListClicked(QListViewItem *, const QPoint &, int);
	void historique();

protected:
	void resizeEvent(QResizeEvent * e);
	void keyPressEvent(QKeyEvent *e);

};

#endif


