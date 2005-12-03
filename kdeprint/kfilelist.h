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

#ifndef KFILELIST_H
#define KFILELIST_H

#include <qwidget.h>
#include <q3ptrlist.h>
#include <kurl.h>

class KListView;
class QToolButton;
class Q3ListViewItem;

class KFileList : public QWidget
{
	Q_OBJECT
public:
	KFileList( QWidget *parent = 0 );
	virtual ~KFileList();

	void setFileList(const QStringList&);
	QStringList fileList() const;
	QSize sizeHint() const;

protected slots:
	void slotAddFile();
	void slotRemoveFile();
	void slotOpenFile();
	void slotSelectionChanged();
	void slotUp();
	void slotDown();

protected:
	void dragEnterEvent(QDragEnterEvent*);
	void dropEvent(QDropEvent*);
	void addFiles(const KURL::List&);
	void selection(Q3PtrList<Q3ListViewItem>&);

private:
	KListView	*m_files;
	QToolButton	*m_add, *m_remove, *m_open, *m_up, *m_down;
	bool		m_block;
};

#endif
