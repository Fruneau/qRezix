/***************************************************************************
                          rzxcomputer.h  -  description
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
#ifndef RZXCOMPUTER_H_2565
#define RZXCOMPUTER_H_2565

#include <qobject.h>
#include <qpixmap.h>
#include <qsocketdevice.h>
#include "rzxhostaddress.h"

/**
  *@author Sylvain Joyeux
  */


class RzxComputer : public QObject  {
	Q_OBJECT
	
public: 
#ifndef Q_OS_MACX
	struct version_t
	{
		unsigned FunnyVersion	:14;
		unsigned MinorVersion	:7;
		unsigned MajorVersion	:3;
		unsigned Client			:8;	//1 = ReziX; 2 = XNet, 3 = MacXNet, 4 = CPANet 5 = CocoaXNet 6 = qrezix 7 = mxnet
	};
  struct options_t
	{
		unsigned Server			:6;
		unsigned SysEx				:3;	//0=Inconnu, 1=Win9X, 2=WinNT, 3=Linux, 4=MacOS, 5=MacOS X
		unsigned Promo				:2; //0 = Orange, 1=Jne, 2=Rouje (Chica la rouje !)
		unsigned Repondeur		:2; //0=accepter, 1= repondeur, 2=refuser les messages, 3= unused
		// total 13 bits / 32
		unsigned Capabilities	:19;
	};
#else
	struct version_t
	{
		unsigned Client			:8;	//1 = ReziX; 2 = XNet, 3 = MacXNet, 4 = CPANet 5 = CocoaXNet 6 = qrezix 7 = mxnet
		unsigned MajorVersion	:3;
		unsigned MinorVersion	:7;
		unsigned FunnyVersion	:14;
	};
  struct options_t
	{
		unsigned Capabilities	:19;
		// total 13 bits / 32
		unsigned Repondeur		:2; //0=accepter, 1= repondeur, 2=refuser les messages, 3= unused
		unsigned Promo				:2; //0 = Orange, 1=Jne, 2=Rouje (Chica la rouje !)
		unsigned SysEx				:3;	//0=Inconnu, 1=Win9X, 2=WinNT, 3=Linux, 4=MacOS, 5=MacOS X
		unsigned Server			:6;
	};
#endif
	unsigned int ServerFlags;
	
	enum Server {
		SERVER_SAMBA = 1,
		SERVER_FTP = 2,
		SERVER_HOTLINE = 4,
		SERVER_HTTP = 8,
		SERVER_NEWS = 16
	};

	enum ServerFlags {
		FLAG_SAMBA = 1,
		FLAG_FTP = 2,
		FLAG_HOTLINE = 4,
		FLAG_HTTP = 8,
		FLAG_NEWS = 16
	};

	
	enum SysEx {
		SYSEX_UNKNOWN = 0,
		SYSEX_WIN9X = 1,
		SYSEX_WINNT = 2,
		SYSEX_LINUX = 3,
		SYSEX_MACOS = 4,
		SYSEX_MACOSX = 5
	};

	enum Promal {
		PROMAL_UNK = 0,
		PROMAL_ORANGE = 1,
		PROMAL_ROUJE = 2,
		PROMAL_JONE = 3
	};

	static const char * promalText[4];
	
	enum Repondeur {
		REP_ACCEPT = 0,
		REP_ON = 1,
		REP_REFUSE = 2
	};	
	
	//définition des capabilities supplémentaires connues
	enum Capabilities {
		CAP_ON = 1,
		CAP_CHAT = 2,
		CAP_XPLO = 4
	};

	RzxComputer();
	~RzxComputer();

	/** Pour la creation de localHost */
	void initLocalHost();

	bool parse(const QString& params);

	QString serialize(bool stamp = false);

	void setName(const QString& text);
	void setPromo(int promo);
	void setRepondeur(bool);
	void setServers(int servers);
	void setServerFlags(int serverFlags);
	void setRemarque(const QString& text);
	void setIcon(const QPixmap& image);
	
	QString getName() const;
	int getPromo() const;
	QString getPromoText() const;
	int getRepondeur() const;
	int getServers() const;
	int getServerFlags() const;
	QString getRemarque() const;
	QPixmap getIcon() const;
	QString getClient() const;
	QString getResal(bool shortname = true) const;
	bool can(unsigned int cap);

	QString getFilename() const;
	options_t getOptions() const;
	version_t getVersion() const;
	RzxHostAddress getIP() const;
	unsigned long getFlags() const;
	
	void loadIcon();
	void removePreviousIcons();
	void autoSetOs();

	void runScanFtp();
	void stopScanFtp();

signals: // Signals
	void isUpdated();
	void needIcon(const RzxHostAddress&);

public slots:
	void scanServers();

protected:
	QString name;
	options_t options;
	version_t version;
	RzxHostAddress ip;
	unsigned long flags;
	unsigned long stamp;
	QString remarque;
	QPixmap icon;
	QTimer *delayScan;
};

#endif
