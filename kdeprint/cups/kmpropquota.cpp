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

#include "kmpropquota.h"
#include "kmprinter.h"
#include "kmwizard.h"

#include <qlabel.h>
#include <qlayout.h>
#include <klocale.h>

// some forward declarations (see kmwquota.cpp)
const char* unitKeyword(int);
int findUnit(int&);

KMPropQuota::KMPropQuota(QWidget *parent, const char *name)
: KMPropWidget(parent,name)
{
	m_period = new QLabel(this);
	m_sizelimit = new QLabel(this);
	m_pagelimit = new QLabel(this);

	QLabel	*l1 = new QLabel(i18n("&Period:"), this);
	QLabel	*l2 = new QLabel(i18n("&Size limit (KB):"), this);
	QLabel	*l3 = new QLabel(i18n("&Page limit:"), this);

	l1->setBuddy(m_period);
	l2->setBuddy(m_sizelimit);
	l3->setBuddy(m_pagelimit);

	QGridLayout	*main_ = new QGridLayout(this, 4, 2, 10, 10);
	main_->setColStretch(1,1);
	main_->setRowStretch(3,1);
	main_->addWidget(l1,0,0);
	main_->addWidget(l2,1,0);
	main_->addWidget(l3,2,0);
	main_->addWidget(m_period,0,1);
	main_->addWidget(m_sizelimit,1,1);
	main_->addWidget(m_pagelimit,2,1);

	m_title = i18n("Quotas");
	m_header = i18n("Quotas settings");
	m_pixmap = "lock";
}

KMPropQuota::~KMPropQuota()
{
}

void KMPropQuota::setPrinter(KMPrinter *p)
{
	if (p && p->isPrinter())
	{
		int	qu(0), si(0), pa(0), un(0);
		qu = p->option("job-quota-period").toInt();
		si = p->option("job-k-limit").toInt();
		pa = p->option("job-page-limit").toInt();
		if (si == 0 && pa == 0)
			qu = -1;
		if (qu > 0)
			un = findUnit(qu);
		m_period->setText(qu == -1 ? i18n("No quota") : QString::number(qu).append(" ").append(i18n(unitKeyword(un))));
		m_sizelimit->setText(si ? QString::number(si) : i18n("None"));
		m_pagelimit->setText(pa ? QString::number(pa) : i18n("None"));
		emit enable(true);
		emit enableChange(p->isLocal());
	}
	else
	{
		emit enable(false);
		m_period->setText("");
		m_sizelimit->setText("");
		m_pagelimit->setText("");
	}
}

void KMPropQuota::configureWizard(KMWizard *w)
{
	w->configure(KMWizard::Custom+3,KMWizard::Custom+3,true);
}
