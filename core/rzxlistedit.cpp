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
#include <QShortcut>

#include <RzxIconCollection>
#include <RzxComputerList>
#include <RzxConnectionLister>

#include <RzxListEdit>
#include <RzxProperty>

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
	connect(ui->list, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
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

	disconnect(this, SLOT(tryRemove()));
	RzxProperty *prop = qobject_cast<RzxProperty*>(window());
	if(prop)
		connect(prop, SIGNAL(deleteWanted()), this, SLOT(tryRemove()));

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
	QStringList lst = list->humanReadable();
	foreach(QString name, lst)
	{
		const RzxComputer *computer = RzxConnectionLister::global()->getComputerByName(name);
		if(!computer)
			new QListWidgetItem(RzxIconCollection::getIcon(Rzx::ICON_AWAY), name, ui->list);
		else
			new QListWidgetItem(computer->icon(), name, ui->list);
	}
	ui->list->sortItems();
}

///Ajoute l'élément en cours d'édition à la liste
void RzxListEdit::add()
{
	const QString text = ui->editNew->text().simplified();
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
		computer->emitUpdate();

	ui->editNew->clear();
	refresh();
}

///Teste si le focus appartient à la fenêtre courante avant d'appeler remove
void RzxListEdit::tryRemove()
{
	if(ui->list->hasFocus())
		remove();
}

///Supprime l'élément sélectionné
void RzxListEdit::remove()
{
	if(!list) return;

	const QList<QListWidgetItem*> items = ui->list->selectedItems();
	if(!items.size()) return;

	foreach(QListWidgetItem *item, items)
	{
		RzxComputer *computer = RzxConnectionLister::global()->getComputerByName(item->text());
		list->remove(item->text());
		if(computer)
			computer->emitUpdate();
	}
	refresh();
}

///Le texte a été édité
void RzxListEdit::edited(const QString& text)
{
	ui->btnAdd->setEnabled(!text.simplified().isEmpty());
}

///La sélection a changé
void RzxListEdit::selectionChanged()
{
	ui->btnDel->setEnabled(ui->list->currentRow() >= 0);
}
