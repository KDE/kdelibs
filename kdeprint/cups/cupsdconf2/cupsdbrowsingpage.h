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

#ifndef CUPSDBROWSINGPAGE_H
#define CUPSDBROWSINGPAGE_H

#include "cupsdpage.h"

class KIntNumInput;
class QCheckBox;
class EditList;
class QComboBox;

class CupsdBrowsingPage : public CupsdPage
{
	Q_OBJECT

public:
	CupsdBrowsingPage(QWidget *parent = 0);

	bool loadConfig(CupsdConf*, QString&);
	bool saveConfig(CupsdConf*, QString&);
	void setInfos(CupsdConf*);

protected slots:
	void slotAdd();
	void slotEdit(int);
	void slotDefaultList();
	void intervalChanged(int);

private:
	KIntNumInput	*browseport_, *browseinterval_, *browsetimeout_;
	EditList	*browseaddresses_;
	QComboBox	*browseorder_;
	QCheckBox	*browsing_, *cups_, *slp_;
	QCheckBox	*useimplicitclasses_, *hideimplicitmembers_, *useshortnames_, *useanyclasses_;
};

#endif
