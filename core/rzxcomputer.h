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

#include <QObject>
#include <QPixmap>
#include <QFlags>
#include <QMetaType>

#include <RzxGlobal>
#include <RzxConfig>
#include <RzxHostAddress>
#include <RzxNetwork>

/**
  *@author Sylvain Joyeux
  */

///Représentation d'un ordinateur
/** Gère la totalité des informations représentant un ordinateur en fournissant
 * un accès facile et intuitif au données, des signaux qui permettent d'informer
 * des changements d'état de la machine.
 *
 * Cette classe fait partie du coeur de qRezix et constituant le moyen de stockage
 * des informations concernant les utilisateurs.
 */
class  RZX_CORE_EXPORT RzxComputer : public QObject {
/******************************* En-tête Qt ***************************/
	Q_OBJECT
	Q_PROPERTY(QString name READ name WRITE setName)
	Q_PROPERTY(Servers servers READ servers WRITE setServers)
	Q_PROPERTY(Servers serverFlags READ serverFlags WRITE setServerFlags)
	Q_PROPERTY(QString remarque READ remarque WRITE setRemarque)
	Q_PROPERTY(QString properties READ properties WRITE setProperties)
	Q_PROPERTY(QPixmap icon READ icon WRITE setIcon)
	Q_PROPERTY(RzxHostAddress ip READ ip WRITE setIP)
	Q_PROPERTY(RzxNetwork* network READ network)
	Q_PROPERTY(quint32 stamp READ stamp)
	//Commentés parce que Rzx est un namespace et donc ça marche pas :/
	//pourtant Qt est aussi un namespace...
	//Q_PROPERTY(Rzx::Promal promo READ promo WRITE setPromo)
	//Q_PROPERTY(Rzx::ConnectionState state READ state WRITE setState)
	Q_ENUMS(ServerFlags)
	Q_FLAGS(Servers)


/************* Définition des différentes données *********************/
///Décrit différentes informations concernant l'ordinateur
/** Cette structure provient du protocole xNet
 */
#if defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN
	struct options_t
	{
		unsigned Server			:6;
		unsigned SysEx				:3;	//0=Inconnu, 1=Win9X, 2=WinNT, 3=Linux, 4=MacOS, 5=MacOS X, 6=BSD
		unsigned Promo				:2; //0 = Orange, 1=Jne, 2=Rouje (Chica la rouje ! <== bah nan, à la jône !!!)
		unsigned Repondeur		:2; //0=accepter, 1= repondeur, 2=refuser les messages, 3= unused
		// total 13 bits / 32
		unsigned Capabilities	:19;
	};
#else
  struct options_t
	{
		unsigned Capabilities	:19;
		// total 13 bits / 32
		unsigned Repondeur		:2; //0=accepter, 1= repondeur, 2=refuser les messages, 3= unused
		unsigned Promo				:2; //0 = Orange, 1=Jne, 2=Rouje (Chica la rouje !) Chic à la jône (X03) !!!
		unsigned SysEx				:3;	//0=Inconnu, 1=Win9X, 2=WinNT, 3=Linux, 4=MacOS, 5=MacOS X, 6=BSD
		unsigned Server			:6;
	};
#endif

	enum Repondeur {
		REP_ACCEPT = 0,
		REP_ON = 1,
		REP_REFUSE = 2
	};
	
public:
///Description de la verion du client xNet
/** La structure de ce numéro hérite du protocole xNet
 */
#if defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN
	struct version_t
	{
		unsigned FunnyVersion	:14;
		unsigned MinorVersion	:7;
		unsigned MajorVersion	:3;
		unsigned Client			:8;	//1 = ReziX; 2 = XNet, 3 = MacXNet, 4 = CPANet 5 = CocoaXNet 6 = qrezix 7 = mxnet
	};
#else
	struct version_t
	{
		unsigned Client			:8;	//1 = ReziX; 2 = XNet, 3 = MacXNet, 4 = CPANet 5 = CocoaXNet 6 = qrezix 7 = mxnet
		unsigned MajorVersion	:3;
		unsigned MinorVersion	:7;
		unsigned FunnyVersion	:14;
	};
#endif
	enum ServerFlags {
		SERVER_SAMBA = 1,
		SERVER_FTP = 2,
		SERVER_HOTLINE = 4,
		SERVER_PRINTER = SERVER_HOTLINE,
		SERVER_HTTP = 8,
		SERVER_NEWS = 16
	};

	Q_DECLARE_FLAGS(Servers, ServerFlags)

private:
	///Chaînes de caractères des promos
	static const char *promalText[4];

	///Chaînes de caractères des OS
	static const char *osText[7];


/************* Représentation de localhost *********************/
	QTimer *delayScan;
	static RzxComputer *m_localhost;
	int locked;
	bool edited;
	bool updated;
	bool testLocalhost;

protected:
	/** Pour la creation de localHost */
	void initLocalhost();
	void autoSetOs();

public:
	static RzxComputer *localhost();
	static void buildLocalhost();
	bool isLocalhost() const;
	bool isLocalhostObject() const;

protected slots:
	//Reconstruction de la liste des serveurs
	void scanServers();


/************* Construction et destruction d'instances *********************/
private:
	RzxNetwork *m_network;

	//private car il ne doit être utilisé que pour la construction de localhost
	RzxComputer();

public:
	RzxComputer(RzxNetwork *, const RzxHostAddress&, const QString&, quint32, quint32, quint32, quint32, const QString&);
	~RzxComputer();

//Réseau
	RzxNetwork *network() const;

	void update(const QString&, quint32, quint32, quint32, const QString&);
	QString serialize(const QString& pattern) const;
	void loadIcon();

public slots:
	void logout();
	void login();
	void emitUpdate();

	void lock();
	void unlock();


//Quelques signaux
signals:
	void update(RzxComputer*);
	void needIcon(const RzxHostAddress&);
	void favoriteStateChanged(RzxComputer*);
	void stateChanged(RzxComputer*);

protected:
	void emitStateChanged();

//Favoris/ignore
public:
	bool isFavorite() const;
	bool isIgnored() const;

public slots:
	void ban();
	void unban();
	void addToFavorites();
	void removeFromFavorites();


/******************* Propriétés de l'objet ********************************/
private:
	QString m_name;
	QString m_remarque;
	RzxHostAddress m_ip;

	Servers m_serverFlags;
	options_t m_options;
	version_t m_version;
	unsigned long m_flags;
	unsigned long m_stamp;

	QPixmap m_icon;

	bool connected;

public:
	const QString &name() const;
	QString remarque(bool lb = false) const;

	Rzx::Promal promo() const;
	QString promoText() const;

	Rzx::SysEx sysEx() const;
	QString sysExText() const;

	Rzx::ConnectionState state() const;
	bool isOnResponder() const;

	unsigned long flags() const;
	options_t options() const;
	quint32 stamp() const;
	QPixmap icon() const;

	Servers servers() const;
	Servers serverFlags() const;
	bool hasSambaServer() const;
	bool hasFtpServer() const;
	bool hasHttpServer() const;
	bool hasNewsServer() const;
	bool hasPrinter() const;
	bool hasEmailAddress() const;

	const RzxHostAddress &ip() const;
	QString rezalName(bool shortname = true) const;
	int rezal() const;
	bool isSameGateway(RzxComputer *computer = 0) const;
	bool isSameGateway(const RzxComputer&) const;

	version_t version() const;
	QString client() const;
	bool can(Rzx::Capabilities) const;
	bool canBeChatted() const;
	bool canBeChecked(bool = true) const;

public slots:
	void setName(const QString& text);
	void setRemarque(const QString& text);
	void setPromo(Rzx::Promal promo);
	void setState(Rzx::ConnectionState);
	void setState(bool);
	void setConnection(bool);
	void setServers(Servers);
	void setServerFlags(Servers);
	void setIcon(const QPixmap& image);
	void setIP(const RzxHostAddress&);
	void addCapabilities(int, bool = true);


/******************* Accès aux serveurs distantsz ***************************************/
public slots:
	void ftp(const QString& path = QString()) const;
	void http(const QString& path = QString()) const;
	void samba(const QString& path = QString()) const;
	void news(const QString& path = QString()) const;
	void mail() const;


/******************* Information non intrinsèque à l'objet ********************************/
public:
	QString properties() const;

public slots:
	//Chat, historique, propriétés
	void history();
	void checkProperties();
	void setProperties(const QString&);
	void chat();
	void sendChat(Rzx::ChatMessageType, const QString& = QString());
	void receiveChat(Rzx::ChatMessageType, const QString& = QString());

signals:
	void receiveChat(RzxComputer *, Rzx::ChatMessageType, const QString&);

public:
	///Fonction de test pour automatiser les filtres sur les RzxComputer
	typedef bool (testComputer)(const RzxComputer*);
};

///Déclaration pour le MetaType RzxComputer dans le but d'utiliser le RzxComputer comme metatype
Q_DECLARE_METATYPE(RzxComputer*)

RZX_CORE_EXPORT RzxComputer::testComputer testComputerChat;
RZX_CORE_EXPORT RzxComputer::testComputer testComputerProperties;
RZX_CORE_EXPORT RzxComputer::testComputer testComputerFtp;
RZX_CORE_EXPORT RzxComputer::testComputer testComputerHttp;
RZX_CORE_EXPORT RzxComputer::testComputer testComputerSamba;
RZX_CORE_EXPORT RzxComputer::testComputer testComputerNews;
RZX_CORE_EXPORT RzxComputer::testComputer testComputerMail;
RZX_CORE_EXPORT RzxComputer::testComputer testComputerFavorite;
RZX_CORE_EXPORT RzxComputer::testComputer testComputerNotFavorite;
RZX_CORE_EXPORT RzxComputer::testComputer testComputerBan;
RZX_CORE_EXPORT RzxComputer::testComputer testComputerNotBan;
RZX_CORE_EXPORT RzxComputer::testComputer testComputerSameGateway;
RZX_CORE_EXPORT bool computerLessThan(const RzxComputer*, const RzxComputer*);

#endif
