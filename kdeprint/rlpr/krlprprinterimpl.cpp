/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
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

#include "krlprprinterimpl.h"
#include "kprinter.h"
#include "kmfactory.h"
#include "kmmanager.h"
#include "kmprinter.h"

#include <qfile.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <klocale.h>

KRlprPrinterImpl::KRlprPrinterImpl(QObject *parent, const char *name, const QStringList & /*args*/)
    : KPrinterImpl(parent)
{
}

KRlprPrinterImpl::~KRlprPrinterImpl()
{
}

bool KRlprPrinterImpl::setupCommand(QString& cmd, KPrinter *printer)
{
	// retrieve the KMPrinter object, to get host and queue name
	KMPrinter	*rpr = KMFactory::self()->manager()->findPrinter(printer->printerName());
	if (!rpr)
		return false;

	QString	host(rpr->option("host")), queue(rpr->option("queue"));
	if (!host.isEmpty() && !queue.isEmpty())
	{
		QString		exestr = KStandardDirs::findExe("rlpr");
		if (exestr.isEmpty())
		{
			printer->setErrorMessage(i18n("The <b>%1</b> executable could not be found in your path. Check your installation.").arg("rlpr"));
			return false;
		}

		cmd = QString::fromLatin1("%1 -H %2 -P %3 -\\#%4").arg(exestr).arg(quote(host)).arg(quote(queue)).arg(printer->numCopies());

		// proxy settings
		KConfig	*conf = KMFactory::self()->printConfig();
		conf->setGroup("RLPR");
		QString	host = conf->readEntry("ProxyHost",QString::null), port = conf->readEntry("ProxyPort",QString::null);
		if (!host.isEmpty())
		{
			cmd.append(" -X ").append(quote(host));
			if (!port.isEmpty()) cmd.append(" --port=").append(port);
		}

		return true;
	}
	else
	{
		printer->setErrorMessage(i18n("The printer is incompletely defined. Try to reinstall it."));
		return false;
	}
}
