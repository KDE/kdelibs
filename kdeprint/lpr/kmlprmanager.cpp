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
#include "apshandler.h"
#include "lprngtoolhandler.h"
#include "lprsettings.h"
#include "driver.h"
#include "editentrydialog.h"

#include <qfileinfo.h>
#include <qptrlist.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kprinter.h>
#include <kprocess.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <klibloader.h>

#include <stdlib.h>
#include <unistd.h>

KMLprManager::KMLprManager(QObject *parent, const char *name)
: KMManager(parent,name)
{
	m_handlers.setAutoDelete(true);
	m_handlerlist.setAutoDelete(false);
	m_entries.setAutoDelete(true);

	m_lpchelper = new LpcHelper(this);
	m_currentprinter = 0;

	setHasManagement(getuid() == 0);
	setPrinterOperationMask(
		KMManager::PrinterEnabling |
		KMManager::PrinterConfigure |
		KMManager::PrinterTesting |
		KMManager::PrinterCreation |
		KMManager::PrinterRemoval |
		KMManager::PrinterTesting
	);

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
		// notify handlers
		QPtrListIterator<LprHandler>	hit(m_handlerlist);
		for (; hit.current(); ++hit)
			hit.current()->reset();

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
		QPtrListIterator<KMPrinter>	it(m_printers);
		for (; it.current(); ++it)
			if (!it.current()->isSpecial())
			{
				it.current()->setDiscarded(false);
				checkPrinterState(it.current());
			}
	}
}

void KMLprManager::insertHandler(LprHandler *handler)
{
    m_handlers.insert(handler->name(), handler);
    m_handlerlist.append(handler);
    kdDebug() << "Handler: " << handler->name() << endl;
}

void KMLprManager::initHandlers()
{
	m_handlers.clear();
	m_handlerlist.clear();

	insertHandler(new MaticHandler(this));
	insertHandler(new ApsHandler(this));
	insertHandler(new LPRngToolHandler(this));

	// now load external handlers
	QStringList	l = KGlobal::dirs()->findAllResources("data", "kdeprint/lpr/*.la");
	for (QStringList::ConstIterator it=l.begin(); it!=l.end(); ++it)
	{
		KLibrary	*library = KLibLoader::self()->library(QFile::encodeName(*it));
		if (library)
		{
			kdDebug() << "loading external handler from " << *it << endl;
			LprHandler*(*func)(KMManager*) = (LprHandler*(*)(KMManager*))(library->symbol("create_handler"));
			if (func)
				insertHandler(func(this));
			else
				kdDebug() << "couldn't find the symbol 'create_handler'" << endl;
		}
	}

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
	{
		KMPrinter::PrinterState	st = m_lpchelper->state(prt);
		prt->setState(st);
		prt->setAcceptJobs(!(st & KMPrinter::Rejecting));
	}
	else
	{
		prt->setState(KMPrinter::Idle);
		prt->setAcceptJobs(true);
	}
}

DrMain* KMLprManager::loadPrinterDriver(KMPrinter *prt, bool config)
{
	if (!prt)
		return NULL;

	LprHandler	*handler = findHandler(prt);
	PrintcapEntry	*entry = findEntry(prt);
	if (handler && entry)
	{
		DrMain	*driver = handler->loadDriver(prt, entry, config);
		if (driver)
			driver->set("handler", handler->name());
		return driver;
	}
	return NULL;
}

DrMain* KMLprManager::loadFileDriver(const QString& filename)
{
	int	p = filename.find('/');
	QString	handler_str = (p != -1 ? filename.left(p) : QString::fromLatin1("default"));
	LprHandler	*handler = m_handlers.find(handler_str);
	if (handler)
	{
		DrMain	*driver = handler->loadDbDriver(filename);
		if (driver)
			driver->set("handler", handler->name());
		return driver;
	}
	return NULL;
}

bool KMLprManager::enablePrinter(KMPrinter *prt, bool state)
{
	QString	msg;
	if (!m_lpchelper->enable(prt, state, msg))
	{
		setErrorMsg(msg);
		return false;
	}
	return true;
}

bool KMLprManager::startPrinter(KMPrinter *prt, bool state)
{
	QString	msg;
	if (!m_lpchelper->start(prt, state, msg))
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
	{
		bool	mustSave(false);
		if (handler->savePrinterDriver(prt, entry, driver, &mustSave))
		{
			if (mustSave)
				return savePrintcapFile();
			return true;
		}
	}
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
		return true;
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
	PrintcapEntry	*oldEntry = m_entries.find(prt->printerName());

	// look for the handler and re-create entry
	LprHandler	*handler(0);
	// To look for the handler, either we base ourselves
	// on the driver (1: new printer, 2: modifying driver)
	// or we use the handler of the existing printer
	// (modifying something else, handler stays the same)
	if (prt->driver())
		handler = m_handlers.find(prt->driver()->get("handler"));
	else if (oldEntry)
		handler = findHandler(prt);
	else
		handler = m_handlers.find("default");
	if (!handler)
	{
		setErrorMsg(i18n("Internal error: no handler defined."));
		return false;
	}
	prt->setOption("kde-lpr-handler", handler->name());

	// we reload the driver if the printer object doesn't have one
	// and there's an old entry (sometimes needed to keep the backend
	// like in Foomatic)
	if (!prt->driver() && oldEntry)
		prt->setDriver(handler->loadDriver(prt, oldEntry, true));

	QString	sd = LprSettings::self()->baseSpoolDir();
	if (sd.isEmpty())
	{
		setErrorMsg(i18n("Couldn't determine spool directory. See options dialog."));
		return false;
	}
	sd.append("/").append(prt->printerName());
	if (!KStandardDirs::makeDir(sd, 0755))
	{
		setErrorMsg(i18n("Unable to create the spool directory %1. Check that you "
		                 "have the required permissions for that operation.").arg(sd));
		return false;
	}
	PrintcapEntry	*entry = handler->createEntry(prt);
	if (!entry)
		return false;	// error should be set in the handler
	// old entry can be removed now
	m_entries.remove(prt->printerName());
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
		{
			result = handler->savePrinterDriver(prt, entry, prt->driver());
		}

		// in case of LPRng, we need to tell the daemon about new printer
		if (LprSettings::self()->mode() == LprSettings::LPRng)
		{
			QString	msg;
			if (!m_lpchelper->restart(msg))
			{
				setErrorMsg(i18n("The printer has been created but the print daemon "
				                 "could not be restarted. %1").arg(msg));
				return false;
			}
		}
	}
	return result;
}

bool KMLprManager::removePrinter(KMPrinter *prt)
{
	LprHandler	*handler = findHandler(prt);
	PrintcapEntry	*entry = findEntry(prt);
	if (handler && entry)
	{
		if (handler->removePrinter(prt, entry))
		{
			QString	sd = entry->field("sd");
			// first try to save the printcap file, and if
			// successfull, remove the spool directory
			m_entries.take(prt->printerName());
			bool	status = savePrintcapFile();
			if (status)
			{
				// printcap file saved, entry can be deleted now
				delete entry;
				status =  (::system(QFile::encodeName("rm -rf " + KProcess::quote(sd))) == 0);
				if (!status)
					setErrorMsg(i18n("Unable to remove spool directory %1. "
					                 "Check that you have write permissions "
					                 "for that directory.").arg(sd));
				return status;
			}
			else
				// push back the non-removed entry
				m_entries.insert(prt->printerName(), entry);
		}
	}
	return false;
}

QString KMLprManager::driverDbCreationProgram()
{
	return QString::fromLatin1("make_driver_db_lpr");
}

QString KMLprManager::driverDirectory()
{
	QPtrListIterator<LprHandler>	it(m_handlerlist);
	QString	dbDirs;
	for (; it.current(); ++it)
	{
		QString	dir = it.current()->driverDirectory();
		if (!dir.isEmpty())
			dbDirs.append(dir).append(":");
	}
	if (!dbDirs.isEmpty())
		dbDirs.truncate(dbDirs.length()-1);
	return dbDirs;
}

QString KMLprManager::printOptions(KPrinter *prt)
{
	KMPrinter	*mprt = findPrinter(prt->printerName());
	QString	opts;
	if (mprt)
	{
		LprHandler	*handler = findHandler(mprt);
		if (handler)
			return handler->printOptions(prt);
	}
	return QString::null;
}

void KMLprManager::createPluginActions(KActionCollection *coll)
{
	KAction	*act = new KAction(i18n("&Edit printcap Entry..."), "kdeprint_report", 0, this, SLOT(slotEditPrintcap()), coll, "plugin_editprintcap");
	act->setGroup("plugin");
}

void KMLprManager::validatePluginActions(KActionCollection *coll, KMPrinter *prt)
{
	m_currentprinter = prt;
	// FIXME: disabled until completion
	coll->action("plugin_editprintcap")->setEnabled(0 && hasManagement() && prt && !prt->isSpecial());
}

void KMLprManager::slotEditPrintcap()
{
	if (!m_currentprinter ||
	    KMessageBox::warningYesNo(NULL,
	    i18n("Editing a printcap entry manually should only be "
		 "done by confirmed system administrator. This may "
		 "prevent your printer from working. Do you want to "
		 "continue?"), QString::null, QString::null, QString::null,
	    "editPrintcap") == KMessageBox::No)
		return;

	PrintcapEntry	*entry = findEntry(m_currentprinter);
	EditEntryDialog	dlg(entry, NULL);
	if (dlg.exec())
	{
	}
}

QString KMLprManager::stateInformation()
{
	return i18n("Spooler type: %1").arg(LprSettings::self()->mode() == LprSettings::LPR ? "LPR (BSD compatible)" : "LPRng");
}

#include "kmlprmanager.moc"
