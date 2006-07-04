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

#include "kmdriverdb.h"
#include "kmdbentry.h"
#include "kmdbcreator.h"
#include "kmmanager.h"
#include "kmfactory.h"
#include <kdebug.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qfileinfo.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kmessagebox.h>

KMDriverDB* KMDriverDB::m_self = 0;

KMDriverDB* KMDriverDB::self()
{
	if (!m_self)
	{
		m_self = new KMDriverDB();
		Q_CHECK_PTR(m_self);
	}
	return m_self;
}

KMDriverDB::KMDriverDB(QObject *parent)
: QObject(parent)
{
        m_creator = new KMDBCreator(this );
        m_creator->setObjectName( "db-creator" );
	connect(m_creator,SIGNAL(dbCreated()),SLOT(slotDbCreated()));

}

KMDriverDB::~KMDriverDB()
{
  clear();
}

void KMDriverDB::clear()
{
  QHashIterator<QString, QHash<QString, KMDBEntryList*>* > it( m_entries );
  while ( it.hasNext() ) {
    it.next();
    qDeleteAll(*it.value());
  }

  it = m_pnpentries;
  while ( it.hasNext() ) {
    it.next();
    qDeleteAll(*it.value());
  }

  qDeleteAll(m_entries);
  qDeleteAll(m_pnpentries);
}

QString KMDriverDB::dbFile()
{
	// this calls insure missing directories creation
	QString	filename = KStandardDirs::locateLocal("data",QString::fromLatin1("kdeprint/printerdb_%1.txt").arg(KMFactory::self()->printSystem()));
	return filename;
}

void KMDriverDB::init(QWidget *parent)
{
	QFileInfo	dbfi(dbFile());
	QString		dirname = KMFactory::self()->manager()->driverDirectory();
	QStringList	dbDirs = dirname.split(':', QString::SkipEmptyParts);
	bool	createflag(false);

	for (QStringList::ConstIterator it=dbDirs.begin(); it!=dbDirs.end() && !createflag; ++it)
		if (!(*it).startsWith("module:") && !m_creator->checkDriverDB(*it, dbfi.lastModified()))
			createflag = true;

	if (createflag)
	{
		// starts DB creation and wait for creator signal
		if (!m_creator->createDriverDB(dirname,dbfi.absoluteFilePath(),parent))
			KMessageBox::error(parent, KMFactory::self()->manager()->errorMsg().prepend("<qt>").append("</qt>"));
	}
	else if (m_entries.count() == 0)
	{
		// call directly the slot as the DB won't be re-created
		// this will (re)load the driver DB
		slotDbCreated();
	}
	else
		// no need to refresh, and already loaded, just emit signal
		emit dbLoaded(false);
}

void KMDriverDB::slotDbCreated()
{
	// DB should be created, check creator status
	if (m_creator->status())
	{
		// OK, load DB and emit signal
		loadDbFile();
		emit dbLoaded(true);
	}
	else
		// error while creating DB, notify the DB widget
		emit error(KMManager::self()->errorMsg());
	// be sure to emit this signal to notify the DB widget
	//emit dbLoaded(true);
}

KMDBEntryList* KMDriverDB::findEntry(const QString& manu, const QString& model)
{
	QHash<QString, KMDBEntryList*>	*models = m_entries.value(manu, 0);
	if (models)
		return models->value(model, 0);
	return 0;
}

KMDBEntryList* KMDriverDB::findPnpEntry(const QString& manu, const QString& model)
{
	QHash<QString, KMDBEntryList*>	*models = m_pnpentries.value(manu, 0);
	if (models)
		return models->value(model, 0);
	return 0;
}

QHash<QString, KMDBEntryList*>* KMDriverDB::findModels(const QString& manu)
{
	return m_entries.value(manu, 0);
}

void KMDriverDB::insertEntry(KMDBEntry *entry)
{
	// first check entry
	if (!entry->validate())
	{
		kDebug() << "Incorrect entry, skipping...(" << entry->file << ")" << endl;
		delete entry;
		return;
	}

	// insert it in normal entries
	QHash<QString, KMDBEntryList*>	*models = m_entries.value(entry->manufacturer, 0);
	if (!models)
	{
		models = new QHash<QString, KMDBEntryList*>();
		models->reserve(17);
		m_entries.insert(entry->manufacturer,models);
	}
	KMDBEntryList	*list = models->value(entry->model, 0);
	if (!list)
	{
		list = new KMDBEntryList;
		models->insert(entry->model,list);
	}
	list->append(entry);

	if (!entry->pnpmanufacturer.isEmpty() && !entry->pnpmodel.isEmpty())
	{
		// insert it in PNP entries
		models = m_pnpentries.value(entry->manufacturer, 0);
		if (!models)
		{
			models = new QHash<QString, KMDBEntryList*>();
			models->reserve(17);
			m_pnpentries.insert(entry->manufacturer,models);
		}
		list = models->value(entry->model, 0);
		if (!list)
		{
			list = new KMDBEntryList;
			models->insert(entry->model,list);
		}
		list->append(entry);
	}

	// don't block GUI
	qApp->processEvents();
}

/*
  Driver DB file format:
	FILE=<path>
	MANUFACTURER=<string>
	MODEL=<string>
	PNPMANUFACTURER=<string>
	PNPMODEL=<string>
	DESCRIPTION=<string>
*/

void KMDriverDB::loadDbFile()
{
	// first clear everything
  clear();

	QFile	f(dbFile());
	if (f.exists() && f.open(QIODevice::ReadOnly))
	{
		QTextStream	t(&f);
		QString		line;
		QStringList	words;
		KMDBEntry	*entry(0);

		while (!t.atEnd())
		{
			line = t.readLine().trimmed();
			if (line.isEmpty())
				continue;
			int	p = line.indexOf('=');
			if (p == -1)
				continue;
			words.clear();
			words << line.left(p) << line.mid(p+1);
			if (words[0] == "FILE")
			{
				if (entry) insertEntry(entry);
				entry = new KMDBEntry;
				entry->file = words[1];
			}
			else if (words[0] == "MANUFACTURER" && entry)
				entry->manufacturer = words[1].toUpper();
			else if (words[0] == "MODEL" && entry)
				entry->model = words[1];
			else if (words[0] == "MODELNAME" && entry)
				entry->modelname = words[1];
			else if (words[0] == "PNPMANUFACTURER" && entry)
				entry->pnpmanufacturer = words[1].toUpper();
			else if (words[0] == "PNPMODEL" && entry)
				entry->pnpmodel = words[1];
			else if (words[0] == "DESCRIPTION" && entry)
				entry->description = words[1];
			else if (words[0] == "RECOMMANDED" && entry && words[1].toLower() == "yes")
				entry->recommended = true;
			else if (words[0] == "DRIVERCOMMENT" && entry)
				entry->drivercomment = ("<qt>"+words[1].replace("&lt;", "<").replace("&gt;", ">")+"</qt>");
		}
		if (entry)
			insertEntry(entry);
	}
}
#include "kmdriverdb.moc"
