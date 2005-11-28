/***************************************************************************
                          rzxrezal.h  -  description
                             -------------------
    begin                : Mon Aug 15 2005
    copyright            : (C) 2005 Florent Bruneau
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
#include <RzxBaseModule>

#ifndef RZXREZAL_H
#define RZXREZAL_H

#include <QDockWidget>

#include "rzxmainuiglobal.h"

class QAbstractItemView;

/**
 @author Florent Bruneau
 */

///Décrit une interface à implémenter pour fournir un afficheur
/** Cette classe fourni une interface pour définir les Rezals pour QRezix...
 * Elle est donc censé fournir des informations sur le contenu du 'rezal' telles que
 * son nom, sa version, les dispositions qu'il accepte au sein de la fenêtre principale...
 *
 * La suite explique une petite note historique sur qRezix...
 *
 * <h2>Historique</h2>
 * A l'origine de qRezix, le RzxRezal était un QListView qui gérait la quasi
 * totalité de qRezix... avec la version 1.6 de qRezix, les choses ont été
 * légèrement modifiées. Le RzxConnectionLister est venu au jour pour gérer
 * la liste des éléments connectés.
 *
 * La migration à Qt4 aura signé le glas de ce RzxRezal tel qu'on l'a connu.
 * La gestion concraite des connexion est totalement gérée par RzxConnectionLister
 * et tout le reste est concentré dans des classes spécialisées. qRezix devient
 * modulaire et apparaît pour l'affichage un ensemble de classe RzxRezal*. Ce système
 * est basé sur le nouveau système Model/View de Qt4. Le modèle commun est 
 * RzxRezalModel qui implémente QAbstractModel, des outils pour simplifier le travail
 * des différents afficheurs (comme RzxRezalSearch, RzxRezalPopup...)
 * et l'affichage est réalisé par d'autres RzxRezal* (RzxRezalView, RzxRezalDetail...).
 *
 * Cette nouvelle version de qRezix, dont le développement a débuté avec la
 * migration vers Qt4 se doit d'être modulaire. Il est donc nécessaire pour avoir
 * des modules d'avoir une interface commune à tous les modules. qRezix en lui-même
 * n'est désormais plus qu'une centrale de chargement de modules qui compte les
 * connexions des utilisateurs. L'interface principale est une de ces modules.
 * Et cette interface est elle-même modulaire en fournissant des afficheurs
 * personnalisables. Ces afficheurs sont dont équivalent à l'historique RzxRezal.
 * Il est donc naturel d'avoir ressucité l'ancienne RzxRezal pour nommé les afficheurs
 * qui doivent donc implémenter cette interface.
 */
class RZX_MAINUI_EXPORT RzxRezal:public RzxBaseModule
{
	Q_ENUMS(TypeFlags)
	Q_FLAGS(Type)

	public:
		enum TypeFlags {
			TYP_NONE = 0,
			TYP_DOCKABLE = 1,
			TYP_CENTRAL = 2,
			TYP_ALL = TYP_DOCKABLE | TYP_CENTRAL,
			TYP_INDEXED = 4,
			TYP_INDEX = 8
		};
		Q_DECLARE_FLAGS(Type, TypeFlags)
	
	private:
		Type m_type;
		QDockWidget *dock;

	public:
		RzxRezal(const QString&, const QString&, const Rzx::Version&);
		virtual ~RzxRezal();
		virtual bool isInitialised() const;

		QDockWidget *dockWidget() const;
		void setDockWidget(QDockWidget *);

		///Retourne une fenêtre utilisable pour l'affichage.
		virtual QAbstractItemView *widget() = 0;
		///Retourne les caractéristiques du rezal en tant que dock
		virtual QDockWidget::DockWidgetFeatures features() const = 0;
		///Retourne les positions autorisées du rezal en tant que dock
		virtual Qt::DockWidgetAreas allowedAreas() const = 0;
		///Indique l'état de préférence
		virtual bool floating() const = 0;
		///Indique la position de préférence
		virtual Qt::DockWidgetArea area() const = 0;
		virtual void updateLayout();

		const Type& type() const;
	protected:
		void setType(const Type&);
};

#define RZX_REZAL_EXPORT(MODULE) RZX_BASEMODULE_EXPORT(getRezal, getRezalName, getRezalVersion, RzxRezal, MODULE)

#endif
