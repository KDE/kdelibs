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

#include "klprprinterimpl.h"
#include "kprinter.h"
#include "kmlprmanager.h"

#include <kstandarddirs.h>
#include <qfile.h>
#include <stdlib.h>

KLprPrinterImpl::KLprPrinterImpl(QObject *parent, const char *name, const QStringList & /*args*/)
: KPrinterImpl(parent,name)
{
	m_exepath = KStandardDirs::findExe("lpr");
}

KLprPrinterImpl::~KLprPrinterImpl()
{
}

bool KLprPrinterImpl::setupCommand(QString& cmd, KPrinter *printer)
{
	// check printer object
	if (!printer || m_exepath.isEmpty())
		return false;

	cmd = QString::fromLatin1("%1 -P %1 '-#%1'").arg(m_exepath).arg(quote(printer->printerName())).arg( printer->numCopies() );
	QString	opts = static_cast<KMLprManager*>(KMManager::self())->printOptions(printer);
	if (!opts.isEmpty())
		cmd += (" " + opts);
	return true;
}

void KLprPrinterImpl::broadcastOption(const QString& key, const QString& value)
{
	KPrinterImpl::broadcastOption(key,value);
	if (key == "kde-pagesize")
	{
		QString	pagename = QString::fromLatin1(pageSizeToPageName((KPrinter::PageSize)value.toInt()));
		KPrinterImpl::broadcastOption("PageSize",pagename);
	}
}
