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

#include "susehandler.h"
#include "printcapentry.h"
#include "kmprinter.h"
#include "lprsettings.h"
#include "driver.h"
#include "kmmanager.h"
#include "kprinter.h"

#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>

SuSEHandler::SuSEHandler(KMManager *mgr)
: LprHandler("suse", mgr)
{
}

bool SuSEHandler::validate(PrintcapEntry *entry)
{
	QString	cm=  entry->field("cm");
	return (cm.startsWith("lpdfilter"));
}

bool SuSEHandler::completePrinter(KMPrinter *prt, PrintcapEntry *entry, bool shortmode)
{
	QString	cm, lp;
	int	p;

	lp = entry->field("lp");
	cm = entry->field("cm");
	QStringList	l = QStringList::split(' ', cm, false);
	if (l.count() < 1 || l[0] != "lpdfilter")
		return false;

	if (lp.isEmpty() || (lp[0] != '|' && cm.find("redirect=") == -1))
		LprHandler::completePrinter(prt, entry, shortmode);
	else
	{
		if ((p = lp.find("samba_print")) != -1)
		{
			// SMB printer
			prt->setLocation(i18n("Network printer (%1)").arg("smb"));
			p += 11;
			QRegExp	re("//([^/]+)/\\\\\"([^\"]+)\\\\\"\\s+\\\\\"([^\"]+)\\\\\"\\s+-U\\s+\\\\\"([^\"]+)\\\\\"");
			if (lp.mid(p).find(re) != -1)
			{
				KURL	url = ("smb://" + re.cap(1) + "/" + re.cap(2));
				url.setUser(re.cap(4));
				url.setPass(re.cap(3));
				prt->setDevice(url);
			}
		}
		else if ((p = lp.find("novell_print")) != -1)
		{
			// NCP printer
			prt->setLocation(i18n("Network printer (%1)").arg("Novell"));
			p += 12;
			QRegExp	re("-S\\s+(\\S+)\\s+-U\\s+\\\\\"([^\"]+)\\\\\"\\s+-P\\s+\\\\\"([^\"]+)\\\\\"\\s+-q\\s+\\\\\"([^\"]+)\\\\\"");
			if (lp.mid(p).find(re) != -1)
			{
				KURL	url = ("ncp://" + re.cap(1) + "/" + re.cap(4));
				url.setUser(re.cap(2));
				url.setPass(re.cap(3));
				prt->setDevice(url);
			}
		}
		else if ((p = cm.find("redirect=")) != -1)
		{
			QString	redirprt = cm.mid(p+9, cm.find(' ', p));
			prt->setLocation(i18n("Print queue redirected to %1").arg(redirprt));
			prt->setDevice(KURL("redirect://" + redirprt));
		}
		else
			prt->setLocation(i18n("Unknown (unrecognized entry)"));
	}

	prt->setDescription(i18n("SuSE printer (%1)").arg(QFile::exists(dataDir(prt->printerName())+"/yast2") ? i18n("configured with YaST"): i18n("unknown")));
	prt->setDriverInfo(cm);
	return true;
}

DrMain* SuSEHandler::loadDriver(KMPrinter *prt, PrintcapEntry *entry, bool config)
{
	manager()->setErrorMsg("This operation is currently not supported.");
	return NULL;
}

DrMain* SuSEHandler::loadDbDriver(const QString& s)
{
	manager()->setErrorMsg("This operation is currently not supported.");
	return NULL;
}

QString SuSEHandler::driverDirInternal()
{
	return locateDir("lpdfilter", "/usr/lib:/usr/local/lib:/opt/lib:/usr/libexec:/usr/local/libexec:/opt/libexec");
}

QString SuSEHandler::dataDir(const QString& prname)
{
	return QString::fromLatin1("/etc/lpdfilter/%1").arg(prname);
}

PrintcapEntry* SuSEHandler::createEntry(KMPrinter *prt)
{
	return NULL;
	/*
	QString	prot = prt->device().protocol();
	if (prot != "parallel" && prot != "lpd" && prot != "smb" && prot != "socket")
	{
		manager()->setErrorMsg(i18n("Unsupported backend: %1.").arg(prot));
		return NULL;
	}
	PrintcapEntry	*entry = new PrintcapEntry;

	return entry;
	*/
}

bool SuSEHandler::savePrinterDriver(KMPrinter*, PrintcapEntry *entry, DrMain *driver, bool *mustSave)
{
	return false;
}

QString SuSEHandler::printOptions(KPrinter *printer)
{
	QString	optstr;
	return optstr;
}
