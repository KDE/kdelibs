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

#include <qfile.h>
#include <kstandarddirs.h>
#include <klocale.h>

KLpdUnixPrinterImpl::KLpdUnixPrinterImpl(QObject *parent, const char *name)
: KPrinterImpl(parent,name)
{
}

KLpdUnixPrinterImpl::~KLpdUnixPrinterImpl()
{
}

void KLpdUnixPrinterImpl::initLpPrint(QString& cmd, KPrinter *printer)
{
	cmd += QString::fromLatin1(" -d %1 -n%2").arg(quote(printer->printerName())).arg(printer->numCopies());
}

void KLpdUnixPrinterImpl::initLprPrint(QString& cmd, KPrinter *printer)
{
	cmd += QString::fromLatin1(" -P %1 '-#%2'").arg(quote(printer->printerName())).arg(printer->numCopies());
}

// look for executable, starting with "lpr"
QString KLpdUnixPrinterImpl::executable()
{
	QString	exe = KStandardDirs::findExe("lpr");
	if (exe.isEmpty())
		exe = KStandardDirs::findExe("lp");
	return exe;
}

bool KLpdUnixPrinterImpl::setupCommand(QString& cmd, KPrinter *printer)
{
	QString		exe = executable();
	if (!exe.isEmpty())
	{
		cmd = exe;
		if (exe.right(3) == "lpr")
			initLprPrint(cmd,printer);
		else
			initLpPrint(cmd,printer);
		return true;
	}
	else
		printer->setErrorMessage(i18n("No valid print executable was found in your path. Check your installation."));
	return false;
}
