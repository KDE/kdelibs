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

#include "kmwdriver.h"
#include "kmwizard.h"
#include "kmprinter.h"
#include "kmdriverdbwidget.h"

#include <qlayout.h>
#include <klocale.h>

KMWDriver::KMWDriver(QWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_ID = KMWizard::Driver;
	m_title = i18n("Printer model selection");
	m_nextpage = KMWizard::DriverTest;

	m_widget = new KMDriverDbWidget(this);

	QVBoxLayout	*lay1 = new QVBoxLayout(this, 0, 0);
	lay1->addWidget(m_widget);
}

void KMWDriver::initPrinter(KMPrinter *p)
{
	m_widget->init();
	if (p)
		m_widget->setDriver(p->manufacturer(),p->model());
}

void KMWDriver::updatePrinter(KMPrinter *p)
{
	if (p)
	{
		p->setManufacturer(QString::null);
		p->setModel(QString::null);
		p->setDbEntry(0);
		p->setDriverInfo(QString::null);
		p->setOption("kde-driver",QString::null);
		setNextPage(KMWizard::DriverTest);
		if (m_widget->isRaw())
		{
			p->setDriverInfo(i18n("Raw printer"));
			p->setOption("kde-driver","raw");
		}
		else
		{
			p->setManufacturer(m_widget->manufacturer());
			p->setModel(m_widget->model());
			if (m_widget->isExternal())
			{
				p->setDriverInfo(m_widget->description());
				p->setOption("kde-driver",m_widget->driverFile());
			}
			else
			{
				KMDBEntryList	*drvs = m_widget->drivers();
				if (drvs->count() == 1)
				{
					p->setDbEntry(drvs->getFirst());
					p->setDriverInfo(drvs->getFirst()->description);
				}
				else
					setNextPage(KMWizard::DriverSelect);
			}
		}
	}
}

bool KMWDriver::isValid(QString& msg)
{
	if (m_widget->isRaw() || m_widget->isExternal() || m_widget->drivers())
		return true;
	else
	{
		msg = i18n("Internal error: unable to locate the driver!");
		return false;
	}
}
