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

#include "kmconfigfilter.h"
#include "kmmanager.h"
#include "kmfactory.h"

#include <klistbox.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <klocale.h>
#include <kconfig.h>
#include <kiconloader.h>

KMConfigFilter::KMConfigFilter(QWidget *parent, const char *name)
: KMConfigPage(parent, name)
{
	setPageName(i18n("Filter"));
	setPageHeader(i18n("Printer Filtering Settings"));
	setPagePixmap("filter");

	QGroupBox	*box = new QGroupBox(0, Qt::Vertical, i18n("Printer Filter"), this);

	m_list1 = new KListBox(box);
	m_list1->setSelectionMode(KListBox::Extended);
	m_list2 = new KListBox(box);
	m_list2->setSelectionMode(KListBox::Extended);
	m_add = new QPushButton(box);
	m_add->setPixmap(SmallIcon("forward"));
	m_remove = new QPushButton(box);
	m_remove->setPixmap(SmallIcon("back"));
	m_locationre = new QLineEdit(box);
	QLabel	*lab = new QLabel(box);
	lab->setText(i18n("The printer filtering allows you to view only a specific set of "
	                  "printers instead of all of them. This may be useful when there are a "
			  "lot of printers available but you only use a few ones. Select the "
			  "printers you want to see from the list on the left or enter a <b>Location</b> "
			  "filter (ex: Group_1*). Both are cumulative and ignored if empty."));
	lab->setTextFormat(Qt::RichText);
	QLabel	*lab1 = new QLabel(i18n("Location filter:"), box);

	QVBoxLayout	*l0 = new QVBoxLayout(this, 5, 10);
	l0->addWidget(box, 1);
	QVBoxLayout	*l1 = new QVBoxLayout(box->layout(), 10);
	l1->addWidget(lab);
	QGridLayout	*l2 = new QGridLayout(0, 4, 3, 0, 10);
	l1->addLayout(l2);
	l2->setRowStretch(0, 1);
	l2->setRowStretch(3, 1);
	l2->setColStretch(0, 1);
	l2->setColStretch(2, 1);
	l2->addMultiCellWidget(m_list1, 0, 3, 0, 0);
	l2->addMultiCellWidget(m_list2, 0, 3, 2, 2);
	l2->addWidget(m_add, 1, 1);
	l2->addWidget(m_remove, 2, 1);
	QHBoxLayout	*l3 = new QHBoxLayout(0, 0, 10);
	l1->addLayout(l3, 0);
	l3->addWidget(lab1, 0);
	l3->addWidget(m_locationre, 1);

	connect(m_add, SIGNAL(clicked()), SLOT(slotAddClicked()));
	connect(m_remove, SIGNAL(clicked()), SLOT(slotRemoveClicked()));
	connect(m_list1, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));
	connect(m_list2, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));
	m_add->setEnabled(false);
	m_remove->setEnabled(false);
}

void KMConfigFilter::loadConfig(KConfig *conf)
{
	conf->setGroup("Filter");
	QStringList	m_plist = conf->readListEntry("Printers");
	QPtrListIterator<KMPrinter>	it(*(KMManager::self()->printerListComplete(false)));
	for (; it.current(); ++it)
	{
		if (!it.current()->isSpecial() && !it.current()->isVirtual())
		{
			KListBox	*lb = (m_plist.find(it.current()->printerName()) == m_plist.end() ? m_list1 : m_list2);
			lb->insertItem(SmallIcon(it.current()->pixmap()), it.current()->printerName());
		}
	}
	m_list1->sort();
	m_list2->sort();
	m_locationre->setText(conf->readEntry("LocationRe"));
}

void KMConfigFilter::saveConfig(KConfig *conf)
{
	conf->setGroup("Filter");
	QStringList	plist;
	for (uint i=0; i<m_list2->count(); i++)
		plist << m_list2->text(i);
	conf->writeEntry("Printers", plist);
	conf->writeEntry("LocationRe", m_locationre->text());
}

void KMConfigFilter::transfer(KListBox *from, KListBox *to)
{
	for (uint i=0; i<from->count();)
	{
		if (from->isSelected(i))
		{
			to->insertItem(*(from->pixmap(i)), from->text(i));
			from->removeItem(i);
		}
		else
			i++;
	}
	to->sort();
}

void KMConfigFilter::slotAddClicked()
{
	transfer(m_list1, m_list2);
}

void KMConfigFilter::slotRemoveClicked()
{
	transfer(m_list2, m_list1);
}

void KMConfigFilter::slotSelectionChanged()
{
	const KListBox	*lb = static_cast<const KListBox*>(sender());
	if (!lb)
		return;
	QPushButton	*pb = (lb == m_list1 ? m_add : m_remove);
	for (uint i=0; i<lb->count(); i++)
		if (lb->isSelected(i))
		{
			pb->setEnabled(true);
			return;
		}
	pb->setEnabled(false);
}

#include "kmconfigfilter.moc"
