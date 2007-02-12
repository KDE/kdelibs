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

#include "kmwclass.h"
#include "kmwizard.h"
#include "kmfactory.h"
#include "kmmanager.h"
#include "kmprinter.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <klistwidget.h>
#include <klocale.h>
#include <kiconloader.h>

KMWClass::KMWClass(QWidget *parent)
    : KMWizardPage(parent)
{
	m_ID = KMWizard::Class;
	m_title = i18n("Class Composition");
	m_nextpage = KMWizard::Name;

	m_list1 = new KListWidget(this);
	m_list1->setSelectionMode(QListWidget::ExtendedSelection);
	m_list2 = new KListWidget(this);
	m_list2->setSelectionMode(QListWidget::ExtendedSelection);

	QToolButton	*add = new QToolButton(this);
	QToolButton	*remove = new QToolButton(this);
	add->setIcon(BarIcon("forward"));
	remove->setIcon(BarIcon("back"));
	connect(add,SIGNAL(clicked()),SLOT(slotAdd()));
	connect(remove,SIGNAL(clicked()),SLOT(slotRemove()));

	QLabel	*l1 = new QLabel(i18n("Available printers:"), this);
	QLabel	*l2 = new QLabel(i18n("Class printers:"), this);

        QHBoxLayout	*lay1 = new QHBoxLayout(this);
        lay1->setMargin(0);
        lay1->setSpacing(15);
        QVBoxLayout	*lay2 = new QVBoxLayout();
        QVBoxLayout	*lay3 = new QVBoxLayout();
        QVBoxLayout *lay4 = new QVBoxLayout();
        lay1->addLayout(lay3, 1);
        lay1->addLayout(lay2, 0);
	lay1->addLayout(lay4, 1);
        lay3->setMargin(0);
        lay3->setSpacing(0);
        lay3->addWidget(l1, 0);
        lay3->addWidget(m_list1, 1);
        lay2->setMargin(0);
        lay2->setSpacing(20);
        lay2->addStretch(1);
        lay2->addWidget(add, 0);
        lay2->addWidget(remove, 0);
        lay2->addStretch(1);
        lay4->setMargin(0);
        lay4->setSpacing(0);
        lay4->addWidget(l2, 0);
        lay4->addWidget(m_list2, 1);
}

KMWClass::~KMWClass()
{
}

bool KMWClass::isValid(QString& msg)
{
	if (m_list2->count() == 0)
	{
		msg = i18n("You must select at least one printer.");
		return false;
	}
	return true;
}

void KMWClass::initPrinter(KMPrinter *p)
{
	QStringList	members = p->members();
	KMManager	*mgr = KMFactory::self()->manager();

	// first load available printers
	QList<KMPrinter*> list = mgr->printerList(false);
	m_list1->clear();
	QListIterator<KMPrinter*>	it(list);
	while (it.hasNext()) {
		KMPrinter *printer(it.next());
		if (printer->instanceName().isEmpty() && !printer->isClass(true) &&
			!printer->isSpecial() && !members.contains(printer->name()))
        {
            QListWidgetItem* item = new QListWidgetItem();
            item->setIcon(SmallIcon(printer->pixmap()));
            item->setText(printer->name());
			m_list1->addItem(item);
        }
	}
	m_list1->model()->sort(0);

	// set class printers
	m_list2->clear();
	for (QStringList::ConstIterator it=members.begin(); it!=members.end(); ++it)
	{
		KMPrinter	*pr = mgr->findPrinter(*it);
		if (pr) 
        {
            QListWidgetItem* item = new QListWidgetItem();
            item->setIcon(SmallIcon(pr->pixmap()));
            item->setText(*it);
            m_list2->addItem(item);
        }
	}
	m_list2->model()->sort(0);
}

void KMWClass::updatePrinter(KMPrinter *p)
{
	QStringList	members;
	for (int i=0; i<m_list2->count(); i++)
		members.append(m_list2->item(i)->text());
	p->setMembers(members);
}

void KMWClass::slotAdd()
{
	for (int i=0;i<m_list1->count();i++)
		if (m_list1->item(i)->isSelected())
		{
            m_list2->addItem(m_list1->item(i)->clone());
			QListWidgetItem* item = m_list1->takeItem(i--);
            delete item;
		}
	m_list2->model()->sort(0);
}

void KMWClass::slotRemove()
{
	for (int i=0;i<m_list2->count();i++)
		if (m_list2->item(i)->isSelected())
		{
            m_list1->addItem(m_list2->item(i)->clone());
			QListWidgetItem* item = m_list2->takeItem(i--);
            delete item;
		}
	m_list1->model()->sort(0);
}
#include "kmwclass.moc"
