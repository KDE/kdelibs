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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "printerfilter.h"
#include "kmprinter.h"
#include "kmfactory.h"

#include <kconfig.h>
#include <kglobal.h>
#include <kdebug.h>

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
	// filter enable state is saved on a per application basis,
	// so this option is retrieve from the application config file
	conf = KGlobal::config();
	conf->setGroup("KPrinter Settings");
	m_enabled = conf->readBoolEntry("FilterEnabled", false);
}

void PrinterFilter::setEnabled(bool on)
{
	m_enabled = on;
	KConfig	*conf = KGlobal::config();
	conf->setGroup("KPrinter Settings");
	conf->writeEntry("FilterEnabled", m_enabled);
}

bool PrinterFilter::filter(KMPrinter *prt)
{
	if (m_enabled)
	{
		if ((!m_locationRe.isEmpty() && m_locationRe.exactMatch(prt->location())) ||
		    m_printers.find(prt->printerName()) != m_printers.end())
			return true;
		else
			return false;
	}
	return true;
}
