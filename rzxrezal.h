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
#include <q3listview.h>
#include <q3dict.h>
#include <q3popupmenu.h>
#include <q3socket.h>
#include <qtimer.h>
#include <qdatetime.h>
//Added by qt3to4:
#include <QKeyEvent>
#include <QResizeEvent>
#include "rzxdict.h"

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
class RzxPopupMenu : public Q3PopupMenu {
	Q_OBJECT
	public:
		RzxPopupMenu( QWidget * parent = 0, const char * name = 0 );

	protected:
		void keyPressEvent(QKeyEvent *e);
};

class RzxRezal : public Q3ListView  {
	Q_OBJECT

	RzxConnectionLister *lister;
	Q3Dict<RzxItem> itemByIp;

public: 
	RzxRezal(QWidget * parent, const char * name = 0);
	~RzxRezal();
	
	void showNotFavorites(bool val);
	void loginFromLister(bool val);

	// Numero des colonnes
	enum NumColonne
		{ ColIcone = 0, ColNom = 1, ColRemarque = 2, ColSamba = 3, ColFTP = 4,
		  /*ColHotline = 5,*/ ColHTTP = 5, ColNews = 6, ColOS = 7, ColGateway = 8,
		  ColPromo = 9, ColResal = 10, ColIP = 11, ColClient = 12, numColonnes = 13 };

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

	QTime search_timeout;
	QString search_patern;

	RzxDict<QString,RzxItem*> search_items;	// Arbre binaire de recherche équilibré
						// contenant les associations nom->Item
	Q3Dict<QString> search_items0;	// dictionnaire avec référencement des noms/IP
	
	NumColonne lastColumnClicked;  // Pour savoir sur quelle icone on double-clique
	
public slots: // Public slots
	void creePopUpMenu(Q3ListViewItem *inutile1,const QPoint & pos,int inutile);
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
	void set_search(const QString& msg);
	void favoriteRemoved(RzxComputer*);
	void favoriteAdded(RzxComputer*);
	void newFavorite(RzxComputer*);
	void changeFavorite(RzxComputer*);
	void lostFavorite(RzxComputer*);

protected slots: // Protected slots
	void onListDblClicked(Q3ListViewItem * sender);
	void onListClicked(Q3ListViewItem *, const QPoint &, int);
	void historique();
	void redrawSelectedIcon(Q3ListViewItem *sel = NULL);
	void buildToolTip(Q3ListViewItem* i) const;
	void logBufLogins();

protected:
	void resizeEvent(QResizeEvent * e);
	void keyPressEvent(QKeyEvent *e);

};

#endif


