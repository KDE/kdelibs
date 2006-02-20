/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef CUPSDSECURITYPAGE_H
#define CUPSDSECURITYPAGE_H

#include "cupsdpage.h"
#include <QList>

class QLineEdit;
class EditList;
class QDirLineEdit;
struct CupsLocation;

class CupsdSecurityPage : public CupsdPage
{
	Q_OBJECT

public:
	CupsdSecurityPage(QWidget *parent = 0);
	~CupsdSecurityPage();

	bool loadConfig(CupsdConf*, QString&);
	bool saveConfig(CupsdConf*, QString&);
	void setInfos(CupsdConf*);

protected Q_SLOTS:
	void slotAdd();
	void slotEdit(int);
	void slotDefaultList();
	void slotDeleted(int);

private:
	QLineEdit	*remoteroot_, *systemgroup_;
	QDirLineEdit	*encryptcert_, *encryptkey_;
	EditList	*locations_;

	QList<CupsLocation*>	locs_;
};

#endif
