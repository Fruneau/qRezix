/***************************************************************************
            rzxlistedit  -  fenêtre d'édition de computer list
                             -------------------
    begin                : Thu Dec 15 2005
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
#include <RzxIconCollection>
#include <RzxComputerList>
#include <RzxConnectionLister>

#include <RzxListEdit>

#include "ui_rzxlistedit.h"

///Constructeur
RzxListEdit::RzxListEdit(QWidget *parent)
	:QWidget(parent), list(NULL)
{
	ui = new Ui::RzxListEdit();
	ui->setupUi(this);
	ui->btnAdd->setEnabled(false);
	ui->btnDel->setEnabled(false);
	changeTheme();
	RzxIconCollection::connect(this, SLOT(changeTheme()));
	connect(ui->list, SIGNAL(currentRowChanged(int)), this, SLOT(selectionChanged(int)));
	connect(ui->editNew, SIGNAL(textChanged(const QString&)), this, SLOT(edited(const QString&)));
	connect(ui->btnAdd, SIGNAL(clicked()), this, SLOT(add()));
	connect(ui->btnDel, SIGNAL(clicked()), this, SLOT(remove()));
}

///Destructeur
RzxListEdit::~RzxListEdit()
{
	delete ui;
}

///Défini la liste d'ordinateurs à utiliser
void RzxListEdit::setList(RzxComputerList *m_list)
{
	list = m_list;
	refresh();
}

///Change le thème d'icône utilisé...
void RzxListEdit::changeTheme()
{
	ui->btnAdd->setIcon(RzxIconCollection::getIcon(Rzx::ICON_LOAD));
	ui->btnDel->setIcon(RzxIconCollection::getIcon(Rzx::ICON_UNLOAD));
	refresh();
}

///Raffraîchi l'affichage de la liste
void RzxListEdit::refresh()
{
	ui->list->clear();
	if(!list) return;
	QStringList lst = list->humanReadable(true);
	foreach(QString name, lst)
	{
		if(name.size() >= 3 && name.right(3) == "(*)")
			new QListWidgetItem(RzxIconCollection::getIcon(Rzx::ICON_AWAY), name.left(name.size() - 3), ui->list);
		else
			new QListWidgetItem(RzxIconCollection::getIcon(Rzx::ICON_HERE), name, ui->list);
	}
}

///Ajoute l'élément en cours d'édition à la liste
void RzxListEdit::add()
{
	QString text = ui->editNew->text();
	if(!list || text.isEmpty()) return;
	RzxComputer *computer = NULL;
	if(text.contains("."))
	{
		list->add(RzxHostAddress(text));
		computer = RzxConnectionLister::global()->getComputerByIP(text);
	}
	else
	{
		list->add(text);
		computer = RzxConnectionLister::global()->getComputerByName(text);
	}
	if(computer)
		emit added(computer);

	ui->editNew->clear();
	refresh();
}

///Supprime l'élément sélectionné
void RzxListEdit::remove()
{
	int i = ui->list->currentRow();
	if(list)
	{
		RzxComputer *computer = RzxConnectionLister::global()->getComputerByName(ui->list->currentItem()->text());
		list->remove(i);
		if(computer)
			emit deleted(computer);
	}
	refresh();
}

///Le texte a été édité
void RzxListEdit::edited(const QString& text)
{
	ui->btnAdd->setEnabled(!text.isEmpty());
}

///La sélection a changé
void RzxListEdit::selectionChanged(int i)
{
	ui->btnDel->setEnabled(i >= 0);
}
