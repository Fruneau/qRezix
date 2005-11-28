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

///D�crit une interface � impl�menter pour fournir un afficheur
/** Cette classe fourni une interface pour d�finir les Rezals pour QRezix...
 * Elle est donc cens� fournir des informations sur le contenu du 'rezal' telles que
 * son nom, sa version, les dispositions qu'il accepte au sein de la fen�tre principale...
 *
 * La suite explique une petite note historique sur qRezix...
 *
 * <h2>Historique</h2>
 * A l'origine de qRezix, le RzxRezal �tait un QListView qui g�rait la quasi
 * totalit� de qRezix... avec la version 1.6 de qRezix, les choses ont �t�
 * l�g�rement modifi�es. Le RzxConnectionLister est venu au jour pour g�rer
 * la liste des �l�ments connect�s.
 *
 * La migration � Qt4 aura sign� le glas de ce RzxRezal tel qu'on l'a connu.
 * La gestion concraite des connexion est totalement g�r�e par RzxConnectionLister
 * et tout le reste est concentr� dans des classes sp�cialis�es. qRezix devient
 * modulaire et appara�t pour l'affichage un ensemble de classe RzxRezal*. Ce syst�me
 * est bas� sur le nouveau syst�me Model/View de Qt4. Le mod�le commun est 
 * RzxRezalModel qui impl�mente QAbstractModel, des outils pour simplifier le travail
 * des diff�rents afficheurs (comme RzxRezalSearch, RzxRezalPopup...)
 * et l'affichage est r�alis� par d'autres RzxRezal* (RzxRezalView, RzxRezalDetail...).
 *
 * Cette nouvelle version de qRezix, dont le d�veloppement a d�but� avec la
 * migration vers Qt4 se doit d'�tre modulaire. Il est donc n�cessaire pour avoir
 * des modules d'avoir une interface commune � tous les modules. qRezix en lui-m�me
 * n'est d�sormais plus qu'une centrale de chargement de modules qui compte les
 * connexions des utilisateurs. L'interface principale est une de ces modules.
 * Et cette interface est elle-m�me modulaire en fournissant des afficheurs
 * personnalisables. Ces afficheurs sont dont �quivalent � l'historique RzxRezal.
 * Il est donc naturel d'avoir ressucit� l'ancienne RzxRezal pour nomm� les afficheurs
 * qui doivent donc impl�menter cette interface.
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

		///Retourne une fen�tre utilisable pour l'affichage.
		virtual QAbstractItemView *widget() = 0;
		///Retourne les caract�ristiques du rezal en tant que dock
		virtual QDockWidget::DockWidgetFeatures features() const = 0;
		///Retourne les positions autoris�es du rezal en tant que dock
		virtual Qt::DockWidgetAreas allowedAreas() const = 0;
		///Indique l'�tat de pr�f�rence
		virtual bool floating() const = 0;
		///Indique la position de pr�f�rence
		virtual Qt::DockWidgetArea area() const = 0;
		virtual void updateLayout();

		const Type& type() const;
	protected:
		void setType(const Type&);
};

#define RZX_REZAL_EXPORT(MODULE) RZX_BASEMODULE_EXPORT(getRezal, getRezalName, getRezalVersion, RzxRezal, MODULE)

#endif
