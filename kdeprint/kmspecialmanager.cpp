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

#include "kmspecialmanager.h"
#include "kmmanager.h"
#include "kmprinter.h"
#include "kdeprintcheck.h"

#include <kstddirs.h>
#include <ksimpleconfig.h>

KMSpecialManager::KMSpecialManager(KMManager *parent, const char *name)
: QObject(parent,name), m_mgr(parent), m_loaded(false)
{
}

bool KMSpecialManager::savePrinters()
{
	KSimpleConfig	conf(locateLocal("data","kdeprint/specials.desktop"));

	// first clear existing groups
	conf.setGroup("General");
	int	n = conf.readNumEntry("Number",0);
	for (int i=0;i<n;i++)
		conf.deleteGroup(QString::fromLatin1("Printer %1").arg(i),true);

	// then add printers
	n = 0;
	QPtrListIterator<KMPrinter>	it(m_mgr->m_printers);
	for (;it.current();++it)
	{
		if (!it.current()->isSpecial()) continue;
		conf.setGroup(QString::fromLatin1("Printer %1").arg(n));
		conf.writeEntry("Name",it.current()->name());
		conf.writeEntry("Description",it.current()->description());
		conf.writeEntry("Comment",it.current()->location());
		conf.writeEntry("Command",it.current()->option("kde-special-command"));
		conf.writeEntry("File",it.current()->option("kde-special-file"));
		conf.writeEntry("Icon",it.current()->pixmap());
		conf.writeEntry("Extension",it.current()->option("kde-special-extension"));
		conf.writeEntry("Require",it.current()->option("kde-special-require"));
		n++;
	}
	conf.setGroup("General");
	conf.writeEntry("Number",n);

	return true;
}

bool KMSpecialManager::loadPrinters()
{
	if (m_loaded) return true;

	KSimpleConfig	conf(locate("data","kdeprint/specials.desktop"));
	conf.setGroup("General");
	int	n = conf.readNumEntry("Number",0);
	for (int i=0;i<n;i++)
	{
		QString	grpname = QString::fromLatin1("Printer %1").arg(i);
		if (!conf.hasGroup(grpname)) continue;
		conf.setGroup(grpname);
		KMPrinter	*printer = new KMPrinter;
		printer->setName(conf.readEntry("Name"));
		printer->setPrinterName(printer->name());
		printer->setDescription(conf.readEntry("Description"));
		printer->setLocation(conf.readEntry("Comment"));
		printer->setOption("kde-special-command",conf.readEntry("Command"));
		printer->setOption("kde-special-file",conf.readEntry("File"));
		printer->setOption("kde-special-extension",conf.readEntry("Extension"));
		printer->setOption("kde-special-require",conf.readEntry("Require"));
		printer->setPixmap(conf.readEntry("Icon","unknown"));
		printer->setType(KMPrinter::Special);
		if (!KdeprintChecker::check(&conf))
			printer->addType(KMPrinter::Invalid);
		printer->setState(KMPrinter::Idle);
		m_mgr->addPrinter(printer);
	}

	return true;
}

void KMSpecialManager::refresh()
{
	if (!m_loaded)
		loadPrinters();
	else
	{
		QPtrListIterator<KMPrinter>	it(m_mgr->m_printers);
		for (;it.current();++it)
			if (it.current()->isSpecial())
			{
				it.current()->setDiscarded(false);
				it.current()->setType(KMPrinter::Special);
				if (KdeprintChecker::check(QStringList::split(',',it.current()->option("kde-special-require"),false)))
					it.current()->addType(KMPrinter::Invalid);
			}
	}
}
