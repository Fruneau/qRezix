/***************************************************************************
                       rzxchatpopup  -  description
                             -------------------
    begin                : Sat Nov 19 2005
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
#ifndef RZXCHATPOPUP_H
#define RZXCHATPOPUP_H

#include <QFrame>

/**
 @author Florent Bruneau
 */

class QAbstractButton;

///Classe pour les fenêtre propriété et historique du chat
/** Affiche une frame qui permet de contenir les données des propriétés et de l'historique.<br>
  * Cette classe permet simplement une abstraction de la chose, sans pour autant apporter quoi que ce soit de vraiment nouveau
  */
class RzxChatPopup : public QFrame
{
	Q_OBJECT

	QAbstractButton *button;

	public:
		RzxChatPopup(QAbstractButton *btn, QWidget *parent = NULL);
		~RzxChatPopup() { }
		
		void forceVisible(bool pos);
		void move();
};

inline void RzxChatPopup::forceVisible(bool pos)
{
	if(pos) show();
	else hide();
}



#endif
