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
#include "lprsettings.h"

#include <qfileinfo.h>
#include <qlist.h>
#include <klocale.h>
#include <kstandarddirs.h>
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
	QFileInfo	fi(LprSettings::self()->printcapFile());

	if (m_lpchelper)
		m_lpchelper->updateStates();

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
			if (!it.current()->isSpecial())
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
		return NULL;
	}
	return handler;
}

PrintcapEntry* KMLprManager::findEntry(KMPrinter *prt)
{
	PrintcapEntry	*entry = m_entries.find(prt->printerName());
	if (!entry)
	{
		return NULL;
	}
	return entry;
}

bool KMLprManager::completePrinter(KMPrinter *prt)
{
	LprHandler	*handler = findHandler(prt);
	PrintcapEntry	*entry = findEntry(prt);
	if (handler && entry)
		return handler->completePrinter(prt, entry, false);
	return false;
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
	return NULL;
}

DrMain* KMLprManager::loadFileDriver(const QString& filename)
{
	int	p = filename.find('/');
	QString	handler_str = (p != -1 ? filename.left(p) : QString::fromLatin1("default"));
	LprHandler	*handler = m_handlers.find(handler_str);
	if (handler)
		return handler->loadDbDriver(filename);
	return NULL;
}

bool KMLprManager::enablePrinter(KMPrinter *prt)
{
	QString	msg;
	if (!m_lpchelper->enable(prt, msg))
	{
		setErrorMsg(msg);
		return false;
	}
	return true;
}

bool KMLprManager::disablePrinter(KMPrinter *prt)
{
	QString	msg;
	if (!m_lpchelper->disable(prt, msg))
	{
		setErrorMsg(msg);
		return false;
	}
	return true;
}

bool KMLprManager::savePrinterDriver(KMPrinter *prt, DrMain *driver)
{
	LprHandler	*handler = findHandler(prt);
	PrintcapEntry	*entry = findEntry(prt);
	if (handler && entry)
		return handler->savePrinterDriver(prt, entry, driver);
	return false;
}

bool KMLprManager::savePrintcapFile()
{
	if (!LprSettings::self()->isLocalPrintcap())
	{
		setErrorMsg(i18n("The printcap file is a remote file (NIS). It cannot be written."));
		return false;
	}
	QFile	f(LprSettings::self()->printcapFile());
	if (f.open(IO_WriteOnly))
	{
		QTextStream	t(&f);
		QDictIterator<PrintcapEntry>	it(m_entries);
		for (; it.current(); ++it)
		{
			it.current()->writeEntry(t);
		}
	}
	else
	{
		setErrorMsg(i18n("Unable to save printcap file. Check that "
		                 "you have write permissions for that file."));
		return false;
	}
}

bool KMLprManager::createPrinter(KMPrinter *prt)
{
	// remove existing printcap entry
	m_entries.remove(prt->printerName());

	// look for the handler and re-create entry
	LprHandler	*handler = findHandler(prt);
	if (!handler)
	{
		setErrorMsg(i18n("Unrecognized entry."));
		return false;
	}
	QString	sd = LprSettings::self()->baseSpoolDir();
	if (sd.isEmpty())
	{
		setErrorMsg(i18n("Couldn't determine spool directory. See options dialog."));
		return false;
	}
	sd.append("/").append(prt->printerName());
	if (!KStandardDirs::makeDir(sd, 0700))
	{
		setErrorMsg(i18n("Unable to create the spool directory %1. Check that you "
		                 "have the required permissions for that operation.").arg(sd));
		return false;
	}
	PrintcapEntry	*entry = handler->createEntry(prt);
	if (!entry)
		return false;	// error should be set in the handler
	entry->name = prt->printerName();
	entry->addField("sh", Field::Boolean);
	entry->addField("mx", Field::Integer, "0");
	entry->addField("sd", Field::String, sd);
	if (!prt->option("kde-aliases").isEmpty())
		entry->aliases += QStringList::split("|", prt->option("kde-aliases"), false);

	// insert the new entry and save printcap file
	m_entries.insert(prt->printerName(), entry);
	bool	result = savePrintcapFile();
	if (result)
	{
		if (prt->driver())
			result = savePrinterDriver(prt, prt->driver());
	}
	return result;
}
