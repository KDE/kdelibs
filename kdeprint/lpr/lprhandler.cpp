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
    KURL uri;
    if (!val.isEmpty() && val != "/dev/null")
    {
        prt->setLocation(i18n("Local printer on %1").arg(val));
        uri.setProtocol("parallel");
        uri.setPath(val);
    }
    else if (!(val = entry->field("rm")).isEmpty())
    {
        prt->setLocation(i18n("Remote queue (%1) on %2").arg(entry->field("rp")).arg(val));
        uri.setProtocol("lpd");
        uri.setHost(val);
        uri.setPath("/" + entry->field("rp"));
    }
    else
        prt->setLocation(i18n("Unknown (unrecognized entry)"));
    prt->setDevice(uri);
	return true;
}

DrMain* LprHandler::loadDriver(KMPrinter*, PrintcapEntry*, bool)
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

PrintcapEntry* LprHandler::createEntry(KMPrinter *prt)
{
	// this default handler only supports local parallel and remote lpd URIs
	KURL	uri = prt->device();
	QString	prot = uri.protocol();
	if (!prot.isEmpty() && prot != "parallel" && prot != "file" && prot != "lpd")
	{
		manager()->setErrorMsg(i18n("Unsupported backend: %1.").arg(prot));
		return NULL;
	}
	PrintcapEntry	*entry = new PrintcapEntry;
	entry->comment = "# Default handler";
	if (prot == "lpd")
	{
		entry->addField("rm", Field::String, uri.host());
		QString	rp = uri.path();
		if (rp[0] == '/')
			rp = rp.mid(1);
		entry->addField("rp", Field::String, rp);
		// force this entry to null (otherwise it seems it's redirected
		// to /dev/lp0 by default)
		entry->addField("lp", Field::String, QString::null);
	}
	else
	{
		entry->addField("lp", Field::String, uri.path());
	}
	return entry;
}

bool LprHandler::removePrinter(KMPrinter*, PrintcapEntry*)
{
	return true;
}

QString LprHandler::printOptions(KPrinter*)
{
	return QString::null;
}

void LprHandler::reset()
{
}
