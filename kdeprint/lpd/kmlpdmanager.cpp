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
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qmap.h>

#include <klocale.h>
#include <kstddirs.h>

#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>

// only there to allow testing on my system. Should be removed
// when everything has proven to be working and stable
QString	lpdprefix = "";
QString ptPrinterType(KMPrinter*);

//************************************************************************************************

KMLpdManager::KMLpdManager(QObject *parent, const char *name)
: KMManager(parent,name)
{
	m_entries.setAutoDelete(true);
	m_ptentries.setAutoDelete(true);
	setHasManagement(true);
	setPrinterOperationMask(KMManager::PrinterCreation|KMManager::PrinterConfigure|KMManager::PrinterRemoval);
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

bool KMLpdManager::createPrinter(KMPrinter *printer)
{
	// 1) create the printcap entry
	PrintcapEntry	*ent = m_entries.take(printer->printerName());
	if (!ent)
	{
		ent = new PrintcapEntry();
		ent->m_name = printer->printerName();
	}
	else
		ent->m_args.clear();
	// Standard options
	if (printer->device().protocol() == "lpd")
	{
		// remote lpd queue
		ent->m_args["rm"] = printer->device().host();
		ent->m_args["rp"] = printer->device().path().replace(QRegExp("/"),QString::fromLatin1(""));
		ent->m_args["lpd_bounce"] = "true";
	}
	ent->m_args["mx"] = (printer->option("mx").isEmpty() ? "#0" : printer->option("mx"));
	ent->m_args["sh"] = QString::null;
	// create spool directory (if necessary) and update PrintcapEntry object
	if (!createSpooldir(ent))
	{
		setErrorMsg(i18n("Unable to create spool directory %1 for printer <b>%2</b>.").arg(ent->arg("sd")).arg(ent->m_name));
		delete ent;
		return false;
	}
	if (printer->driver() && printer->driver()->get("drtype") == "printtool")
		if (!createPrinttoolEntry(printer,ent))
		{
			setErrorMsg(i18n("Unable to save informations for printer <b>%1</b>.").arg(printer->printerName()));
			delete ent;
			return false;
		}
	
	// 2) save the printer driver (if any)
	if (printer->driver())
	{
		// necessary, otherwise we won't know the spool directory.
		printer->setOption("sd",ent->arg("sd"));
		if (!savePrinterDriver(printer,printer->driver()))
		{
			setErrorMsg(i18n("Unable to save driver for printer <b>%1</b>.").arg(printer->printerName()));
			delete ent;
			return false;
		}
	}

	// 3) change permissions of spool directory
	QString	cmd = QString::fromLatin1("chmod -R go-rwx %1 && chown -R lp.lp %2").arg(ent->arg("sd")).arg(ent->arg("sd"));
	if (system(cmd.latin1()) != 0)
	{
		setErrorMsg(i18n("Unable to set correct permissions on spool directory %1 for printer <b>%2</b>.").arg(ent->arg("sd")).arg(ent->m_name));
		delete ent;
		return false;
	}

	// 4) write the printcap file
	m_entries.insert(ent->m_name,ent);
	if (!writePrintcapFile(QString::fromLatin1("%1/etc/printcap").arg(lpdprefix)))
	{
		setErrorMsg(i18n("Unable to write printcap file."));
		return false;
	}
	return true;
}

bool KMLpdManager::removePrinter(KMPrinter *printer)
{
	PrintcapEntry	*ent = m_entries.take(printer->printerName());
	if (ent)
	{
		if (!writePrintcapFile(QString::fromLatin1("%1/etc/printcap").arg(lpdprefix)))
		{
			setErrorMsg(i18n("Unable to write printcap file."));
			m_entries.insert(ent->m_name,ent);
			return false;
		}
		system(QString::fromLatin1("rm -rf %1").arg(ent->arg("sd")).latin1());
		delete ent;
		return true;
	}
	else
	{
		setErrorMsg(i18n("Couldn't find printer <b>%1</b> in printcap file.").arg(printer->printerName()));
		return false;
	}
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

bool KMLpdManager::writePrintcapFile(const QString& filename)
{
	QFile	f(filename);
	if (f.open(IO_WriteOnly))
	{
		QTextStream	t(&f);
		t << "# File generated by KDE print (LPD plugin).\n#Don't edit by hand." << endl << endl;
		QDictIterator<PrintcapEntry>	it(m_entries);
		for (;it.current();++it)
			it.current()->writeEntry(t);
		return true;
	}
	return false;
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

bool KMLpdManager::savePrinttoolCfgFile(const QString& templatefile, const QString& dirname, const QMap<QString,QString>& options)
{
	// defines input and output file
	QString	fname = QFileInfo(templatefile).fileName();
	fname.replace(QRegExp("\\.in$"),QString::fromLatin1(""));
	QFile	fin(templatefile);
	QFile	fout(dirname + "/" + fname);
	if (fin.exists() && fin.open(IO_ReadOnly) && fout.open(IO_WriteOnly))
	{
		QTextStream	tin(&fin), tout(&fout);
		QString		line, name;
		int		p(-1);
		while (!tin.eof())
		{
			line = tin.readLine().stripWhiteSpace();
			if (line.isEmpty() || line[0] == '#')
			{
				tout << line << endl;
				continue;
			}
			if (line.startsWith("export "))
			{
				tout << "export ";
				line.replace(0,7,QString::fromLatin1(""));
			}
			if ((p=line.find('=')) != -1)
			{
				name = line.left(p);
				tout << name << '=' << options[name] << endl;
			}
		}
		return true;
	}
	else return false;
}

bool KMLpdManager::savePrinterDriver(KMPrinter *printer, DrMain *driver)
{
	// we need to find the spool directory: either find the corresponding
	// printcap entry, or look for a "sd" option in printer (should be the
	// case when adding the printer, for example).
	QString	spooldir;
	PrintcapEntry	*ent;
	if ((ent=m_entries.find(printer->printerName())) != NULL)
		spooldir = ent->arg("sd");
	else
		spooldir = printer->option("sd");

	if (driver->get("drtype") == "printtool" && !spooldir.isEmpty())
	{
		QMap<QString,QString>	options;
		driver->getOptions(options,true);
		// add some standard options
		options["DESIRED_TO"] = "ps";
		options["PRINTER_TYPE"] = ptPrinterType(printer);
		options["PS_SEND_EOF"] = "NO";
		// write various driver files using templates
		if (savePrinttoolCfgFile(driverDirectory()+"/general.cfg.in",spooldir,options) &&
		    savePrinttoolCfgFile(driverDirectory()+"/postscript.cfg.in",spooldir,options) &&
		    savePrinttoolCfgFile(driverDirectory()+"/textonly.cfg.in",spooldir,options))
			return true;
	}
	return false;
}

bool KMLpdManager::createPrinttoolEntry(KMPrinter *printer, PrintcapEntry *entry)
{
	KURL	dev(printer->device());
	QString	prot = dev.protocol(), cmd, sd(entry->arg("sd"));
	entry->m_args["if"] = sd+QString::fromLatin1("/filter");
	cmd = QString::fromLatin1("cp %1/master-filter %2/filter").arg(driverDirectory()).arg(sd);
	if (system(cmd.latin1()) != 0)
		return false;
	if (prot == "smb" || prot == "ncp" || prot == "direct")
	{
		entry->m_args["af"] = sd+QString::fromLatin1("/acct");
		QFile	f(sd+QString::fromLatin1("/.config"));
		if (f.open(IO_WriteOnly))
		{
			QTextStream	t(&f);
			if (prot == "direct")
			{
				t << "printer_ip=" << dev.host() << endl;
				t << "port=" << dev.port() << endl;
			}
			else if (prot == "smb")
			{
				QStringList	l = QStringList::split('/',dev.path(),false);
				if (l.count() == 2)
				{
					t << "share='\\\\" << l[0] << '\\' << l[1] << '\'' << endl;
					t << "workgroup='" << dev.host() << '\'' << endl;
				}
				else if (l.count() == 1)
				{
					t << "share='\\\\" << dev.host() << '\\' << l[0] << '\'' << endl;
					t << "workgroup=" << endl;
				}
				t << "hostip=" << endl;
				t << "user='" << dev.user() << '\'' << endl;
				t << "password='" << dev.pass() << '\'' << endl;
			}
			else if (prot == "ncp")
			{
				t << "server=" << dev.host() << endl;
				t << "queue=" << dev.path().replace(QRegExp("/"),QString::fromLatin1("")) << endl;
				t << "user=" << dev.user() << endl;
				t << "password=" << dev.pass() << endl;
			}
		}
		else return false;
		entry->m_args["lp"] = QString::fromLatin1("/dev/null");
	}
	else if (prot != "lpd")
		entry->m_args["lp"] = dev.path();
	return true;
}

bool KMLpdManager::createSpooldir(PrintcapEntry *entry)
{
	// first check if it has a "sd" defined
	if (entry->arg("sd").isEmpty())
		entry->m_args["sd"] = QString::fromLatin1("/var/spool/lpd/")+entry->m_name;
	QString	sd = entry->arg("sd");
	if (!KStandardDirs::exists(sd))
	{
		// create directory and change permissions/owner
		if (!KStandardDirs::makeDir(sd,0700)) return false;
		struct passwd	*lp_pw = getpwnam("lp");
		if (lp_pw && chown(QFile::encodeName(sd),lp_pw->pw_uid,lp_pw->pw_gid) != 0)
			return false;
	}
	return true;
}

//************************************************************************************************

QString ptPrinterType(KMPrinter *p)
{
	QString	type, prot = p->device().protocol();
	if (prot == "lpd") type = "REMOTE";
	else if (prot == "smb") type = "SMB";
	else if (prot == "ncp") type = "NCP";
	else if (prot == "socket") type = "DIRECT";
	else type = "LOCAL";
	return type;
}
