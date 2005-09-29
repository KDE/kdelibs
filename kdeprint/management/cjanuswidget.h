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

#ifndef CJANUSWIDGET_H
#define CJANUSWIDGET_H

#include <qwidget.h>
#include <qpixmap.h>
#include <qptrlist.h>

class QWidgetStack;
class QLabel;
class QListBoxItem;

class CJanusWidget : public QWidget
{
	Q_OBJECT
public:
	CJanusWidget(QWidget *parent, const char *name);
	~CJanusWidget();

	void addPage(QWidget *w, const QString& text, const QString& header, const QPixmap& pix);
	void enablePage(QWidget *w);
	void disablePage(QWidget *w);
	void clearPages();

protected slots:
	void slotSelected(QListBoxItem*);

public:
	class CPage;
	class CListBox;
	class CListBoxItem;

private:
	CPage* findPage(QWidget *w);
	CPage* findPage(QListBoxItem *i);
	QListBoxItem* findPrevItem(CPage*);

private:
	QPtrList<CPage>		m_pages;
	CListBox		*m_iconlist;
	QLabel			*m_header;
	QWidgetStack		*m_stack;
	QWidget			*m_empty;
};

#endif
