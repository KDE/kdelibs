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
	KMPrinter* printer() const 	{ return m_printer; }
	void updatePrinter(KMPrinter *p);

private:
	KMPrinter	*m_printer;
	int		m_state;
};

KMListViewItem::KMListViewItem(QListView *parent, const QString& txt)
: QListViewItem(parent,txt), m_printer(0), m_state(0)
{
}

KMListViewItem::KMListViewItem(QListViewItem *parent, const QString& txt)
: QListViewItem(parent,txt), m_printer(0), m_state(0)
{
}

KMListViewItem::KMListViewItem(QListViewItem *parent, KMPrinter *p)
: QListViewItem(parent)
{
	m_state = 0;
	updatePrinter(p);
}

void KMListViewItem::updatePrinter(KMPrinter *p)
{
	m_printer = p;
	bool	update(false);
	if (m_printer)
	{
		int	oldstate = m_state;
		int	st(m_printer->isValid() ? (int)KIcon::DefaultState : (int)KIcon::LockOverlay);
		m_state = ((p->isHardDefault() ? 0x1 : 0x0) | (p->ownSoftDefault() ? 0x2 : 0x0) | (p->isValid() ? 0x4 : 0x0));
		update = (oldstate != m_state);
		if (m_printer->name() != text(0))
			setText(0,m_printer->name());
		setPixmap(0,SmallIcon(m_printer->pixmap(),0,st));
	}
	setDiscarded(false);
	if (update) repaint();
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

	connect(this,SIGNAL(rightButtonClicked(QListViewItem*,const QPoint&,int)),SLOT(slotRightButtonClicked(QListViewItem*,const QPoint&,int)));
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

void KMListView::resizeEvent(QResizeEvent *e)
{
	QListView::resizeEvent(e);
	int	w = width()-2*frameWidth();
	if (verticalScrollBar()->isVisible())
		w -= verticalScrollBar()->sizeHint().width();
	setColumnWidth(0,w);
}

void KMListView::slotRightButtonClicked(QListViewItem *item, const QPoint& p, int)
{
	KMListViewItem	*citem = dynamic_cast<KMListViewItem*>(item);
	emit rightButtonClicked((citem ? citem->printer() : NULL),p);
}

KMListViewItem* KMListView::findItem(KMPrinter *p)
{
	if (p)
	{
		QPtrListIterator<KMListViewItem>	it(m_items);
		for (;it.current();++it)
			if (it.current()->printer() && it.current()->printer()->name() == p->name()
			    && it.current()->printer()->isClass() == p->isClass())
				return it.current();
	}
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
                        if (!it.current()->instanceName().isEmpty())
                                continue;
			item = findItem(it.current());
			if (!item)
			{
				item = new KMListViewItem((it.current()->isSpecial() ? m_specials : (it.current()->isClass(false) ? m_classes : m_printers)),it.current());
				m_items.append(item);
				changed = true;
			}
			else
				item->updatePrinter(it.current());
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

void KMListView::slotSelectionChanged()
{
	KMListViewItem	*item = (KMListViewItem*)currentItem();
	emit printerSelected((item && !item->isDiscarded() ? item->printer() : NULL));
}

void KMListView::setPrinter(KMPrinter *p)
{
	QPtrListIterator<KMListViewItem>	it(m_items);
	for (;it.current();++it)
		if (it.current()->printer() == p)
		{
			setSelected(it.current(),true);
			break;
		}
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
