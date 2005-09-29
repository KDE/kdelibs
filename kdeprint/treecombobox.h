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

#ifndef TREECOMBOBOX_H
#define TREECOMBOBOX_H

#include <qlistbox.h>
#include <qcombobox.h>
#include <qstringlist.h>

/**
 * Class that represents a single object in the tree
 */
class TreeListBoxItem : public QListBoxPixmap
{
public:
	TreeListBoxItem(QListBox *lb, const QPixmap& pix, const QString& txt, bool oneBlock = false);

	virtual int width(const QListBox *lb) const;

protected:
	virtual void paint(QPainter *p);
	int stepSize() const { return 16; }

private:
	QStringList	m_path;
	int		m_depth;
	TreeListBoxItem	*m_child, *m_next, *m_parent;
};

/**
 * Class for the listbox shown on popup
 */
class TreeListBox : public QListBox
{
	friend class TreeListBoxItem;
public:
	TreeListBox(QWidget *parent = 0, const char *name = 0);

protected:
	virtual void paintCell(QPainter *p, int row, int col);

private:
	bool	m_painting;
};

/**
 * Main class.
 */
class TreeComboBox : public QComboBox
{
public:
	TreeComboBox(QWidget *parent = 0, const char *name = 0);
	void insertItem(const QPixmap& pix, const QString& txt, bool oneBlock = false);

private:
	QListBox	*m_listbox;
};

#endif
