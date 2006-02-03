/***************************************************************************
                      rzxproperty.cpp  -  description
                         -------------------
copyright            : (C) 2002 by Benoit Casoetto
email                : benoit.casoetto@m4x.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QPushButton>
#include <QToolButton>
#include <QToolBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QTextEdit>
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
#include <QStack>
#include <QShortcut>
#include <QImageReader>

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
#include <RzxFavoriteList>
#include <RzxBanList>
#include <RzxLoaderProp>
#include <RzxUtilsLauncher>

///Initialise la liste des sports
const RzxProperty::Sport RzxProperty::sports[] = {
	{ "athletism", QT_TR_NOOP("Athletism"), 1 },
	{ "rowing", QT_TR_NOOP("Rowing"), 2 },
	{ "badminton", QT_TR_NOOP("Badminton"), 17 },
	{ "basketball", QT_TR_NOOP("Basketball"), 3 },
	{ "orienting", QT_TR_NOOP("Orienting"), 4 },
	{ "riding", QT_TR_NOOP("Riding"), 5 },
	{ "climbing", QT_TR_NOOP("Climbing"), 6 },
	{ "fencing", QT_TR_NOOP("Fencing"), 7 },
	{ "football", QT_TR_NOOP("Football"), 8 },
	{ "golf", QT_TR_NOOP("Golf"), 9 },
	{ "handball", QT_TR_NOOP("Handball"), 10 },
	{ "judo", QT_TR_NOOP("Judo"), 11 },
	{ "swimming", QT_TR_NOOP("Swimming"), 12 },
	{ "rugby", QT_TR_NOOP("Rugby"), 13 },
	{ "tennis", QT_TR_NOOP("Tennis"), 14 },
	{ "sailing", QT_TR_NOOP("Sailing"), 15 },
	{ "volleyball", QT_TR_NOOP("Volleyball"), 16 }
};
#define SportNumber 17

///Permet de trier les sports en fonction de la langue actuelle
bool sportLessThan(const RzxProperty::Sport* s1, const RzxProperty::Sport* s2)
{
	return RzxProperty::tr(s1->name).toLower() < RzxProperty::tr(s2->name).toLower();
}

///Construction de la fenêtre de propriétés
RzxProperty::RzxProperty(QWidget *parent)
	:QDialog(parent)
{
	setupUi(this);
	setAttribute(Qt::WA_QuitOnClose,false);
	setWindowModality(Qt::NonModal);
	RzxStyle::useStyleOnWindow(this);
	RzxIconCollection::connect(this, SLOT(changeTheme()));

	//Connection des boutons	
	connect(btnBrowseWorkDir, SIGNAL( clicked() ), this, SLOT( launchDirSelectDialog() ) );
	connect(btnMiseAJour, SIGNAL( clicked() ), this, SLOT( miseAJour() ) );
	connect(btnAnnuler, SIGNAL( clicked() ), this, SLOT( annuler() ) );
	connect(btnOK, SIGNAL( clicked() ), this, SLOT( oK() ) );
	connect(btnBrowse, SIGNAL( clicked() ), this, SLOT( chooseIcon() ) );
	connect(btnAboutQt, SIGNAL(clicked()), this, SLOT(aboutQt()));

	connect(cmbMenuIcons, SIGNAL(activated(int)), this,SLOT(lockCmbMenuText(int)));
	connect(languageBox, SIGNAL(activated(const QString&)), this, SLOT(changeLanguage(const QString&)));
	connect(cmbIconTheme, SIGNAL(activated(const QString&)), this, SLOT(changeTheme(const QString&)));

	//Connection pour l'affichage des propriétés
	connect(listCache, SIGNAL(itemSelectionChanged()), this, SLOT(fillCacheView()));
	connect(btnDeleteCache, SIGNAL(clicked()), this, SLOT(deleteCache()));
	connect(btnSendMail, SIGNAL(clicked()), this, SLOT(sendMailToCache()));
	connect(this, SIGNAL(deleteWanted()), this, SLOT(tryDeleteCache()));

	//Connection des différentes combinaisons clavier...
	new QShortcut(Qt::Key_Backspace, this, SIGNAL(deleteWanted()));
	new QShortcut(Qt::Key_Delete, this, SIGNAL(deleteWanted()));
	new QShortcut(Qt::Key_Enter, this, SLOT(emitEnterPressed()));
	new QShortcut(Qt::Key_Return, this, SLOT(emitEnterPressed()));


	//Navigation entre les différentes pages
	connect(lbMenu, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), 
		this, SLOT(changePage(QTreeWidgetItem*, QTreeWidgetItem*)));

	//Pour que le pseudo soit rfc-complient
	//et pas trop long non plus...
	hostname->setValidator( new QRegExpValidator(QRegExp("[a-zA-Z0-9](-?[a-zA-Z0-9])*"), hostname) );
	hostname->setMaxLength(32);
	
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
	moduleLoader->setLoader(RzxApplication::instance());
	networkLoader->setLoader(RzxConnectionLister::global());

#ifndef WIN32
	btnAboutQt->hide();
#endif
#if defined(WIN32) || defined(Q_OS_MAC)
	lblWorkDir_2->hide();
	txtBeepCmd->hide();
	lblSoundIcon->hide();
	lineSound->hide();
#endif

#ifdef Q_OS_MAC
	lstTheme->setSizeIncrement(QSize(1,1));
	new QShortcut(QKeySequence("Ctrl+M"), this, SLOT(showMinimized()));
#endif

	//fenêtre pour les props vides, index == 3
	prefStack->addWidget(new QWidget());
	
	//construction du menu
	lbMenu->header()->hide();
	generalItem = new QTreeWidgetItem(lbMenu);
	generalItem->setText(0, tr("Infos"));
	generalItem->setData(0, Qt::UserRole, UserInfo);
	lbMenu->expandItem(generalItem);

	favoritesItem = new QTreeWidgetItem(lbMenu);
	favoritesItem->setText(0, tr("Contacts"));
	favoritesItem->setData(0, Qt::UserRole, Favorites);
	lbMenu->expandItem(favoritesItem);
	listFavorites->setList(RzxFavoriteList::global());
	listBanned->setList(RzxBanList::global());

	layoutItem = new QTreeWidgetItem(lbMenu);
	layoutItem->setText(0, tr("Layout"));
	layoutItem->setData(0, Qt::UserRole, Layout);
	lbMenu->expandItem(layoutItem);

	modulesItem = new QTreeWidgetItem(lbMenu);
	modulesItem->setText(0, tr("Modules"));
	modulesItem->setData(0, Qt::UserRole, Modules);
	lbMenu->expandItem(modulesItem);
	buildModules<RzxModule>(RzxApplication::modulesList(), modulesItem);
	moduleLoader->setPropertyParent(modulesItem);

	networkItem = new QTreeWidgetItem(lbMenu);
	networkItem->setText(0, tr("Network"));
	networkItem->setData(0, Qt::UserRole, Network);
	lbMenu->expandItem(networkItem);
	buildModules<RzxNetwork>(RzxConnectionLister::global()->moduleList(), networkItem);
	networkLoader->setPropertyParent(networkItem);

	QList<RzxBaseLoaderProp*> loaders = findChildren<RzxBaseLoaderProp*>();
	foreach(RzxBaseLoaderProp *loader, loaders)
		loader->connectToPropertyWindow(this);

	initDlg();
	changeTheme();
	lbMenu->setCurrentItem(generalItem);
}

///Fermeture de la fenêtre
RzxProperty::~RzxProperty()
{
	closeModules<RzxNetwork>(RzxConnectionLister::global()->moduleList());
	closeModules<RzxModule>(RzxApplication::modulesList());
}

///Demande le changement du thème actuel
void RzxProperty::changeTheme(const QString& theme)
{
	RzxIconCollection::setTheme(theme);
}

///Demande le changement de langue
void RzxProperty::changeLanguage(const QString& lang)
{
	RzxTranslator::setLanguage(lang);
}

/// Change le thème d'icône de la fenêtre de préférence
/** Le changement de thème correspond à la reconstruction de la listbox de menu (pour que les icônes soient conformes au thème choisi), et au changement des icônes OK, Annuler, Appliquer */
void RzxProperty::changeTheme()
{
	btnAnnuler->setIcon(RzxIconCollection::getIcon(Rzx::ICON_CANCEL));
	btnOK->setIcon(RzxIconCollection::getIcon(Rzx::ICON_OK));
	btnMiseAJour->setIcon(RzxIconCollection::getIcon(Rzx::ICON_APPLY));

	generalItem->setIcon(0, RzxIconCollection::getIcon(Rzx::ICON_PROPRIETES));
	favoritesItem->setIcon(0, RzxIconCollection::getIcon(Rzx::ICON_FAVORITE));
	layoutItem->setIcon(0, RzxIconCollection::getIcon(Rzx::ICON_LAYOUT));
	modulesItem->setIcon(0, RzxIconCollection::getIcon(Rzx::ICON_PLUGIN));
	networkItem->setIcon(0, RzxIconCollection::getIcon(Rzx::ICON_NETWORK));

	CbSamba->setIcon(RzxIconCollection::getIcon(Rzx::ICON_SAMBA));
	CbFTP->setIcon(RzxIconCollection::getIcon(Rzx::ICON_FTP));
	CbHTTP->setIcon(RzxIconCollection::getIcon(Rzx::ICON_HTTP));
	CbNews->setIcon(RzxIconCollection::getIcon(Rzx::ICON_NEWS));
	CbPrinter->setIcon(RzxIconCollection::getIcon(Rzx::ICON_PRINTER));

	lblFtpIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_FTP).scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	lblHttpIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_HTTP).scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	lblNewsIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_NEWS).scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	lblMailIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_MAIL).scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	lblSoundIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_SOUNDON).scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

	chkAutoResponder->setIcon(RzxIconCollection::getResponderIcon());

	btnBrowseWorkDir->setIcon(RzxIconCollection::getIcon(Rzx::ICON_FILE));
	btnBrowse->setIcon(RzxIconCollection::getIcon(Rzx::ICON_FILE));

	btnSendMail->setIcon(RzxIconCollection::getIcon(Rzx::ICON_MAIL));
	btnDeleteCache->setIcon(RzxIconCollection::getIcon(Rzx::ICON_UNLOAD));

	setWindowIcon(RzxIconCollection::getIcon(Rzx::ICON_PREFERENCES));

	changeThemeModules<RzxNetwork>(RzxConnectionLister::global()->moduleList(), networkItem);
	changeThemeModules<RzxModule>(RzxApplication::modulesList(), modulesItem);

	fillThemeView();
	fillCacheList();
	fillCacheView();
	initPromoCombo();
	initSportCombo();
}

///Rempli la listview d'affichage du thème d'icône
void RzxProperty::fillThemeView()
{
	static QList<Rzx::Icon> icons = QList<Rzx::Icon>() << Rzx::ICON_JONE << Rzx::ICON_FTP << Rzx::ICON_LAYOUT << Rzx::ICON_SAMEGATEWAY
		<< Rzx::ICON_PLUGIN << Rzx::ICON_FAVORITE << Rzx::ICON_SAMBA << Rzx::ICON_PROPRIETES << Rzx::ICON_NETWORK << Rzx::ICON_BAN
		<< Rzx::ICON_HISTORIQUE << Rzx::ICON_HTTP << Rzx::ICON_SOUNDON << Rzx::ICON_OS0 << Rzx::ICON_OS1 << Rzx::ICON_OS2
		<< Rzx::ICON_OS3 << Rzx::ICON_OS4 << Rzx::ICON_OS5 << Rzx::ICON_OS6 << Rzx::ICON_PRINTER << Rzx::ICON_ORANJE
		<< Rzx::ICON_AWAY << Rzx::ICON_PREFERENCES << Rzx::ICON_ROUJE;

	lstTheme->clear();
	foreach(Rzx::Icon i, icons)
	{
		QListWidgetItem *item = new QListWidgetItem(lstTheme);
		item->setIcon(RzxIconCollection::getIcon(i));
	}
}

///Remplis la QListWidget avec la des données disponibles en cache
void RzxProperty::fillCacheList()
{
	listCache->clear();
	const QList<RzxHostAddress> addr = RzxConfig::cachedList();
	foreach(RzxHostAddress ip, addr)
	{
		QString name = RzxConfig::getNameFromCache(ip, "$p $n ($s)");
		if(name.right(2) == "()") name = RzxConfig::getNameFromCache(ip, "$p $n");
		QListWidgetItem *item = new QListWidgetItem(listCache);
		item->setText(name);
		item->setData(Qt::UserRole, ip.toString());
		const RzxComputer *computer = RzxConnectionLister::global()->getComputerByIP(ip);
		if(computer)
			item->setIcon(computer->icon());
		else
			item->setIcon(RzxIconCollection::getIcon(Rzx::ICON_OFF));
	}
}

///Affiche les données associées à la cache sélectionnée...
void RzxProperty::fillCacheView()
{
	showCache->clear();
	const QListWidgetItem *item = listCache->currentItem();
	RzxHostAddress ip;
	if(!item)
		ip = RzxComputer::localhost()->ip();
	else
	 	ip = RzxHostAddress(item->data(Qt::UserRole).toString());
	RzxConfig::fillWithCache(ip, showCache);
	showCache->setEnabled(item);
	btnDeleteCache->setEnabled(item);
	btnSendMail->setEnabled(item && !RzxConfig::getEmailFromCache(ip).isEmpty());
}

///Vérifie que la fenêtre de gestion des cache a la focus avant d'appeler deleteCache
void RzxProperty::tryDeleteCache()
{
	if(listCache->hasFocus())
		deleteCache();
}

///Action en cliquant sur le bouton de gestion des propriétés
void RzxProperty::deleteCache()
{
	const QList<QListWidgetItem*> items = listCache->selectedItems();
	if(!items.size()) return;

	foreach(QListWidgetItem *item, items)
	{
		const RzxHostAddress ip = RzxHostAddress(item->data(Qt::UserRole).toString());
		RzxConfig::deleteCache(ip);
	}
	fillCacheList();
}

///Action en cliquant sur le bouton d'envoie de mail
void RzxProperty::sendMailToCache()
{
	const QListWidgetItem *item = listCache->currentItem();
	if(!item) return;

	const RzxHostAddress ip = RzxHostAddress(item->data(Qt::UserRole).toString());
	const QString email = RzxConfig::getEmailFromCache(ip);
	if(!email.isEmpty())
		RzxUtilsLauncher::mail(email);
}

///Crée une entrée dans la liste des fenêtres et l'ajoute à la pile
QTreeWidgetItem* RzxProperty::createPage(QWidget *widget, const QString& name, const QIcon& icon, QTreeWidgetItem *parent)
{
	QTreeWidgetItem *item = NULL;

	int index = 0;
	if(widget)
		index = prefStack->addWidget(widget);
	else
		index = Blank;

	item = new QTreeWidgetItem(parent);
	item->setText(0, name);
	item->setIcon(0, icon);
	item->setData(0, Qt::UserRole, index);
	lbMenu->expandItem(item);
	return item;
}

///Rajoute un module dans la liste des modules présent
/** Le module à rajouter est celui qui a le nom indiqué et doit être rajouter
 * dans l'arborescence sur le QTreeWidgetItem* indiqué
 */
void RzxProperty::addModule(const QString& name, QTreeWidgetItem *parent)
{
	QList<RzxBaseModule*> module;
	if(parent == modulesItem)
		module << RzxApplication::instance()->module(name);
	else if(parent == networkItem)
		module << RzxConnectionLister::global()->module(name);
	else
	{
		//Recherche de l'arborescence des modules
		QStack<QString> moduleNames;
		moduleNames.push(name);
		QTreeWidgetItem *item = parent;
		do
		{
			moduleNames.push(item->text(0));
			item = item->parent();
		}
		while(item && item != modulesItem && item != networkItem);
		if(!item) return;

		//Parcours l'arborescence des modules
		QString moduleName = moduleNames.pop();
		RzxBaseModule *mod = NULL; 
		if(item == modulesItem)
			mod = RzxApplication::instance()->module(moduleName);
		else
			mod = RzxConnectionLister::global()->module(moduleName);
		while(!moduleNames.isEmpty())
		{
			if(!mod) return;
			moduleName = moduleNames.pop();
			mod = mod->childModule(moduleName);
		}
		if(mod) module << mod;
	}
	if(!module[0])
		return;
	buildModules<RzxBaseModule>(module, parent);
	module[0]->propInit();
}

///Retire un module dans la liste des modules présents
/** Ce module est identifié par son nom et son père dans l'aborescence.
 */
void RzxProperty::deleteModule(const QString& name, QTreeWidgetItem *parent)
{
	for(int i = 0 ; i < parent->childCount() ; i++)
	{
		QTreeWidgetItem * item = parent->child(i);
		if(item->text(0) == name)
		{
			deletePage(item);
			break;
		}
	}
}

///Supprime l'objet associé au QTreeWidgetItem
/** Et reconstruit tous les autres objets pour prendre en compte le décallage
 */
void RzxProperty::deletePage(QTreeWidgetItem *item)
{
	if(!item) return;
	const int page = item->data(0, Qt::UserRole).toInt();
	if(page != Blank)
		rebuildIndexes(page);
	while(item->childCount())
		deletePage(item->child(0));
	if(page != Blank)
		prefStack->removeWidget(prefStack->widget(page));
	delete item;
}

///Reconstruit les index associés aux QTreeWidgetItem
void RzxProperty::rebuildIndexes(const int page, QTreeWidgetItem *item)
{
	if(item)
	{
		const int data = item->data(0, Qt::UserRole).toInt();
		if(data > page)
			item->setData(0, Qt::UserRole, data-1);
		for(int i = 0 ; i < item->childCount() ; i++)
			rebuildIndexes(page, item->child(i));
	}
	else
		for(int i = 0 ; i < lbMenu->topLevelItemCount() ; i++)
			rebuildIndexes(page, lbMenu->topLevelItem(i));
}

///La page doit changer, on met à jour le titre
void RzxProperty::changePage(QTreeWidgetItem *current, QTreeWidgetItem *)
{
	if(current)
	{
		lblTitle->setText("<h2>"+current->text(0)+"</h2>");
		lblTitleIcon->setPixmap(current->icon(0).pixmap(22));
		int page = current->data(0, Qt::UserRole).toInt();
		if(page >= prefStack->count())
			page = 0;
		if(page == Layout)
			fillThemeView();
		prefStack->setCurrentIndex(page);
	}
}

///Remplissage de la boîte des langues
void RzxProperty::initLangCombo()
{
	languageBox->clear();
	const QStringList list = RzxTranslator::languageIdList();
	for(int i = 0 ; i < list.count() ; i++)
		languageBox->addItem(RzxIconCollection::getIcon(list[i]), RzxTranslator::name(list[i]));
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

///Initialise la liste des sports
void RzxProperty::initSportCombo()
{
	int index;
	if(!cmbSport->count())
		index = RzxConfig::numSport();
	else
		index = cmbSport->itemData(cmbSport->currentIndex()).toInt();

	cmbSport->clear();
	cmbSport->addItem(tr("None"), 0);
	QList<const Sport*> list;
	for(int i = 0 ; i < SportNumber ; i++)
		list << &sports[i];
	qSort(list.begin(), list.end(), sportLessThan);
	for(int i = 0 ; i < SportNumber ; i++)
	{
		cmbSport->addItem(RzxIconCollection::getIcon(list[i]->icon), tr(list[i]->name), list[i]->id);
		cmbSport->setItemData(cmbSport->count() - 1, list[i]->name, Qt::UserRole + 1);
	}

	cmbSport->setCurrentIndex(cmbSport->findData(index));
}

///Initialise la liste des promos
void RzxProperty::initPromoCombo()
{
	int index;
	if(!cmbPromo->count())
		index = -1;
	else
		index = cmbPromo->currentIndex();

	cmbPromo->clear();
	cmbPromo->addItem(RzxIconCollection::getIcon(Rzx::ICON_ORANJE), tr("orange"));
	cmbPromo->addItem(RzxIconCollection::getIcon(Rzx::ICON_ROUJE), tr("rouje"));
	cmbPromo->addItem(RzxIconCollection::getIcon(Rzx::ICON_JONE), tr("jone"));

	if(index == -1)
		cmbPromo->setCurrentIndex( RzxComputer::localhost()->promo() - 1 );
	else
		cmbPromo->setCurrentIndex(index);
}

///Initialise la boîte de dialogue avec les données courante de configuration
/** ou si def = true, avec les données par défaut
 */
void RzxProperty::initDlg(bool def)
{
	RzxConfig * config = RzxConfig::global();

	// iteration sur l'ensemble des objets QLineEdit
	QList<QLineEdit*> l = findChildren<QLineEdit*>(QRegExp("txt.*"));
	foreach(QLineEdit *edit, l)
		edit->setText(config->value(edit->objectName(), "").toString());
	
	initThemeCombo();
	initLangCombo();
	initStyleCombo();
	initSportCombo();
	initPromoCombo();
	fillCacheList();

	cbStyleForAll->setChecked(RzxConfig::useStyleForAll());
	
	hostname->setText( RzxComputer::localhost()->name() );
	remarque->setPlainText( RzxComputer::localhost()->remarque(true) );

	chkAutoResponder->setChecked( RzxConfig::autoResponder() );


	cmbMenuIcons->setCurrentIndex(RzxConfig::menuIconSize(def));
	cmbMenuText->setCurrentIndex(RzxConfig::menuTextPosition(def));
	lockCmbMenuText(RzxConfig::menuIconSize(def));

	int servers = RzxComputer::localhost()->serverFlags();
	CbSamba->setChecked(servers & RzxComputer::SERVER_SAMBA);
	CbFTP->setChecked(servers & RzxComputer::SERVER_FTP);
	CbHTTP->setChecked(servers & RzxComputer::SERVER_HTTP);
	CbNews->setChecked(servers & RzxComputer::SERVER_NEWS);
	CbPrinter->setChecked(servers & RzxComputer::SERVER_PRINTER);

	clientFtp ->clear();
	clientHttp ->clear();
	clientNews ->clear();
	clientMail ->clear();

#ifdef WIN32
	// attention a rzxutilslauncher.cpp en cas de modif
	clientFtp->addItem("standard");
	clientFtp->addItem("IExplorer");
	clientFtp->addItem("LeechFTP");
	clientFtp->addItem("SmartFTP");
	clientFtp->addItem("FileZilla");

	clientHttp->addItem("standard");
	clientHttp->addItem("IExplorer");
	clientHttp->addItem("Firefox");
	clientHttp->addItem("Opera");

	clientNews->addItem("standard");
	clientNews->addItem("Thunderbird");
	clientMail->addItem("standard");
	clientMail->addItem("Thunderbird");
#else
#ifdef Q_OS_MAC
	// commandes à exécuter sous macos X
	clientFtp->addItem("Default");
	clientHttp->addItem("Default");
	clientNews->addItem("Default");
	clientMail->addItem("Default");
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
	clientNews->addItem("opera");

	clientMail->addItem("kmail");
	clientMail->addItem("thunderbird");
	clientMail->addItem("opera");
#endif //MAC
#endif //WIN32

#define setValue(cb, cmd) \
	if(cb->findText(cmd) == -1) cb->addItem(cmd); \
	cb->setCurrentIndex(cb->findText(cmd))

	if(!def)
	{
		setValue(clientFtp, RzxConfig::ftpCmd());
		setValue(clientHttp, RzxConfig::httpCmd());
		setValue(clientNews, RzxConfig::newsCmd());
		setValue(clientMail, RzxConfig::mailCmd());
	}
#undef setValue
	txtWorkDir->setText( RzxConfig::ftpPath(def) );
	
	pxmIcon->setPixmap(RzxIconCollection::global()->localhostPixmap());

	listFavorites->refresh();
	listBanned->refresh();

	initModules<RzxNetwork>(RzxConnectionLister::global()->moduleList(), def);
	initModules<RzxModule>(RzxApplication::modulesList(), def);
}

///Met à jour l'objet représentant localhost
void RzxProperty::updateLocalHost()
{
	if(RzxComputer::localhost()->name().toLower() != hostname->text().toLower())
		RzxComputer::localhost()->setName(hostname->text());
	if(RzxComputer::localhost()->remarque(true) != remarque->toPlainText())
		RzxComputer::localhost()->setRemarque(remarque->toPlainText());
	if(RzxComputer::localhost()->promo() != cmbPromo->currentIndex()+1)
		RzxComputer::localhost()->setPromo((Rzx::Promal)(cmbPromo->currentIndex() + 1));
	if(RzxConfig::autoResponder() != chkAutoResponder->isChecked())
		RzxComputer::localhost()->setState(chkAutoResponder->isChecked());

	RzxComputer::Servers servers;
	if ( CbSamba->isChecked() )
		servers |= RzxComputer::SERVER_SAMBA;
	if ( CbFTP->isChecked() )
		servers |= RzxComputer::SERVER_FTP;
	if ( CbHTTP->isChecked() )
		servers |= RzxComputer::SERVER_HTTP;
	if ( CbNews->isChecked() )
		servers |= RzxComputer::SERVER_NEWS;
	if ( CbPrinter->isChecked() )
		servers |= RzxComputer::SERVER_PRINTER;

	const QPixmap *localhostIcon = pxmIcon->pixmap();
	if(RzxIconCollection::global()->localhostPixmap().serialNumber() != localhostIcon->serialNumber() && !localhostIcon->isNull())
	{
		RzxIconCollection::global()->setLocalhostPixmap(*localhostIcon);
		RzxComputer::localhost()->setIcon(*localhostIcon);
	}

	const RzxComputer::Servers oldservers = RzxComputer::localhost()->serverFlags();
	if(servers != oldservers)
		RzxComputer::localhost()->setServerFlags(servers);
}

///Mets à jours les données globales de qRezix avec ce qu'a rentré l'utilisateur
bool RzxProperty::miseAJour()
{
	RzxComputer::localhost()->lock();
	updateModules<RzxNetwork>(RzxConnectionLister::global()->moduleList());
	updateModules<RzxModule>(RzxApplication::modulesList());

	//Vérification de validité des données
	if(!hostname->hasAcceptableInput())
	{
		RzxMessageBox::information(this, tr("Incorrect properties"),
			tr("Your computer name is not valid...<br>"
				"It can only contain letters, numbers and '-' signs."));
		RzxComputer::localhost()->unlock();
		return false;
	}
	
	// iteration sur l'ensemble des objets QCheckBox
	QList<QLineEdit*> l = findChildren<QLineEdit*>(QRegExp("txt.*"));
	foreach(QLineEdit *edit, l)
		RzxConfig::global()->setValue(edit->objectName(), edit->text());
	
	//Indique si les données 'partagées' ont été modifiées
	updateLocalHost();
	
	RzxConfig::setDnsName(RzxComputer::localhost()->name() );
	RzxConfig::setRemarque(RzxComputer::localhost()->remarque());
	RzxConfig::setPromo(RzxComputer::localhost()->promo() );
	RzxConfig::setRepondeur(RzxComputer::localhost()->state() );
	RzxConfig::setServers(RzxComputer::localhost()->serverFlags());

	RzxConfig::setBeepCmd(txtBeepCmd->text());
	RzxConfig::setHttpCmd( clientHttp -> currentText() );
	RzxConfig::setFtpCmd( clientFtp -> currentText() );
	RzxConfig::setNewsCmd( clientNews -> currentText() );
	RzxConfig::setMailCmd( clientMail -> currentText() );

	RzxConfig::setFtpPath(txtWorkDir->text() );
	RzxConfig::setPropSport(cmbSport->itemData(cmbSport->currentIndex(), Qt::UserRole + 1).toString());
	RzxConfig::setNumSport(cmbSport->itemData(cmbSport->currentIndex()).toInt());
	
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

	//Sauvegarde du fichier du conf
	RzxConfig::global()->flush();
	RzxComputer::localhost()->unlock();
	return true;
}

///Retourne la liste des informations non complétées
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

///Affiche une boite de dialogue en cas de manque d'information
/** Retourne le choix de l'utilisateur
 */
int RzxProperty::infoCompleteMessage()
{
	return QMessageBox::question(this, tr("Incomplete Data"), tr("In order to use qRezix, you must fill in the following information :\n")
			+ infoNeeded()
			+ tr("Press OK to do so, or Cancel to quit qRezix"),
			QMessageBox::Ok, QMessageBox::Cancel);
}

///L'utilisateur ne veut pas que ses modications soient prises en compte
void RzxProperty::annuler()
{
	if(RzxConfig::infoCompleted() || (infoCompleteMessage() == QMessageBox::Cancel))
		close();
	initDlg();
}

///L'utilisateur veut que ses modifications soient prises en compte
void RzxProperty::oK()
{
	if(!miseAJour())
		return;
	annuler();
}

///Fonction générique pour la recherche d'un fichier à charger
QString RzxProperty::browse(const QString& name, const QString& title, const QString& glob, QWidget *parent)
{
	QString filter = name + " (" + glob + ")";
	const QString filename = QFileDialog::getOpenFileName(parent, title, QString::null, filter);
	if(parent)
		parent->raise();
	return filename;
}

///Affiche la boîte de dialoge due choix d'icône
void RzxProperty::chooseIcon()
{
	const QList<QByteArray> rawFormats = QImageReader::supportedImageFormats();
	QString formats;
	foreach(QByteArray format, rawFormats)
		formats += " *." + format.toLower();
	QString file = browse(tr("Icons"), tr("Icon selection"), formats, this);
	if ( file.isEmpty() ) return ;

	QPixmap icon;
	if (!icon.load(file)) {
		RzxMessageBox::warning( this,
		                      tr("Error !"),
		                      tr("Invalid File Selected."));
		return ;
	}

	pxmIcon->setPixmap(icon.scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}


///Affiche la boite de selection du répertoire de stockage des données pour le ftp
void RzxProperty::launchDirSelectDialog()
{
	QString temp;
	if ( !RzxConfig::global()->ftpPath().isNull() )
		temp = QFileDialog::getExistingDirectory(this, tr("Choose default ftp folder"),
					 RzxConfig::global()->ftpPath()); 
	else
		temp = QFileDialog::getExistingDirectory(this, tr("Choose default ftp folder"), ".");

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

///Pour bien initialiser le fenêtre lstTheme
void RzxProperty::setVisible(bool visible)
{
	QDialog::setVisible(visible);
	if(visible) fillThemeView();
}

///Pour gérer la pression de Enter
void RzxProperty::emitEnterPressed()
{
	bool used = false;
	emit enterPressed(used);
	if(!used)
		oK();
}

///Change la langue...
void RzxProperty::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	if(e->type() == QEvent::LanguageChange)
	{
		QTreeWidgetItem *item = lbMenu->currentItem();
		int text = cmbMenuText->currentIndex();
		cmbMenuText->clear();
		int icon = cmbMenuIcons->currentIndex();
		cmbMenuIcons->clear();
		retranslateUi(this);
		cmbMenuText->setCurrentIndex(text);
		cmbMenuIcons->setCurrentIndex(icon);
		changeTheme();
		generalItem->setText(0, tr("Infos"));
		favoritesItem->setText(0, tr("Contacts"));
		layoutItem->setText(0, tr("Layout"));
		modulesItem->setText(0, tr("Modules"));
		networkItem->setText(0, tr("Network"));
		initSportCombo();
		initPromoCombo();
		fillCacheView();
		changePage(item, item);
	}
}
