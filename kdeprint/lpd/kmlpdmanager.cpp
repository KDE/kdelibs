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

#include "kmlpdmanager.h"
#include "kmprinter.h"
#include "kmdbentry.h"
#include "driver.h"
#include "lpdtools.h"

#include <qfile.h>
#include <qtextstream.h>
#include <qmap.h>

#include <klocale.h>

// only there to allow testing on my system. Should be removed
// when everything has proven to be working and stable
QString	lpdprefix = "/tmp/opt";

//************************************************************************************************

KMLpdManager::KMLpdManager(QObject *parent, const char *name)
: KMManager(parent,name)
{
	m_entries.setAutoDelete(true);
	m_ptentries.setAutoDelete(true);
	setHasManagement(true);
	setPrinterOperationMask(KMManager::PrinterCreation|KMManager::PrinterConfigure);
}

KMLpdManager::~KMLpdManager()
{
}

QString KMLpdManager::driverDbCreationProgram()
{
	return QString::fromLatin1("make_driver_db_lpd");
}

QString KMLpdManager::driverDirectory()
{
	return QString::fromLatin1("/usr/lib/rhs/rhs-printfilters");
}

bool KMLpdManager::completePrinter(KMPrinter *printer)
{
	return completePrinterShort(printer);
}

bool KMLpdManager::completePrinterShort(KMPrinter *printer)
{
	PrintcapEntry	*entry = m_entries.find(printer->name());
	if (entry)
	{
		printer->setDescription(i18n("Local printer queue"));
		return true;
	}
	else return false;
}

void KMLpdManager::listPrinters()
{
	m_entries.clear();
	loadPrintcapFile(QString::fromLatin1("%1/etc/printcap").arg(lpdprefix));

	QDictIterator<PrintcapEntry>	it(m_entries);
	for (;it.current();++it)
	{
		KMPrinter	*printer = it.current()->createPrinter();
		addPrinter(printer);
	}
}

void KMLpdManager::loadPrintcapFile(const QString& filename)
{
	QFile	f(filename);
	if (f.exists() && f.open(IO_ReadOnly))
	{
		QTextStream	t(&f);
		QString		line;
		PrintcapEntry	*entry;
		while (!t.eof())
		{
			line = getPrintcapLine(t);
			if (line.isEmpty())
				continue;
			entry = new PrintcapEntry;
			if (entry->readLine(line))
				m_entries.insert(entry->m_name,entry);
			else
			{
				delete entry;
				break;
			}
		}
	}
}

void KMLpdManager::loadPrinttoolDb(const QString& filename)
{
	QFile	f(filename);
	if (f.exists() && f.open(IO_ReadOnly))
	{
		QTextStream	t(&f);
		PrinttoolEntry	*entry = new PrinttoolEntry;
		while (entry->readEntry(t))
		{
			m_ptentries.insert(entry->m_name,entry);
			entry = new PrinttoolEntry;
		}
		delete entry;
	}
}

DrMain* KMLpdManager::loadDbDriver(KMDBEntry *entry)
{
	QString	ptdbfilename = driverDirectory() + "/printerdb";
	if (entry->file == ptdbfilename)
	{
		if (m_ptentries.count() == 0)
			loadPrinttoolDb(ptdbfilename);
		PrinttoolEntry	*ptentry = m_ptentries.find(entry->modelname);
		if (ptentry)
		{
			DrMain	*dr = ptentry->createDriver();
			return dr;
		}
	}
	return NULL;
}

DrMain* KMLpdManager::loadPrinterDriver(KMPrinter *printer, bool config)
{
	PrintcapEntry	*entry = m_entries.find(printer->name());
	if (!entry)
		return NULL;

	// check for printtool driver (only for configuration)
	QString	sd = entry->arg("sd");
	if (QFile::exists(sd+"/postscript.cfg") && config)
	{
		QMap<QString,QString>	map = loadPrinttoolCfgFile(sd+"/postscript.cfg");
		PrinttoolEntry	*ptentry = findGsDriver(map["GSDEVICE"]);
		if (!ptentry)
			return NULL;
		DrMain	*dr = ptentry->createDriver();
		dr->setOptions(map);
		map = loadPrinttoolCfgFile(sd+"/general.cfg");
		dr->setOptions(map);
		map = loadPrinttoolCfgFile(sd+"/textonly.cfg");
		dr->setOptions(map);
		return dr;
	}

	// default
	return NULL;
}

PrinttoolEntry* KMLpdManager::findGsDriver(const QString& gsdriver)
{
	if (m_ptentries.count() == 0)
		loadPrinttoolDb(driverDirectory()+"/printerdb");
	QDictIterator<PrinttoolEntry>	it(m_ptentries);
	for (;it.current();++it)
		if (it.current()->m_gsdriver == gsdriver)
			return it.current();
	return NULL;
}

QMap<QString,QString> KMLpdManager::loadPrinttoolCfgFile(const QString& filename)
{
	QFile	f(filename);
	QMap<QString,QString>	map;
	if (f.exists() && f.open(IO_ReadOnly))
	{
		QTextStream	t(&f);
		QString		line, name, val;
		int 		p(-1);
		while (!t.eof())
		{
			line = getPrintcapLine(t);
			if (line.isEmpty())
				break;
			if (line.startsWith("export "))
				line.replace(0,7,"");
			if ((p=line.find('=')) != -1)
			{
				name = line.left(p);
				val = line.right(line.length()-p-1);
				val.replace(QRegExp("\""),"");
				if (!name.isEmpty() && !val.isEmpty())
					map[name] = val;
			}
		}
	}
	return map;
}
