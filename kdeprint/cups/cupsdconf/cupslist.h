/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
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

#ifndef CUPSLIST_H
#define	CUPSLIST_H

#include <qwidget.h>
#include <qstringlist.h>

class QPushButton;
class QListView;
class QListViewItem;

class CupsListBox : public QWidget
{
	Q_OBJECT;
public:
	CupsListBox(QWidget *parent = 0, const char *name = 0);
	CupsListBox(int columns, QWidget *parent = 0, const char *name = 0);
	~CupsListBox();

	QString text(int index, int column = 0) const;
	void insertItem(const QString& str, int index = -1);
	void insertItem(const QStringList& strs, int index = -1);
	void setAddMessage(const QString& msg, int index = -1);
	int count() const;
	void setColumnText(int column, const QString& txt);

protected slots:
	void addClicked();
	void removeClicked();

protected:
	void createView(int columns);
	void createLayout();
	void createButtons();
	void init(int columns);
	QListViewItem* findItemAtIndex(int i) const;

private:
	QListView	*list_;
	QPushButton	*add_;
	QPushButton	*remove_;
	QStringList	addmsg_;
	int		columns_;
};

#endif
