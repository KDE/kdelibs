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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "klpdprinterimpl.h"
#include "kprinter.h"

#include <qfile.h>
#include <kstandarddirs.h>
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

bool KLpdPrinterImpl::setupCommand(QString& cmd, KPrinter *printer)
{
	QString	exestr = executable();
	if (exestr.isEmpty())
	{
		printer->setErrorMessage(i18n("The <b>%1</b> executable could not be found in your path. Check your installation.").arg("lpr"));
		return false;
	}
	cmd = QString::fromLatin1("%1 -P %2 '-#%3'").arg(exestr).arg(quote(printer->printerName())).arg(printer->numCopies());
	return true;
}
