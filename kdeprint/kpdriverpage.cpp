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

#include "kpdriverpage.h"
#include "driverview.h"
#include "driver.h"

#include <qlayout.h>
#include <klocale.h>

KPDriverPage::KPDriverPage(KMPrinter *p, DrMain *d, QWidget *parent, const char *name)
: KPrintDialogPage(p,d,parent,name)
{
	setTitle(i18n("Advanced"));

	m_view = new DriverView(this);
	m_view->setAllowFixed(false);
	if (driver()) m_view->setDriver(driver());

	QVBoxLayout	*lay1 = new QVBoxLayout(this, 10, 0);
	lay1->addWidget(m_view);
}

KPDriverPage::~KPDriverPage()
{
}

bool KPDriverPage::isValid(QString& msg)
{
	if (m_view->hasConflict())
	{
		msg = i18n("<nobr>Some options selected are in conflict. You must resolve those conflicts<br>"
 			   "before continuing. See <b>Advanced</b> tab for detailed information.</nobr>");
		return false;
	}
	return true;
}

void KPDriverPage::setOptions(const QMap<QString,QString>& opts)
{
	m_view->setOptions(opts);
}

void KPDriverPage::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	m_view->getOptions(opts,incldef);
}
