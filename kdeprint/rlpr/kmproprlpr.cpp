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

#include "kmproprlpr.h"
#include "kmprinter.h"
#include "kmwizard.h"

#include <qlabel.h>
#include <qlayout.h>
#include <klocale.h>

KMPropRlpr::KMPropRlpr(QWidget *parent, const char *name)
: KMPropWidget(parent,name)
{
	m_host = new QLabel("",this);
	m_queue = new QLabel("",this);

	QLabel	*l1 = new QLabel(i18n("Host:"), this);
	QLabel	*l2 = new QLabel(i18n("Queue:"), this);

	// layout
	QGridLayout	*main_ = new QGridLayout(this, 3, 2, 10, 7);
	main_->setColStretch(0,0);
	main_->setColStretch(1,1);
	main_->setRowStretch(2,1);
	main_->addWidget(l1,0,0);
	main_->addWidget(l2,1,0);
	main_->addWidget(m_host,0,1);
	main_->addWidget(m_queue,1,1);

	m_pixmap = "connect_established";
	m_title = i18n("Queue");
	m_header = i18n("Remote LPD Queue Settings");
}

KMPropRlpr::~KMPropRlpr()
{
}

void KMPropRlpr::setPrinter(KMPrinter *p)
{
	if (p && !p->isSpecial())
	{
		m_host->setText(p->option("host"));
		m_queue->setText(p->option("queue"));
		emit enable(true);
	}
	else
	{
		emit enable(false);
		m_host->setText("");
		m_queue->setText("");
	}
}

void KMPropRlpr::configureWizard(KMWizard *w)
{
	w->configure(KMWizard::Custom+1,KMWizard::Custom+1,true);
}
