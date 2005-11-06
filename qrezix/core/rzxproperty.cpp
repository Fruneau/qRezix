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
#include <QBitmap>
#include <QRegExpValidator>
#include <QPixmap>
#include <QSize>
#include <QHeaderView>

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
#include <RzxIconCollection>
#include <RzxApplication>
#include <RzxModule>
#include <RzxConnectionLister>
#include <RzxNetwork>
#include <RzxTranslator>
#include <RzxStyle>

RZX_GLOBAL_INIT(RzxProperty)

RzxProperty::RzxProperty(QWidget *parent)
	:QDialog(parent)
{
	object = this;
	setupUi(this);
	RzxStyle::useStyleOnWindow(this);
	
	connect( btnBrowseWorkDir, SIGNAL( clicked() ), this, SLOT( launchDirSelectDialog() ) );
	connect( btnMiseAJour, SIGNAL( clicked() ), this, SLOT( miseAJour() ) );
	connect( btnAnnuler, SIGNAL( clicked() ), this, SLOT( annuler() ) );
	connect( btnOK, SIGNAL( clicked() ), this, SLOT( oK() ) );
	connect( btnBrowse, SIGNAL( clicked() ), this, SLOT( chooseIcon() ) );
	connect(btnAboutQt, SIGNAL(clicked()), this, SLOT(aboutQt()));
	connect( lbMenu, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(changePage(QTreeWidgetItem*, QTreeWidgetItem*)));

	//connect( btnChangePass, SIGNAL(clicked()), RzxServerListener::object(), SLOT(changePass()));
	RzxIconCollection::connect(this, SLOT(changeTheme()));
	connect(cmbMenuIcons, SIGNAL(activated(int)), this,SLOT(lockCmbMenuText(int)));

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
	lvPlugInList->setHeaderLabels(QStringList() << tr("Name") << tr("Version") << tr("Description"));
	lvNetworks->setIconSize(QSize(16,16));
	lvNetworks->setUniformRowHeights(false);
	lvNetworks->setHeaderLabels(QStringList() << tr("Name") << tr("Version") << tr("Description"));

#ifndef WIN32
	btnAboutQt->hide();
#endif
#if defined(WIN32) || defined(Q_OS_MAC)
	lblWorkDir_2->hide();
	txtBeepCmd->hide();
#endif

	lbMenu->header()->hide();
	generalItem = new QTreeWidgetItem(lbMenu);
	generalItem->setText(0, tr("Infos"));
	generalItem->setData(0, Qt::UserRole, 0);
	lbMenu->expandItem(generalItem);

	confItem = new QTreeWidgetItem(lbMenu);
	confItem->setText(0, tr("Settings"));
	confItem->setData(0, Qt::UserRole, 1);
	lbMenu->expandItem(confItem);
	buildModules<RzxModule>(RzxApplication::modulesList(), lvPlugInList, confItem);

	networkItem = new QTreeWidgetItem(lbMenu);
	networkItem->setText(0, tr("Network"));
	networkItem->setData(0, Qt::UserRole, 2);
	lbMenu->expandItem(networkItem);
	buildModules<RzxNetwork>(RzxConnectionLister::global()->moduleList(), lvNetworks, networkItem);

	initDlg();
	changeTheme();
	lbMenu->setCurrentItem(generalItem);
}

RzxProperty::~RzxProperty()
{
	closeModules<RzxNetwork>(RzxConnectionLister::global()->moduleList());
	closeModules<RzxModule>(RzxApplication::modulesList());
	RZX_GLOBAL_CLOSE
}

/// Change le thème d'icône de la fenêtre de préférence
/** Le changement de thème correspond à la reconstruction de la listbox de menu (pour que les icônes soient conformes au thème choisi), et au changement des icônes OK, Annuler, Appliquer */
void RzxProperty::changeTheme()
{
	btnAnnuler->setIcon(RzxIconCollection::getIcon(Rzx::ICON_CANCEL));
	btnOK->setIcon(RzxIconCollection::getIcon(Rzx::ICON_OK));
	btnMiseAJour->setIcon(RzxIconCollection::getIcon(Rzx::ICON_APPLY));

	generalItem->setIcon(0, RzxIconCollection::getIcon(Rzx::ICON_PROPRIETES));
	confItem->setIcon(0, RzxIconCollection::getIcon(Rzx::ICON_PREFERENCES));
	networkItem->setIcon(0, RzxIconCollection::getIcon(Rzx::ICON_NETWORK));

//	changeThemeModules<RzxNetwork>(RzxConnectionLister::global()->moduleList(), i);
//	changeThemeModules<RzxModule>(RzxApplication::modulesList(), i);
}

///La page doit changer, on met à jour le titre
void RzxProperty::changePage(QTreeWidgetItem *current, QTreeWidgetItem *)
{
	if(current)
	{
		lblTitle->setText("<h2>"+current->text(0)+"</h2>");
		lblTitleIcon->setPixmap(current->icon(0).pixmap(22));
		prefStack->setCurrentIndex(current->data(0, Qt::UserRole).toInt());
	}
}

///Remplissage de la boîte des langues
void RzxProperty::initLangCombo()
{
	languageBox->clear();
	languageBox->insertItems(0, RzxTranslator::translationsList());
	languageBox->setCurrentIndex(languageBox->findText(RzxTranslator::translation()));
}

///Initialise la liste des thèmes
void RzxProperty::initThemeCombo()
{
	cmbIconTheme->clear();
	cmbIconTheme->insertItems(0, RzxIconCollection::themeList());
	cmbIconTheme->setCurrentIndex(cmbIconTheme->findText(RzxIconCollection::theme()));
}

///Initialise la liste des styles
void RzxProperty::initStyleCombo()
{
	cmbStyle->clear();
	cmbStyle->insertItems(0, RzxStyle::styleList());
	cmbStyle->setCurrentIndex(cmbStyle->findText(RzxStyle::style()));
}

/** No descriptions */
void RzxProperty::initDlg(bool def)
{
	RzxConfig * config = RzxConfig::global();

	// iteration sur l'ensemble des objets QLineEdit
	QList<QLineEdit*> l = findChildren<QLineEdit*>(QRegExp("txt.*"));
	foreach(QLineEdit *edit, l)
		edit->setText(config->value(edit->objectName(), "").toString());
	
//	btnChangePass->setEnabled(!RzxServerListener::object()->isSocketClosed());
	
	initThemeCombo();
	initLangCombo();
	initStyleCombo();
	cbStyleForAll->setChecked(RzxConfig::useStyleForAll());
	
	hostname->setText( RzxComputer::localhost()->name() );
	remarque->setText( RzxComputer::localhost()->remarque() );

	chkAutoResponder->setChecked( RzxConfig::autoResponder() );

	cmbPromo->setCurrentIndex( RzxComputer::localhost()->promo() - 1 );

	cmbMenuIcons->setCurrentIndex(RzxConfig::menuIconSize(def));
	cmbMenuText->setCurrentIndex(RzxConfig::menuTextPosition(def));
	lockCmbMenuText(RzxConfig::menuIconSize(def));

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
	// commandes à exécuter sous macos X
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

	if(!def)
	{
		setValue(clientFtp, RzxConfig::ftpCmd());
		setValue(clientHttp, RzxConfig::httpCmd());
		setValue(clientNews, RzxConfig::newsCmd());
	}
#undef setValue
	txtWorkDir->setText( RzxConfig::ftpPath(def) );
	
	pxmIcon->setPixmap(RzxIconCollection::global()->localhostPixmap());

	cmbSport->setCurrentIndex( RzxConfig::numSport(def) );

	initModules<RzxNetwork>(RzxConnectionLister::global()->moduleList(), def);
	initModules<RzxModule>(RzxApplication::modulesList(), def);
}

///Met à jour l'objet représentant localhost
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
	updateModules<RzxNetwork>(RzxConnectionLister::global()->moduleList());
	updateModules<RzxModule>(RzxApplication::modulesList());

	//Vérification de validité des données
	if(!hostname->hasAcceptableInput())
	{
		RzxMessageBox::information(this, tr("Incorrect properties"),
			tr("Your computer name is not valid...<br>"
				"It can only contain letters, numbers and '-' signs."));
		return false;
	}
	else if(!remarque->hasAcceptableInput())
	{
		RzxMessageBox::information(this, tr("Incorrect properties"),
			tr("Your comment is not valid...<br>"
				"It cannot contain linebreaks or other special characters"));
		return false;
	}
	
	//Mise à jours des données de configuration
	RzxConfig *cfgObject = RzxConfig::global();
	
	// iteration sur l'ensemble des objets QCheckBox
	QList<QLineEdit*> l = findChildren<QLineEdit*>(QRegExp("txt.*"));
	foreach(QLineEdit *edit, l)
		cfgObject->setValue(edit->objectName(), edit->text());
	
//	bool iconSizeChanged = (cfgObject->computerIconSize() != cmbIconSize->currentIndex() || cfgObject->computerIconHighlight() != cbHighlight->isChecked());
	bool themeChanged = RzxIconCollection::theme() != cmbIconTheme->currentText();

	//Indique si les données 'partagées' ont été modifiées
	//localHostUpdated = true ==> besoin de rafraichir le serveur
	bool localHostUpdated = updateLocalHost();
	
	RzxConfig::setDnsName(RzxComputer::localhost()->name() );
	RzxConfig::setRemarque(RzxComputer::localhost()->remarque());
	RzxConfig::setPromo(RzxComputer::localhost()->promo() );
	RzxConfig::setRepondeur(RzxComputer::localhost()->state() );
	RzxConfig::setServers(RzxComputer::localhost()->serverFlags());

	RzxConfig::setBeepCmd(txtBeepCmd->text());
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
	
	RzxConfig::setUseStyleForAll(cbStyleForAll->isChecked());
	if(RzxStyle::style() != cmbStyle->currentText())
		RzxStyle::setStyle(cmbStyle->currentText());
	
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
	

	//On ne change la langue qu'au dernier moment car ça réinitialise toutes les boîtes
	if(languageBox->currentText() != RzxTranslator::translation())
		RzxTranslator::setLanguage(languageBox->currentText());
	
	//Sauvegarde du fichier du conf
	RzxConfig::global()->flush();
	return true;
}

QString RzxProperty::infoNeeded()
{
	QString msg = "";
	if(RzxConfig::propLastName() == "")
		msg += "\t" + tr("Last name:").remove(':') + "\n";
	if(RzxConfig::propName() == "")
		msg += "\t" + tr("Name:").remove(':') + "\n";
	if(RzxConfig::propCasert() == "")
		msg += "\t" + tr("Room:").remove(':') + "\n";
	if(RzxConfig::propMail() == "")
		msg += "\t" + tr("E-mail:").remove(':') + "\n";
	if(RzxConfig::propTel() == "")
		msg += "\t" + tr("Phone number:").remove(':') + "\n";
	return msg;
}

int RzxProperty::infoCompleteMessage()
{
	return QMessageBox::question(this, tr("Incomplete Data"), tr("In order to use qRezix, you must fill in the following information :\n")
			+ infoNeeded()
			+ tr("Press OK to do so, or Cancel to quit qRezix"),
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

///Fonction générique pour la recherche d'un fichier à charger
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
		                      tr("Invalid File Selected."));
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
		QTreeWidgetItem *item = lbMenu->currentItem();
		retranslateUi(this);
		lbMenu->setCurrentItem(item);
		lvPlugInList->setHeaderLabels(QStringList() << tr("Name") << tr("Version") << tr("Description"));
		changeTheme();
		initDlg();
	}
}
