/***************************************************************************
                      rzxproperty.cpp  -  description
                         -------------------
copyright            : (C) 2002 by Benoit Casoetto
email                : benoit.casoetto@m4x.org
***************************************************************************/
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qobjectlist.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qimage.h>
#include <qdir.h>

#include <qapplication.h>
#include <qmessagebox.h>
#include <qbitmap.h>

#include "dnsvalidator.h"
#include "rzxproperty.h"

#ifdef WITH_KDE
#include <kfiledialog.h>
#else
#include <qfiledialog.h>
#endif

#include "rzxconfig.h"
#include "rzxcomputer.h"
#include "rzxserverlistener.h"
#include "rzxpluginloader.h"
#include <qspinbox.h>
#include "rzxrezal.h"
#include "qrezix.h"
#include "q.xpm"

RzxProperty::RzxProperty( QRezix*parent ) : frmPref( parent ) {
	QPixmap iconeProg( ( const char ** ) q );
	iconeProg.setMask( iconeProg.createHeuristicMask() );
	setIcon( iconeProg );
	connect( btnBrowseWorkDir, SIGNAL( clicked() ), this, SLOT( launchDirSelectDialog() ) );
	connect( btnMiseAJour, SIGNAL( clicked() ), this, SLOT( miseAJour() ) );
	connect( btnAnnuler, SIGNAL( clicked() ), this, SLOT( annuler() ) );
	connect( btnOK, SIGNAL( clicked() ), this, SLOT( oK() ) );
	connect( btnBrowse, SIGNAL( clicked() ), this, SLOT( chooseIcon() ) );
	connect( btnBeepBrowse, SIGNAL( clicked() ), this, SLOT( chooseBeep() ) );
	connect(btnAboutQt, SIGNAL(clicked()), this, SLOT(aboutQt()));

	btnBeepBrowse -> setEnabled(false);
	txtBeep -> setEnabled(false);
	connect( chkBeep, SIGNAL(toggled(bool)), btnBeepBrowse, SLOT(setEnabled(bool)));
	connect( chkBeep, SIGNAL(toggled(bool)), txtBeep, SLOT(setEnabled(bool)));
	
	hostname->setValidator( new DnsValidator() );
#ifndef WIN32
	btnAboutQt->hide();
#endif
	initDlg();
}

RzxProperty::~RzxProperty() {}

void RzxProperty::initLangCombo(){
	RzxConfig * config = RzxConfig::globalConfig();
	QDictIterator<QTranslator> it(config->translations);
	languageBox->clear();
	languageBox->insertItem("English");
	for(it.toFirst() ; it.current(); ++it){
		languageBox->insertItem(it.currentKey());
	}
}	

void RzxProperty::initThemeCombo() {
	RzxConfig * config = RzxConfig::globalConfig();

	QStringList userThemes, sysThemes;
	QStringList::ConstIterator themeIt;
	cmbIconTheme -> clear();
	
	QDir temp = config -> userDir();
	if (temp.cd(RzxConfig::themePath)) {
		temp.setSorting(QDir::Name | QDir::IgnoreCase);
		temp.setFilter(QDir::Dirs);
		userThemes = temp.entryList();
		for ( themeIt = userThemes.begin(); themeIt != userThemes.end(); themeIt++ ) {
			if ( (*themeIt).compare( "." ) && (*themeIt).compare( ".." ) )
				cmbIconTheme -> insertItem( *themeIt );
		}
	}
	
	temp = config -> systemDir();
	if (temp.cd(RzxConfig::themePath)) {
		temp.setSorting(QDir::Name | QDir::IgnoreCase);
		temp.setFilter(QDir::Dirs);
		sysThemes = temp.entryList();
		for ( themeIt = sysThemes.begin(); themeIt != sysThemes.end(); themeIt++ ) {
			if ( userThemes.contains(*themeIt) ) continue;
			if ( (*themeIt).compare( "." ) && (*themeIt).compare( ".." ) )
				cmbIconTheme -> insertItem( *themeIt );
		}
	}
}

/** No descriptions */
void RzxProperty::initDlg() {
	RzxConfig * config = RzxConfig::globalConfig();

	QObjectList * l = queryList( "QLineEdit", "txt*" );
	QObjectListIt it( *l );             // iteration sur l'ensemble des objets QLineEdit
	QObject * obj;
	while ((obj = it())) {
		static_cast<QLineEdit *> (obj) -> setText( config -> readEntry( obj -> name(), "" ) );
	}
	delete l;
	
	initThemeCombo();
	initLangCombo();
	chkBeep->setChecked( RzxConfig::beep() );
	
	hostname->setText( RzxConfig::localHost() -> getName() );
	remarque->setText( RzxConfig::localHost() -> getRemarque() );

	server_name->setText( RzxConfig::serverName() );
	chat_port->setValue( RzxConfig::chatPort() );
	server_port->setValue( RzxConfig::serverPort() );
	reconnection->setValue( RzxConfig::reconnection() / 1000 );
	ping_timeout->setValue( RzxConfig::pingTimeout() / 1000 );
	chkAutoResponder->setChecked( RzxConfig::autoResponder() );

	cmbIconSize->setCurrentItem( RzxConfig::computerIconSize() );
	cmbPromo->setCurrentItem( RzxConfig::localHost() ->getPromo() - 1 );
	cmdDoubleClic->setCurrentItem( RzxConfig::doubleClicRole() );
	cmbIconTheme -> setCurrentItem( 0 );

	cmbMenuText->setCurrentItem(RzxConfig::menuTextPosition());
	cmbMenuIcons->setCurrentItem(RzxConfig::menuIconSize());
	
	int i;	// oui, moins beau, mais c parceque VC++ 6 est pas compatible avec
			// la dernière norme C++ ANSI
	for ( i = 0; i < cmbIconTheme -> count(); i++ )
		if ( !RzxConfig::iconTheme().compare( cmbIconTheme->text( i ) ) )
			cmbIconTheme->setCurrentItem( i );

	int servers = RzxConfig::localHost() ->getServerFlags();
	CbSamba->setChecked( servers & RzxComputer::FLAG_SAMBA );
	CbFTP->setChecked( servers & RzxComputer::FLAG_FTP );
	//CbHotline->setChecked( servers & RzxComputer::SERVER_HOTLINE );
	CbHTTP->setChecked( servers & RzxComputer::FLAG_HTTP );
	CbNews->setChecked( servers & RzxComputer::FLAG_NEWS );


	//QRezix * rezix = getRezix();
	int colonnes = RzxConfig::colonnes();
	cbcIcone ->setChecked( colonnes & (1<<RzxRezal::ColIcone) );
	cbcNom ->setChecked( colonnes & (1<<RzxRezal::ColNom) );
	cbcRemarque->setChecked( colonnes & (1<<RzxRezal::ColRemarque) );
	cbcSamba ->setChecked( colonnes & (1<<RzxRezal::ColSamba) );
	cbcFTP ->setChecked( colonnes & (1<<RzxRezal::ColFTP) );
	//cbcHotline ->setChecked( colonnes & (1<<RzxRezal::ColHotline) );
	cbcHTTP ->setChecked( colonnes & (1<<RzxRezal::ColHTTP) );
	cbcNews ->setChecked( colonnes & (1<<RzxRezal::ColNews) );
	cbcOS ->setChecked( colonnes & (1<<RzxRezal::ColOS) );
	cbcGateway ->setChecked( colonnes & (1<<RzxRezal::ColGateway) );
	cbcPromo ->setChecked( colonnes & (1<<RzxRezal::ColPromo) );
	cbQuit->setChecked(RzxConfig::showQuit());

	clientFtp ->clear();
	clientHttp ->clear();
	clientNews ->clear();


#ifdef WIN32
	// attention a rzxrezal.cpp en cas de modif

	clientFtp->insertItem("standard");
	clientFtp->insertItem("LeechFTP");
	clientFtp->insertItem("SmartFTP");

	clientHttp->insertItem("standard");
	clientHttp->insertItem("iExplore", 1);
	clientHttp->insertItem("Opera", 2);

	clientNews->insertItem("standard");
#else
	// commandes a executer sous nux

	clientFtp->insertItem("gftp", 0);
	clientFtp->insertItem("lftp", 1);

	clientHttp->insertItem("galeon", 0);
	clientHttp->insertItem("konqueror", 1);
	clientHttp->insertItem("lynx", 2);
	clientHttp->insertItem("mozilla", 3);
	clientHttp->insertItem("firefox", 4);
	clientHttp->insertItem("netscape", 5);
	clientHttp->insertItem("opera", 6);

	clientNews->insertItem("knode", 0);
#endif

	clientFtp->setCurrentText(RzxConfig::globalConfig()->ftpCmd());
	clientHttp->setCurrentText(RzxConfig::globalConfig()->httpCmd());
	clientNews->setCurrentText(RzxConfig::globalConfig()->newsCmd());

/*
	clientFtp->setCurrentItem( 0 );
	for ( i = 0; i < clientFtp->count(); i++ )
		if ( !RzxConfig::globalConfig() ->ftpCmd().compare( clientFtp->text( i ) ) )
			clientFtp->setCurrentItem( i );

	clientHttp->setCurrentItem( 0 );
	for ( i = 0; i < clientHttp->count(); i++ )
		if ( !RzxConfig::globalConfig() ->httpCmd().compare( clientHttp->text( i ) ) )
			clientHttp->setCurrentItem( i );

	clientNews->setCurrentItem( 0 );
	for ( i = 0; i < clientNews->count(); i++ )
		if ( !RzxConfig::globalConfig() ->newsCmd().compare( clientNews->text( i ) ) )
			clientNews->setCurrentItem( i );
*/

	txtWorkDir->setText( RzxConfig::globalConfig() ->FTPPath() );
	writeColDisplay();

	//adjustAuto->setChecked( RzxConfig::globalConfig() ->readEntry( "autoCol", 1 ) );
	cbSystray->setChecked( RzxConfig::globalConfig() ->useSystray() );
	cbPropertiesWarning->setChecked(RzxConfig::globalConfig() -> warnCheckingProperties() );
	cbPrintTime->setChecked(RzxConfig::globalConfig() -> printTime());
	
	QPixmap localhostIcon = *RzxConfig::localhostIcon();
	pxmIcon -> setPixmap( localhostIcon );

	cmbSport->setCurrentItem( RzxConfig::globalConfig() -> numSport() );
	
	languageBox->setCurrentItem( 0 );
	for ( i = 0; i < languageBox->count(); i++ ){
		if (tr("English")==languageBox->text(i)){
			languageBox->setCurrentItem( i );
		}
	}

	RzxPlugInLoader::global()->makePropListView(lvPlugInList, btnPlugInProp);
}


bool RzxProperty::updateLocalHost()
{
	bool refresh = (RzxConfig::localHost() -> getName().lower() != hostname->text().lower());
	RzxConfig::localHost() -> setName(hostname->text());
	if(refresh) RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_DNSNAME, NULL);
	
	refresh = refresh || RzxConfig::localHost()->getRemarque() != remarque->text();
	RzxConfig::localHost() -> setRemarque(remarque -> text());
	refresh = refresh || RzxConfig::localHost()->getPromo() != cmbPromo->currentItem()+1;
	RzxConfig::localHost() -> setPromo(cmbPromo->currentItem() + 1);
	refresh = refresh || RzxConfig::autoResponder() != chkAutoResponder->isChecked();
	RzxConfig::localHost() -> setRepondeur(chkAutoResponder -> isChecked());

	int servers = 0;
	if ( CbSamba->isChecked() )
		servers |= RzxComputer::FLAG_SAMBA;
	if ( CbFTP->isChecked() )
		servers |= RzxComputer::FLAG_FTP;
	if ( CbHTTP->isChecked() )
		servers |= RzxComputer::FLAG_HTTP;
	if ( CbNews->isChecked() )
		servers |= RzxComputer::FLAG_NEWS;

	int oldservers = RzxConfig::localHost()->getServerFlags();
	if((servers & RzxComputer::FLAG_SAMBA) ^ (oldservers & RzxComputer::FLAG_SAMBA))
		RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_DISPSMB, NULL);
	if((servers & RzxComputer::FLAG_NEWS) ^ (oldservers & RzxComputer::FLAG_NEWS))
		RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_DISPNEWS, NULL);
	if((servers & RzxComputer::FLAG_HTTP) ^ (oldservers & RzxComputer::FLAG_HTTP))
		RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_DISPHTTP, NULL);
	if((servers & RzxComputer::FLAG_FTP) ^ (oldservers & RzxComputer::FLAG_FTP))
		RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_DISPFTP, NULL);
	
	RzxConfig::localHost() -> setServerFlags(servers);
	return refresh || servers != oldservers;
}


void RzxProperty::miseAJour() {
	RzxConfig * cfgObject = RzxConfig::globalConfig();
	QRezix * ui = getRezix();
	
	QObjectList * l = queryList( "QLineEdit", "txt*" );
	QObjectListIt it( *l );             // iteration sur l'ensemble des objets QCheckBox
	QObject * obj;
	while ( (obj = it()) ) { // Appliquer l'algorithme qui suit tous ces objets
		cfgObject->writeEntry( obj->name(), static_cast<QLineEdit *>(obj) -> text() );
	}
	delete l;
	
	bool iconSizeChanged = cfgObject -> computerIconSize() != cmbIconSize -> currentItem();
	bool themeChanged = RzxConfig::iconTheme().compare( cmbIconTheme -> currentText() );

	bool localHostUpdated = updateLocalHost();
	
	cfgObject -> writeEntry( "dnsname", RzxConfig::localHost() -> getName() );
	cfgObject -> writeEntry( "comment", RzxConfig::localHost() -> getRemarque() );
	cfgObject -> writeEntry( "promo", RzxConfig::localHost() -> getPromo() );
	cfgObject -> writeEntry( "repondeur", RzxConfig::localHost() -> getRepondeur() );
	cfgObject -> writeEntry( "servers", RzxConfig::localHost() -> getServerFlags() );

	cfgObject -> writeEntry( "doubleClic", cmdDoubleClic->currentItem() );
	cfgObject -> writeEntry( "iconsize", cmbIconSize -> currentItem() );
	cfgObject -> writeEntry( "beep", chkBeep->isChecked() ? 1 : 0 );
	cfgObject -> writeEntry( "txtBeep", txtBeep->text());
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
	cfgObject -> writeEntry( "autoCol",1);
	cfgObject -> writeEntry( "useSystray", cbSystray->isChecked() ? 1 : 0 );
	RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_WORKSPACE, NULL);
	cfgObject -> writeEntry( "FTPPath", txtWorkDir->text() );
	cfgObject -> writeEntry( "txtSport", cmbSport->currentText() );
	cfgObject -> writeEntry( "numSport", cmbSport->currentItem());
	cfgObject -> writeEntry( "language", languageBox->currentText() );
	cfgObject -> writeShowQuit(cbQuit->isChecked());

	if(RzxConfig::menuTextPosition() != cmbMenuText->currentItem() || RzxConfig::menuIconSize() != cmbMenuIcons->currentItem())
	{
		cfgObject->writeEntry("menuTextPos", cmbMenuText->currentItem());
		cfgObject->writeEntry("menuIconSize", cmbMenuIcons->currentItem());
		emit cfgObject->iconFormatChange();
		RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_ICONSIZE, NULL);
		RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_ICONTEXT, NULL);
	}
	
	if (ui -> rezal) {
		ui -> rezal -> afficheColonnes();
		if (cfgObject -> autoColonne())
			ui -> rezal -> adapteColonnes();
	}

//	cfgObject -> write(); //flush du fichier de conf

	QPixmap * localhostIcon = pxmIcon -> pixmap();
	if ( RzxConfig::localhostIcon() -> serialNumber() != localhostIcon -> serialNumber() && !( localhostIcon -> isNull() ) )
	{
		localHostUpdated = true;
		RzxConfig::saveIcon( "localhost", *localhostIcon );
		if (!RzxServerListener::object() -> isSocketClosed())
			RzxServerListener::object() -> sendIcon( localhostIcon -> convertToImage() );
	}

	if(localHostUpdated)
		serverUpdate();

	if (ui -> btnAutoResponder)	
		ui -> btnAutoResponder -> setOn( RzxConfig::localHost() -> getRepondeur() );

	QScrollView::ScrollBarMode mode = QScrollView::AlwaysOff;
	if (ui -> rezal) ui -> rezal -> setHScrollBarMode( mode );

	if (ui -> tray)
		ui -> tray -> setVisible(cbSystray->isChecked());
	
	RzxConfig::globalConfig() -> writeEntry("warnCheckingProperties", (cbPropertiesWarning->isChecked() ? 1: 0));
	RzxConfig::globalConfig() -> writeEntry("printTime", cbPrintTime->isChecked() ? 1 : 0);
		
	if ( iconSizeChanged && ui -> rezal)
		ui -> rezal -> redrawAllIcons();
	if ( themeChanged )
	{
		RzxConfig::setIconTheme( QObject::parent(), cmbIconTheme -> currentText() );
		RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_THEME, NULL);
	}
		
}

bool RzxProperty::infoCompleted()
{
	return RzxConfig::propLastName() != "" && RzxConfig::propName() != "" && RzxConfig::numSport() && RzxConfig::propCasert() != "" 
		&& RzxConfig::propMail() != "" && RzxConfig::propTel() != "";
}

QString RzxProperty::infoNeeded()
{
	QString msg = "";
	if(RzxConfig::propLastName() == "")
		msg += "\t" + tr("Surname:").remove(':') + "\n";
	if(RzxConfig::propName() == "")
		msg += "\t" + tr("First name:").remove(':') + "\n";
	if(!RzxConfig::numSport())
		msg += "\t" + tr("Sport:").remove(':') + "\n";
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
	if(!infoCompleted())
	{
		if(infoCompleteMessage() == QMessageBox::Cancel)
			QRezix::global()->close();
	}
	else
		close();
}


void RzxProperty::oK() {
	miseAJour();
	if(tr("English")!=languageBox->currentText())
		RzxConfig::setLanguage(languageBox->currentText());
	annuler();
}


/** No descriptions */
QRezix * RzxProperty::getRezix() const {
	QObject * object = parent();
	if ( !object -> isA( "QRezix" ) )
		return 0;
	return ( QRezix * ) object;
}

/** No descriptions */
void RzxProperty::serverUpdate() {
	// MAJ sur le serveur
	RzxServerListener * server = RzxServerListener::object();
	if (server -> isSocketClosed()) return;
	RzxComputer * localhostObject = RzxConfig::localHost();
	server -> sendRefresh( localhostObject );
}

void RzxProperty::writeColDisplay() {
	int colonnesAffichees = 0;
	if ( cbcIcone ->isChecked() ) colonnesAffichees |= 1<<RzxRezal::ColIcone;
	if ( cbcNom ->isChecked() ) colonnesAffichees |= 1<<RzxRezal::ColNom;
	if ( cbcRemarque->isChecked() ) colonnesAffichees |= 1<<RzxRezal::ColRemarque;
	if ( cbcSamba ->isChecked() ) colonnesAffichees |= 1<<RzxRezal::ColSamba;
	if ( cbcFTP ->isChecked() ) colonnesAffichees |= 1<<RzxRezal::ColFTP;
	//if ( cbcHotline ->isChecked() ) colonnesAffichees |= 1<<RzxRezal::ColHotline;
	if ( cbcHTTP ->isChecked() ) colonnesAffichees |= 1<<RzxRezal::ColHTTP;
	if ( cbcNews ->isChecked() ) colonnesAffichees |= 1<<RzxRezal::ColNews;
	if ( cbcOS ->isChecked() ) colonnesAffichees |= 1<<RzxRezal::ColOS;
	if ( cbcGateway ->isChecked() ) colonnesAffichees |= 1<<RzxRezal::ColGateway;
	if ( cbcPromo ->isChecked() ) colonnesAffichees |= 1<<RzxRezal::ColPromo;

	RzxConfig::globalConfig() ->writeEntry( "colonnes", colonnesAffichees );
}


QString RzxProperty::browse(const QString& name, const QString& title, const QString& glob) {
#ifdef WITH_KDE
	QString filter = glob + "|" + name + " (" + glob + ")";
	QString file = KFileDialog::getOpenFileName( QString::null, filter, this, title );
#else
	QString filter = name + " (" + glob + ")";
	QString file = QFileDialog::getOpenFileName( QString::null, filter, this, "ChooseIcon", title);
#endif
	return file;
}
/** No descriptions */
void RzxProperty::chooseIcon() {
	QString file = browse(tr("Icons"), tr("Icon selection"), "*.png *.jpg *.bmp");
	if ( file.isEmpty() ) return ;

	QPixmap icon;
	if ( !icon.load( file ) ) {
		QMessageBox::warning( this,
		                      tr("Error !"),
		                      tr("Selected file is not valid"),
		                      tr("OK") );
		return ;
	}

	pxmIcon -> setPixmap( icon );
}

void RzxProperty::chooseBeep() {
	QString file = browse(tr("All files"), tr("Sound file selection"), "*");
	if (file.isEmpty()) return;

	txtBeep -> setText(file);
}

void RzxProperty::launchDirSelectDialog() {
	QString temp;
	if ( RzxConfig::globalConfig() ->FTPPath() )
#ifdef WITH_KDE

		temp = KFileDialog::getExistingDirectory ( RzxConfig::globalConfig() ->FTPPath()
		        , 0, tr("Choose default ftp folder") );
	else
		temp = KFileDialog::getExistingDirectory ( ".", 
					 0, tr("Choose default ftp folder") );
#else

		temp = QFileDialog::getExistingDirectory ( RzxConfig::globalConfig() ->FTPPath().latin1(), 
					0, 0, tr("Choose default ftp folder"), true );
	else
		temp = QFileDialog::getExistingDirectory ( ".", 
					0, 0, tr("Choose default ftp folder"), true );
#endif

	if ( temp )
		txtWorkDir->setText( temp );
}

///Affichage de la fenêtre A propos de Qt...
void RzxProperty::aboutQt()
{
	QMessageBox::aboutQt(this);
}

