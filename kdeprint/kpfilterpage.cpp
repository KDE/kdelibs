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
#include "kxmlcommand.h"

#include <qpushbutton.h>
#include <qheader.h>
#include <qinputdialog.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qtextview.h>
#include <klistview.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kdebug.h>

KPFilterPage::KPFilterPage(QWidget *parent, const char *name)
: KPrintDialogPage(parent,name)
{
	setTitle(i18n("Filters"));
	m_activefilters.setAutoDelete(true);
	m_valid = true;

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

	m_info = new QTextView(this);
	m_info->setPaper(colorGroup().background());

	QGridLayout	*l1 = new QGridLayout(this, 2, 2, 0, 10);
	l1->setColStretch(0, 1);
	QVBoxLayout	*l2 = new QVBoxLayout(0, 0, 0);
	l1->addWidget(m_view, 0, 0);
	l1->addLayout(l2, 0, 1);
	l2->addWidget(m_add);
	l2->addWidget(m_remove);
	l2->addSpacing(5);
	l2->addWidget(m_up);
	l2->addWidget(m_down);
	l2->addSpacing(5);
	l2->addWidget(m_configure);
	l2->addStretch(1);
	l1->addMultiCellWidget(m_info, 1, 1, 0, 1);
	slotItemSelected(0);

	resize(100,50);
}

KPFilterPage::~KPFilterPage()
{
}

void KPFilterPage::updateButton()
{
/*  QListViewItem	*item = m_view->currentItem();
  bool state=(item!=0);
  m_remove->setEnabled(state);
  m_up->setEnabled((state && item->itemAbove() != 0));
  m_down->setEnabled((state && item->itemBelow() != 0));
  m_configure->setEnabled(state);*/
}

void KPFilterPage::slotAddClicked()
{
	if (m_filters.count() == 0)
	{
		m_filters = KXmlCommandManager::self()->commandListWithDescription();
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
	QString	choice = QInputDialog::getItem(i18n("Add Filter"),i18n("Select the filter to add:"),l,0,false,&ok,this);
	if (ok)
	{
		int		index = m_filters.findIndex(choice)-1;
		KXmlCommand	*cmd = KXmlCommandManager::self()->loadCommand(m_filters[index]);
		QStringList	filters = activeList();
		int		pos = KXmlCommandManager::self()->insertCommand(filters, cmd->name());
		QListViewItem	*prev(0);
		if (pos > 0)
		{
			prev = m_view->firstChild();
			for (int i=1;prev && i<pos;i++)
				prev = prev->nextSibling();
		}
		m_activefilters.insert(cmd->name(), cmd);
		QListViewItem	*item = new QListViewItem(m_view, prev, choice, cmd->name());
		item->setPixmap(0, SmallIcon("filter"));
		checkFilterChain();
	}
}

void KPFilterPage::slotRemoveClicked()
{
	if (m_view->selectedItem())
	{
		QString	idname = m_view->selectedItem()->text(1);
		delete m_view->selectedItem();
		m_activefilters.remove(idname);
		checkFilterChain();
		if (m_view->currentItem())
			m_view->setSelected(m_view->currentItem(), true);
		slotItemSelected(m_view->currentItem());
	}
}

void KPFilterPage::slotUpClicked()
{
	QListViewItem	*item = m_view->selectedItem();
	if (item && item->itemAbove())
	{
		QListViewItem	*clone = new QListViewItem(m_view,item->itemAbove()->itemAbove(),item->text(0),item->text(1));
		clone->setPixmap(0, SmallIcon("filter"));
		delete item;
		m_view->setSelected(clone, true);
		checkFilterChain();
	}
}

void KPFilterPage::slotDownClicked()
{
	QListViewItem	*item = m_view->selectedItem();
	if (item && item->itemBelow())
	{
		QListViewItem	*clone = new QListViewItem(m_view,item->itemBelow(),item->text(0),item->text(1));
		clone->setPixmap(0, SmallIcon("filter"));
		delete item;
		m_view->setSelected(clone, true);
		checkFilterChain();
	}
}

void KPFilterPage::slotConfigureClicked()
{
	KXmlCommand	*filter = currentFilter();
	if (!filter || !KXmlCommandManager::self()->configure(filter, this))
		KMessageBox::error(this,i18n("Internal error: unable to load filter."));
}

void KPFilterPage::slotItemSelected(QListViewItem *item)
{
	m_remove->setEnabled((item != 0));
	m_up->setEnabled((item != 0 && item->itemAbove() != 0));
	m_down->setEnabled((item != 0 && item->itemBelow() != 0));
	m_configure->setEnabled((item != 0));
	updateInfo();
}

void KPFilterPage::setOptions(const QMap<QString,QString>& opts)
{
	QStringList	filters = QStringList::split(',',opts["_kde-filters"],false);
	// remove unneeded filters
	QDictIterator<KXmlCommand>	dit(m_activefilters);
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
		KXmlCommand	*f(0);
		if ((f=m_activefilters.find(*sit)) == 0)
		{
			f = KXmlCommandManager::self()->loadCommand(*sit);
			if (f)
			{
				m_activefilters.insert(*sit,f);
				f->setOptions(opts);
			}
		}
		if (f)
			item = new QListViewItem(m_view,item,f->description(),f->name());
	}
	checkFilterChain();
}

void KPFilterPage::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	QStringList	filters = activeList();
	for (QStringList::ConstIterator it=filters.begin(); it!=filters.end(); ++it)
	{
		KXmlCommand	*f = m_activefilters.find(*it);
		if (f)
			f->getOptions(opts, incldef);
	}
	if (filters.count() > 0 || incldef)
	{
		opts["_kde-filters"] = filters.join(",");
	}
}

QStringList KPFilterPage::activeList()
{
	QStringList	list;
	QListViewItem	*item = m_view->firstChild();
	while (item)
	{
		list.append(item->text(1));
		item = item->nextSibling();
	}
	return list;
}

KXmlCommand* KPFilterPage::currentFilter()
{
	KXmlCommand	*filter(0);
	if (m_view->selectedItem())
		filter = m_activefilters.find(m_view->selectedItem()->text(1));
	return filter;
}

void KPFilterPage::checkFilterChain()
{
	QListViewItem	*item = m_view->firstChild();
	bool		ok(true);
	m_valid = true;
	while (item)
	{
		item->setPixmap(0, (ok ? SmallIcon("filter") : SmallIcon("filterstop")));
		KXmlCommand	*f1 = m_activefilters.find(item->text(1));
		if (f1 && item->nextSibling())
		{
			KXmlCommand	*f2 = m_activefilters.find(item->nextSibling()->text(1));
			if (f2)
			{
				if (!f2->acceptMimeType(f1->mimeType()))
				{
					item->setPixmap(0, SmallIcon("filterstop"));
					ok = false;
					m_valid = false;
				}
				else
					ok = true;
			}
		}
		item = item->nextSibling();
	}
}

bool KPFilterPage::isValid(QString& msg)
{
	if (!m_valid)
	{
		msg = i18n("<p>The filter chain is wrong. The output format of at least one filter is not supported by its follower. See <b>Filters</b> tab for more information.</p>");
	}
	return m_valid;
}

void KPFilterPage::updateInfo()
{
	QString	txt;
	KXmlCommand	*f = currentFilter();
	if (f)
	{
		QString	templ("<b>%1:</b> %2<br>");
		txt.append(templ.arg(i18n("Name")).arg(f->name()));
		txt.append(templ.arg(i18n("Requirements")).arg(f->requirements().join(", ")));
		txt.append(templ.arg(i18n("Input")).arg(f->inputMimeTypes().join(", ")));
		txt.append(templ.arg(i18n("Output")).arg(f->mimeType()));
	}
	m_info->setText(txt);
}

#include "kpfilterpage.moc"
