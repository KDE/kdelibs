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

#ifndef KMLISTVIEW_H
#define KMLISTVIEW_H

#include <qlistview.h>
#include <qptrlist.h>

class KMListViewItem;
class KMPrinter;

class KMListView : public QListView
{
	Q_OBJECT
public:
	KMListView(QWidget *parent = 0, const char *name = 0);
	~KMListView();

	void setPrinterList(QPtrList<KMPrinter> *list);
	void setPrinter(const QString&);
	void setPrinter(KMPrinter*);

signals:
	void rightButtonClicked(const QString&, const QPoint&);
	void printerSelected(const QString&);

protected slots:
	void slotRightButtonClicked(QListViewItem*, const QPoint&, int);
	void slotSelectionChanged();
	void slotOnItem(QListViewItem*);
	void slotOnViewport();

protected:
	KMListViewItem* findItem(KMPrinter*);
	KMListViewItem* findItem(const QString&);

private:
	QPtrList<KMListViewItem>	m_items;
	KMListViewItem		*m_root, *m_classes, *m_printers, *m_specials;
};

#endif
