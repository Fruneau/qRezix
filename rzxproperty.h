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
#include <QDialog>
#include "ui_rzxpropertyui.h"

#define NB_COL 10

/**
  *@author Benoit Casoetto
  */

class Q3Dns;
class QRezix;

class RzxProperty : public QDialog, public Ui::frmPref {
	Q_OBJECT
	
public: 
	RzxProperty(QRezix*parent);
	~RzxProperty();

	bool infoCompleted();
	QString infoNeeded();
	int infoCompleteMessage();

signals:
	void end();
	
public slots: // Public slots
	/** Met a jour la base globale de configuration a partir des modifications de l'utilisateur */
	bool miseAJour();
	void oK();
	void annuler();
	void initDlg();
	void aboutQt();
	static void serverUpdate();
	void changeTheme();


protected: // Protected methods
	QRezix * getRezix() const;
	void writeColDisplay();

private:
	void initLangCombo();
	void initThemeCombo();
	bool updateLocalHost();
	QPixmap localhostIcon;
	QString browse(const QString& name, const QString& title, const QString& glob);
	virtual void languageChange();


protected slots: // Protected slots
	void launchDirSelectDialog();
	void chooseIcon();
	void chooseBeep();
	void chooseBeepConnection();
	void lockCmbMenuText(int index);
	void changePage(int i);
};

#endif
