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
#include "kmfactory.h"
#include "lpdtools.h"
#include "gschecker.h"
#include "kpipeprocess.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qmap.h>
#include <qregexp.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kprocess.h>

#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

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
	setHasManagement(getuid() == 0);
	setPrinterOperationMask(KMManager::PrinterCreation|KMManager::PrinterConfigure|KMManager::PrinterRemoval|KMManager::PrinterEnabling);
	m_gschecker = new GsChecker(this,"GsChecker");
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
		QString	type(entry->comment(2)), driver(entry->comment(7)), lp(entry->arg("lp"));
		printer->setDescription(i18n("Local printer queue (%1)").arg(type.isEmpty() ? i18n("Unknown Type of local printer queue", "Unknown") : type));
		printer->setLocation(i18n("<Not available>"));
		printer->setDriverInfo(driver.isEmpty() ? i18n("Unknown Driver", "Unknown") : driver);
		// device
		KURL	url;
		if (!entry->arg("rm").isEmpty())
		{
			url = QString::fromLatin1("lpd://%1/%2").arg(entry->arg("rm")).arg(entry->arg("rp"));
			printer->setDescription(i18n("Remote LPD queue %1@%2").arg(entry->arg("rp")).arg(entry->arg("rm")));
		}
		else if (!lp.isEmpty() && lp != "/dev/null")
			url = QString::fromLatin1("parallel:%1").arg(lp);
		else if (QFile::exists(entry->arg("sd")+"/.config"))
		{
			QMap<QString,QString>	map = loadPrinttoolCfgFile(entry->arg("sd")+"/.config");
			if (type == "SMB")
			{
				QStringList	l = QStringList::split('\\',map["share"],false);
				if (map["workgroup"].isEmpty())
					url = QString::fromLatin1("smb://%1/%2").arg(l[0]).arg(l[1]);
				else
					url = QString::fromLatin1("smb://%1/%2/%3").arg(map["workgroup"]).arg(l[0]).arg(l[1]);
				url.setUser(map["user"]);
				url.setPass(map["password"]);
			}
			else if (type == "DIRECT")
				url = QString::fromLatin1("socket://%1:%2").arg(map["printer_ip"]).arg(map["port"]);
			else if (type == "NCP")
			{
				url = QString::fromLatin1("ncp://%1/%2").arg(map["server"]).arg(map["queue"]);
				url.setUser(map["user"]);
				url.setPass(map["password"]);
			}
		}
		printer->setDevice(url);
		return true;
	}
	else return false;
}

bool KMLpdManager::createPrinter(KMPrinter *printer)
{
	// 1) create the printcap entry
	PrintcapEntry	*ent = findPrintcapEntry(printer->printerName());
	if (!ent)
	{
		ent = new PrintcapEntry();
		ent->m_name = printer->printerName();
	}
	else
	{
		if (!printer->driver() && printer->option("kde-driver") != "raw")
			printer->setDriver(loadPrinterDriver(printer,true));
		// remove it from current entries
		ent = m_entries.take(ent->m_name);
		ent->m_args.clear();
	}
	// Standard options
	if (printer->device().protocol() == "lpd")
	{
		// remote lpd queue
		ent->m_args["rm"] = printer->device().host();
		ent->m_args["rp"] = printer->device().path().replace(QRegExp("/"),QString::fromLatin1(""));
		ent->m_args["lpd_bounce"] = "true";
		ent->m_comment = QString::fromLatin1("##PRINTTOOL3## REMOTE");
	}
	ent->m_args["mx"] = (printer->option("mx").isEmpty() ? "#0" : printer->option("mx"));
	ent->m_args["sh"] = QString::null;
	// create spool directory (if necessary) and update PrintcapEntry object
	if (!createSpooldir(ent))
	{
		setErrorMsg(i18n("Unable to create spool directory %1 for printer %2.").arg(ent->arg("sd")).arg(ent->m_name));
		delete ent;
		return false;
	}
	if (!printer->driver() || printer->driver()->get("drtype") == "printtool")
		if (!createPrinttoolEntry(printer,ent))
		{
			setErrorMsg(i18n("Unable to save information for printer <b>%1</b>.").arg(printer->printerName()));
			delete ent;
			return false;
		}

	// 2) write the printcap file
	m_entries.insert(ent->m_name,ent);
	if (!writePrinters())
		return false;

	// 3) save the printer driver (if any)
	if (printer->driver())
	{
		if (!savePrinterDriver(printer,printer->driver()))
		{
			m_entries.remove(ent->m_name);
			writePrinters();
			return false;
		}
	}

	// 4) change permissions of spool directory
	QCString cmd = "chmod -R o-rwx,g+rwX ";
	cmd += QFile::encodeName(KProcess::quote(ent->arg("sd")));
	cmd += "&& chown -R lp.lp ";
	cmd += QFile::encodeName(KProcess::quote(ent->arg("sd")));
	if (system(cmd.data()) != 0)
	{
		setErrorMsg(i18n("Unable to set correct permissions on spool directory %1 for printer <b>%2</b>.").arg(ent->arg("sd")).arg(ent->m_name));
		return false;
	}

	return true;
}

bool KMLpdManager::removePrinter(KMPrinter *printer)
{
	PrintcapEntry	*ent = findPrintcapEntry(printer->printerName());
	if (ent)
	{
		ent = m_entries.take(printer->printerName());
		if (!writePrinters())
		{
			m_entries.insert(ent->m_name,ent);
			return false;
		}
		QCString cmd = "rm -rf ";
		cmd += QFile::encodeName(KProcess::quote(ent->arg("sd")));
		system(cmd.data());
		delete ent;
		return true;
	}
	else
		return false;
}

bool KMLpdManager::enablePrinter(KMPrinter *printer, bool state)
{
	KPipeProcess	proc;
	QString		cmd = QString::fromLatin1("%1 %2 %3").arg(programName(0)).arg(state ? "up" : "down").arg(printer->printerName());
	if (proc.open(cmd))
	{
		QTextStream	t(&proc);
		QString		buffer;
		while (!t.eof())
			buffer.append(t.readLine());
		if (buffer.startsWith("?Privilege"))
		{
			setErrorMsg(i18n("Permission denied: you must be root."));
			return false;
		}
		return true;
	}
	else
	{
		setErrorMsg(i18n("Unable to execute command \"%1\".").arg(cmd));
		return false;
	}
}

bool KMLpdManager::enablePrinter(KMPrinter *printer)
{
	return enablePrinter(printer,true);
}

bool KMLpdManager::disablePrinter(KMPrinter *printer)
{
	return enablePrinter(printer,false);
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

	checkStatus();
}

QString KMLpdManager::programName(int f)
{
	KConfig	*conf = KMFactory::self()->printConfig();
	conf->setGroup("LPD");
	switch (f)
	{
		case 0: return conf->readEntry("LpdCommand","/usr/sbin/lpc");
		case 1: return conf->readEntry("LpdQueue","lpq");
		case 2: return conf->readEntry("LpdRemove","lprm");
	}
	return QString::null;
}

void KMLpdManager::checkStatus()
{
	KPipeProcess	proc;
	QString		cmd = QString::fromLatin1("%1 status all").arg(programName(0));
	if (proc.open(cmd))
	{
		QTextStream	t(&proc);
		QString		line;
		KMPrinter	*printer(0);
		int		p(-1);
		while (!t.eof())
		{
			line = t.readLine().stripWhiteSpace();
			if (line.isEmpty())
				continue;
			if ((p=line.find(':')) != -1)
				printer = findPrinter(line.left(p));
			else if (line.startsWith("printing") && printer)
				printer->setState(line.find("enabled") != -1 ? KMPrinter::Idle : KMPrinter::Stopped);
			else if (line.find("entries") != -1 && printer)
				if (!line.startsWith("no") && printer->state() == KMPrinter::Idle)
					printer->setState(KMPrinter::Processing);
		}
	}
}

bool KMLpdManager::writePrinters()
{
	if (!writePrintcapFile(QString::fromLatin1("%1/etc/printcap").arg(lpdprefix)))
	{
		setErrorMsg(i18n("Unable to write printcap file."));
		return false;
	}
	return true;
}

void KMLpdManager::loadPrintcapFile(const QString& filename)
{
	QFile	f(filename);
	if (f.exists() && f.open(IO_ReadOnly))
	{
		QTextStream	t(&f);
		QString		line, comment;
		PrintcapEntry	*entry;
		while (!t.eof())
		{
			line = getPrintcapLine(t,&comment);
			if (line.isEmpty())
				continue;
			entry = new PrintcapEntry;
			if (entry->readLine(line))
			{
				m_entries.insert(entry->m_name,entry);
				entry->m_comment = comment;
			}
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

PrinttoolEntry* KMLpdManager::findPrinttoolEntry(const QString& name)
{
	if (m_ptentries.count() == 0)
		loadPrinttoolDb(driverDirectory()+"/printerdb");
	PrinttoolEntry	*ent = m_ptentries.find(name);
	if (!ent)
		setErrorMsg(i18n("Couldn't find driver <b>%1</b> in printtool database.").arg(name));
	return ent;
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
		PrinttoolEntry	*ptentry = findPrinttoolEntry(entry->modelname);
		if (ptentry)
		{
			DrMain	*dr = ptentry->createDriver();
			return dr;
		}
	}
	return NULL;
}

PrintcapEntry* KMLpdManager::findPrintcapEntry(const QString& name)
{
	PrintcapEntry	*ent = m_entries.find(name);
	if (!ent)
		setErrorMsg(i18n("Couldn't find printer <b>%1</b> in printcap file.").arg(name));
	return ent;
}

DrMain* KMLpdManager::loadPrinterDriver(KMPrinter *printer, bool config)
{
	PrintcapEntry	*entry = findPrintcapEntry(printer->name());
	if (!entry)
		return NULL;

	// check for printtool driver (only for configuration)
	QString	sd = entry->arg("sd"), dr(entry->comment(7));
	if (QFile::exists(sd+"/postscript.cfg") && config && !dr.isEmpty())
	{
		QMap<QString,QString>	map = loadPrinttoolCfgFile(sd+"/postscript.cfg");
		PrinttoolEntry	*ptentry = findPrinttoolEntry(dr);
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
	if (entry->m_comment.startsWith("##PRINTTOOL3##"))
		setErrorMsg(i18n("No driver found (raw printer)"));
	else
		setErrorMsg(i18n("Printer type not recognized."));
	return NULL;
}

bool KMLpdManager::checkGsDriver(const QString& gsdriver)
{
	if (gsdriver == "ppa" || gsdriver == "POSTSCRIPT" || gsdriver == "TEXT")
		return true;
	else if (!m_gschecker->checkGsDriver(gsdriver))
	{
		setErrorMsg(i18n("The driver device <b>%1</b> is not compiled in your GhostScript distribution. Check your installation or use another driver.").arg(gsdriver));
		return false;
	}
	return true;
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
				val.replace(QRegExp("'"),"");
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
	// To be able to save a printer driver, a printcap entry MUST exist.
	// We can then retrieve the spool directory from it.
	QString	spooldir;
	PrintcapEntry	*ent = findPrintcapEntry(printer->printerName());
	if (!ent)
		return false;
	spooldir = ent->arg("sd");

	if (driver->get("drtype") == "printtool" && !spooldir.isEmpty())
	{
		QMap<QString,QString>	options;
		driver->getOptions(options,true);
		// add some standard options
		options["DESIRED_TO"] = "ps";
		options["PRINTER_TYPE"] = ent->comment(2);	// get type from printcap entry (works in anycases)
		options["PS_SEND_EOF"] = "NO";
		if (!checkGsDriver(options["GSDEVICE"]))
			return false;
		QString	resol(options["RESOLUTION"]), color(options["COLOR"]);
		// update entry comment to make printtool happy and save printcap file
		ent->m_comment = QString::fromLatin1("##PRINTTOOL3## %1 %2 %3 %4 {} {%5} %6 {}").arg(options["PRINTER_TYPE"]).arg(options["GSDEVICE"]).arg((resol.isEmpty() ? QString::fromLatin1("NAxNA") : resol)).arg(options["PAPERSIZE"]).arg(driver->name()).arg((color.isEmpty() ? QString::fromLatin1("Default") : color.right(color.length()-15)));
		ent->m_args["if"] = spooldir+QString::fromLatin1("/filter");
		if (!writePrinters())
			return false;
		// write various driver files using templates
		QCString cmd = "cp ";
		cmd += QFile::encodeName(KProcess::quote(driverDirectory()+"/master-filter"));
		cmd += " ";
		cmd += QFile::encodeName(KProcess::quote(spooldir + "/filter"));
		if (system(cmd.data()) == 0 &&
		    savePrinttoolCfgFile(driverDirectory()+"/general.cfg.in",spooldir,options) &&
		    savePrinttoolCfgFile(driverDirectory()+"/postscript.cfg.in",spooldir,options) &&
		    savePrinttoolCfgFile(driverDirectory()+"/textonly.cfg.in",spooldir,options))
			return true;
		setErrorMsg(i18n("Unable to write driver associated files in spool directory."));
	}
	return false;
}

bool KMLpdManager::createPrinttoolEntry(KMPrinter *printer, PrintcapEntry *entry)
{
	KURL	dev(printer->device());
	QString	prot = dev.protocol(), sd(entry->arg("sd"));
	entry->m_comment = QString::fromLatin1("##PRINTTOOL3## %1").arg(ptPrinterType(printer));
	if (prot == "smb" || prot == "ncp" || prot == "socket")
	{
		entry->m_args["af"] = sd+QString::fromLatin1("/acct");
		QFile	f(sd+QString::fromLatin1("/.config"));
		if (f.open(IO_WriteOnly))
		{
			QTextStream	t(&f);
			if (prot == "socket")
			{
				t << "printer_ip=" << dev.host() << endl;
				t << "port=" << dev.port() << endl;
				entry->m_args["if"] = driverDirectory()+QString::fromLatin1("/directprint");
			}
			else if (prot == "smb")
			{
				QStringList	l = QStringList::split('/',dev.path(),false);
				if (l.count() == 2)
				{
					t << "share='\\\\" << l[0] << '\\' << l[1] << '\'' << endl;
				}
				else if (l.count() == 1)
				{
					t << "share='\\\\" << dev.host() << '\\' << l[0] << '\'' << endl;
				}
				t << "hostip=" << endl;
				t << "user='" << dev.user() << '\'' << endl;
				t << "password='" << dev.pass() << '\'' << endl;
				t << "workgroup='" << (l.count() == 2 ? dev.host() : QString::fromLatin1("")) << '\'' << endl;
				entry->m_args["if"] = driverDirectory()+QString::fromLatin1("/smbprint");
			}
			else if (prot == "ncp")
			{
				t << "server=" << dev.host() << endl;
				t << "queue=" << dev.path().replace(QRegExp("/"),QString::fromLatin1("")) << endl;
				t << "user=" << dev.user() << endl;
				t << "password=" << dev.pass() << endl;
				entry->m_args["if"] = driverDirectory()+QString::fromLatin1("/ncpprint");
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
		if (!KStandardDirs::makeDir(sd,0750))
			return false;
		struct passwd	*lp_pw = getpwnam("lp");
		if (lp_pw && chown(QFile::encodeName(sd),lp_pw->pw_uid,lp_pw->pw_gid) != 0)
			return false;
	}
	return true;
}

bool KMLpdManager::validateDbDriver(KMDBEntry *entry)
{
	PrinttoolEntry	*ptentry = findPrinttoolEntry(entry->modelname);
	return (ptentry && checkGsDriver(ptentry->m_gsdriver));
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
