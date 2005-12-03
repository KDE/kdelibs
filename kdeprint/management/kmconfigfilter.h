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

#ifndef KMCONFIGFILTER_H
#define KMCONFIGFILTER_H

#include "kmconfigpage.h"

class KListBox;
class QToolButton;
class QLineEdit;

class KMConfigFilter : public KMConfigPage
{
	Q_OBJECT
public:
	KMConfigFilter(QWidget *parent = 0);

	void loadConfig(KConfig*);
	void saveConfig(KConfig*);

protected slots:
	void slotSelectionChanged();
	void slotAddClicked();
	void slotRemoveClicked();

protected:
	void transfer(KListBox *from, KListBox *to);

private:
	KListBox	*m_list1, *m_list2;
	QToolButton	*m_add, *m_remove;
	QLineEdit	*m_locationre;
};

#endif
