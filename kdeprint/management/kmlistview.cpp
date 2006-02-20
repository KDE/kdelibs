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

#include "kmlistview.h"
#include "kmprinter.h"
#include "kmobject.h"

#include <q3header.h>
#include <qpainter.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kcursor.h>

class KMListViewItem : public Q3ListViewItem, public KMObject
{
public:
	KMListViewItem(Q3ListView *parent, const QString& txt);
	KMListViewItem(Q3ListViewItem *parent, const QString& txt);
	KMListViewItem(Q3ListViewItem *parent, KMPrinter *p);

	virtual void paintCell(QPainter*, const QColorGroup&, int, int, int);
	void updatePrinter(KMPrinter *p);
	bool isClass() const	{ return m_isclass; }

protected:
	void init(KMPrinter *p = 0);

private:
	int	m_state;
	bool	m_isclass;
};

KMListViewItem::KMListViewItem(Q3ListView *parent, const QString& txt)
: Q3ListViewItem(parent,txt)
{
	init();
}

KMListViewItem::KMListViewItem(Q3ListViewItem *parent, const QString& txt)
: Q3ListViewItem(parent,txt)
{
	init();
}

KMListViewItem::KMListViewItem(Q3ListViewItem *parent, KMPrinter *p)
: Q3ListViewItem(parent)
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
	Q3ListViewItem::paintCell(p,cg,c,w,a);
}

//************************************************************************************************

KMListView::KMListView(QWidget *parent, const char *name)
: Q3ListView(parent,name)
{
	addColumn("");
	header()->hide();
	setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	setLineWidth(1);
	setSorting(0);

	connect(this,SIGNAL(contextMenuRequested(Q3ListViewItem*,const QPoint&,int)),SLOT(slotRightButtonClicked(Q3ListViewItem*,const QPoint&,int)));
	connect(this,SIGNAL(selectionChanged()),SLOT(slotSelectionChanged()));
	connect(this,SIGNAL(onItem(Q3ListViewItem*)),SLOT(slotOnItem(Q3ListViewItem*)));
	connect(this,SIGNAL(onViewport()),SLOT(slotOnViewport()));

	m_root = new KMListViewItem(this,i18n("Print System"));
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

void KMListView::slotRightButtonClicked(Q3ListViewItem *item, const QPoint& p, int)
{
	emit rightButtonClicked(item && item->depth() == 2 ? item->text(0) : QString(), p);
}

KMListViewItem* KMListView::findItem(KMPrinter *p)
{
	if (p)
	{
		QListIterator<KMListViewItem*>	it(m_items);
		bool	isVirtual(p->isVirtual()), isClass(p->isClass());
		while (it.hasNext()) {
      KMListViewItem *item(it.next());
			if (isVirtual)
			{
				if (item->depth() == 3 && item->text(0) == p->instanceName()
						&& item->parent()->text(0) == p->printerName())
					return item;
			}
			else
			{
				if (item->isClass() == isClass && item->text(0) == p->name())
					return item;
			}
    }
	}
	return 0;
}

KMListViewItem* KMListView::findItem(const QString& prname)
{
	QListIterator<KMListViewItem*>	it(m_items);
	while (it.hasNext()) {
    KMListViewItem *item(it.next());
		if (item->depth() == 2 && item->text(0) == prname)
			return item;
  }
	return 0;
}

void KMListView::setPrinterList(QList<KMPrinter*> *list)
{
	bool 	changed(false);

	QListIterator<KMListViewItem*>	it(m_items);
	while (it.hasNext())
		it.next()->setDiscarded(true);

	if (list)
	{
		QListIterator<KMPrinter*>	it(*list);
		KMListViewItem			*item (0);
		while (it.hasNext())
		{
      KMPrinter *printer(it.next());
			item = findItem(printer);
			if (!item)
			{
				if (printer->isVirtual())
				{
					KMListViewItem	*pItem = findItem(printer->printerName());
					if (!pItem)
						continue;
					item = new KMListViewItem(pItem, printer);
					pItem->setOpen(true);
				}
				else
					item = new KMListViewItem((printer->isSpecial() ? m_specials : (printer->isClass(false) ? m_classes : m_printers)),printer);
				m_items.append(item);
				changed = true;
			}
			else
				item->updatePrinter(printer);
		}
	}

	QList<KMListViewItem*>	deleteList;
	for (int i=0; i<m_items.count(); i++)
		if (m_items.at(i)->isDiscarded())
		{
			// instance items are put in front of the list
			// so that they are destroyed first
			KMListViewItem	*item = m_items.takeAt(i);
			if (item->depth() == 2)
				deleteList.append(item);
			else
				deleteList.prepend(item);
			i--;
			changed = true;
		}
	qDeleteAll(deleteList);
  deleteList.clear();

	if (changed) sort();
	emit selectionChanged();
}

void KMListView::slotSelectionChanged()
{
	KMListViewItem	*item = static_cast<KMListViewItem*>(currentItem());
	emit printerSelected((item && !item->isDiscarded() && item->depth() == 2 ? item->text(0) : QString()));
}

void KMListView::setPrinter(const QString& prname)
{
	QListIterator<KMListViewItem*>	it(m_items);
	while (it.hasNext()) {
    KMListViewItem *item(it.next());
		if (item->text(0) == prname)
		{
			setSelected(item,true);
			break;
		}
  }
}

void KMListView::setPrinter(KMPrinter *p)
{
	setPrinter(p ? p->name() : QString());
}

void KMListView::slotOnItem(Q3ListViewItem *)
{
	setCursor(KCursor::handCursor());
}

void KMListView::slotOnViewport()
{
	setCursor(KCursor::arrowCursor());
}
#include "kmlistview.moc"
