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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef KMICONVIEW_H
#define KMICONVIEW_H

#include <kiconview.h>
#include <qptrlist.h>

#include "kmobject.h"

class KMPrinter;

class KMIconViewItem : public QIconViewItem, public KMObject
{
public:
	KMIconViewItem(QIconView *parent, KMPrinter *p);
	void updatePrinter(KMPrinter *printer = 0, int mode = QIconView::Bottom);
	bool isClass() const	{ return m_isclass; }

protected:
	virtual void paintItem(QPainter*, const QColorGroup&);
	virtual void calcRect(const QString& text_ = QString::null);

private:
	int		m_mode;
	QString		m_pixmap;
	char		m_state;
	bool		m_isclass;
};

class KMIconView : public KIconView
{
	Q_OBJECT
public:
	enum ViewMode { Big, Small };

	KMIconView(QWidget *parent = 0, const char *name = 0);
	~KMIconView();

	void setPrinterList(QPtrList<KMPrinter> *list);
	void setPrinter(const QString&);
	void setPrinter(KMPrinter*);
	void setViewMode(ViewMode);

signals:
	void rightButtonClicked(const QString&, const QPoint&);
	void printerSelected(const QString&);

protected slots:
	void slotRightButtonClicked(QIconViewItem*, const QPoint&);
	void slotSelectionChanged();

private:
	KMIconViewItem* findItem(KMPrinter *p);

private:
	QPtrList<KMIconViewItem>	m_items;
	ViewMode		m_mode;
};

#endif
