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

#include "kpfilterpage.h"
#include "kprintfilter.h"
#include "kmfactory.h"
#include "kmfiltermanager.h"

#include <qpushbutton.h>
#include <qheader.h>
#include <qinputdialog.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <klistview.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>

KPFilterPage::KPFilterPage(QWidget *parent, const char *name)
: KPrintDialogPage(parent,name)
{
	setTitle(i18n("Filters"));
	m_activefilters.setAutoDelete(true);

	m_view = new KListView(this);
	m_view->addColumn("");
	m_view->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	m_view->setLineWidth(1);
	m_view->setSorting(-1);
	m_view->header()->hide();
	connect(m_view,SIGNAL(selectionChanged(QListViewItem*)),SLOT(slotItemSelected(QListViewItem*)));

	m_add = new QPushButton(QString::null, this);
	m_add->setPixmap(BarIcon("filter"));
	QToolTip::add(m_add, i18n("Add filter"));
	m_remove = new QPushButton(QString::null, this);
	m_remove->setPixmap(BarIcon("remove"));
	QToolTip::add(m_remove, i18n("Remove filter"));
	m_up = new QPushButton(QString::null, this);
	m_up->setPixmap(BarIcon("up"));
	QToolTip::add(m_up, i18n("Move filter up"));
	m_down = new QPushButton(QString::null, this);
	m_down->setPixmap(BarIcon("down"));
	QToolTip::add(m_down, i18n("Move filter down"));
	m_configure = new QPushButton(QString::null, this);
	m_configure->setPixmap(BarIcon("configure"));
	QToolTip::add(m_configure, i18n("Configure filter"));
	connect(m_add,SIGNAL(clicked()),SLOT(slotAddClicked()));
	connect(m_remove,SIGNAL(clicked()),SLOT(slotRemoveClicked()));
	connect(m_up,SIGNAL(clicked()),SLOT(slotUpClicked()));
	connect(m_down,SIGNAL(clicked()),SLOT(slotDownClicked()));
	connect(m_configure,SIGNAL(clicked()),SLOT(slotConfigureClicked()));
	connect(m_view,SIGNAL(doubleClicked(QListViewItem*)),SLOT(slotConfigureClicked()));

	QHBoxLayout	*l1 = new QHBoxLayout(this, 10, 10);
	QVBoxLayout	*l2 = new QVBoxLayout(0, 0, 0);
	l1->addWidget(m_view,1);
	l1->addLayout(l2,0);
	l2->addWidget(m_add);
	l2->addWidget(m_remove);
	l2->addSpacing(5);
	l2->addWidget(m_up);
	l2->addWidget(m_down);
	l2->addSpacing(5);
	l2->addWidget(m_configure);
	l2->addStretch(1);

	m_remove->setEnabled(false);
	m_up->setEnabled(false);
	m_down->setEnabled(false);
	m_configure->setEnabled(false);

	resize(100,50);
}

KPFilterPage::~KPFilterPage()
{
}

void KPFilterPage::slotAddClicked()
{
	if (m_filters.count() == 0)
	{
		m_filters = KMFactory::self()->filterManager()->filterList();
	}
	QStringList	l;
	for (int i=0;i<(int)m_filters.count();i+=2)
		if (m_activefilters.find(m_filters[i]) == 0)
			l.append(m_filters[i+1]);
	if (l.count() == 0)
	{
		KMessageBox::error(this,i18n("No more available filters."));
		return;
	}
	bool	ok;
	QString	choice = QInputDialog::getItem(i18n("Add filter"),i18n("Select the filter to add"),l,0,false,&ok,this);
	if (ok)
	{
		int	index = m_filters.findIndex(choice)-1;
		KPrintFilter	*filter = KMFactory::self()->filterManager()->filter(m_filters[index]);
		m_activefilters.insert(filter->idName(),filter);
		new QListViewItem(m_view,choice,filter->idName());
	}
}

void KPFilterPage::slotRemoveClicked()
{
	if (m_view->selectedItem())
	{
		QString	idname = m_view->selectedItem()->text(1);
		delete m_view->selectedItem();
		m_activefilters.remove(idname);
	}
}

void KPFilterPage::slotUpClicked()
{
	QListViewItem	*item = m_view->currentItem();
	if (item && item->itemAbove())
	{
		QListViewItem	*clone = new QListViewItem(m_view,item->itemAbove()->itemAbove(),item->text(0),item->text(1));
		delete item;
		m_view->setCurrentItem(clone);
	}
}

void KPFilterPage::slotDownClicked()
{
	QListViewItem	*item = m_view->currentItem();
	if (item && item->itemBelow())
	{
		QListViewItem	*clone = new QListViewItem(m_view,item->itemBelow(),item->text(0),item->text(1));
		delete item;
		m_view->setCurrentItem(clone);
	}
}

void KPFilterPage::slotConfigureClicked()
{
	KPrintFilter	*filter = currentFilter();
	if (!filter || !filter->configure(this))
		KMessageBox::error(this,i18n("Internal error: unable to load filter."));
}

void KPFilterPage::slotItemSelected(QListViewItem *item)
{
	m_remove->setEnabled((item != 0));
	m_up->setEnabled((item != 0 && item->itemAbove() != 0));
	m_down->setEnabled((item != 0 && item->itemBelow() != 0));
	m_configure->setEnabled((item != 0));
}

void KPFilterPage::setOptions(const QMap<QString,QString>& opts)
{
	QStringList	filters = QStringList::split(',',opts["_kde-filters"],false);
	// remove unneeded filters
	QDictIterator<KPrintFilter>	dit(m_activefilters);
	for (;dit.current();)
	{
		if (filters.find(dit.currentKey()) == filters.end())
			m_activefilters.remove(dit.currentKey());
		else
		{
			dit.current()->setOptions(opts);
			++dit;
		}
	}
	// add needed filters
	m_view->clear();
	QListViewItem	*item(0);
	for (QStringList::ConstIterator sit=filters.begin(); sit!=filters.end(); ++sit)
	{
		KPrintFilter	*f(0);
		if ((f=m_activefilters.find(*sit)) == 0)
		{
			f = KMFactory::self()->filterManager()->filter(*sit);
			if (f)
			{
				m_activefilters.insert(*sit,f);
				f->setOptions(opts);
			}
		}
		if (f)
			item = new QListViewItem(m_view,item,f->description(),f->idName());
	}
}

void KPFilterPage::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	QString	val;
	if (m_view->childCount() > 0)
	{
		QListViewItem	*item = m_view->firstChild();
		while (item)
		{
			KPrintFilter	*filter = m_activefilters.find(item->text(1));
			if (filter)
			{
				val.append(item->text(1)).append(",");
				filter->getOptions(opts,incldef);
			}
			item = item->nextSibling();
		}
	}
	if (!val.isEmpty() || incldef)
	{
		val.truncate(val.length()-1);
		opts["_kde-filters"] = val;
	}
}

KPrintFilter* KPFilterPage::currentFilter()
{
	KPrintFilter	*filter(0);
	if (m_view->currentItem())
		filter = m_activefilters.find(m_view->currentItem()->text(1));
	return filter;
}

#include "kpfilterpage.moc"
