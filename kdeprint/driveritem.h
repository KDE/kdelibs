/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
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

#ifndef DRIVERITEM_H
#define DRIVERITEM_H

#include <QTreeWidget>

class DrBase;

class DriverItem : public QTreeWidgetItem
{
public:
	DriverItem(QTreeWidget *parent, DrBase *item);
	DriverItem(QTreeWidgetItem *parent, QTreeWidgetItem *after, DrBase *item);

	void updateText();
	void updateTextRecursive();
	DrBase* drItem() const 	{ return m_item; }
	bool hasConflict()	{ return m_conflict; }
	bool updateConflict();

private:
	DrBase	*m_item;
	bool	m_conflict;
};

#endif
