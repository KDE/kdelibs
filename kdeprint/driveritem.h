/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef DRIVERITEM_H
#define DRIVERITEM_H

#include <qlistview.h>

class DrBase;

class DriverItem : public QListViewItem
{
public:
	DriverItem(QListView *parent, DrBase *item);
	DriverItem(QListViewItem *parent, QListViewItem *after, DrBase *item);

	void updateText();
	void updateTextRecursive();
	DrBase* drItem() const 	{ return m_item; }
	bool hasConflict()	{ return m_conflict; }
	bool updateConflict();

	void paintCell(QPainter*, const QColorGroup&, int, int, int);

private:
	DrBase	*m_item;
	bool	m_conflict;
};

#endif
