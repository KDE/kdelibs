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

#include "kmlpdunixmanager.h"
#include "kmfactory.h"
#include "kmprinter.h"

#include <qfile.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <stdlib.h>

/*****************
 * Utility class *
 *****************/
class KTextBuffer
{
public:
	KTextBuffer(QIODevice *dev) : m_stream(dev) {}
	bool eof() const { return (m_stream.eof() && m_linebuf.isEmpty()); }
	QString readLine();
	void unreadLine(const QString& l) { m_linebuf = l; }
private:
	QTextStream	m_stream;
	QString		m_linebuf;
};

QString KTextBuffer::readLine()
{
	QString	line;
	if (!m_linebuf.isEmpty())
	{
		line = m_linebuf;
		m_linebuf = QString::null;
	}
	else
		line = m_stream.readLine();
	return line;
}

/*****************************
 * Various parsing functions *
 *****************************/

// Extract a line from a KTextBuffer:
//	'#' -> comments
//	'\' -> line continue
//	':' or '|' -> line continue (LPRng)
//
// New entry is detected by a line which have first character different from
// '#', '|', ':'. The line is then put back in the IODevice.
QString readLine(KTextBuffer& t)
{
	QString	line, buffer;
	bool	lineContinue(false);

	while (!t.eof())
	{
		buffer = t.readLine().stripWhiteSpace();
		if (buffer.isEmpty() || buffer[0] == '#')
			continue;
		if (buffer[0] == '|' || buffer[0] == ':' || lineContinue || line.isEmpty())
		{
			line.append(buffer);
			if (line.right(1) == "\\")
			{
				line.truncate(line.length()-1);
				line = line.stripWhiteSpace();
				lineContinue = true;
			}
			else
				lineContinue = false;
		}
		else
		{
			t.unreadLine(buffer);
			break;
		}
	}
	return line;
}

// extact an entry (printcap-like)
QMap<QString,QString> readEntry(KTextBuffer& t)
{
	QString	line = readLine(t);
	QMap<QString,QString>	entry;

	if (!line.isEmpty())
	{
		QStringList	l = QStringList::split(':',line,false);
		if (l.count() > 0)
		{
			int 	p(-1);
			if ((p=l[0].find('|')) != -1)
				entry["printer-name"] = l[0].left(p);	// only keep first name (discard aliases
			else
				entry["printer-name"] = l[0];
			for (uint i=1; i<l.count(); i++)
				if ((p=l[i].find('=')) != -1)
					entry[l[i].left(p).stripWhiteSpace()] = l[i].right(l[i].length()-p-1).stripWhiteSpace();
				else
					entry[l[i].stripWhiteSpace()] = QString::null;
		}
	}
	return entry;
}

// create basic printer from entry
KMPrinter* createPrinter(const QMap<QString,QString>& entry)
{
	KMPrinter	*printer = new KMPrinter();
	printer->setName(entry["printer-name"]);
	printer->setPrinterName(entry["printer-name"]);
	printer->setType(KMPrinter::Printer);
	printer->setState(KMPrinter::Idle);
	return printer;
}
KMPrinter* createPrinter(const QString& prname)
{
	QMap<QString,QString>	map;
	map["printer-name"] = prname;
	return createPrinter(map);
}

// this function support LPRng piping feature, it defaults to
// /etc/printcap in any other cases (basic support)
QString getPrintcapFileName()
{
	// check if LPRng system
	QString	printcap("/etc/printcap");
	QFile	f("/etc/lpd.conf");
	if (f.exists() && f.open(IO_ReadOnly))
	{
		kdDebug() << "/etc/lpd.conf found: probably LPRng system" << endl;
		QTextStream	t(&f);
		QString		line;
		while (!t.eof())
		{
			line = t.readLine().stripWhiteSpace();
			if (line.startsWith("printcap_path="))
			{
				kdDebug() << "printcap_path entry found: " << line << endl;
				QString	pcentry = line.mid(14).stripWhiteSpace();
				kdDebug() << "printcap_path value: " << pcentry << endl;
				if (pcentry[0] == '|')
				{ // printcap through pipe
					printcap = locateLocal("tmp","printcap");
					QString	cmd = QString::fromLatin1("echo \"all\" | %1 > %2").arg(pcentry.mid(1)).arg(printcap);
					kdDebug() << "printcap obtained through pipe" << endl << "executing: " << cmd << endl;
					::system(cmd.local8Bit());
				}
				break;
			}
		}
	}
	kdDebug() << "printcap file returned: " << printcap << endl;
	return printcap;
}

// "/etc/printcap" file parsing (Linux/LPR)
void KMLpdUnixManager::parseEtcPrintcap()
{
	QFile	f(getPrintcapFileName());
	if (f.exists() && f.open(IO_ReadOnly))
	{
		KTextBuffer	t(&f);
		QMap<QString,QString>	entry;

		while (!t.eof())
		{
			entry = readEntry(t);
			if (entry.isEmpty() || !entry.contains("printer-name") || entry.contains("server"))
				continue;
			if (entry["printer-name"] == "all")
			{
				if (entry.contains("all"))
				{
					// find separator
					int	p = entry["all"].find(QRegExp("[^a-zA-Z0-9_\\s-]"));
					if (p != -1)
					{
						QChar	c = entry["all"][p];
						QStringList	prs = QStringList::split(c,entry["all"],false);
						for (QStringList::ConstIterator it=prs.begin(); it!=prs.end(); ++it)
						{
							KMPrinter	*printer = ::createPrinter(*it);
							printer->setDescription(i18n("Description unavailable"));
							addPrinter(printer);
						}
					}
				}
			}
			else
			{
				KMPrinter	*printer = ::createPrinter(entry);
				if (entry.contains("rm"))
					printer->setDescription(i18n("Remote printer queue on %1").arg(entry["rm"]));
				else
					printer->setDescription(i18n("Local printer"));
				addPrinter(printer);
			}
		}
	}
}

// helper function for NIS support in Solaris-2.6 (use "ypcat printers.conf.byname")
QString getEtcPrintersConfName()
{
	QString	printersconf("/etc/printers.conf");
	if (!QFile::exists(printersconf))
	{
		// standard file not found, try NIS
		printersconf = locateLocal("tmp","printers.conf");
		QString	cmd = QString::fromLatin1("ypcat printers.conf.byname > %1").arg(printersconf);
		kdDebug() << "printers.conf obtained from NIS server: " << cmd << endl;
		::system(QFile::encodeName(cmd));
	}
	return printersconf;
}

// "/etc/printers.conf" file parsing (Solaris 2.6)
void KMLpdUnixManager::parseEtcPrintersConf()
{
	QFile	f(getEtcPrintersConfName());
	if (f.exists() && f.open(IO_ReadOnly))
	{
		KTextBuffer	t(&f);
		QMap<QString,QString>	entry;
		QString		default_printer;

		while (!t.eof())
		{
			entry = readEntry(t);
			if (entry.isEmpty() || !entry.contains("printer-name"))
				continue;
			QString	prname = entry["printer-name"];
			if (prname == "_default")
			{
				if (entry.contains("use"))
					default_printer = entry["use"];
			}
			else if (prname != "_all")
			{
				KMPrinter	*printer = ::createPrinter(entry);
				if (entry.contains("bsdaddr"))
				{
					QStringList	l = QStringList::split(',',entry["bsdaddr"],false);
					printer->setDescription(i18n("Remote printer queue on %1").arg(l[0]));
				}
				else
					printer->setDescription(i18n("Local printer"));
				addPrinter(printer);
			}
		}

		if (!default_printer.isEmpty())
			setSoftDefault(findPrinter(default_printer));
	}
}

// "/etc/lp/printers/" directory parsing (Solaris non-2.6)
void KMLpdUnixManager::parseEtcLpPrinters()
{
	QDir	d("/etc/lp/printers");
	const QFileInfoList	*prlist = d.entryInfoList(QDir::Dirs);
	if (!prlist)
		return;

	QFileInfoListIterator	it(*prlist);
	for (;it.current();++it)
	{
		if (it.current()->fileName() == "." || it.current()->fileName() == "..")
			continue;
		QFile	f(it.current()->absFilePath() + "/configuration");
		if (f.exists() && f.open(IO_ReadOnly))
		{
			KTextBuffer	t(&f);
			QString		line, remote;
			while (!t.eof())
			{
				line = readLine(t);
				if (line.isEmpty()) continue;
				if (line.startsWith("Remote:"))
				{
					QStringList	l = QStringList::split(':',line,false);
					if (l.count() > 1) remote = l[1];
				}
			}
			KMPrinter	*printer = new KMPrinter;
			printer->setName(it.current()->fileName());
			printer->setPrinterName(it.current()->fileName());
			printer->setType(KMPrinter::Printer);
			printer->setState(KMPrinter::Idle);
			if (!remote.isEmpty())
				printer->setDescription(i18n("Remote printer queue on %1").arg(remote));
			else
				printer->setDescription(i18n("Local printer"));
			addPrinter(printer);
		}
	}
}

// "/etc/lp/member/" directory parsing (HP-UX)
void KMLpdUnixManager::parseEtcLpMember()
{
	QDir	d("/etc/lp/member");
	const QFileInfoList	*prlist = d.entryInfoList(QDir::Files);
	if (!prlist)
		return;

	QFileInfoListIterator	it(*prlist);
	for (;it.current();++it)
	{
		KMPrinter	*printer = new KMPrinter;
		printer->setName(it.current()->fileName());
		printer->setPrinterName(it.current()->fileName());
		printer->setType(KMPrinter::Printer);
		printer->setState(KMPrinter::Idle);
		printer->setDescription(i18n("Local printer"));
		addPrinter(printer);
	}
}

// "/usr/spool/lp/interfaces/" directory parsing (IRIX 6.x)
void KMLpdUnixManager::parseSpoolInterface()
{
	QDir	d("/usr/spool/interfaces/lp");
	const QFileInfoList	*prlist = d.entryInfoList(QDir::Files);
	if (!prlist)
		return;

	QFileInfoListIterator	it(*prlist);
	for (;it.current();++it)
	{
		QFile	f(it.current()->absFilePath());
		if (f.exists() && f.open(IO_ReadOnly))
		{
			KTextBuffer	t(&f);
			QString		line, remote;

			while (!t.eof())
			{
				line = t.readLine().stripWhiteSpace();
				if (line.startsWith("HOSTNAME"))
				{
					QStringList	l = QStringList::split('=',line,false);
					if (l.count() > 1) remote = l[1];
				}
			}

			KMPrinter	*printer = new KMPrinter;
			printer->setName(it.current()->fileName());
			printer->setPrinterName(it.current()->fileName());
			printer->setType(KMPrinter::Printer);
			printer->setState(KMPrinter::Idle);
			if (!remote.isEmpty())
				printer->setDescription(i18n("Remote printer queue on %1").arg(remote));
			else
				printer->setDescription(i18n("Local printer"));
			addPrinter(printer);
		}
	}
}

//*********************************************************************************************************

KMLpdUnixManager::KMLpdUnixManager(QObject *parent, const char *name)
: KMManager(parent,name)
{
	m_loaded = false;
}

void KMLpdUnixManager::listPrinters()
{
	// load only once, if already loaded, just keep them (remove discard flag)
	if (!m_loaded)
	{
		parseEtcPrintcap();
		parseEtcPrintersConf();
		parseEtcLpPrinters();
		parseEtcLpMember();
		parseSpoolInterface();
		m_loaded = true;
	}
	else
		discardAllPrinters(false);
}
