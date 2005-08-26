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

#include <QDialog>

#include <RzxGlobal>
#include "ui_rzxpropertyui.h"

#define NB_COL 10

/**
  *@author Benoit Casoetto
  */

class RzxProperty : public QDialog, public Ui::frmPref
{
	Q_OBJECT
	RZX_GLOBAL(RzxProperty)

public: 
	RzxProperty(QWidget *parent = NULL);
	~RzxProperty();

	static QString browse(const QString& name, const QString& title, const QString& glob);

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
	void changeTheme();


protected: // Protected methods
	virtual void changeEvent(QEvent*);

private:
	void initLangCombo();
	void initThemeCombo();
	bool updateLocalHost();
	QPixmap localhostIcon;

protected slots: // Protected slots
	void launchDirSelectDialog();
	void chooseIcon();
	void lockCmbMenuText(int index);
	void changePage(QListWidgetItem*, QListWidgetItem*);
};

#endif
