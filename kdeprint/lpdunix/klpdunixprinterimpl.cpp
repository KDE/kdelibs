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

#include "klpdunixprinterimpl.h"
#include "kprinter.h"
#include "kprintprocess.h"

#include <qfile.h>
#include <kstddirs.h>
#include <klocale.h>

KLpdUnixPrinterImpl::KLpdUnixPrinterImpl(QObject *parent, const char *name)
: KPrinterImpl(parent,name)
{
}

KLpdUnixPrinterImpl::~KLpdUnixPrinterImpl()
{
}

void KLpdUnixPrinterImpl::initLpPrint(KProcess *proc, KPrinter *printer)
{
	(*proc) << "-d" << printer->printerName() << QString::fromLatin1("-n%1").arg(printer->numCopies());
}

void KLpdUnixPrinterImpl::initLprPrint(KProcess *proc, KPrinter *printer)
{
	(*proc) << "-P" << printer->printerName() << QString::fromLatin1("-#%1").arg(printer->numCopies());
}

// look for executable, starting with "lpr"
QString KLpdUnixPrinterImpl::executable()
{
	QString	exe = KStandardDirs::findExe("lpr");
	if (exe.isEmpty())
		exe = KStandardDirs::findExe("lp");
	return exe;
}

bool KLpdUnixPrinterImpl::printFiles(KPrinter *printer, const QStringList& files)
{
	QString		exe = executable();
	if (!exe.isEmpty())
	{
		KPrintProcess	*proc = new KPrintProcess;
		proc->setExecutable(exe);
		if (exe.right(3) == "lpr")
			initLprPrint(proc,printer);
		else
			initLpPrint(proc,printer);
		bool 	canPrint(false);
		for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
			if (QFile::exists(*it))
			{
				*proc << *it;
				canPrint = true;
			}
			else
				qDebug("File not found: %s",(*it).latin1());
		if (canPrint)
			if (!startPrintProcess(proc,printer))
			{
				printer->setErrorMessage(i18n("Unable to start child print process."));
				return false;
			}
			else return true;
		else
		{
			printer->setErrorMessage(i18n("No valid file was found for printing. Operation aborted."));
			return false;
		}
	}
	else
		printer->setErrorMessage(i18n("No valid print executable was found in your path. Check your installation."));
	return false;
}
