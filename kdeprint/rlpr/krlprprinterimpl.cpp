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

#include "krlprprinterimpl.h"
#include "kprinter.h"
#include "kmfactory.h"

#include <qfile.h>
#include <kstddirs.h>
#include <kprocess.h>
#include <kconfig.h>

KRlprPrinterImpl::KRlprPrinterImpl(QObject *parent, const char *name)
: KPrinterImpl(parent,name)
{
}

KRlprPrinterImpl::~KRlprPrinterImpl()
{
}

bool KRlprPrinterImpl::printFiles(KPrinter *printer, const QStringList& files)
{
	QString	host(printer->option("host")), queue(printer->option("queue"));
	if (host.isEmpty() && queue.isEmpty())
	{
		KProcess	proc;
		proc << KStandardDirs::findExe("rlpr");
		proc << QString::fromLatin1("-H%1").arg(host) << QString::fromLatin1("-P%1").arg(queue) << QString::fromLatin1("-#%1").arg(printer->numCopies());

		// proxy settings
		KConfig	*conf = KMFactory::self()->printConfig();
		conf->setGroup("RLPR");
		QString	host = conf->readEntry("ProxyHost",QString::null), port = conf->readEntry("ProxyPort",QString::null);
		if (!host.isEmpty())
		{
			proc << QString::fromLatin1("-X%1").arg(host);
			if (!port.isEmpty()) proc << QString::fromLatin1("--port=%1").arg(port);
		}

		bool 	canPrint(false);
		for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
			if (QFile::exists(*it))
			{
				proc << *it;
				canPrint = true;
			}
			else
				qDebug("File not found: %s",(*it).latin1());
		if (canPrint) return proc.start(KProcess::Block,KProcess::NoCommunication);
		else return false;
	}
	else
		return false;
}
