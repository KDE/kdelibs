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
#include "kxmlcommand.h"
#include "driver.h"

#include <qfile.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <ksimpleconfig.h>
#include <klocale.h>
#include <kdebug.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

KMSpecialManager::KMSpecialManager(KMManager *parent, const char *name)
: QObject(parent,name), m_mgr(parent), m_loaded(false)
{
}

bool KMSpecialManager::savePrinters()
{
	// for root, use a global location.
	QString	confname;
	if (getuid() == 0)
	{
		confname = locate("data", "kdeprint/specials.desktop");
		if (confname.startsWith(KGlobal::dirs()->localkdedir()))
		{
			// seems there's a problem here
			m_mgr->setErrorMsg(i18n("A file share/kdeprint/specials.desktop was found in your "
						"local KDE directory. This file probably comes from a previous KDE "
						"release and should be removed in order to manage global pseudo "
						"printers."));
			return false;
		}
	}
	else
		confname = locateLocal("data","kdeprint/specials.desktop");

	KSimpleConfig	conf(confname);

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
		if (!it.current()->isSpecial() || it.current()->isVirtual()) continue;
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

	// set read access for anybody in case of global location
	if (getuid() == 0)
	{
		conf.sync();
		::chmod(QFile::encodeName(confname), S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	}

	// force reload on next update
	m_loaded = false;

	return true;
}

bool KMSpecialManager::loadPrinters()
{
	if (m_loaded) return true;

	bool	result(true);
	QString	localDir = KGlobal::dirs()->localkdedir();
	QStringList	files = KGlobal::dirs()->findAllResources("data", "kdeprint/specials.desktop");
	// local files should processed last, so we need to reorder the list
	// and put local files at the end
	QStringList	orderedFiles;
	for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
	{
		if ((*it).startsWith(localDir))
			orderedFiles.append(*it);
		else
			orderedFiles.prepend(*it);
	}
	// then parse the ordered list
	for (QStringList::ConstIterator it=orderedFiles.begin(); it!=orderedFiles.end() && result; ++it)
	{
		// skip the local file for root
		if (getuid() == 0 && (*it).startsWith(localDir))
			break;
		else
			result = loadDesktopFile(*it);
	}

	return result;
}

bool KMSpecialManager::loadDesktopFile(const QString& filename)
{
	KSimpleConfig	conf(filename);
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
		//if (!KdeprintChecker::check(&conf))
		if ( !KXmlCommandManager::self()->checkCommand( printer->option( "kde-special-command" ), 
					KXmlCommandManager::None, KXmlCommandManager::None, 0 ) )
			printer->addType(KMPrinter::Invalid);
		printer->setState(KMPrinter::Idle);
		printer->setAcceptJobs(true);
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

KXmlCommand* KMSpecialManager::loadCommand(KMPrinter *pr)
{
	KXmlCommand	*xmlCmd = loadCommand(pr->option("kde-special-command"));
	if (xmlCmd && xmlCmd->driver())
		xmlCmd->driver()->set("text", pr->printerName());
	return xmlCmd;
}

KXmlCommand* KMSpecialManager::loadCommand(const QString& xmlId)
{
	return KXmlCommandManager::self()->loadCommand(xmlId, true);
}

DrMain* KMSpecialManager::loadDriver(KMPrinter *pr)
{
	KXmlCommand	*xmlCmd;
	DrMain	*driver(0);

	if ((xmlCmd=loadCommand(pr)) != 0)
	{
		driver = xmlCmd->takeDriver();
		delete xmlCmd;
	}

	return driver;
}

QString KMSpecialManager::setupCommand(const QString& cmd, const QMap<QString,QString>& opts)
{
	QString	s(cmd);
	if (!s.isEmpty())
	{
		KXmlCommand	*xmlCmd = loadCommand(cmd);
		if (xmlCmd)
		{
			s = xmlCmd->buildCommand(opts, false, false);
			delete xmlCmd;
		}
	}

	return s;
}
