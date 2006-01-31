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
	connect(ui->editNew, SIGNAL(editTextChanged(const QString&)), this, SLOT(edited(const QString&)));
	connect(ui->btnAdd, SIGNAL(clicked()), this, SLOT(add()));
	connect(ui->btnDel, SIGNAL(clicked()), this, SLOT(remove()));
	connect(ui->list, SIGNAL(computerDropped(const QString&)), this, SLOT(add(const QString&)));
	connect(ui->list, SIGNAL(computerDropped(const RzxHostAddress&)), this, SLOT(add(const RzxHostAddress&)));
	connect(ui->list, SIGNAL(dropFinished()), this, SLOT(lightRefresh()));
	
	const QList<RzxComputer*> computers = RzxConnectionLister::global()->computerList();
	foreach(RzxComputer *computer, computers)
		connectComputer(computer);
	connect(RzxConnectionLister::global(), SIGNAL(login(RzxComputer*)), this, SLOT(refresh(RzxComputer*)));
	connect(RzxConnectionLister::global(), SIGNAL(login(RzxComputer*)), this, SLOT(connectComputer(RzxComputer*)));
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
	disconnect(this, SLOT(enterPressed(bool&)));
	RzxProperty *prop = qobject_cast<RzxProperty*>(window());
	if(prop)
	{
		connect(prop, SIGNAL(deleteWanted()), this, SLOT(tryRemove()));
		connect(prop, SIGNAL(enterPressed(bool&)), this, SLOT(enterPressed(bool&)));
	}

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
	lightRefresh();
	ui->editNew->clear();
	if(!list) return;

	QList<RzxComputer*> computerList = RzxConnectionLister::global()->computerList();
	qSort(computerList.begin(), computerList.end(), computerLessThan);
	ui->editNew->addItem(tr("Name or IP address"));
	for(int i = 0 ; i < computerList.size() ; i++)
	{
		const RzxComputer *computer = computerList[i];
		if(!list->contains(computer))
			ui->editNew->addItem(computer->icon(), computer->name());
	}
	for(int i = 0 ; i < computerList.size() ; i++)
	{
		const RzxComputer *computer = computerList[i];
		if(!list->contains(computer))
			ui->editNew->addItem(computer->icon(), computer->ip().toString() + " (" + computer->name() + ")");
	}
}

///Effectue un raffraichissement alléger... sans regénérer la liste des ordinateurs
void RzxListEdit::lightRefresh()
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
	const QString text = ui->editNew->currentText().simplified().section(' ', 0, 0);
	if(!list || text.isEmpty() || text.contains(" ")) return;
	if(text.contains("."))
		add(RzxHostAddress(text));
	else
		add(text);

	ui->editNew->clearEditText();
	lightRefresh();
}

///Ajoute l'élément dont le nom est indiqué
void RzxListEdit::add(const QString& name)
{
	if(!list) return;

	list->add(name);
	RzxComputer *computer = RzxConnectionLister::global()->getComputerByName(name);
	if(computer)
		computer->emitUpdate();
}

///Ajoute la machine dont l'IP est indiquée
void RzxListEdit::add(const RzxHostAddress& ip)
{
	if(!list) return;

	list->add(ip);
	RzxComputer *computer = RzxConnectionLister::global()->getComputerByIP(ip);
	if(computer)
		computer->emitUpdate();
}

///Gère la pression sur la touche Enter
void RzxListEdit::enterPressed(bool& used)
{
	if(used) return;

	if(ui->btnAdd->isEnabled() && (ui->editNew->hasFocus() || ui->btnAdd->hasFocus()))
	{
		used = true;
		add();
	}
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
		if(!item) continue;
		RzxComputer *computer = RzxConnectionLister::global()->getComputerByName(item->text());
		list->remove(item->text());
		if(computer)
			computer->emitUpdate();
	}
	ui->btnDel->setEnabled(false);
	lightRefresh();
}

///Le texte a été édité
void RzxListEdit::edited(const QString& text)
{
	const QString test = text.simplified();
	QRegExp mask("^\\d+\\.\\d+\\.\\d+\\.\\d+ \\(.*\\)$");
	bool verif = !test.isEmpty() && (!test.contains(QRegExp("\\s")) || !mask.indexIn(test));
	ui->btnAdd->setEnabled(verif);
}

///La sélection a changé
void RzxListEdit::selectionChanged()
{
	ui->btnDel->setEnabled(ui->list->currentRow() >= 0);
}

///Connecte le List Edit pour prendre en compte les changement des ordinateurs
void RzxListEdit::connectComputer(RzxComputer *computer)
{
	connect(computer, SIGNAL(update(RzxComputer*)), this, SLOT(refresh(RzxComputer*)));
}

///Raffraichi l'affichage si l'ordinateur indiqué est dans la liste
void RzxListEdit::refresh(RzxComputer *computer)
{
	if(!computer || !list) return;
	if(list->contains(computer) || !ui->list->findItems(computer->name(), Qt::MatchExactly).isEmpty())
		lightRefresh();
}
