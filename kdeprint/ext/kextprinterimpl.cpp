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

#include "kextprinterimpl.h"
#include "kprinter.h"
#include "kprintprocess.h"

#include <qfile.h>
#include <kstddirs.h>
#include <kconfig.h>
#include <klocale.h>

KExtPrinterImpl::KExtPrinterImpl(QObject *parent, const char *name)
: KPrinterImpl(parent,name)
{
}

KExtPrinterImpl::~KExtPrinterImpl()
{
}

void KExtPrinterImpl::preparePrinting(KPrinter *printer)
{
	printer->setOption("kde-qtcopies",QString::number(printer->numCopies()));
}

bool KExtPrinterImpl::printFiles(KPrinter *printer, const QStringList& files)
{
	QStringList	args = QStringList::split(QRegExp("\\s"),printer->option("kde-printcommand"),false);
	if (args.count() == 0)
	{
		printer->setErrorMessage(i18n("Empty print command."));
		return false;
	}
	QString	exe = KStandardDirs::findExe(args[0]);
	if (exe.isEmpty())
	{
		printer->setErrorMessage(i18n("The <b>%1</b> executable could not be found in your path. Check your installation.").arg(args[0]));
		return false;
	}
	args[0] = exe;

	KPrintProcess	*proc = new KPrintProcess;
	// add command line
	for (QStringList::ConstIterator it=args.begin(); it!=args.end(); ++it)
		*proc << *it;
	return startPrinting(proc,printer,files);
}
