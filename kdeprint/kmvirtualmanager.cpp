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

#include "kmvirtualmanager.h"
#include "kmprinter.h"
#include "kmfactory.h"
#include "kmmanager.h"
#include "kprinter.h"

#include <stdlib.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qfileinfo.h>
#include <klocale.h>

QString instanceName(const QString& prname, const QString& instname)
{
	QString	str(prname);
	if (!instname.isEmpty())
		str.append("/"+instname);
	return str;
}

KMVirtualManager::KMVirtualManager(KMManager *parent, const char *name)
: QObject(parent,name), m_manager(parent)
{
}

KMVirtualManager::~KMVirtualManager()
{
}

KMPrinter* KMVirtualManager::findPrinter(const QString& name)
{
        return m_manager->findPrinter(name);
}

KMPrinter* KMVirtualManager::findInstance(KMPrinter *p, const QString& name)
{
	QString	instname(instanceName(p->printerName(),name));
	return findPrinter(instname);
}

void KMVirtualManager::addPrinter(KMPrinter *p)
{
	if (p && p->isValid())
	{
		KMPrinter	*other = findPrinter(p->name());
		if (other)
		{
			other->copy(*p);
			// Replace default options with the new loaded ones: this is needed
			// if we want to handle 2 lpoptions correctly (system-wide and local).
			// Anyway, the virtual printers will be reloaded only if something has
			// changed in one of the files, so it's better to reset everything, to
			// be sure to use the new changes. Edited options will be left unchanged.
			other->setDefaultOptions(p->defaultOptions());
			delete p;
		}
		else
                        m_manager->addPrinter(p);
	}
	else
		delete p;
}

void KMVirtualManager::setDefault(KMPrinter *p, bool saveflag)
{
        m_manager->setSoftDefault(p);
        m_defaultprinter = (p ? p->printerName() : QString::null);
	if (saveflag) triggerSave();
}

bool KMVirtualManager::isDefault(KMPrinter *p, const QString& name)
{
	QString	instname(instanceName(p->printerName(),name));
	KMPrinter	*printer = findPrinter(instname);
	if (printer)
		return printer->isSoftDefault();
	else
		return false;
}

void KMVirtualManager::create(KMPrinter *p, const QString& name)
{
	QString	instname = instanceName(p->printerName(),name);
	if (findPrinter(instname) != NULL) return;
	KMPrinter	*printer = new KMPrinter;
	printer->setName(instname);
	printer->setPrinterName(p->printerName());
	printer->setInstanceName(name);
	printer->setType(p->type()|KMPrinter::Virtual);
        m_manager->addPrinter(printer);
	triggerSave();
}

void KMVirtualManager::copy(KMPrinter *p, const QString& src, const QString& name)
{
	QString	instsrc(instanceName(p->printerName(),src)), instname(instanceName(p->printerName(),name));
	KMPrinter	*prsrc = findPrinter(instsrc);
	if (!prsrc || findPrinter(instname) != NULL) return;
	KMPrinter	*printer = new KMPrinter;
	printer->copy(*prsrc);
	printer->setName(instname);
	printer->setInstanceName(name);
        printer->setDefaultOptions(prsrc->defaultOptions());
        m_manager->addPrinter(printer);
	triggerSave();
}

void KMVirtualManager::remove(KMPrinter *p, const QString& name)
{
        QString	instname = instanceName(p->printerName(),name);
	KMPrinter	*printer = findPrinter(instname);
	if (!printer) return;
        if (name.isEmpty())
        { // remove default instance => only remove options, keep the KMPrinter object
                printer->setDefaultOptions(QMap<QString,QString>());
                printer->setEditedOptions(QMap<QString,QString>());
                printer->setEdited(false);
        }
        else
	        m_manager->m_printers.removeRef(printer);
	triggerSave();
}

void KMVirtualManager::setAsDefault(KMPrinter *p, const QString& name)
{
	QString	instname(instanceName(p->printerName(),name));
	KMPrinter	*printer = findPrinter(instname);
	if (!printer)
	{ // create it if necessary
		create(p,name);
		printer = findPrinter(instname);
	}
	if (printer)
		setDefault(printer,true);
}

void KMVirtualManager::refresh()
{
	QFileInfo	fi(getenv("HOME") + QString::fromLatin1("/.lpoptions"));
	QFileInfo	fi2(QString::fromLatin1("/etc/cups/lpoptions"));

	if (!m_checktime.isValid() || m_checktime < QMAX(fi.lastModified(),fi2.lastModified()))
	{
                m_defaultprinter = QString::null;
		if (fi2.exists())
			loadFile(fi2.absFilePath());
		if (fi.exists())
                	loadFile(fi.absFilePath());
		m_checktime = QMAX(fi.lastModified(),fi2.lastModified());
	}
        else
        { // parse printers looking for instances -> undiscarded them, real printers
          // are undiscarded by the manager itself. Also update printer status.
                QListIterator<KMPrinter>        it(m_manager->m_printers);
                for (;it.current();++it)
                        if (!it.current()->instanceName().isEmpty())
			{
				checkPrinter(it.current());
				if (it.current()->isValid()) it.current()->setDiscarded(false);
			}
        }
}

void KMVirtualManager::checkPrinter(KMPrinter *p)
{
	KMPrinter	*realprinter = m_manager->findPrinter(p->printerName());
	if (!realprinter || realprinter->isDiscarded())
	{
		p->setType(KMPrinter::Invalid);
		p->setState(KMPrinter::Unknown);
	}
	else
	{
		p->setType(realprinter->type()|KMPrinter::Virtual);
		p->setState(realprinter->state());
	}
}

QString KMVirtualManager::defaultPrinterName()
{
        return m_defaultprinter;
}

void KMVirtualManager::virtualList(QList<KMPrinter>& list, const QString& prname)
{
	// load printers if necessary
	refresh();

	// then look for instances
	list.setAutoDelete(false);
	list.clear();
	QListIterator<KMPrinter>	it(m_manager->m_printers);
	for (;it.current();++it)
		if (it.current()->printerName() == prname)
			list.append(it.current());
}

void KMVirtualManager::loadFile(const QString& filename)
{
	QFile	f(filename);
	if (f.exists() && f.open(IO_ReadOnly))
	{
		QTextStream	t(&f);

		QString		line;
		QStringList	words;
		QStringList	pair;
		KMPrinter	*printer, *realprinter;

		while (!t.eof())
		{
			line = t.readLine().stripWhiteSpace();
			if (line.isEmpty()) continue;
			words = QStringList::split(' ',line,false);
			if (words.count() < 2) continue;
			pair = QStringList::split('/',words[1],false);
			realprinter = m_manager->findPrinter(pair[0]);
			if (realprinter && !realprinter->isDiscarded())
			{ // keep only instances corresponding to an existing and
			  // non discarded printer.
			  	// "clone" the real printer and modify settings as needed
				printer = new KMPrinter(*realprinter);
				printer->setName(words[1]);
				printer->setPrinterName(pair[0]);
				if (pair.count() > 1)
					printer->setInstanceName(pair[1]);
				printer->addType(KMPrinter::Virtual);
				// parse options
				for (uint i=2; i<words.count(); i++)
				{
					pair = QStringList::split('=',words[i],false);
					printer->setDefaultOption(pair[0],(pair.count() > 1 ? pair[1] : QString::null));
				}
				// add printer to the manager
				addPrinter(printer);	// don't use "printer" after this point !!!
				// check default state
				if (words[0].lower() == "default")
					setDefault(findPrinter(words[1]),false);
			}
		}
	}
}

void KMVirtualManager::triggerSave()
{
	QString	filename = getenv("HOME") + QString::fromLatin1("/.lpoptions");
	saveFile(filename);
	m_checktime = QFileInfo(filename).lastModified();
}

void KMVirtualManager::saveFile(const QString& filename)
{
	QFile	f(filename);
	if (f.open(IO_WriteOnly))
	{
		QTextStream	t(&f);
		QListIterator<KMPrinter>	it(m_manager->m_printers);
		for (;it.current();++it)
		{
			if (it.current()->isSpecial()) continue;
			t << (it.current()->isSoftDefault() ? "Default " : "Dest ") << it.current()->name();
			QMap<QString,QString>	opts = it.current()->defaultOptions();
			for (QMap<QString,QString>::ConstIterator oit=opts.begin(); oit!=opts.end(); ++oit)
			{
				t << ' ' << oit.key();
				if (!oit.data().isEmpty())
					t << '=' << oit.data();
			}
			t << endl;
		}
	}
}
