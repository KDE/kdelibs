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

#include "lprhandler.h"
#include "kmprinter.h"
#include "printcapentry.h"
#include "kmmanager.h"

#include <klocale.h>

LprHandler::LprHandler(const QString& name, KMManager *mgr)
: m_name(name), m_manager(mgr)
{
}

bool LprHandler::validate(PrintcapEntry*)
{
	return true;
}

KMPrinter* LprHandler::createPrinter(PrintcapEntry *entry)
{
	KMPrinter	*prt = new KMPrinter;
	prt->setPrinterName(entry->name);
	prt->setName(entry->name);
	prt->setType(KMPrinter::Printer);
	return prt;
}

bool LprHandler::completePrinter(KMPrinter *prt, PrintcapEntry *entry, bool)
{
    prt->setDescription(i18n("Unknown (unrecognized entry)"));
    QString val = entry->field("lp");
    if (!val.isEmpty() && val != "/dev/null")
        prt->setLocation(i18n("Local printer on %1").arg(val));
    else if (!(val = entry->field("rm")).isEmpty())
        prt->setLocation(i18n("Remote queue (%1) on %2").arg(entry->field("rp")).arg(val));
    else
        prt->setLocation(i18n("Unknown (unrecognized entry)"));
	return true;
}

DrMain* LprHandler::loadDriver(KMPrinter*, PrintcapEntry*)
{
	manager()->setErrorMsg(i18n("Unrecognized entry."));
	return NULL;
}

bool LprHandler::savePrinterDriver(KMPrinter*, PrintcapEntry*, DrMain*)
{
	manager()->setErrorMsg(i18n("Unrecognized entry."));
	return false;
}

DrMain* LprHandler::loadDbDriver(const QString&)
{
	manager()->setErrorMsg(i18n("Unrecognized entry."));
	return NULL;
}
