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

#include "kmvirtualmanager.h"
#include "kmprinter.h"
#include "kmfactory.h"
#include "kmmanager.h"
#include "kprinter.h"

#include <stdlib.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include <unistd.h>

static QString instanceName(const QString& prname, const QString& instname)
{
	QString	str(prname);
	if (!instname.isEmpty())
		str.append("/"+instname);
	return str;
}

KMVirtualManager::KMVirtualManager(KMManager *parent)
: QObject(parent), m_manager(parent)
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
        m_defaultprinter = (p ? p->printerName() : QString());
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
	if (!name.isEmpty())
		printer->setType(p->type()|KMPrinter::Virtual);
	// we need some options to know how to load the driver
	if (p->isSpecial())
		printer->setOptions(p->options());
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
	        m_manager->m_printers.removeAll(printer);
	triggerSave();
}

void KMVirtualManager::setAsDefault(KMPrinter *p, const QString& name, QWidget *parent)
{
	QString	instname(instanceName(p->printerName(),name));

	if ( p->isSpecial() )
	{
		if ( KMessageBox::warningContinueCancel( parent,
					i18n( "<qt>You are about to set a pseudo-printer as your personal default. "
						  "This setting is specific to KDE and will not be available outside KDE "
						  "applications. Note that this will only make your personal default printer "
						  "as undefined for non-KDE applications and should not prevent you from "
						  "printing normally. Do you really want to set <b>%1</b> as your personal default?</qt>" ).arg( instname ),
					QString(), i18n("Set as Default"), "setSpecialAsDefault" ) == KMessageBox::No )
			return;
	}

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
	QFileInfo	fi(QDir::homePath() + QFile::decodeName("/.lpoptions"));
	QFileInfo	fi2(QFile::decodeName("/etc/cups/lpoptions"));

	// if root, then only use global file: trick -> use twice the same file
	if (getuid() == 0)
		fi.setFile(fi2.absoluteFilePath());

	if (!m_checktime.isValid() || m_checktime < qMax(fi.lastModified(),fi2.lastModified()))
	{
                m_defaultprinter.clear();
		if (fi2.exists())
			loadFile(fi2.absoluteFilePath());
		if (fi.exists() && fi.absoluteFilePath() != fi2.absoluteFilePath())
                	loadFile(fi.absoluteFilePath());
		m_checktime = qMax(fi.lastModified(),fi2.lastModified());
	}
        else
        { // parse printers looking for instances -> undiscarded them, real printers
          // are undiscarded by the manager itself. Also update printer status.
                QListIterator<KMPrinter*> it(m_manager->m_printers);
                while (it.hasNext()) {
                  KMPrinter *printer(it.next());
                        if (!printer->instanceName().isEmpty()) {
                                checkPrinter(printer);
                                if (printer->isValid())
                                  printer->setDiscarded(false);
                        }
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
		if (!p->instanceName().isEmpty())
			p->setType(realprinter->type()|KMPrinter::Virtual);
		p->setState(realprinter->state());
	}
}

QString KMVirtualManager::defaultPrinterName()
{
        return m_defaultprinter;
}

void KMVirtualManager::virtualList(QList<KMPrinter*>& list, const QString& prname)
{
	// load printers if necessary
	refresh();

	// then look for instances
	list.clear();
	kDebug(500) << "KMVirtualManager::virtualList() prname=" << prname << endl;
	QListIterator<KMPrinter*>	it(m_manager->m_printers);
	while (it.hasNext()) {
    KMPrinter *printer(it.next());
		if (printer->printerName() == prname)
			list.append(printer);
  }
}

void KMVirtualManager::loadFile(const QString& filename)
{
	QFile	f(filename);
	if (f.exists() && f.open(QIODevice::ReadOnly))
	{
		QTextStream	t(&f);

		QString		line;
		QStringList	words;
		QStringList	pair;
		KMPrinter	*printer, *realprinter;

		while (!t.atEnd())
		{
			line = t.readLine().trimmed();
			if (line.isEmpty()) continue;
			words = line.split(' ', QString::SkipEmptyParts);
			if (words.count() < 2) continue;
			pair = words[1].split('/', QString::SkipEmptyParts);
			realprinter = m_manager->findPrinter(KUrl::decode_string(pair[0]));
			if (realprinter && !realprinter->isDiscarded())
			{ // keep only instances corresponding to an existing and
			  // non discarded printer.
			  	// "clone" the real printer and modify settings as needed
				printer = new KMPrinter(*realprinter);
				printer->setName(KUrl::decode_string(words[1]));
				printer->setPrinterName(KUrl::decode_string(pair[0]));
				if (pair.count() > 1)
				{
					printer->setInstanceName(KUrl::decode_string(pair[1]));
					printer->addType(KMPrinter::Virtual);
				}
				// parse options
				for (int i=2; i<words.count(); i++)
				{
					pair = words[i].split('=', QString::SkipEmptyParts);
					printer->setDefaultOption(pair[0],(pair.count() > 1 ? pair[1] : QString()));
				}
				// add printer to the manager
				addPrinter(printer);	// don't use "printer" after this point !!!
				// check default state
				if (words[0].toLower().startsWith("default"))
					setDefault(findPrinter(KUrl::decode_string(words[1])),false);
			}
		}
	}
}

void KMVirtualManager::triggerSave()
{
	QString	filename;
	if (getuid() == 0)
	{
		if (KStandardDirs::makeDir(QFile::decodeName("/etc/cups")))
			filename = QFile::decodeName("/etc/cups/lpoptions");
	}
	else
		filename = QDir::homePath() + QFile::decodeName("/.lpoptions");
	if (!filename.isEmpty())
	{
		saveFile(filename);
		m_checktime = QFileInfo(filename).lastModified();
	}
}

void KMVirtualManager::saveFile(const QString& filename)
{
	QFile	f(filename);
	if (f.open(QIODevice::WriteOnly))
	{
		QTextStream	t(&f);
		QListIterator<KMPrinter*>	it(m_manager->m_printers);
		while (it.hasNext())
		{
      KMPrinter *printer(it.next());
			if (printer->isSpecial())
			{
				t << ( printer->isSoftDefault() ? "DefaultSpecial " : "Special " );
				t << KUrl::encode_string_no_slash( printer->printerName() );
				if ( !printer->instanceName().isEmpty() )
					t << "/" << KUrl::encode_string_no_slash( printer->instanceName() );
			}
			else
				t << (printer->isSoftDefault() ? "Default " : "Dest ") << printer->name();
			QMap<QString,QString>	opts = printer->defaultOptions();
			for (QMap<QString,QString>::ConstIterator oit=opts.begin(); oit!=opts.end(); ++oit)
			{
				t << ' ' << oit.key();
				if (!oit.value().isEmpty())
					t << '=' << oit.value();
			}
			t << endl;
		}
	}
}

bool KMVirtualManager::testInstance(KMPrinter *p)
{
	QString	testpage = KMManager::self()->testPage();
	if (testpage.isEmpty())
		return false;
	else
	{
		KPrinter	pr;
		pr.setPrinterName(p->printerName());
		pr.setSearchName(p->name());
		pr.setOptions(p->defaultOptions());
		return (pr.printFiles(QStringList(testpage)));
	}
}

void KMVirtualManager::reload()
{
	reset();
}

void KMVirtualManager::configChanged()
{
	reset();
}
