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

#include "kmlistview.h"
#include "kmprinter.h"
#include "kmobject.h"

#include <qheader.h>
#include <qpainter.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kcursor.h>

class KMListViewItem : public QListViewItem, public KMObject
{
public:
	KMListViewItem(QListView *parent, const QString& txt);
	KMListViewItem(QListViewItem *parent, const QString& txt);
	KMListViewItem(QListViewItem *parent, KMPrinter *p);

	virtual void paintCell(QPainter*, const QColorGroup&, int, int, int);
	void updatePrinter(KMPrinter *p);
	bool isClass() const	{ return m_isclass; }

protected:
	void init(KMPrinter *p = 0);

private:
	int	m_state;
	bool	m_isclass;
};

KMListViewItem::KMListViewItem(QListView *parent, const QString& txt)
: QListViewItem(parent,txt)
{
	init();
}

KMListViewItem::KMListViewItem(QListViewItem *parent, const QString& txt)
: QListViewItem(parent,txt)
{
	init();
}

KMListViewItem::KMListViewItem(QListViewItem *parent, KMPrinter *p)
: QListViewItem(parent)
{
	init(p);
}

void KMListViewItem::init(KMPrinter *p)
{
	m_state = 0;
	if (p)
		updatePrinter(p);
	setSelectable(depth() == 2);
}

void KMListViewItem::updatePrinter(KMPrinter *p)
{
	bool	update(false);
	if (p)
	{
		int	oldstate = m_state;
		int	st(p->isValid() ? (int)KIcon::DefaultState : (int)KIcon::LockOverlay);
		m_state = ((p->isHardDefault() ? 0x1 : 0x0) | (p->ownSoftDefault() ? 0x2 : 0x0) | (p->isValid() ? 0x4 : 0x0));
		update = (oldstate != m_state);
		QString	name = (p->isVirtual() ? p->instanceName() : p->name());
		if (name != text(0))
			setText(0, name);
		setPixmap(0, SmallIcon(p->pixmap(), 0, st));
		m_isclass = p->isClass();
	}
	setDiscarded(false);
	if (update)
		repaint();
}

void KMListViewItem::paintCell(QPainter *p, const QColorGroup& cg, int c, int w, int a)
{
	if (m_state != 0)
	{
		QFont	f(p->font());
		if (m_state & 0x1) f.setBold(true);
		if (m_state & 0x2) f.setItalic(true);
		p->setFont(f);
	}
	QListViewItem::paintCell(p,cg,c,w,a);
}

//************************************************************************************************

KMListView::KMListView(QWidget *parent, const char *name)
: QListView(parent,name)
{
	m_items.setAutoDelete(false);

	addColumn("");
	header()->hide();
	setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	setLineWidth(1);
	setSorting(0);

	connect(this,SIGNAL(contextMenuRequested(QListViewItem*,const QPoint&,int)),SLOT(slotRightButtonClicked(QListViewItem*,const QPoint&,int)));
	connect(this,SIGNAL(selectionChanged()),SLOT(slotSelectionChanged()));
	connect(this,SIGNAL(onItem(QListViewItem*)),SLOT(slotOnItem(QListViewItem*)));
	connect(this,SIGNAL(onViewport()),SLOT(slotOnViewport()));

	m_root = new KMListViewItem(this,i18n("Print system"));
	m_root->setPixmap(0,SmallIcon("kdeprint_printer"));
	m_root->setOpen(true);
	m_classes = new KMListViewItem(m_root,i18n("Classes"));
	m_classes->setPixmap(0,SmallIcon("package"));
	m_classes->setOpen(true);
	m_printers = new KMListViewItem(m_root,i18n("Printers"));
	m_printers->setPixmap(0,SmallIcon("package"));
	m_printers->setOpen(true);
	m_specials = new KMListViewItem(m_root,i18n("Specials"));
	m_specials->setPixmap(0,SmallIcon("package"));
	m_specials->setOpen(true);

	sort();
}

KMListView::~KMListView()
{
}

void KMListView::slotRightButtonClicked(QListViewItem *item, const QPoint& p, int)
{
	emit rightButtonClicked(item && item->depth() == 2 ? item->text(0) : QString::null, p);
}

KMListViewItem* KMListView::findItem(KMPrinter *p)
{
	if (p)
	{
		QPtrListIterator<KMListViewItem>	it(m_items);
		bool	isVirtual(p->isVirtual()), isClass(p->isClass());
		for (;it.current();++it)
			if (isVirtual)
			{
				if (it.current()->depth() == 3 && it.current()->text(0) == p->instanceName()
						&& it.current()->parent()->text(0) == p->printerName())
					return it.current();
			}
			else
			{
				if (it.current()->isClass() == isClass && it.current()->text(0) == p->name())
					return it.current();
			}
	}
	return 0;
}

KMListViewItem* KMListView::findItem(const QString& prname)
{
	QPtrListIterator<KMListViewItem>	it(m_items);
	for (; it.current(); ++it)
		if (it.current()->depth() == 2 && it.current()->text(0) == prname)
			return it.current();
	return 0;
}

void KMListView::setPrinterList(QPtrList<KMPrinter> *list)
{
	bool 	changed(false);

	QPtrListIterator<KMListViewItem>	it(m_items);
	for (;it.current();++it)
		it.current()->setDiscarded(true);

	if (list)
	{
		QPtrListIterator<KMPrinter>	it(*list);
		KMListViewItem			*item (0);
		for (;it.current();++it)
		{
			item = findItem(it.current());
			if (!item)
			{
				if (it.current()->isVirtual())
				{
					KMListViewItem	*pItem = findItem(it.current()->printerName());
					if (!pItem)
						continue;
					item = new KMListViewItem(pItem, it.current());
					pItem->setOpen(true);
				}
				else
					item = new KMListViewItem((it.current()->isSpecial() ? m_specials : (it.current()->isClass(false) ? m_classes : m_printers)),it.current());
				m_items.append(item);
				changed = true;
			}
			else
				item->updatePrinter(it.current());
		}
	}

	QPtrList<KMListViewItem>	deleteList;
	deleteList.setAutoDelete(true);
	for (uint i=0; i<m_items.count(); i++)
		if (m_items.at(i)->isDiscarded())
		{
			// instance items are put in front of the list
			// so that they are destroyed first
			KMListViewItem	*item = m_items.take(i);
			if (item->depth() == 2)
				deleteList.append(item);
			else
				deleteList.prepend(item);
			i--;
			changed = true;
		}
	deleteList.clear();

	if (changed) sort();
	emit selectionChanged();
}

void KMListView::slotSelectionChanged()
{
	KMListViewItem	*item = static_cast<KMListViewItem*>(currentItem());
	emit printerSelected((item && !item->isDiscarded() && item->depth() == 2 ? item->text(0) : QString::null));
}

void KMListView::setPrinter(const QString& prname)
{
	QPtrListIterator<KMListViewItem>	it(m_items);
	for (;it.current();++it)
		if (it.current()->text(0) == prname)
		{
			setSelected(it.current(),true);
			break;
		}
}

void KMListView::setPrinter(KMPrinter *p)
{
	setPrinter(p ? p->name() : QString::null);
}

void KMListView::slotOnItem(QListViewItem *)
{
	setCursor(KCursor::handCursor());
}

void KMListView::slotOnViewport()
{
	setCursor(KCursor::arrowCursor());
}
#include "kmlistview.moc"
