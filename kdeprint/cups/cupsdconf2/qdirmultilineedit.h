/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2002 Michael Goffioul <kdeprint@swing.be>
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

#ifndef QDIRMULTILINEEDIT_H
#define QDIRMULTILINEEDIT_H

#include <qwidget.h>

class KListView;
class QListViewItem;
class QPushButton;

class QDirMultiLineEdit : public QWidget
{
	Q_OBJECT

public:
	QDirMultiLineEdit(QWidget *parent = 0, const char *name = 0);
	~QDirMultiLineEdit();

	void setURLs(const QStringList&);
	QStringList urls();

protected:
	void addURL(const QString&);

private slots:
	void slotAddClicked();
	void slotRemoveClicked();
	void slotSelected(QListViewItem*);

private:
	KListView	*m_view;
	QPushButton	*m_add, *m_remove;
};

#endif
