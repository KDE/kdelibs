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
#include "matichandler.h"

#include <qfileinfo.h>
#include <qfile.h>
#include <klocale.h>

KMLprManager::KMLprManager(QObject *parent, const char *name)
: KMManager(parent,name)
{
	m_handlers.setAutoDelete(true);
	m_entries.setAutoDelete(true);
	
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
				QDictIterator<LprHandler>	it(m_handlers);
				for (; it.current(); ++it)
					if (it.current()->validate(entry))
					{
						KMPrinter	*prt = it.current()->createPrinter(entry);
						prt->setState(KMPrinter::Idle);
						prt->setOption("kde-lpr-handler", it.currentKey());
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
		discardAllPrinters(false);
}

void KMLprManager::initHandlers()
{
	LprHandler	*handler;
	
	m_handlers.clear();
	handler = new LprHandler("invalid");
	m_handlers.insert(handler->name(), handler);
	handler = new MaticHandler;
	m_handlers.insert(handler->name(), handler);
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
