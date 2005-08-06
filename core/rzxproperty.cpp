/***************************************************************************
                      rzxproperty.cpp  -  description
                         -------------------
copyright            : (C) 2002 by Benoit Casoetto
email                : benoit.casoetto@m4x.org
***************************************************************************/
#include <QPushButton>
#include <QToolButton>
#include <QToolBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QImage>
#include <QDir>
#include <QMessageBox>
#include <QStackedWidget>
#include <QBitmap>
#include <QRegExpValidator>
#include <QPixmap>
#include <QTranslator>
#include <QTreeWidget>
#include <QListWidget>
#include <QSize>

#ifdef WITH_KDE
#include <kfiledialog.h>
#else
#include <QFileDialog>
#endif

#include "rzxproperty.h"

#include "defaults.h"

#include "rzxmessagebox.h"
#include "rzxhostaddress.h"
#include "rzxconfig.h"
#include "rzxcomputer.h"
#include "rzxserverlistener.h"
#include "rzxpluginloader.h"
#include "rzxiconcollection.h"

#include "../mainui/qrezix.h"
#include "../mainui/rzxrezalmodel.h"
#include "../tray/rzxtrayicon.h"
#include "../rzxapplication.h"

RzxProperty::RzxProperty(QWidget *parent)
	:QDialog(parent)
{
	setupUi(this);
	
	connect( btnBrowseWorkDir, SIGNAL( clicked() ), this, SLOT( launchDirSelectDialog() ) );
	connect( btnMiseAJour, SIGNAL( clicked() ), this, SLOT( miseAJour() ) );
	connect( btnAnnuler, SIGNAL( clicked() ), this, SLOT( annuler() ) );
	connect( btnOK, SIGNAL( clicked() ), this, SLOT( oK() ) );
	connect( btnBrowse, SIGNAL( clicked() ), this, SLOT( chooseIcon() ) );
	connect( btnBeepBrowse, SIGNAL( clicked() ), this, SLOT( chooseBeep() ) );
	connect( btnBeepBrowse_3, SIGNAL( clicked()), this, SLOT(chooseBeepConnection())) ;
	connect(btnAboutQt, SIGNAL(clicked()), this, SLOT(aboutQt()));
	connect(cmbMenuIcons, SIGNAL(activated(int)), this,SLOT(lockCmbMenuText(int)));
	connect( lbMenu, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(changePage(QListWidgetItem*, QListWidgetItem*)));

	btnBeepBrowse -> setEnabled(false);
	btnBeepBrowse_3 -> setEnabled(false);
	txtBeep -> setEnabled(false);
	txtBeepFavorites -> setEnabled(false);
	connect( chkBeep, SIGNAL(toggled(bool)), btnBeepBrowse, SLOT(setEnabled(bool)));
	connect( chkBeep, SIGNAL(toggled(bool)), txtBeep, SLOT(setEnabled(bool)));
	connect( chkBeepFavorites, SIGNAL(toggled(bool)), btnBeepBrowse_3, SLOT(setEnabled(bool)));
	connect( chkBeepFavorites, SIGNAL(toggled(bool)), txtBeepFavorites, SLOT(setEnabled(bool)));
	connect( btnChangePass, SIGNAL(clicked()), RzxServerListener::object(), SLOT(changePass()));
	connect(RzxIconCollection::global(), SIGNAL(themeChanged(const QString& )), this, SLOT(changeTheme()));

	//Pour que le pseudo soit rfc-complient
	hostname->setValidator( new QRegExpValidator(QRegExp("[a-zA-Z0-9](-?[a-zA-Z0-9])*"), hostname) );
	//Les pseudos trop longs c'est vraiment imbitable
	hostname->setMaxLength(32);

	//La remarque doit tenir en une seule ligne...
	remarque->setValidator( new QRegExpValidator(QRegExp("[^\n\r]+"), remarque) );
	
	//Pour éviter que les gens mettent un | dans leurs propriétés
	QRegExpValidator *propValidator = new QRegExpValidator(QRegExp("[^|]*"), this);
	txtName->setValidator(propValidator);
	txtSurname->setValidator(propValidator);
	txtFirstname->setValidator(propValidator);
	txtMail->setValidator(propValidator);
	txtWeb->setValidator(propValidator);
	txtPhone->setValidator(propValidator);
	txtCasert->setValidator(propValidator);
	lbMenu->setIconSize(QSize(32,32));

	//Initialisation de la treeview des plugins
	lvPlugInList->setIconSize(QSize(16,16));
	lvPlugInList->setUniformRowHeights(false);
	lvPlugInList->setHeaderLabels(QStringList() << tr("Name") << tr("Description") << tr("Version"));

#ifndef WIN32
	btnAboutQt->hide();
#endif
#if defined(WIN32) || defined(Q_OS_MAC)
	lblWorkDir_2->hide();
	txtBeepCmd->hide();
#endif

#ifdef Q_OS_MAC
	groupSystray->hide();
#endif

#ifndef Q_OS_UNIX
	sbTraySize->hide();
	lblTraySize->hide();
#endif
	btnPlugInReload->hide();

	initDlg();
	changeTheme();
	lbMenu->setCurrentRow(0);
}

RzxProperty::~RzxProperty() {}

/// Change le thème d'icône de la fenêtre de préférence
/** Le changement de thème correspond à la reconstruction de la listbox de menu (pour que les icônes soient conformes au thème choisi), et au changement des icônes OK, Annuler, Appliquer */
void RzxProperty::changeTheme()
{
	btnAnnuler->setIcon(RzxIconCollection::getIcon(Rzx::ICON_CANCEL));
	btnOK->setIcon(RzxIconCollection::getIcon(Rzx::ICON_OK));
	btnMiseAJour->setIcon(RzxIconCollection::getIcon(Rzx::ICON_APPLY));

#define setIcon(icon, name) lbMenu->item(name)->setIcon(icon)
	QPixmap pixmap; //Pour le newItem
	if(RzxIconCollection::getIcon(Rzx::ICON_SYSTRAYHERE).isNull())
		setIcon(RzxIconCollection::qRezixIcon(), 0);
	else
		setIcon(RzxIconCollection::getIcon(Rzx::ICON_SYSTRAYHERE), 0);
	setIcon(RzxIconCollection::getIcon(Rzx::ICON_LAYOUT), 1);
	setIcon(RzxIconCollection::getIcon(Rzx::ICON_NETWORK), 2);
	setIcon(RzxIconCollection::getIcon(Rzx::ICON_PREFERENCES), 3);
	setIcon(RzxIconCollection::getIcon(Rzx::ICON_PLUGIN), 4);
#undef setIcon
}

void RzxProperty::changePage(QListWidgetItem *current, QListWidgetItem *)
{
	if(current)
		lblTitle->setText("<h2>"+current->text()+"</h2>");
}

///Remplissage de la boîte des langues
void RzxProperty::initLangCombo()
{
	languageBox->clear();
	languageBox->insertItems(0, RzxConfig::translationsList());
	languageBox->setCurrentIndex(languageBox->findText(RzxConfig::translation()));
}

///Initialise la liste des thèmes
void RzxProperty::initThemeCombo() {
	cmbIconTheme -> clear();
	cmbIconTheme->insertItems(0, RzxIconCollection::global()->themeList());
	cmbIconTheme->setCurrentIndex(cmbIconTheme->findText(RzxIconCollection::global()->theme()));
}

/** No descriptions */
void RzxProperty::initDlg()
{
	RzxConfig * config = RzxConfig::global();

	// iteration sur l'ensemble des objets QLineEdit
	QList<QLineEdit*> l = findChildren<QLineEdit*>(QRegExp("txt.*"));
	foreach(QLineEdit *edit, l)
		edit->setText(config->readEntry(edit->objectName(), ""));
	
	btnChangePass->setEnabled(!RzxServerListener::object()->isSocketClosed());
	
	initThemeCombo();
	initLangCombo();
	chkBeep->setChecked( RzxConfig::beep() );
	chkBeepFavorites->setChecked( RzxConfig::beepConnection());
	cbWarnFavorite->setChecked( RzxConfig::showConnection());
	
	hostname->setText( RzxComputer::localhost()->name() );
	remarque->setText( RzxComputer::localhost()->remarque() );

	server_name->setText( RzxConfig::serverName() );
	chat_port->setValue( RzxConfig::chatPort() );
	server_port->setValue( RzxConfig::serverPort() );
	reconnection->setValue( RzxConfig::reconnection() / 1000 );
	ping_timeout->setValue( RzxConfig::pingTimeout() / 1000 );
	chkAutoResponder->setChecked( RzxConfig::autoResponder() );

	cmbIconSize->setCurrentIndex( RzxConfig::computerIconSize() );
	cmbPromo->setCurrentIndex( RzxComputer::localhost()->promo() - 1 );
	cmdDoubleClic->setCurrentIndex( RzxConfig::doubleClicRole() );

	cmbMenuIcons->setCurrentIndex(RzxConfig::menuIconSize());
	cmbMenuText->setCurrentIndex(RzxConfig::menuTextPosition());
	lockCmbMenuText(RzxConfig::menuIconSize());

	cmbDefaultTab -> setCurrentIndex(RzxConfig::defaultTab());

	int servers = RzxComputer::localhost()->serverFlags();
	CbSamba->setChecked(servers & RzxComputer::SERVER_SAMBA);
	CbFTP->setChecked(servers & RzxComputer::SERVER_FTP);
	CbHTTP->setChecked(servers & RzxComputer::SERVER_HTTP);
	CbNews->setChecked(servers & RzxComputer::SERVER_NEWS);
	
	int tooltip = RzxConfig::tooltip();
	cbTooltips->setChecked(tooltip & RzxConfig::Enable);
	cbTooltipFtp->setChecked(tooltip & RzxConfig::Ftp);
	cbTooltipHttp->setChecked(tooltip & RzxConfig::Http);
	cbTooltipNews->setChecked(tooltip & RzxConfig::News);
	cbTooltipSamba->setChecked(tooltip & RzxConfig::Samba);
	cbTooltipPromo->setChecked(tooltip & RzxConfig::Promo);
	cbTooltipOS->setChecked(tooltip & RzxConfig::OS);
	cbTooltipVersion->setChecked(tooltip & RzxConfig::Client);
	cbTooltipIP->setChecked(tooltip & RzxConfig::IP);
	cbTooltipResal->setChecked(tooltip & RzxConfig::Resal);
	cbTooltipFeatures->setChecked(tooltip & RzxConfig::Features);
	cbTooltipProperties->setChecked(tooltip & RzxConfig::Properties);

	int colonnes = RzxConfig::colonnes();
	cbcNom ->setChecked( colonnes & (1<<RzxRezalModel::ColNom) );
	cbcRemarque->setChecked( colonnes & (1<<RzxRezalModel::ColRemarque) );
	cbcSamba ->setChecked( colonnes & (1<<RzxRezalModel::ColSamba) );
	cbcFTP ->setChecked( colonnes & (1<<RzxRezalModel::ColFTP) );
	cbcHTTP ->setChecked( colonnes & (1<<RzxRezalModel::ColHTTP) );
	cbcNews ->setChecked( colonnes & (1<<RzxRezalModel::ColNews) );
	cbcOS ->setChecked( colonnes & (1<<RzxRezalModel::ColOS) );
	cbcGateway ->setChecked( colonnes & (1<<RzxRezalModel::ColGateway) );
	cbcPromo ->setChecked( colonnes & (1<<RzxRezalModel::ColPromo) );
	cbcResal ->setChecked( colonnes & (1<<RzxRezalModel::ColRezal) );
	cbcIP ->setChecked( colonnes & (1<<RzxRezalModel::ColIP) );
	cbcClient ->setChecked( colonnes & (1<<RzxRezalModel::ColClient) );
	cbQuit->setChecked(RzxConfig::showQuit());

	cbHighlight->setChecked(RzxConfig::computerIconHighlight());
	cbRefuseAway->setChecked(RzxConfig::refuseWhenAway());
	sbTraySize->setValue(RzxConfig::traySize());
	
	clientFtp ->clear();
	clientHttp ->clear();
	clientNews ->clear();


#ifdef WIN32
	// attention a rzxutilslauncher.cpp en cas de modif
	clientFtp->addItem("standard");
	clientFtp->addItem("iExplore");
	clientFtp->addItem("LeechFTP");
	clientFtp->addItem("SmartFTP");

	clientHttp->addItem("standard");
	clientHttp->addItem("iExplore", 1);
	clientHttp->addItem("Opera", 2);

	clientNews->addItem("standard");
#else
#ifdef Q_OS_MAC
	// commandes à exécuter sous macos X
	clientFtp->addItem("Default");
	clientHttp->addItem("Default");
	clientNews->addItem("Default");
#else
	// commandes a executer sous nux
	clientFtp->addItem("gftp");
	clientFtp->addItem("lftp");

	clientHttp->addItem("galeon");
	clientHttp->addItem("konqueror");
	clientHttp->addItem("lynx");
	clientHttp->addItem("mozilla");
	clientHttp->addItem("firefox");
	clientHttp->addItem("netscape");
	clientHttp->addItem("opera");

	clientNews->addItem("knode");
#endif //MAC
#endif //WIN32

#define setValue(cb, cmd) \
	if(!cb->findText(cmd)) cb->addItem(cmd); \
	cb->setCurrentIndex(cb->findText(cmd))

	setValue(clientFtp, RzxConfig::global()->ftpCmd());
	setValue(clientHttp, RzxConfig::global()->httpCmd());
	setValue(clientNews, RzxConfig::global()->newsCmd());
//	clientFtp->setItemText(RzxConfig::global()->ftpCmd());
//	clientHttp->setItemText(RzxConfig::global()->httpCmd());
//	clientNews->setItemText(RzxConfig::global()->newsCmd());
#undef setValue
	txtWorkDir->setText( RzxConfig::global()->FTPPath() );
	writeColDisplay();
	
	cbSystray->setChecked( RzxConfig::global() ->useSystray() );
	cbSearch->setChecked( RzxConfig::global() ->useSearch() );
	cbPropertiesWarning->setChecked(RzxConfig::global() -> warnCheckingProperties() );
	cbPrintTime->setChecked(RzxConfig::global() -> printTime());
	
	pxmIcon->setPixmap(RzxIconCollection::global()->localhostPixmap());

	cmbSport->setCurrentIndex( RzxConfig::global() -> numSport() );
	
	if(((RzxApplication*)RzxApplication::instance())->isInitialised())
		RzxPlugInLoader::global()->makePropListView(lvPlugInList, btnPlugInProp, btnPlugInReload);
}

///Met à jour l'objet représentant localhost
bool RzxProperty::updateLocalHost()
{
	bool refresh = (RzxComputer::localhost()->name().toLower() != hostname->text().toLower());
	RzxComputer::localhost()->setName(hostname->text());
	if(refresh) RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_DNSNAME, NULL);
	
	refresh = refresh || RzxComputer::localhost()->remarque() != remarque->text();
	RzxComputer::localhost() -> setRemarque(remarque -> text());
	refresh = refresh || RzxComputer::localhost()->promo() != cmbPromo->currentIndex()+1;
	RzxComputer::localhost()->setPromo((Rzx::Promal)(cmbPromo->currentIndex() + 1));
	refresh = refresh || RzxConfig::autoResponder() != chkAutoResponder->isChecked();
	RzxComputer::localhost()->setState(chkAutoResponder->isChecked());
	
	refresh = refresh || (RzxConfig::refuseWhenAway() ^ cbRefuseAway->isChecked());

	QFlags<RzxComputer::ServerFlags> servers;
	if ( CbSamba->isChecked() )
		servers |= RzxComputer::SERVER_SAMBA;
	if ( CbFTP->isChecked() )
		servers |= RzxComputer::SERVER_FTP;
	if ( CbHTTP->isChecked() )
		servers |= RzxComputer::SERVER_HTTP;
	if ( CbNews->isChecked() )
		servers |= RzxComputer::SERVER_NEWS;

	QFlags<RzxComputer::ServerFlags> oldservers = RzxComputer::localhost()->serverFlags();
	if((servers & RzxComputer::SERVER_SAMBA) ^ (oldservers & RzxComputer::SERVER_SAMBA))
		RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_DISPSMB, NULL);
	if((servers & RzxComputer::SERVER_NEWS) ^ (oldservers & RzxComputer::SERVER_NEWS))
		RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_DISPNEWS, NULL);
	if((servers & RzxComputer::SERVER_HTTP) ^ (oldservers & RzxComputer::SERVER_HTTP))
		RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_DISPHTTP, NULL);
	if((servers & RzxComputer::SERVER_FTP) ^ (oldservers & RzxComputer::SERVER_FTP))
		RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_DISPFTP, NULL);
	
	RzxComputer::localhost()->setServerFlags(servers);
	return refresh || servers != oldservers;
}


bool RzxProperty::miseAJour() {
	//Vérification que les données sont correctes
	if(!hostname->hasAcceptableInput())
	{
		RzxMessageBox::information(this, tr("Bad properties"),
			tr("Your computer name is not valid...<br>"
				"A computer name can only contains letters, numbers and '-'"));
		return false;
	}
	else if(!remarque->hasAcceptableInput())
	{
		RzxMessageBox::information(this, tr("Bad properties"),
			tr("Your comment is not valid...<br>"
				"It can't contain linebreaks or other 'special' character"));
		return false;
	}
	
	//Mise à jours des données de configuration
	RzxConfig *cfgObject = RzxConfig::global();
	QRezix *ui = getRezix();
	
	// iteration sur l'ensemble des objets QCheckBox
	QList<QLineEdit*> l = findChildren<QLineEdit*>(QRegExp("txt.*"));
	foreach(QLineEdit *edit, l)
		cfgObject->writeEntry(edit->objectName(), edit->text());
	
//	bool iconSizeChanged = (cfgObject->computerIconSize() != cmbIconSize->currentIndex() || cfgObject->computerIconHighlight() != cbHighlight->isChecked());
	bool themeChanged = RzxConfig::iconTheme() != cmbIconTheme->currentText();

	//Indique si les données 'partagées' ont été modifiées
	//localHostUpdated = true ==> besoin de rafraichir le serveur
	bool localHostUpdated = updateLocalHost();
	
	cfgObject -> writeEntry( "dnsname", RzxComputer::localhost()->name() );
	cfgObject -> writeEntry( "comment", RzxComputer::localhost()->remarque() );
	cfgObject -> writeEntry( "promo", RzxComputer::localhost()->promo() );
	cfgObject -> writeEntry( "repondeur", RzxComputer::localhost()->state() );
	cfgObject -> writeEntry( "servers", RzxComputer::localhost()->serverFlags() );

	cfgObject -> writeEntry( "doubleClic", cmdDoubleClic->currentIndex() );
	cfgObject -> writeEntry( "iconsize", cmbIconSize -> currentIndex() );
	cfgObject -> writeEntry( "iconhighlight", cbHighlight->isChecked());
	cfgObject -> writeEntry( "beep", chkBeep->isChecked() ? 1 : 0 );
	cfgObject -> writeEntry( "beepConnection", chkBeepFavorites->isChecked() ? 1: 0);
	cfgObject -> writeEntry( "showConnection", cbWarnFavorite->isChecked() ? 1 : 0);
	cfgObject -> writeEntry( "txtBeep", txtBeep->text());
	cfgObject -> writeEntry( "txtBeepConnection", txtBeepFavorites->text());
	cfgObject -> writeEntry( "txtBeepCmd", txtBeepCmd->text());
	if(server_name->text() != cfgObject->serverName() || server_port->value() != cfgObject->serverPort())
	{
		cfgObject -> writeEntry( "server_name", server_name->text() );
		cfgObject -> writeEntry( "server_port", server_port->value() );
		RzxServerListener::object()->sendPart();
		RzxServerListener::object()->setupConnection();
	}
	else
	{
		cfgObject -> writeEntry( "server_name", server_name->text() );
		cfgObject -> writeEntry( "server_port", server_port->value() );
	}

	cfgObject -> writeEntry( "chat_port", chat_port->value() );
	cfgObject -> writeEntry( "reconnection", reconnection->value() * 1000 );
	cfgObject -> writeEntry( "ping_timeout", ping_timeout->value() * 1000 );

	cfgObject -> httpCmd( clientHttp -> currentText() );
	cfgObject -> ftpCmd( clientFtp -> currentText() );
	cfgObject -> newsCmd( clientNews -> currentText() );

	writeColDisplay();
	cfgObject -> writeEntry( "useSystray", cbSystray->isChecked() ? 1 : 0 );
	cfgObject -> writeEntry( "useSearch", cbSearch->isChecked() ? 1 : 0 );
	cfgObject -> writeEntry( "defaultTab", cmbDefaultTab ->currentIndex() );
	
	RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_WORKSPACE, NULL);
	cfgObject -> writeEntry( "FTPPath", txtWorkDir->text() );
	cfgObject -> writeEntry( "txtSport", cmbSport->currentText() );
	cfgObject -> writeEntry( "numSport", cmbSport->currentIndex());
	cfgObject -> writeEntry( "language", languageBox->currentText() );
	cfgObject -> writeShowQuit(cbQuit->isChecked());
	
	cfgObject -> writeEntry( "refuseAway", cbRefuseAway->isChecked());
	RzxConfig::setAutoResponder(RzxConfig::autoResponder());

#ifdef Q_OS_UNIX
	if(sbTraySize->value() != RzxConfig::traySize())
		cfgObject->writeEntry("traysize", sbTraySize->value());
#endif

	if(RzxConfig::menuTextPosition() != cmbMenuText->currentIndex() || RzxConfig::menuIconSize() != cmbMenuIcons->currentIndex())
	{
		cfgObject->writeEntry("menuTextPos", cmbMenuText->currentIndex());
		cfgObject->writeEntry("menuIconSize", cmbMenuIcons->currentIndex());

		emit cfgObject->iconFormatChange();
		RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_ICONSIZE, NULL);
		RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_ICONTEXT, NULL);
	}
	
	unsigned int tooltip = 0;
	if(cbTooltips->isChecked()) tooltip += RzxConfig::Enable;
	if(cbTooltipFtp->isChecked()) tooltip += RzxConfig::Ftp;
	if(cbTooltipHttp->isChecked()) tooltip += RzxConfig::Http;
	if(cbTooltipNews->isChecked()) tooltip += RzxConfig::News;
	if(cbTooltipSamba->isChecked()) tooltip += RzxConfig::Samba;
	if(cbTooltipPromo->isChecked()) tooltip += RzxConfig::Promo;
	if(cbTooltipOS->isChecked()) tooltip += RzxConfig::OS;
	if(cbTooltipVersion->isChecked()) tooltip += RzxConfig::Client;
	if(cbTooltipIP->isChecked()) tooltip += RzxConfig::IP;
	if(cbTooltipResal->isChecked()) tooltip += RzxConfig::Resal;
	if(cbTooltipFeatures->isChecked()) tooltip += RzxConfig::Features;
	if(cbTooltipProperties->isChecked()) tooltip += RzxConfig::Properties;
	cfgObject->writeEntry( "tooltip", tooltip);

	const QPixmap *localhostIcon = pxmIcon->pixmap();
	if(RzxIconCollection::global()->localhostPixmap().serialNumber() != localhostIcon->serialNumber() && !localhostIcon->isNull())
	{
		localHostUpdated = true;
		RzxIconCollection::global()->setLocalhostPixmap(*localhostIcon);
		if (ui->isInitialised() && !RzxServerListener::object() -> isSocketClosed())
			RzxServerListener::object() -> sendIcon(localhostIcon->toImage());
	}

	if(localHostUpdated)
		RzxServerListener::object()->sendRefresh();

	if(ui) ui->activateAutoResponder(RzxComputer::localhost()->isOnResponder());
		
	RzxConfig::global() -> writeEntry("warnCheckingProperties", (cbPropertiesWarning->isChecked() ? 1: 0));
	RzxConfig::global() -> writeEntry("printTime", cbPrintTime->isChecked() ? 1 : 0);
		
	if(ui) ui->showSearch(cbSearch->isChecked());
	
	if(themeChanged)
	{
		RzxIconCollection::global()->setTheme(cmbIconTheme->currentText());
		cfgObject->writeEntry("theme", cmbIconTheme->currentText());
		RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_THEME, NULL);
	}
	
	/* Mise à jour de l'affichage des Rezal */
	if(ui && ui -> rezal)
		ui -> rezal -> afficheColonnes();

	if (ui && ui -> rezalFavorites)
		ui -> rezalFavorites -> afficheColonnes();
	
	/* Mise à jour de l'état des plugins */
	RzxPlugInLoader::global()->validPropListView();
	
	/* Sauvegarde du fichier du conf */
	RzxConfig::global()->flush();

	//On ne change la langue qu'au dernier moment car ça réinitialise toutes les boîtes
	RzxConfig::setLanguage(languageBox->currentText());
	
	return true;
}

QString RzxProperty::infoNeeded()
{
	QString msg = "";
	if(RzxConfig::propLastName() == "")
		msg += "\t" + tr("Surname:").remove(':') + "\n";
	if(RzxConfig::propName() == "")
		msg += "\t" + tr("First name:").remove(':') + "\n";
	if(RzxConfig::propCasert() == "")
		msg += "\t" + tr("Room:").remove(':') + "\n";
	if(RzxConfig::propMail() == "")
		msg += "\t" + tr("e-mail:").remove(':') + "\n";
	if(RzxConfig::propTel() == "")
		msg += "\t" + tr("Phone number:").remove(':') + "\n";
	return msg;
}

int RzxProperty::infoCompleteMessage()
{
	return QMessageBox::question(this, tr("Incomplete datas"), tr("In order to use qRezix, you have to complete all the following informations :\n")
			+ infoNeeded()
			+ tr("Press OK to reenter these informations, or Cancel to quit qRezix"),
			QMessageBox::Ok, QMessageBox::Cancel);
}

void RzxProperty::annuler() {
	if(RzxConfig::infoCompleted() || (infoCompleteMessage() == QMessageBox::Cancel))
		close();
}


void RzxProperty::oK() {
	if(!miseAJour())
		return;
	annuler();
}


/** No descriptions */
QRezix *RzxProperty::getRezix() const
{
	return qobject_cast<QRezix*>(parent());
}

void RzxProperty::writeColDisplay() {
	int colonnesAffichees = 0;
	if ( cbcNom ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColNom;
	if ( cbcRemarque->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColRemarque;
	if ( cbcSamba ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColSamba;
	if ( cbcFTP ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColFTP;
	if ( cbcHTTP ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColHTTP;
	if ( cbcNews ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColNews;
	if ( cbcOS ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColOS;
	if ( cbcGateway ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColGateway;
	if ( cbcPromo ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColPromo;
	if ( cbcResal ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColRezal;
	if ( cbcClient ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColClient;
	if ( cbcIP ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColIP;

	RzxConfig::global() ->writeEntry( "colonnes", colonnesAffichees );
}


QString RzxProperty::browse(const QString& name, const QString& title, const QString& glob) {
#ifdef WITH_KDE
	QString filter = name + " (" + glob + ")";
	QString file = KFileDialog::getOpenFileName( QString::null, filter, this, title );
#else
	QString filter = name + " (" + glob + ")";
	QString file = QFileDialog::getOpenFileName(this, title, QString::null, filter);
#endif
	return file;
}

/** No descriptions */
void RzxProperty::chooseIcon() {
	QString file = browse(tr("Icons"), tr("Icon selection"), "*.png *.jpg *.bmp");
	if ( file.isEmpty() ) return ;

	QPixmap icon;
	if (!icon.load(file)) {
		RzxMessageBox::warning( this,
		                      tr("Error !"),
		                      tr("Selected file is not valid"));
		return ;
	}

	pxmIcon->setPixmap(icon);
}

void RzxProperty::chooseBeep() {
	QString file = browse(tr("All files"), tr("Sound file selection"), "*");
	if (file.isEmpty()) return;

	txtBeep -> setText(file);
}

void RzxProperty::chooseBeepConnection() {
	QString file = browse(tr("All files"), tr("Sound file selection"), "*");
	if (file.isEmpty()) return;

	txtBeepFavorites -> setText(file);
}

void RzxProperty::launchDirSelectDialog() {
	QString temp;
	if ( !RzxConfig::global()->FTPPath().isNull() )
#ifdef WITH_KDE
		temp = KFileDialog::getExistingDirectory ( RzxConfig::global() ->FTPPath()
		        , 0, tr("Choose default ftp folder") );
	else
		temp = KFileDialog::getExistingDirectory ( ".", 
					 0, tr("Choose default ftp folder") );
#else

		temp = QFileDialog::getExistingDirectory(this, tr("Choose default ftp folder"),
					 RzxConfig::global()->FTPPath()); 
	else
		temp = QFileDialog::getExistingDirectory(this, tr("Choose default ftp folder"), ".");
#endif

	if ( !temp.isNull() )
		txtWorkDir->setText( temp );
}

///Affichage de la fenêtre A propos de Qt...
void RzxProperty::aboutQt()
{
	QMessageBox::aboutQt(this);
}

///Règle les problèmes entre affichage des icônes et du texte
void RzxProperty::lockCmbMenuText(int index)
{
	if(index==0)
	{
		cmbMenuText->setCurrentIndex(1);
		cmbMenuText->setDisabled(true);
	}
	else
		cmbMenuText->setEnabled(true);
}

///Change la langue...
void RzxProperty::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	if(e->type() == QEvent::LanguageChange)
	{
		int row = lbMenu->currentRow();
		retranslateUi(this);
		lbMenu->setCurrentRow(row);
		lvPlugInList->setHeaderLabels(QStringList() << tr("Name") << tr("Description") << tr("Version"));
		changeTheme();
		initDlg();
	}
}
