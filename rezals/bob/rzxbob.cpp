/***************************************************************************
                          rzxbob.cpp  -  description
                             -------------------
    begin                : Sat Jan 27 2007
    copyright            : (C) 2007 by Guillaume Bandet
    email                : guillaume.bandet@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define RZX_MODULE_NAME "Bob"
#define RZX_MODULE_DESCRIPTION "B�bar open state"
#define RZX_MODULE_ICON RzxThemedIcon("bob")

#define RZX_BOB_URL "http://frankiz.eleves.polytechnique.fr/gestion/bob/etat_bob_automatique.php"
#define RZX_BOB_URL_PARAM "?estOuvert"

#include <RzxApplication>

#include <QHttp>
#include <QAction>
#include <QUrl>

#include "rzxbob.h"

///Exporte le rezal
RZX_REZAL_EXPORT(RzxBob)

///RzxBob affiche simplement un bouton dans la
/** toolbar pour indiquer l'�tat du b�b
 */
RzxBob::RzxBob()
		: QObject(0), RzxRezal(RZX_MODULE_NAME, QT_TRANSLATE_NOOP("RzxBaseModule", RZX_MODULE_DESCRIPTION), RZX_MODULE_VERSION), bobState(false), http(0)
{
	beginLoading();
	setType(TYP_TOOL);

	setIcon(RZX_MODULE_ICON);	//a voir
	action = new QAction("BOB", 0);	//mettre l'icone
	connect(action, SIGNAL(triggered()), this, SLOT(checkState()));

	checkState();
	endLoading();
}

///Destructeur
RzxBob::~RzxBob()
{
	beginClosing();
	if(http)
		http->deleteLater();
	endClosing();
}

void RzxBob::checkState()
{
	if(!http)
	{
		http = new QHttp(this);
		connect(http, SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
	}
	http->abort();
	QUrl url(RZX_BOB_URL);
	http->setHost(url.host(), url.port() != -1 ? url.port() : 80);
	http->get(url.path() + RZX_BOB_URL_PARAM);
}

void RzxBob::httpRequestFinished(int id, bool error)
{
	if(error)
	{
		return;
	}
	QByteArray data = http->readAll();
	int taille = data.size();
	if(taille)
		if(data[0] == '0')
		{
			if(taille == 1)
				action->setText("Le B�b est ferm�");
			else
				action->setText(QString().fromUtf8(data.right(taille -1).data(), taille-1));
		}
		else if(data[0] == '1')
		{
			if(taille == 1)
				action->setText("Le B�b est ouvert");
			else
				action->setText(QString().fromUtf8(data.right(taille -1).data(), taille-1));
		}
}

/// Retourne 0 puisque n'utilise pas de fen�tre, a priori la fonction n'est pas appel�e
QAbstractItemView *RzxBob::widget()
{
	return 0;
}

///Retourne les caract�ristiques du rezal en tant que dock
QDockWidget::DockWidgetFeatures RzxBob::features() const
{
	return QDockWidget::NoDockWidgetFeatures;
}

///Retourne les positions autoris�es du rezal en tant que dock
Qt::DockWidgetAreas RzxBob::allowedAreas() const
{
	return Qt::NoDockWidgetArea;
}

///Retourne la position par d�faut du rezal en tant que dock
Qt::DockWidgetArea RzxBob::area() const
{
	return Qt::NoDockWidgetArea;
}

///Retourne l'�tat par d�faut du rezal
bool RzxBob::floating() const
{
	return false;
}

///Retourne le bouton de la barre d'outils, le seul �l�ment graphique de ce module
QAction* RzxBob::toolButton()
{
	return action;
}
