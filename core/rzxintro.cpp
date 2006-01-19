/***************************************************************************
                          rzxintro  -  description
                             -------------------
    begin                : Mon Nov 14 2005
    copyright            : (C) 2005 by Florent Bruneau
    email                : florent.bruneau@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <RzxTranslator>
#include <RzxIconCollection>

#include <RzxIntro>

#include "ui_rzxintro.h"

///Construction de la fenêtre
RzxIntro::RzxIntro()
	:QDialog()
{
	setAttribute( Qt::WA_DeleteOnClose );
	setAttribute(Qt::WA_QuitOnClose,false);
	ui = new Ui::RzxIntro();
	ui->setupUi(this);

	icons << Rzx::ICON_JONE << Rzx::ICON_FTP << Rzx::ICON_LAYOUT << Rzx::ICON_SAMEGATEWAY << Rzx::ICON_PLUGIN
		<< Rzx::ICON_FAVORITE << Rzx::ICON_SAMBA << Rzx::ICON_PROPRIETES << Rzx::ICON_NETWORK << Rzx::ICON_BAN
		<< Rzx::ICON_HISTORIQUE << Rzx::ICON_HTTP << Rzx::ICON_SOUNDON << Rzx::ICON_OS0 << Rzx::ICON_OS1 << Rzx::ICON_OS2
		<< Rzx::ICON_OS3 << Rzx::ICON_OS4 << Rzx::ICON_OS5 << Rzx::ICON_OS6;

	connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->cbLanguage, SIGNAL(activated(const QString&)), this, SLOT(changeLanguage(const QString&)));
	connect(ui->cbIcons, SIGNAL(activated(const QString&)), this, SLOT(changeTheme(const QString&)));

	const QStringList list = RzxTranslator::languageIdList();
	for(int i = 0 ; i < list.count() ; i++)
		ui->cbLanguage->addItem(RzxIconCollection::getIcon(list[i]), RzxTranslator::name(list[i]));
	changeLanguage(RzxTranslator::language());

	ui->cbIcons->addItems(RzxIconCollection::themeList());
	ui->cbIcons->setCurrentIndex(ui->cbIcons->findText(RzxIconCollection::theme()));
	changeTheme(RzxIconCollection::theme());
}

///Fermeture...
RzxIntro::~RzxIntro()
{
	delete ui;
}

///Changement de language
void RzxIntro::changeLanguage(const QString& language)
{
	RzxTranslator::setLanguage(language);
	ui->retranslateUi(this); //Attention cette fonction n'est pas documentée
}

///Changement du thème d'icône
void RzxIntro::changeTheme(const QString& theme)
{
	RzxIconCollection::setTheme(theme);

	ui->listIcons->clear();
	foreach(Rzx::Icon i, icons)
	{
		QListWidgetItem *item = new QListWidgetItem(ui->listIcons);
		item->setIcon(RzxIconCollection::getIcon(i));
	}

	ui->btnOK->setIcon(RzxIconCollection::getIcon(Rzx::ICON_OK));
}
