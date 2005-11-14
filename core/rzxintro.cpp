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


///Construction de la fen�tre
RzxIntro::RzxIntro()
	:QDialog()
{
	setAttribute( Qt::WA_DeleteOnClose );
	setupUi(this);

	icons << Rzx::ICON_JONE << Rzx::ICON_FTP << Rzx::ICON_LAYOUT << Rzx::ICON_SAMEGATEWAY << Rzx::ICON_PLUGIN
		<< Rzx::ICON_FAVORITE << Rzx::ICON_SAMBA << Rzx::ICON_PROPRIETES << Rzx::ICON_NETWORK << Rzx::ICON_BAN
		<< Rzx::ICON_HISTORIQUE << Rzx::ICON_HTTP << Rzx::ICON_SOUNDON << Rzx::ICON_OS0 << Rzx::ICON_OS1 << Rzx::ICON_OS2
		<< Rzx::ICON_OS3 << Rzx::ICON_OS4 << Rzx::ICON_OS5 << Rzx::ICON_OS6;

	connect(btnOK, SIGNAL(clicked()), this, SLOT(close()));
	connect(cbLanguage, SIGNAL(activated(const QString&)), this, SLOT(changeLanguage(const QString&)));
	connect(cbIcons, SIGNAL(activated(const QString&)), this, SLOT(changeTheme(const QString&)));

	cbLanguage->addItems(RzxTranslator::translationsList());
	cbLanguage->setCurrentIndex(cbLanguage->findText(RzxTranslator::language()));
	changeLanguage(RzxTranslator::language());

	cbIcons->addItems(RzxIconCollection::themeList());
	cbIcons->setCurrentIndex(cbIcons->findText(RzxIconCollection::theme()));
	changeTheme(RzxIconCollection::theme());
}

///Fermeture...
RzxIntro::~RzxIntro()
{ }

///Changement de language
void RzxIntro::changeLanguage(const QString& language)
{
	qDebug() << "change language";
	RzxTranslator::setLanguage(language);
	retranslateUi(this); //Attention cette fonction n'est pas document�e
}

///Changement du th�me d'ic�ne
void RzxIntro::changeTheme(const QString& theme)
{
	qDebug() << "change theme";
	RzxIconCollection::global()->setTheme(theme);

	listIcons->clear();
	foreach(Rzx::Icon i, icons)
	{
		QListWidgetItem *item = new QListWidgetItem(listIcons);
		item->setIcon(RzxIconCollection::getIcon(i));
	}

	btnOK->setIcon(RzxIconCollection::getIcon(Rzx::ICON_OK));
}
