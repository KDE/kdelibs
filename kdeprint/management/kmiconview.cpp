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

#include "kmiconview.h"
#include "kmprinter.h"

#include <qpainter.h>
#include <kiconloader.h>

KMIconViewItem::KMIconViewItem(QIconView *parent, KMPrinter *p)
: QIconViewItem(parent)
{
	m_state = 0;
	m_mode = parent->itemTextPos();
	m_pixmap = QString::null;
	m_isclass = false;
	updatePrinter(p, m_mode);
}

void KMIconViewItem::paintItem(QPainter *p, const QColorGroup& cg)
{
	if (m_state != 0)
	{
		QFont	f(p->font());
		if (m_state & 0x1) f.setBold(true);
		if (m_state & 0x2) f.setItalic(true);
		p->setFont(f);
	}
	QIconViewItem::paintItem(p,cg);
}

void KMIconViewItem::updatePrinter(KMPrinter *p, int mode)
{
	bool	update(false);
	int	oldstate = m_state;
	if (p)
	{
		m_state = ((p->isHardDefault() ? 0x1 : 0x0) | (p->ownSoftDefault() ? 0x2 : 0x0) | (p->isValid() ? 0x4 : 0x0));
		update = (oldstate != m_state);
		if (p->name() != text() || update)
			setText(p->name());
		setKey(QString::fromLatin1("%1_%2").arg((p->isSpecial() ? "special" : (p->isClass(false) ? "class" : "printer"))).arg(p->name()));
		m_isclass = p->isClass(false);
	}
	if (mode != m_mode || ((oldstate&0x4) != (m_state&0x4)) || (p && p->pixmap() != m_pixmap))
	{
		int	iconstate = (m_state&0x4 ? (int)KIcon::DefaultState : (int)KIcon::LockOverlay);
		if (p)
			m_pixmap = p->pixmap();
		m_mode = mode;
		if (m_mode == QIconView::Bottom)
			setPixmap(DesktopIcon(m_pixmap, 0, iconstate));
		else
			setPixmap(SmallIcon(m_pixmap, 0, iconstate));
	}
	if (update)
		repaint();
	setDiscarded(false);
}

KMIconView::KMIconView(QWidget *parent, const char *name)
: KIconView(parent,name)
{
	setMode(KIconView::Select);
	setSelectionMode(QIconView::Single);
	setItemsMovable(false);
	setResizeMode(QIconView::Adjust);

	m_items.setAutoDelete(false);
	setViewMode(KMIconView::Big);

	connect(this,SIGNAL(rightButtonClicked(QIconViewItem*,const QPoint&)),SLOT(slotRightButtonClicked(QIconViewItem*,const QPoint&)));
	connect(this,SIGNAL(selectionChanged()),SLOT(slotSelectionChanged()));
}

KMIconView::~KMIconView()
{
}

KMIconViewItem* KMIconView::findItem(KMPrinter *p)
{
	if (p)
	{
		QPtrListIterator<KMIconViewItem>	it(m_items);
		for (;it.current();++it)
			if (it.current()->text() == p->name()
			    && it.current()->isClass() == p->isClass())
				return it.current();
	}
	return 0;
}

void KMIconView::setPrinterList(QPtrList<KMPrinter> *list)
{
	bool	changed(false);

	QPtrListIterator<KMIconViewItem>	it(m_items);
	for (;it.current();++it)
		it.current()->setDiscarded(true);

	if (list)
	{
		QPtrListIterator<KMPrinter>	it(*list);
		KMIconViewItem			*item(0);
		for (;it.current();++it)
		{
                        // only keep real printers (no instances)
                        if (!it.current()->instanceName().isEmpty())
                                continue;
			item = findItem(it.current());
			if (!item)
			{
				item = new KMIconViewItem(this,it.current());
				m_items.append(item);
				changed = true;
			}
			else
				item->updatePrinter(it.current(), itemTextPos());
		}
	}

	for (uint i=0; i<m_items.count(); i++)
		if (m_items.at(i)->isDiscarded())
		{
			delete m_items.take(i);
			i--;
			changed = true;
		}

	if (changed) sort();
	emit selectionChanged();
}

void KMIconView::setViewMode(ViewMode m)
{
	m_mode = m;
	bool	big = (m == KMIconView::Big);
	int	mode = (big ? QIconView::Bottom : QIconView::Right);

	QPtrListIterator<KMIconViewItem>	it(m_items);
	for (;it.current();++it)
		it.current()->updatePrinter(0, mode);

	setArrangement((big ? QIconView::LeftToRight : QIconView::TopToBottom));
	setItemTextPos((QIconView::ItemTextPos)mode);
	//setGridX((big ? 60 : -1));
	setWordWrapIconText(big);
}

void KMIconView::slotRightButtonClicked(QIconViewItem *item, const QPoint& p)
{
	emit rightButtonClicked(item ? item->text() : QString::null, p);
}

void KMIconView::slotSelectionChanged()
{
	KMIconViewItem	*item = static_cast<KMIconViewItem*>(currentItem());
	emit printerSelected((item && !item->isDiscarded() ? item->text() : QString::null));
}

void KMIconView::setPrinter(const QString& prname)
{
	QPtrListIterator<KMIconViewItem>	it(m_items);
	for (; it.current(); ++it)
		if (it.current()->text() == prname)
		{
			setSelected(it.current(), true);
			break;
		}
}

void KMIconView::setPrinter(KMPrinter *p)
{
	setPrinter(p ? p->name() : QString::null);
}

#include "kmiconview.moc"
