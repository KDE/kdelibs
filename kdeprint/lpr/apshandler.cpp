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

#include "apshandler.h"
#include "driver.h"
#include "printcapentry.h"
#include "kmprinter.h"
#include "lprsettings.h"
#include "kmmanager.h"
#include "util.h"
#include "kprinter.h"

#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>
#include <qvaluestack.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>

#include <sys/types.h>
#include <sys/stat.h>

ApsHandler::ApsHandler(KMManager *mgr)
: LprHandler("apsfilter", mgr)
{
	m_counter = 1;
}

bool ApsHandler::validate(PrintcapEntry *entry)
{
	return (entry->field("if").right(9) == "apsfilter");
}

KMPrinter* ApsHandler::createPrinter(PrintcapEntry *entry)
{
	entry->comment = QString::fromLatin1("# APS%1_BEGIN:printer%2").arg(m_counter).arg(m_counter);
	entry->postcomment = QString::fromLatin1("# APS%1_END - don't delete this").arg(m_counter);
	m_counter++;
	return LprHandler::createPrinter(entry);
}

bool ApsHandler::completePrinter(KMPrinter *prt, PrintcapEntry *entry, bool shortmode)
{
	if (LprHandler::completePrinter(prt, entry, shortmode))
	{
		if (!shortmode)
		{
			QMap<QString,QString>	opts = loadResources(entry);
			if (opts.contains("PRINTER"))
			{
				prt->setDescription(i18n("APS Driver (%1)").arg(opts["PRINTER"]));
				prt->setDriverInfo(prt->description());
			}
		}
		if (prt->device().isEmpty())
		{
			QString	smbname(sysconfDir() + "/" + prt->printerName() + "/smbclient.conf");
			QString	ncpname(sysconfDir() + "/" + prt->printerName() + "/netware.conf");
			if (QFile::exists(smbname))
			{
				QMap<QString,QString>	opts = loadVarFile(smbname);
				if (opts.count() == 0)
					prt->setDevice(KURL("smb://<unknown>/<unknown>"));
				else
				{
					KURL	uri;
					uri.setProtocol("smb");
					if (opts["SMB_WORKGROUP"].isEmpty())
					{
						uri.setHost(opts["SMB_SERVER"]);
						uri.setPath("/" + opts["SMB_PRINTER"]);
					}
					else
					{
						uri.setHost(opts["SMB_WORKGROUP"]);
						uri.setPath("/" + opts["SMB_SERVER"] + "/" + opts["SMB_PRINTER"]);
					}
					uri.setUser(opts["SMB_USER"]);
					uri.setPass(opts["SMB_PASSWD"]);
					prt->setDevice(uri);
				}
			}
			else if (QFile::exists(ncpname))
			{
				QMap<QString,QString>	opts = loadVarFile(ncpname);
				if (opts.count() == 0)
					prt->setDevice(KURL("ncp://<unknown>/<unknown>"));
				else
				{
					KURL	uri;
					uri.setProtocol("ncp");
					uri.setHost(opts["NCP_SERVER"]);
					uri.setPath("/" + opts["NCP_PRINTER"]);
					uri.setUser(opts["NCP_USER"]);
					uri.setPass(opts["NCP_PASSWD"]);
					prt->setDevice(uri);
				}
			}
			if (!prt->device().isEmpty())
				prt->setLocation(i18n("Network printer (%1)").arg(prt->device().protocol()));
		}
		return true;
	}
	return false;
}

QString ApsHandler::sysconfDir()
{
	return QFile::encodeName("/etc/apsfilter");
}

QString ApsHandler::shareDir()
{
	return driverDirectory();
}

QString ApsHandler::driverDirInternal()
{
	return locateDir("apsfilter/setup", "/usr/share:/usr/local/share:/opt/share");
}

QMap<QString,QString> ApsHandler::loadResources(PrintcapEntry *entry)
{
	return loadVarFile(sysconfDir() + "/" + (entry ? entry->name : QString::null) + "/apsfilterrc");
}

QMap<QString,QString> ApsHandler::loadVarFile(const QString& filename)
{
	QMap<QString,QString>	opts;
	QFile	f(filename);
	if (f.open(IO_ReadOnly))
	{
		QTextStream	t(&f);
		QString	line;
		int	p(-1);
		while (!t.atEnd())
		{
			line = t.readLine().stripWhiteSpace();
			if (line.isEmpty() || line[0] == '#' || (p = line.find('=')) == -1)
				continue;
			QString	variable = line.left(p).stripWhiteSpace();
			QString	value = line.mid(p+1).stripWhiteSpace();
			if (!value.isEmpty() && value[0] == '\'')
				value = value.mid(1, value.length()-2);
			opts[variable] = value;
		}
	}
	return opts;
}

DrMain* ApsHandler::loadDriver(KMPrinter *prt, PrintcapEntry *entry, bool config)
{
	DrMain	*driver = loadApsDriver(config);
	if (driver /* && config */ )    // Load resources in all case, to get the correct page size
	{
		QMap<QString,QString>	opts = loadResources(entry);
		if ( !config && opts.contains( "PAPERSIZE" ) )
		{
			// this is needed to keep applications informed
			// about the current selected page size
			opts[ "PageSize" ] = opts[ "PAPERSIZE" ];

			// default page size needs to be set to the actual
			// value of the printer driver, otherwise it's blocked
			// to A4
			DrBase *opt = driver->findOption( "PageSize" );
			if ( opt )
				opt->set( "default", opts[ "PageSize" ] );
		}
		driver->setOptions(opts);
		driver->set("gsdriver", opts["PRINTER"]);
	}
	return driver;
}

DrMain* ApsHandler::loadDbDriver(const QString& s)
{
	int	p = s.find('/');
	DrMain	*driver = loadApsDriver(true);
	if (driver)
		driver->set("gsdriver", s.mid(p+1));
	return driver;
}

DrMain* ApsHandler::loadApsDriver(bool config)
{
	DrMain	*driver = loadToolDriver(locate("data", (config ? "kdeprint/apsdriver1" : "kdeprint/apsdriver2")));
	if (driver)
		driver->set("text", "APS Common Driver");
	return driver;
}

void ApsHandler::reset()
{
	m_counter = 1;
}

PrintcapEntry* ApsHandler::createEntry(KMPrinter *prt)
{
	QString	prot = prt->device().protocol();
	if (prot != "parallel" && prot != "lpd" && prot != "smb" && prot != "ncp")
	{
		manager()->setErrorMsg(i18n("Unsupported backend: %1.").arg(prot));
		return NULL;
	}
	QString	path = sysconfDir() + "/" + prt->printerName();
	if (!KStandardDirs::makeDir(path, 0755))
	{
		manager()->setErrorMsg(i18n("Unable to create directory %1.").arg(path));
		return NULL;
	}
	if (prot == "smb" || prot == "ncp")
	{
		// either "smb" or "ncp"
		QFile::remove(path + "/smbclient.conf");
		QFile::remove(path + "/netware.conf");
		QFile	f;
		if (prot == "smb")
		{
			f.setName(path + "/smbclient.conf");
			if (f.open(IO_WriteOnly))
			{
				QTextStream	t(&f);
				QString	w, s, p;
				urlToSmb(prt->device(), w, s, p);
				if (w.isEmpty())
				{
					manager()->setErrorMsg(i18n("Missing element: %1.").arg("Workgroup"));
					return NULL;
				}
				t << "SMB_SERVER='" << s << "'" << endl;
				t << "SMB_PRINTER='" << p << "'" << endl;
				t << "SMB_IP=''" << endl;
				t << "SMB_WORKGROUP='" << w << "'" << endl;
				t << "SMB_BUFFER=1400" << endl;
				t << "SMB_FLAGS='-N'" << endl;
				if (!prt->device().user().isEmpty())
				{
					t << "SMB_USER='" << prt->device().user() << "'" << endl;
					t << "SMB_PASSWD='" << prt->device().pass() << "'" << endl;
				}
			}
			else
			{
				manager()->setErrorMsg(i18n("Unable to create the file %1.").arg(f.name()));
				return NULL;
			}
		}
		else
		{
			f.setName(path + "/netware.conf");
			if (f.open(IO_WriteOnly))
			{
				QTextStream	t(&f);
				t << "NCP_SERVER='" << prt->device().host() << "'" << endl;
				t << "NCP_PRINTER='" << prt->device().path().mid(1) << "'" << endl;
				if (!prt->device().user().isEmpty())
				{
					t << "NCP_USER='" << prt->device().user() << "'" << endl;
					t << "NCP_PASSWD='" << prt->device().pass() << "'" << endl;
				}
			}
			else
			{
				manager()->setErrorMsg(i18n("Unable to create the file %1.").arg(f.name()));
				return NULL;
			}
		}
		// change file permissions
		::chmod(QFile::encodeName(f.name()).data(), S_IRUSR|S_IWUSR);
	}
	PrintcapEntry	*entry = LprHandler::createEntry(prt);
	if (!entry)
	{
		entry = new PrintcapEntry;
		entry->addField("lp", Field::String, "/dev/null");
	}
	QString	sd = LprSettings::self()->baseSpoolDir() + "/" + prt->printerName();
	entry->addField("af", Field::String, sd + "/acct");
	entry->addField("lf", Field::String, sd + "/log");
	entry->addField("if", Field::String, sysconfDir() + "/basedir/bin/apsfilter");
	entry->comment = QString::fromLatin1("# APS%1_BEGIN:printer%2").arg(m_counter).arg(m_counter);
	entry->postcomment = QString::fromLatin1("# APS%1_END").arg(m_counter);
	m_counter++;
	return entry;
}

bool ApsHandler::savePrinterDriver(KMPrinter *prt, PrintcapEntry *entry, DrMain *driver, bool*)
{
	if (driver->get("gsdriver").isEmpty())
	{
		manager()->setErrorMsg(i18n("The APS driver is not defined."));
		return false;
	}
	QFile	f(sysconfDir() + "/" + prt->printerName() + "/apsfilterrc");
	if (f.open(IO_WriteOnly))
	{
		QTextStream	t(&f);
		t << "# File generated by KDEPrint" << endl;
		t << "PRINTER='" << driver->get("gsdriver") << "'" << endl;
		QValueStack<DrGroup*>	stack;
		stack.push(driver);
		while (stack.count() > 0)
		{
			DrGroup	*grp = stack.pop();
			QPtrListIterator<DrGroup>	git(grp->groups());
			for (; git.current(); ++git)
				stack.push(git.current());
			QPtrListIterator<DrBase>	oit(grp->options());
			QString	value;
			for (; oit.current(); ++oit)
			{
				value = oit.current()->valueText();
				switch (oit.current()->type())
				{
					case DrBase::Boolean:
						if (value == "true")
							t << oit.current()->name() << "='" << value << "'" << endl;
						break;
					case DrBase::List:
						if (value != "(empty)")
							t << oit.current()->name() << "='" << value << "'" << endl;
						break;
					case DrBase::String:
						if (!value.isEmpty())
							t << oit.current()->name() << "='" << value << "'" << endl;
						break;
					default:
						break;
				}
			}
		}
		return true;
	}
	else
	{
		manager()->setErrorMsg(i18n("Unable to create the file %1.").arg(f.name()));
		return false;
	}
}

bool ApsHandler::removePrinter(KMPrinter*, PrintcapEntry *entry)
{
	QString	path(sysconfDir() + "/" + entry->name);
	QFile::remove(path + "/smbclient.conf");
	QFile::remove(path + "/netware.conf");
	QFile::remove(path + "/apsfilterrc");
	if (!QDir(path).rmdir(path))
	{
		manager()->setErrorMsg(i18n("Unable to remove directory %1.").arg(path));
		return false;
	}
	return true;
}

QString ApsHandler::printOptions(KPrinter *printer)
{
	QString	optstr;
	QMap<QString,QString>	opts = printer->options();
	for (QMap<QString,QString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
	{
		if (it.key().startsWith("kde-") || it.key().startsWith("_kde-") || it.key().startsWith( "app-" ))
			continue;
		optstr.append((*it)).append(":");
	}
	if (!optstr.isEmpty())
	{
		optstr = optstr.left(optstr.length()-1);
		optstr.prepend("-C '").append("'");
	}
	return optstr;
}
