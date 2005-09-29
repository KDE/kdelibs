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

#include "treecombobox.h"

#include <qpainter.h>

TreeListBoxItem::TreeListBoxItem(QListBox *lb, const QPixmap& pix, const QString& txt, bool oneBlock)
	: QListBoxPixmap(pix, txt)
{
	if (oneBlock)
		m_path = QStringList(txt);
	else
		m_path = QStringList::split('/', text(), false);
	m_depth = m_path.count()-1;
	m_child = m_next = m_parent = 0;

	// insert into QListBox
	if (m_depth == 0)
	{
		TreeListBoxItem	*item = static_cast<TreeListBoxItem*>(lb->item(0));
		while (item && item->m_next)
			item = item->m_next;
		lb->insertItem(this);
		if (item)
			item->m_next = this;
	}
	else
	{
		QString	parentStr = txt.left(txt.length()-m_path[m_depth].length()-1);
		TreeListBoxItem	*parentItem = static_cast<TreeListBoxItem*>(lb->findItem(parentStr, Qt::ExactMatch));
		if (!parentItem)
		{
			// parent not found, add parent first into QListBox
			parentItem = new TreeListBoxItem(lb, QPixmap(), parentStr);
		}
		// search last "child" of the parent item, to put the new one
		// at the end
		TreeListBoxItem	*childItem = static_cast<TreeListBoxItem*>(parentItem), *prevItem = 0;
		while (childItem->next())
		{
			TreeListBoxItem	*nextItem = static_cast<TreeListBoxItem*>(childItem->next());
			if (nextItem->m_depth >= m_depth)
			{
				childItem = nextItem;
				if (childItem->m_depth == m_depth)
					prevItem = childItem;
			}
			else
				break;
		}
		// eventually insert item
		lb->insertItem(this, childItem);
		m_parent = parentItem;
		if (prevItem)
			prevItem->m_next = this;
		else
			parentItem->m_child = this;
	}
}

int TreeListBoxItem::width(const QListBox *lb) const
{
	int	w = m_depth * stepSize() + 2;
	if (pixmap())
		w += (pixmap()->width() + 2);
	if (!m_path[m_depth].isEmpty())
		w += (lb->fontMetrics().width(m_path[m_depth]) + 2);
	return QMAX(w, QListBoxPixmap::width(lb));
}

void TreeListBoxItem::paint(QPainter *p)
{
	if(!static_cast<TreeListBox*>(listBox())->m_painting)
	{
		QListBoxPixmap::paint(p);
		return;
	}

	const QPixmap	*pix = pixmap();
	QRect	r = p->viewport();
	int	h = height(listBox());
	int	xo = (m_depth * stepSize() + 2);
	int	yo = (pix ? (h-pix->height())/2 : 0);

	if (m_depth > 0)
	{
		QPen	oldPen = p->pen();
		p->setPen(listBox()->colorGroup().mid());

		TreeListBoxItem	*item = this;
		int	s = xo-stepSize()/2;
		p->drawLine(s, r.top(), s, h/2);
		p->drawLine(s, h/2, s+stepSize()/2-2, h/2);
		while (item->m_parent)
		{
			if (item->m_next)
				p->drawLine(s, r.top(), s, h);
			item = item->m_parent;
			s -= stepSize();
		}

		p->setPen(oldPen);
	}
	if (pix)
	{
		p->drawPixmap(xo, yo, *pix);
		xo += (pix->width() + 2);
	}
	p->drawText(xo, 0, r.width()-xo, height(listBox()), Qt::AlignLeft, m_path[m_depth]);
}

//-----------------------------------------------------------------------------------------

TreeListBox::TreeListBox(QWidget *parent, const char *name)
	: QListBox(parent, name)
{
	m_painting = false;
}

void TreeListBox::paintCell(QPainter *p, int row, int col)
{
	m_painting = true;
	QListBox::paintCell(p, row, col);
	m_painting = false;
}

//-----------------------------------------------------------------------------------------

TreeComboBox::TreeComboBox(QWidget *parent, const char *name)
	: QComboBox(parent, name)
{
	m_listbox = new TreeListBox(this);
	setListBox(m_listbox);
}

void TreeComboBox::insertItem(const QPixmap& pix, const QString& txt, bool oneBlock)
{
	new TreeListBoxItem(m_listbox, pix, txt, oneBlock);
}
