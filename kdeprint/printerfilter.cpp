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

#include "printerfilter.h"
#include "kmprinter.h"
#include "kmfactory.h"

#include <kconfig.h>

PrinterFilter::PrinterFilter(QObject *parent, const char *name)
: QObject(parent, name)
{
	m_locationRe.setWildcard(true);
	update();
}

PrinterFilter::~PrinterFilter()
{
}

void PrinterFilter::update()
{
	KConfig	*conf = KMFactory::self()->printConfig();
	conf->setGroup("Filter");
	m_locationRe.setPattern(conf->readEntry("LocationRe"));
	m_printers = conf->readListEntry("Printers");
	m_enabled = conf->readBoolEntry("Enabled", false);
}

void PrinterFilter::setEnabled(bool on)
{
	m_enabled = on;
	KConfig	*conf = KMFactory::self()->printConfig();
	conf->setGroup("Filter");
	conf->writeEntry("Enabled", m_enabled);
}

bool PrinterFilter::filter(KMPrinter *prt)
{
	if (m_enabled)
	{
		if (!m_locationRe.isEmpty() && !prt->location().isEmpty() && !m_locationRe.exactMatch(prt->location()))
			return false;
		if (!m_printers.isEmpty() && m_printers.find(prt->printerName()) == m_printers.end())
			return false;
	}
	return true;
}
