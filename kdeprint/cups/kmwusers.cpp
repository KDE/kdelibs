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

#include <config.h>

#include "kmwusers.h"
#include "kmwizard.h"
#include "kmprinter.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <klocale.h>
#include <keditlistbox.h>

KMWUsers::KMWUsers(QWidget *parent, const char *name)
: KMWizardPage(parent, name)
{
	m_ID = KMWizard::Custom+4;
	m_title = i18n("Users Access Settings");
	m_nextpage = KMWizard::Name;

	m_users = new KEditListBox(i18n("Users"), this, 0, false, KEditListBox::Add|KEditListBox::Remove);
	m_type = new QComboBox(this);
	m_type->insertItem(i18n("Allowed Users"));
	m_type->insertItem(i18n("Denied Users"));

	QLabel	*lab1 = new QLabel(i18n("Define here a group of allowed/denied users for this printer."), this);
	QLabel	*lab2 = new QLabel(i18n("&Type:"), this);

	lab2->setBuddy(m_type);

	QVBoxLayout	*l0 = new QVBoxLayout(this, 0, 10);
	QHBoxLayout	*l1 = new QHBoxLayout(0, 0, 10);
	l0->addWidget(lab1, 0);
	l0->addLayout(l1, 0);
	l1->addWidget(lab2, 0);
	l1->addWidget(m_type, 1);
	l0->addWidget(m_users, 1);
}

KMWUsers::~KMWUsers()
{
}

void KMWUsers::initPrinter(KMPrinter *p)
{
	QStringList	l;
	int		i(1);
	if (!p->option("requesting-user-name-denied").isEmpty())
	{
		l = QStringList::split(",", p->option("requesting-user-name-denied"), false);
		if (l.count() == 1 && l[0] == "none")
			l.clear();
	}
	else if (!p->option("requesting-user-name-allowed").isEmpty())
	{
		i = 0;
		l = QStringList::split(",", p->option("requesting-user-name-allowed"), false);
		if (l.count() && l[0] == "all")
			l.clear();
	}
	m_users->insertStringList(l);
	m_type->setCurrentItem(i);
}

void KMWUsers::updatePrinter(KMPrinter *p)
{
	p->removeOption("requesting-user-name-denied");
	p->removeOption("requesting-user-name-allowed");

	QString	str;
	if (m_users->count() > 0)
		str = m_users->items().join(",");
	else
		str = (m_type->currentItem() == 0 ? "all" : "none");
	QString	optname = (m_type->currentItem() == 0 ? "requesting-user-name-allowed" : "requesting-user-name-denied");
	p->setOption(optname, str);
}
#include "kmwusers.moc"
