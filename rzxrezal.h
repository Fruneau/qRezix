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

#include <qstringlist.h>
#include <qlistview.h>
#include <qdict.h>
#include <qpopupmenu.h>
#include <qsocket.h>
#include <qtimer.h>

#define USER_HASH_TABLE_LENGTH 1663

/**
  *@author Sylvain Joyeux (et tout d'autres aussi dans les années qui suivirent !! :))
  */

class RzxItem;
class RzxChat;
class RzxConnectionLister;
class RzxClientListener;
class RzxServerListener;
class RzxComputer;
class RzxHostAddress;

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
	
	RzxConnectionLister *lister;
	QDict<RzxItem> itemByIp;
	
public: 
	RzxRezal(QWidget * parent, const char * name);
	~RzxRezal();
	
	void showNotFavorites(bool val);
	void loginFromLister(bool val);

	// Numero des colonnes
	enum NumColonne
		{ ColIcone = 0, ColNom = 1, ColRemarque = 2, ColSamba = 3, ColFTP = 4,
		  /*ColHotline = 5,*/ ColHTTP = 5, ColNews = 6, ColOS = 7, ColGateway = 8,
		  ColPromo = 9, numColonnes = 10 };

	static const char * colNames[numColonnes];
	RzxItem *selected;
	
	QTimer *timer;

protected: // Protected attributes
	bool dispNotFavorites, warnConnection;
	RzxClientListener * client;
	
	// Pour les filtres
	bool filterOn;
	QString filter;

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

	void afficheColonnes();
	void adapteColonnes();
	//void login(RzxComputer *computer);
	void bufferedLogin(RzxComputer *computer);
	void logout(const QString& ip);
	void logout(RzxComputer *computer);
	void clear();
	void init();
	void redrawAllIcons();
	void activeFilter(bool on);
	void setFilter(const QString &text);

	void removeFromFavorites();
	void addToFavorites();
	
	void removeFromIgnoreList();
	void addToIgnoreList();
	
	RzxChat * chatCreate(const QString& login = 0);
	
	void languageChanged();

signals: // Signals
	void status(const QString& msg, bool fatal);
	void favoriteRemoved(RzxComputer*);
	void favoriteAdded(RzxComputer*);

protected slots: // Protected slots
	void onListDblClicked(QListViewItem * sender);
	void onListClicked(QListViewItem *, const QPoint &, int);
	void historique();
	void redrawSelectedIcon(QListViewItem *sel = NULL);
	void buildToolTip(QListViewItem* i);
	void logBufLogins();

protected:
	void resizeEvent(QResizeEvent * e);
	void keyPressEvent(QKeyEvent *e);

};

#endif


