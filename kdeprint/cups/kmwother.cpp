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

#include "kmwother.h"
#include "kmwizard.h"
#include "kmprinter.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>

#include <klocale.h>

KMWOther::KMWOther(QWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_ID = KMWizard::Custom+5;
	m_title = i18n("URI Selection");
	m_nextpage = KMWizard::Driver;

	m_uri = new QLineEdit(this);
	QLabel	*l1 = new QLabel(this);
	l1->setText(i18n("<p>Enter the URI corresponding to the printer to be installed. "
	                 "Examples:</p><ul>"
					 "<li>smb://[login[:passwd]@]server/printer</li>"
					 "<li>lpd://server/queue</li>"
					 "<li>parallel:/dev/lp0</li></ul>"));
	QLabel	*l2 = new QLabel(i18n("URI:"), this);

	QVBoxLayout	*lay1 = new QVBoxLayout(this, 0, 15);
	QVBoxLayout	*lay2 = new QVBoxLayout(0, 0, 5);
	lay1->addWidget(l1);
	lay1->addLayout(lay2);
	lay1->addStretch(1);
	lay2->addWidget(l2);
	lay2->addWidget(m_uri);
}

void KMWOther::initPrinter(KMPrinter *p)
{
	m_uri->setText(p->device().url());
}

void KMWOther::updatePrinter(KMPrinter *p)
{
	QString	dev = m_uri->text();
	p->setDevice(KURL(dev));
}
