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

#include "kmlprmanager.h"
#include "printcapreader.h"
#include "printcapentry.h"
#include "lpchelper.h"
#include "matichandler.h"

#include <qfileinfo.h>
#include <qlist.h>
#include <klocale.h>
#include <unistd.h>

KMLprManager::KMLprManager(QObject *parent, const char *name)
: KMManager(parent,name)
{
	m_handlers.setAutoDelete(true);
	m_handlerlist.setAutoDelete(false);
	m_entries.setAutoDelete(true);

	m_lpchelper = new LpcHelper(this);

	setHasManagement(/*getuid() == 0*/true);
	setPrinterOperationMask(KMManager::PrinterEnabling|KMManager::PrinterConfigure|KMManager::PrinterTesting);

	initHandlers();
}

void KMLprManager::listPrinters()
{
	QFileInfo	fi("/etc/printcap");

	// update only if needed
	if (!m_updtime.isValid() || m_updtime < fi.lastModified())
	{
		// cleanup previous entries
		m_entries.clear();

		// try to open the printcap file and parse it
		PrintcapReader	reader;
		QFile	f(fi.absFilePath());
		PrintcapEntry	*entry;
		if (f.exists() && f.open(IO_ReadOnly))
		{
			reader.setPrintcapFile(&f);
			while ((entry = reader.nextEntry()) != NULL)
			{
				QPtrListIterator<LprHandler>	it(m_handlerlist);
				for (; it.current(); ++it)
					if (it.current()->validate(entry))
					{
						KMPrinter	*prt = it.current()->createPrinter(entry);
						checkPrinterState(prt);
						prt->setOption("kde-lpr-handler", it.current()->name());
						addPrinter(prt);
						break;
					}
				m_entries.insert(entry->name, entry);
			}
		}

		// save update time
		m_updtime = fi.lastModified();
	}
	else
	{
		discardAllPrinters(false);
		QPtrListIterator<KMPrinter>	it(m_printers);
		for (; it.current(); ++it)
			checkPrinterState(it.current());
	}
}

void KMLprManager::insertHandler(LprHandler *handler)
{
    m_handlers.insert(handler->name(), handler);
    m_handlerlist.append(handler);
}

void KMLprManager::initHandlers()
{
	m_handlers.clear();
	m_handlerlist.clear();

	insertHandler(new MaticHandler(this));

	// default handler
	insertHandler(new LprHandler("default", this));
}

LprHandler* KMLprManager::findHandler(KMPrinter *prt)
{
	QString	handlerstr(prt->option("kde-lpr-handler"));
	LprHandler	*handler(0);
	if (handlerstr.isEmpty() || (handler = m_handlers.find(handlerstr)) == NULL)
	{
		setErrorMsg(i18n("Internal error."));
		return NULL;
	}
	return handler;
}

PrintcapEntry* KMLprManager::findEntry(KMPrinter *prt)
{
	PrintcapEntry	*entry = m_entries.find(prt->printerName());
	if (!entry)
	{
		setErrorMsg(i18n("Internal error."));
		return NULL;
	}
	return entry;
}

bool KMLprManager::completePrinter(KMPrinter *prt)
{
	LprHandler	*handler = findHandler(prt);
	PrintcapEntry	*entry = findEntry(prt);
	if (!handler || !entry)
		return false;

	return handler->completePrinter(prt, entry, false);
}

bool KMLprManager::completePrinterShort(KMPrinter *prt)
{
	LprHandler	*handler = findHandler(prt);
	PrintcapEntry	*entry = findEntry(prt);
	if (!handler || !entry)
		return false;

	return handler->completePrinter(prt, entry, true);
}

void KMLprManager::checkPrinterState(KMPrinter *prt)
{
	if (m_lpchelper)
		prt->setState(m_lpchelper->state(prt));
	else
		prt->setState(KMPrinter::Idle);
}

DrMain* KMLprManager::loadPrinterDriver(KMPrinter *prt, bool)
{
	LprHandler	*handler = findHandler(prt);
	PrintcapEntry	*entry = findEntry(prt);
	if (handler && entry)
		return handler->loadDriver(prt, entry);
	setErrorMsg(i18n("Internal error."));
	return NULL;
}
