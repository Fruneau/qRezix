/***************************************************************************
                          qrezix.h  -  description
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
#ifndef RZXPROPERTY_H
#define RZXPROPERTY_H

#include <qpixmap.h>
#include "rzxhostaddress.h"
#include "rzxpropertyui.h"
#include "qrezix.h"
#include "dnsvalidator.h"

#define NB_COL 10

/**
  *@author Benoit Casoetto
  */

class QDns;

class RzxProperty : public frmPref {
	Q_OBJECT
	
public: 
	RzxProperty(QRezix*parent);
	~RzxProperty();
signals:
	void end();
	
public slots: // Public slots
	/** Met a jour la base globale de configuration a partir des modifications de l'utilisateur */
	void miseAJour();
	void oK();
	void annuler();
	void initDlg();

protected: // Protected methods
	QRezix * getRezix() const;
	void serverUpdate();
	void writeColDisplay();

private:
	void initLangCombo();
	void initThemeCombo();
	void updateLocalHost();
	QPixmap localhostIcon;
	QString browse(const QString& name, const QString& title, const QString& glob);

protected slots: // Protected slots
	void launchDirSelectDialog();
	void chooseIcon();
	void chooseBeep();
};

#endif
