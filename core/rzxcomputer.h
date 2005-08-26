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
class RzxComputer : public QObject  {
	Q_OBJECT
	Q_PROPERTY(QString name READ name WRITE setName)
	Q_PROPERTY(Servers servers READ servers WRITE setServers)
	Q_PROPERTY(Servers serverFlags READ serverFlags WRITE setServerFlags)
	Q_PROPERTY(QString remarque READ remarque WRITE setRemarque)
	Q_PROPERTY(QPixmap icon READ icon WRITE setIcon)
	Q_PROPERTY(RzxHostAddress ip READ ip WRITE setIP)
	Q_PROPERTY(quint32 stamp READ stamp)
	//Commentés parce que Rzx est un namespace et donc ça marche pas :/
	//pourtant Qt est aussi un namespace...
	//Q_PROPERTY(Rzx::Promal promo READ promo WRITE setPromo)
	//Q_PROPERTY(Rzx::ConnectionState state READ state WRITE setState)
	Q_ENUMS(ServerFlags)
	Q_FLAGS(Servers)
	
#ifndef Q_OS_MAC
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
#ifndef Q_OS_MAC
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
		SERVER_HTTP = 8,
		SERVER_NEWS = 16
	};

	Q_DECLARE_FLAGS(Servers, ServerFlags)

private:
	///Chaînes de caractères des promos
	static const char *promalText[4];

	///Chaînes de caractères des OS
	static const char *osText[7];

	static RzxComputer *m_localhost;

	//private car il ne doit être utilisé que pour la construction de localhost
	RzxComputer();

public:
	static RzxComputer *localhost();
	static void buildLocalhost();

	RzxComputer(const RzxHostAddress&, const QString&, quint32, quint32, quint32, quint32, const QString&);
	~RzxComputer();

	QString serialize(const QString& pattern) const;

	void update(const QString&, quint32, quint32, quint32, const QString&);

protected:
	/** Pour la creation de localHost */
	void initLocalhost();

public slots:
	void logout();
	void login();
	void setName(const QString& text);
	void setPromo(Rzx::Promal promo);
	void setState(Rzx::ConnectionState);
	void setState(bool);
	void setServers(Servers);
	void setServerFlags(Servers);
	void setRemarque(const QString& text);
	void setIcon(const QPixmap& image);
	void setIP(const RzxHostAddress&);
	void addCapabilities(int);
	
public:
	const QString &name() const;
	const QString &remarque() const;

	QPixmap icon() const;
	quint32 stamp() const;

	options_t options() const;
	Rzx::Promal promo() const;
	QString promoText() const;
	Rzx::ConnectionState state() const;
	Rzx::SysEx sysEx() const;
	QString sysExText() const;
	Servers servers() const;
	Servers serverFlags() const;
	bool hasSambaServer() const;
	bool hasFtpServer() const;
	bool hasHttpServer() const;
	bool hasNewsServer() const;

	version_t version() const;
	QString client() const;
	bool can(Rzx::Capabilities) const;

	const RzxHostAddress &ip() const;
	QString rezalName(bool shortname = true) const;
	Rzx::RezalId rezal() const;
	bool isSameGateway(RzxComputer *computer = 0) const;
	bool isSameGateway(const RzxComputer&) const;
	bool isLocalhost() const;

	unsigned long flags() const;

	bool isFavorite() const;
	bool isIgnored() const;
	bool isOnResponder() const;
	
	void loadIcon();
	void autoSetOs();

	void runScanFtp();
	void stopScanFtp();

	static Servers toServerFlags(int);

signals: // Signals
	void update(RzxComputer*);
	void needIcon(const RzxHostAddress&);
	void favoriteStateChanged(RzxComputer*);
	void stateChanged(RzxComputer*);

	void wantChat(RzxComputer*);
	void wantProperties(RzxComputer*);
	void wantHistorique(RzxComputer*);

protected:
	void emitStateChanged();

protected slots:
	//Reconstruction de la liste des serveurs
	void scanServers();

public slots:
	//Lancement des clients
	void ftp(const QString& path = QString()) const;
	void http(const QString& path = QString()) const;
	void samba(const QString& path = QString()) const;
	void news(const QString& path = QString()) const;
	//Ban/Favoris
	void ban();
	void unban();
	void addToFavorites();
	void removeFromFavorites();
	//Chat, historique, propriétés
	void historique();
	void proprietes();
	void chat();

protected:
	QString m_name;
	Servers m_serverFlags;
	options_t m_options;
	version_t m_version;
	RzxHostAddress m_ip;
	unsigned long m_flags;
	unsigned long m_stamp;
	QString m_remarque;
	QPixmap m_icon;
	QTimer *delayScan;
	bool connected;
};

///Déclaration pour le MetaType RzxComputer dans le but d'utiliser le RzxComputer comme metatype
Q_DECLARE_METATYPE(RzxComputer*)

///Retourne un objet représentant localhost
/** Localhost est un RzxComputer* qui contient toutes les informations représentant l'ordinateur local */
inline RzxComputer *RzxComputer::localhost()
{
	if(!m_localhost)
		buildLocalhost();
	return m_localhost;
}

///Construit l'objet représentant localhost
inline void RzxComputer::buildLocalhost()
{
	if(m_localhost) return;
	m_localhost = new RzxComputer();
	m_localhost->initLocalhost();
}

///Retourne la version texte du nom du sous-réseau
/** Ne fait que réaliser la conversion en chaîne de caractères du RezalId */
inline QString RzxComputer::rezalName(bool shortname) const
{ return m_ip.rezalName(shortname); }

///Indique si l'objet est dans les favoris
inline bool RzxComputer::isFavorite() const
{ return RzxConfig::global()->isFavorite(*this); }

///Indique si l'objet est dans les machines ignorées
inline bool RzxComputer::isIgnored() const
{ return RzxConfig::global()->isBan(*this); }

///Indique si la machine est sur répondeur
/** Permet de traduire simplement l'état 'sur répondeur' qui correspond à 2 Rzx::ConnectionState différents
 * et donc qui plus casse pied à tester que here et disconnected
 */
inline bool RzxComputer::isOnResponder() const
{
	Rzx::ConnectionState m_state = state();
	return m_state == Rzx::STATE_AWAY || m_state == Rzx::STATE_REFUSE;
}

///Conversion d'un entier en QFlags<ServerFlags> == Servers
inline RzxComputer::Servers RzxComputer::toServerFlags(int rawServers)
{
	Servers servers;
	if(rawServers & RzxComputer::SERVER_SAMBA) servers |= RzxComputer::SERVER_SAMBA;
	if(rawServers & RzxComputer::SERVER_FTP) servers |= RzxComputer::SERVER_FTP;
	if(rawServers & RzxComputer::SERVER_HTTP) servers |= RzxComputer::SERVER_HTTP;
	if(rawServers & RzxComputer::SERVER_NEWS) servers |= RzxComputer::SERVER_NEWS;
	if(rawServers & RzxComputer::SERVER_HOTLINE) servers |= RzxComputer::SERVER_HOTLINE;
	return servers;
}

#endif
