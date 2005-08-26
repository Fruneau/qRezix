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

#include <RzxProperty>

#include <RzxMessageBox>
#include <RzxHostAddress>
#include <RzxConfig>
#include <RzxComputer>
#include <RzxServerListener>
#include <RzxIconCollection>
#include <RzxApplication>
#include <RzxModule>

RZX_GLOBAL_INIT(RzxProperty)

RzxProperty::RzxProperty(QWidget *parent)
	:QDialog(parent)
{
	object = this;
	setupUi(this);
	
	connect( btnBrowseWorkDir, SIGNAL( clicked() ), this, SLOT( launchDirSelectDialog() ) );
	connect( btnMiseAJour, SIGNAL( clicked() ), this, SLOT( miseAJour() ) );
	connect( btnAnnuler, SIGNAL( clicked() ), this, SLOT( annuler() ) );
	connect( btnOK, SIGNAL( clicked() ), this, SLOT( oK() ) );
	connect( btnBrowse, SIGNAL( clicked() ), this, SLOT( chooseIcon() ) );
	connect(btnAboutQt, SIGNAL(clicked()), this, SLOT(aboutQt()));
	connect( lbMenu, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(changePage(QListWidgetItem*, QListWidgetItem*)));

	//connect( btnChangePass, SIGNAL(clicked()), RzxServerListener::object(), SLOT(changePass()));
	connect(RzxIconCollection::global(), SIGNAL(themeChanged(const QString& )), this, SLOT(changeTheme()));
	connect(cmbMenuIcons, SIGNAL(activated(int)), this,SLOT(lockCmbMenuText(int)));

	//Pour que le pseudo soit rfc-complient
	hostname->setValidator( new QRegExpValidator(QRegExp("[a-zA-Z0-9](-?[a-zA-Z0-9])*"), hostname) );
	//Les pseudos trop longs c'est vraiment imbitable
	hostname->setMaxLength(32);

	//La remarque doit tenir en une seule ligne...
	remarque->setValidator( new QRegExpValidator(QRegExp("[^\n\r]+"), remarque) );
	
	//Pour �viter que les gens mettent un | dans leurs propri�t�s
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
	lvPlugInList->setHeaderLabels(QStringList() << tr("Name") << tr("Version") << tr("Description"));

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

	QList<RzxModule*> modules = RzxApplication::modulesList();
	foreach(RzxModule *module, modules)
	{
		QList<QWidget*> props = module->propWidgets();
		QStringList names = module->propWidgetsName();
		foreach(QWidget *widget, props)
		{
			if(widget)
				prefStack->addWidget(widget);
		}
		foreach(QString name, names)
		{
			QListWidgetItem *item = new QListWidgetItem(lbMenu);
			item->setText(name);
			item->setIcon(module->icon());
		}
		QTreeWidgetItem *item = new QTreeWidgetItem(lvPlugInList);
		item->setIcon(0, module->icon());
		item->setText(0, module->name());
		item->setText(1, module->versionString());
		item->setText(2, module->description());
	}

	initDlg();
	changeTheme();
	lbMenu->setCurrentRow(0);
}

RzxProperty::~RzxProperty()
{
	QList<RzxModule*> modules = RzxApplication::modulesList();
	foreach(RzxModule *module, modules)
	{
		if(module)
			module->propClose();
	}
	object = NULL;
}

/// Change le th�me d'ic�ne de la fen�tre de pr�f�rence
/** Le changement de th�me correspond � la reconstruction de la listbox de menu (pour que les ic�nes soient conformes au th�me choisi), et au changement des ic�nes OK, Annuler, Appliquer */
void RzxProperty::changeTheme()
{
	btnAnnuler->setIcon(RzxIconCollection::getIcon(Rzx::ICON_CANCEL));
	btnOK->setIcon(RzxIconCollection::getIcon(Rzx::ICON_OK));
	btnMiseAJour->setIcon(RzxIconCollection::getIcon(Rzx::ICON_APPLY));

#define setIcon(icon, name) lbMenu->item(name)->setIcon(icon)
	QPixmap pixmap; //Pour le newItem
	setIcon(RzxIconCollection::getIcon(Rzx::ICON_PROPRIETES), 0);
	setIcon(RzxIconCollection::getIcon(Rzx::ICON_PREFERENCES), 1);
	setIcon(RzxIconCollection::getIcon(Rzx::ICON_NETWORK), 2);

	int i = 3;
	QList<RzxModule*> modules = RzxApplication::modulesList();
	foreach(RzxModule *module, modules)
	{
		int nb = module->propWidgets().count();
		for(int j = 0 ; j < nb ; j++, i++)
			setIcon(module->icon(), i);
	}
#undef setIcon
}

///La page doit changer, on met � jour le titre
void RzxProperty::changePage(QListWidgetItem *current, QListWidgetItem *)
{
	if(current)
	{
		lblTitle->setText("<h2>"+current->text()+"</h2>");
		lblTitleIcon->setPixmap(current->icon().pixmap(22));
	}
}

///Remplissage de la bo�te des langues
void RzxProperty::initLangCombo()
{
	languageBox->clear();
	languageBox->insertItems(0, RzxConfig::translationsList());
	languageBox->setCurrentIndex(languageBox->findText(RzxConfig::translation()));
}

///Initialise la liste des th�mes
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
		edit->setText(config->value(edit->objectName(), "").toString());
	
//	btnChangePass->setEnabled(!RzxServerListener::object()->isSocketClosed());
	
	initThemeCombo();
	initLangCombo();
	
	hostname->setText( RzxComputer::localhost()->name() );
	remarque->setText( RzxComputer::localhost()->remarque() );

	server_name->setText( RzxConfig::serverName() );
	server_port->setValue( RzxConfig::serverPort() );
	reconnection->setValue( RzxConfig::reconnection() / 1000 );
	ping_timeout->setValue( RzxConfig::pingTimeout() / 1000 );
	chkAutoResponder->setChecked( RzxConfig::autoResponder() );

	cmbPromo->setCurrentIndex( RzxComputer::localhost()->promo() - 1 );

	cmbMenuIcons->setCurrentIndex(RzxConfig::menuIconSize());
	cmbMenuText->setCurrentIndex(RzxConfig::menuTextPosition());
	lockCmbMenuText(RzxConfig::menuIconSize());

	int servers = RzxComputer::localhost()->serverFlags();
	CbSamba->setChecked(servers & RzxComputer::SERVER_SAMBA);
	CbFTP->setChecked(servers & RzxComputer::SERVER_FTP);
	CbHTTP->setChecked(servers & RzxComputer::SERVER_HTTP);
	CbNews->setChecked(servers & RzxComputer::SERVER_NEWS);
	
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
	// commandes � ex�cuter sous macos X
	clientFtp->addItem("Default");
	clientHttp->addItem("Default");
	clientNews->addItem("Default");
#else
	// commandes a executer sous nux
	clientFtp->addItem("gftp");
	clientFtp->addItem("lftp");
	clientFtp->addItem("konqueror");
	clientFtp->addItem("firefox");

	clientHttp->addItem("galeon");
	clientHttp->addItem("konqueror");
	clientHttp->addItem("lynx");
	clientHttp->addItem("mozilla");
	clientHttp->addItem("firefox");
	clientHttp->addItem("netscape");
	clientHttp->addItem("opera");

	clientNews->addItem("knode");
	clientNews->addItem("thunderbird");
#endif //MAC
#endif //WIN32

#define setValue(cb, cmd) \
	if(!cb->findText(cmd)) cb->addItem(cmd); \
	cb->setCurrentIndex(cb->findText(cmd))

	setValue(clientFtp, RzxConfig::ftpCmd());
	setValue(clientHttp, RzxConfig::httpCmd());
	setValue(clientNews, RzxConfig::newsCmd());
//	clientFtp->setItemText(RzxConfig::global()->ftpCmd());
//	clientHttp->setItemText(RzxConfig::global()->httpCmd());
//	clientNews->setItemText(RzxConfig::global()->newsCmd());
#undef setValue
	txtWorkDir->setText( RzxConfig::ftpPath() );
	
	pxmIcon->setPixmap(RzxIconCollection::global()->localhostPixmap());

	cmbSport->setCurrentIndex( RzxConfig::numSport() );

	QList<RzxModule*> modules = RzxApplication::modulesList();
	foreach(RzxModule *module, modules)
		module->propInit();
}

///Met � jour l'objet repr�sentant localhost
bool RzxProperty::updateLocalHost()
{
	bool refresh = (RzxComputer::localhost()->name().toLower() != hostname->text().toLower());
	RzxComputer::localhost()->setName(hostname->text());
	
	refresh = refresh || RzxComputer::localhost()->remarque() != remarque->text();
	RzxComputer::localhost() -> setRemarque(remarque -> text());
	refresh = refresh || RzxComputer::localhost()->promo() != cmbPromo->currentIndex()+1;
	RzxComputer::localhost()->setPromo((Rzx::Promal)(cmbPromo->currentIndex() + 1));
	refresh = refresh || RzxConfig::autoResponder() != chkAutoResponder->isChecked();
	RzxComputer::localhost()->setState(chkAutoResponder->isChecked());
	
	refresh = refresh || (RzxConfig::refuseWhenAway() ^ cbRefuseAway->isChecked());

	RzxComputer::Servers servers;
	if ( CbSamba->isChecked() )
		servers |= RzxComputer::SERVER_SAMBA;
	if ( CbFTP->isChecked() )
		servers |= RzxComputer::SERVER_FTP;
	if ( CbHTTP->isChecked() )
		servers |= RzxComputer::SERVER_HTTP;
	if ( CbNews->isChecked() )
		servers |= RzxComputer::SERVER_NEWS;

	const RzxComputer::Servers oldservers = RzxComputer::localhost()->serverFlags();
	RzxComputer::localhost()->setServerFlags(servers);
	return refresh || servers != oldservers;
}


bool RzxProperty::miseAJour()
{
	QList<RzxModule*> modules = RzxApplication::modulesList();
	foreach(RzxModule *module, modules)
		module->propUpdate();

	//V�rification que les donn�es sont correctes
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
	
	//Mise � jours des donn�es de configuration
	RzxConfig *cfgObject = RzxConfig::global();
	
	// iteration sur l'ensemble des objets QCheckBox
	QList<QLineEdit*> l = findChildren<QLineEdit*>(QRegExp("txt.*"));
	foreach(QLineEdit *edit, l)
		cfgObject->setValue(edit->objectName(), edit->text());
	
//	bool iconSizeChanged = (cfgObject->computerIconSize() != cmbIconSize->currentIndex() || cfgObject->computerIconHighlight() != cbHighlight->isChecked());
	bool themeChanged = RzxConfig::iconTheme() != cmbIconTheme->currentText();

	//Indique si les donn�es 'partag�es' ont �t� modifi�es
	//localHostUpdated = true ==> besoin de rafraichir le serveur
	bool localHostUpdated = updateLocalHost();
	
	RzxConfig::setDnsName(RzxComputer::localhost()->name() );
	RzxConfig::setRemarque(RzxComputer::localhost()->remarque());
	RzxConfig::setPromo(RzxComputer::localhost()->promo() );
	RzxConfig::setRepondeur(RzxComputer::localhost()->state() );
	RzxConfig::setServers(RzxComputer::localhost()->serverFlags());

	RzxConfig::setBeepCmd(txtBeepCmd->text());
	if(server_name->text() != RzxConfig::serverName() || server_port->value() != RzxConfig::serverPort())
	{
		RzxConfig::setServerName(server_name->text() );
		RzxConfig::setServerPort(server_port->value() );
//		RzxServerListener::object()->sendPart();
//		RzxServerListener::object()->setupConnection();
	}
	else
	{
		RzxConfig::setServerName(server_name->text() );
		RzxConfig::setServerPort(server_port->value() );
	}

	RzxConfig::setReconnection(reconnection->value() * 1000 );
	RzxConfig::setPingTimeout(ping_timeout->value() * 1000 );

	RzxConfig::setHttpCmd( clientHttp -> currentText() );
	RzxConfig::setFtpCmd( clientFtp -> currentText() );
	RzxConfig::setNewsCmd( clientNews -> currentText() );

	RzxConfig::setFtpPath(txtWorkDir->text() );
	RzxConfig::setPropSport(cmbSport->currentText() );
	RzxConfig::setNumSport(cmbSport->currentIndex());
	
	RzxConfig::setRefuseWhenAway(cbRefuseAway->isChecked());
	RzxConfig::setAutoResponder(RzxConfig::autoResponder());

	if(RzxConfig::menuTextPosition() != cmbMenuText->currentIndex() || RzxConfig::menuIconSize() != cmbMenuIcons->currentIndex())
	{
		RzxConfig::setMenuTextPosition(cmbMenuText->currentIndex());
		RzxConfig::setMenuIconSize(cmbMenuIcons->currentIndex());
		RzxConfig::emitIconFormatChanged();
	}
	
	const QPixmap *localhostIcon = pxmIcon->pixmap();
	if(RzxIconCollection::global()->localhostPixmap().serialNumber() != localhostIcon->serialNumber() && !localhostIcon->isNull())
	{
		localHostUpdated = true;
		RzxIconCollection::global()->setLocalhostPixmap(*localhostIcon);
/*		if(RzxApplication::instance()->isInitialised() && !RzxServerListener::object() -> isSocketClosed())
			RzxServerListener::object() -> sendIcon(localhostIcon->toImage());*/
	}

/*	if(localHostUpdated)
		RzxServerListener::object()->sendRefresh();*/

	if(themeChanged)
	{
		RzxIconCollection::global()->setTheme(cmbIconTheme->currentText());
		RzxConfig::setIconTheme(cmbIconTheme->currentText());
	}
	
	/* Sauvegarde du fichier du conf */
	RzxConfig::global()->flush();

	//On ne change la langue qu'au dernier moment car �a r�initialise toutes les bo�tes
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

///Fonction g�n�rique pour la recherche d'un fichier � charger
QString RzxProperty::browse(const QString& name, const QString& title, const QString& glob)
{
#ifdef WITH_KDE
	QString filter = name + " (" + glob + ")";
	QString file = KFileDialog::getOpenFileName( QString::null, filter, object, title );
#else
	QString filter = name + " (" + glob + ")";
	QString file = QFileDialog::getOpenFileName(object, title, QString::null, filter);
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

void RzxProperty::launchDirSelectDialog() {
	QString temp;
	if ( !RzxConfig::global()->ftpPath().isNull() )
#ifdef WITH_KDE
		temp = KFileDialog::getExistingDirectory ( RzxConfig::global() ->ftpPath()
		        , 0, tr("Choose default ftp folder") );
	else
		temp = KFileDialog::getExistingDirectory ( ".", 
					 0, tr("Choose default ftp folder") );
#else

		temp = QFileDialog::getExistingDirectory(this, tr("Choose default ftp folder"),
					 RzxConfig::global()->ftpPath()); 
	else
		temp = QFileDialog::getExistingDirectory(this, tr("Choose default ftp folder"), ".");
#endif

	if ( !temp.isNull() )
		txtWorkDir->setText( temp );
}

///Affichage de la fen�tre A propos de Qt...
void RzxProperty::aboutQt()
{
	QMessageBox::aboutQt(this);
}

///R�gle les probl�mes entre affichage des ic�nes et du texte
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
		lvPlugInList->setHeaderLabels(QStringList() << tr("Name") << tr("Version") << tr("Description"));
		changeTheme();
		initDlg();
	}
}
