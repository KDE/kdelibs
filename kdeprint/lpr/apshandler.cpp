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

#include <qfile.h>
#include <qtextstream.h>
#include <qvaluestack.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>

ApsHandler::ApsHandler(KMManager *mgr)
: LprHandler("apsfilter", mgr)
{
}

bool ApsHandler::validate(PrintcapEntry *entry)
{
	return (entry->field("if").right(9) == "apsfilter");
}

KMPrinter* ApsHandler::createPrinter(PrintcapEntry *entry)
{
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
				prt->setLocation(i18n("Network Printer (%1)").arg(prt->device().protocol()));
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
	return QFile::encodeName("/usr/share/apsfilter");
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
	return loadApsDriver(config);
}

DrMain* ApsHandler::loadDbDriver(const QString&)
{
	return loadApsDriver(true);
}

DrMain* ApsHandler::loadApsDriver(bool config)
{
	QFile	f(locate("data", (config ? "kdeprint/apsdriver1" : "kdeprint/apsdriver2")));
	if (f.open(IO_ReadOnly))
	{
		DrMain	*driver = new DrMain;
		QValueStack<DrGroup*>	groups;
		QTextStream	t(&f);
		QStringList	l;
		DrListOption	*lopt(0);
		DrBase	*opt(0);

		groups.push(driver);
		driver->set("text", "APS Common Driver");
		while (!t.atEnd())
		{
			l = QStringList::split('|', t.readLine().stripWhiteSpace(), false);
			if (l.count() == 0)
				continue;
			if (l[0] == "GROUP")
			{
				DrGroup	*grp = new DrGroup;
				grp->setName(l[1]);
				grp->set("text", l[2]);
				groups.top()->addGroup(grp);
				groups.push(grp);
			}
			else if (l[0] == "ENDGROUP")
			{
				groups.pop();
			}
			else if (l[0] == "OPTION")
			{
				opt = 0;
				lopt = 0;
				if (l.count() > 3)
				{
					if (l[3] == "STRING")
						opt = new DrStringOption;
					else if (l[3] == "BOOLEAN")
					{
						lopt = new DrBooleanOption;
						opt = lopt;
					}
				}
				else
				{
					lopt = new DrListOption;
					opt = lopt;
				}
				if (opt)
				{
					opt->setName(l[1]);
					opt->set("text", l[2]);
					groups.top()->addOption(opt);
				}
			}
			else if (l[0] == "CHOICE" && lopt)
			{
				DrBase	*ch = new DrBase;
				ch->setName(l[1]);
				ch->set("text", l[2]);
				lopt->addChoice(ch);
			}
			else if (l[0] == "DEFAULT" && opt)
			{
				opt->setValueText(l[1]);
				opt->set("default", l[1]);
			}
		}
		return driver;
	}
	return NULL;
}
