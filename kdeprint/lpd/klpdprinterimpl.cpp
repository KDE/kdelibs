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

#include "klpdprinterimpl.h"
#include "kprinter.h"
#include "kprintprocess.h"

#include <qfile.h>
#include <kstddirs.h>
#include <klocale.h>

KLpdPrinterImpl::KLpdPrinterImpl(QObject *parent, const char *name)
: KPrinterImpl(parent,name)
{
}

KLpdPrinterImpl::~KLpdPrinterImpl()
{
}

QString KLpdPrinterImpl::executable()
{
	return KStandardDirs::findExe("lpr");
}

bool KLpdPrinterImpl::printFiles(KPrinter *printer, const QStringList& files)
{
	KPrintProcess	proc;
	QString	exestr = executable();
	if (exestr.isEmpty())
	{
		printer->setErrorMessage(i18n("The <b>%1</b> executable could not be found in your path. Check your installation.").arg("lpr"));
		return false;
	}
	proc << exestr;
	proc << "-P" << printer->printerName() << QString::fromLatin1("-#%1").arg(printer->numCopies());
	bool 	canPrint(false);
	for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
		if (QFile::exists(*it))
		{
			proc << *it;
			canPrint = true;
		}
		else
			qDebug("File not found: %s",(*it).latin1());
	if (canPrint)
		if (!proc.print())
		{
			QString	msg = proc.errorMessage();
			printer->setErrorMessage(i18n("The execution of <b>%1</b> failed with message:<p>%2</p>").arg("lpr").arg(proc.errorMessage()));
			return false;
		}
		else return true;
	else
	{
		printer->setErrorMessage(i18n("No valid file was found for printing. Operation aborted."));
		return false;
	}
}
