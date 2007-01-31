/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "engine.h"

#include "entryhandler.h"
#include "providerhandler.h"
#include "entryloader.h"
#include "providerloader.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include <qdir.h>
#include <qdom.h>

using namespace KNS;

Engine::Engine()
{
	m_provider_loader = NULL;
	m_entry_loader = NULL;
}

Engine::~Engine()
{
	shutdown();
}

bool Engine::init(const QString &configfile)
{
	KConfig conf(configfile);
	if(conf.getConfigState() == KConfig::NoAccess)
	{
		kError(550) << "No knsrc file named '" << configfile << "' was found." << endl;
		return false;
	}
	// FIXME: getConfigState() doesn't return NoAccess for non-existing files
	// - bug in kdecore?

	if(!conf.hasGroup("KNewStuff2"))
	{
		kError(550) << "A knsrc file was found but it doesn't contain a KNewStuff2 section." << endl;
		return false;
	}

	conf.setGroup("KNewStuff2");
	QString providersurl = conf.readEntry("ProvidersUrl", QString());
	QString localregistrydir = conf.readEntry("LocalRegistryDir", QString());

	loadProvidersCache();
	loadEntryCache();

	// FIXME: LocalRegistryDir must be created in $KDEHOME if missing?
	// FIXME: rename registry to cache?

	if(!localregistrydir.isEmpty())
	{
		loadRegistry(localregistrydir);
	}

	m_provider_loader = new ProviderLoader();
	m_provider_loader->load(providersurl);

	// FIXME: I think we need a slot so we can delete the loader again
	// we could have one for the entire lifetime, but for entry loaders this
	// would result in too many objects active at once
	connect(m_provider_loader,
		SIGNAL(signalProvidersLoaded(KNS::Providers::List*)),
		SIGNAL(signalProvidersLoaded(KNS::Providers::List*)));
	connect(m_provider_loader,
		SIGNAL(signalProvidersFailed()),
		SIGNAL(signalProvidersFailed()));

	return true;
}

void Engine::loadRegistry(const QString &registrydir)
{
	KStandardDirs d;

	kDebug(550) << "Loading registry in all directories named '" + registrydir + "'." << endl;

	QStringList dirs = d.findDirs("data", registrydir);
	for(QStringList::Iterator it = dirs.begin(); it != dirs.end(); it++)
	{
		kDebug(550) << " + Load from directory '" + (*it) + "'." << endl;
		QDir dir((*it));
		QStringList files = dir.entryList();
		for(QStringList::iterator fit = files.begin(); fit != files.end(); fit++)
		{
			QString filepath = (*it) + "/" + (*fit);
			kDebug(550) << "  + Load from file '" + filepath + "'." << endl;

			bool ret;
			QFile f(filepath);
			ret = f.open(QIODevice::ReadOnly);
			if(!ret)
			{
				kWarning(550) << "The file could not be opened." << endl;
				continue;
			}

			QDomDocument doc;
			ret = doc.setContent(&f);
			if(!ret)
			{
				kWarning(550) << "The file could not be parsed." << endl;
				continue;
			}

			QDomElement root = doc.documentElement();
			if(root.tagName() != "ghnsinstall")
			{
				kWarning(550) << "The file doesn't seem to be of interest." << endl;
				continue;
			}

			QDomElement stuff = root.firstChildElement("stuff");
			if(stuff.isNull())
			{
				kWarning(550) << "Missing GHNS installation metadata." << endl;
				continue;
			}

			EntryHandler handler(stuff);
			if(!handler.isValid())
			{
				kWarning(550) << "Invalid GHNS installation metadata." << endl;
				continue;
			}

			Entry *e = handler.entryptr();
			m_entry_cache.append(e);
			m_entry_index[id(e)] = e;

			// FIXME: we must overwrite cache entries with registered entries
			// and not just append the latter ones
			// (m_entry_index is correct here but m_entry_cache not yet)
		}
	}
}

void Engine::loadProvidersCache()
{
	KStandardDirs d;

	kDebug(550) << "Loading provider cache." << endl;

	// FIXME: one file per ProvidersUrl - put this into the filename
	// FIXME: e.g. http_foo_providers.xml.cache?
	QString cachefile = d.findResource("cache", "knewstuff2-providers.cache.xml");
	if(cachefile.isEmpty())
	{
		kDebug(550) << "Cache not present, skip loading." << endl;
		return;
	}

	kDebug(550) << " + Load from file '" + cachefile + "'." << endl;

	bool ret;
	QFile f(cachefile);
	ret = f.open(QIODevice::ReadOnly);
	if(!ret)
	{
		kWarning(550) << "The file could not be opened." << endl;
		return;
	}

	QDomDocument doc;
	ret = doc.setContent(&f);
	if(!ret)
	{
		kWarning(550) << "The file could not be parsed." << endl;
		return;
	}

	QDomElement root = doc.documentElement();
	if(root.tagName() != "ghnsproviders")
	{
		kWarning(550) << "The file doesn't seem to be of interest." << endl;
		return;
	}

	QDomElement provider = root.firstChildElement("provider");
	if(provider.isNull())
	{
		kWarning(550) << "Missing provider entries in the cache." << endl;
		return;
	}

	while(!provider.isNull())
	{
		ProviderHandler handler(provider);
		if(!handler.isValid())
		{
			kWarning(550) << "Invalid provider metadata." << endl;
			continue;
		}

		Provider *p = handler.providerptr();
		m_provider_cache.append(p);
		m_provider_index[pid(p)] = p;

		provider = root.nextSiblingElement("provider");
	}
}

void Engine::loadEntryCache()
{
	KStandardDirs d;

	kDebug(550) << "Loading entry cache." << endl;

	QStringList cachedirs = d.findDirs("cache", "knewstuff2-entries.cache");
	if(cachedirs.size() == 0)
	{
		kDebug(550) << "Cache directory not present, skip loading." << endl;
		return;
	}
	QString cachedir = cachedirs.first();

	kDebug(550) << " + Load from directory '" + cachedir + "'." << endl;

	QDir dir(cachedir);
	QStringList files = dir.entryList();
	for(QStringList::iterator fit = files.begin(); fit != files.end(); fit++)
	{
		QString filepath = cachedir + "/" + (*fit);
		kDebug(550) << "  + Load from file '" + filepath + "'." << endl;

		bool ret;
		QFile f(filepath);
		ret = f.open(QIODevice::ReadOnly);
		if(!ret)
		{
			kWarning(550) << "The file could not be opened." << endl;
			return;
		}

		QDomDocument doc;
		ret = doc.setContent(&f);
		if(!ret)
		{
			kWarning(550) << "The file could not be parsed." << endl;
			return;
		}

		QDomElement root = doc.documentElement();
		if(root.tagName() != "ghnscache")
		{
			kWarning(550) << "The file doesn't seem to be of interest." << endl;
			return;
		}

		QDomElement stuff = root.firstChildElement("stuff");
		if(stuff.isNull())
		{
			kWarning(550) << "Missing GHNS cache metadata." << endl;
			return;
		}

		EntryHandler handler(stuff);
		if(!handler.isValid())
		{
			kWarning(550) << "Invalid GHNS installation metadata." << endl;
			continue;
		}

		Entry *e = handler.entryptr();
		m_entry_cache.append(e);
		m_entry_index[id(e)] = e;
	}
}

void Engine::shutdown()
{
	m_entry_index.clear();
	m_provider_index.clear();

	qDeleteAll(m_entry_cache);
	qDeleteAll(m_provider_cache);

	m_entry_cache.clear();
	m_provider_cache.clear();
}

void Engine::mergeProviders(Provider::List providers)
{
	for(Provider::List::Iterator it = providers.begin(); it != providers.end(); it++)
	{
		// TODO: find entry in providercache, replace if needed
		Provider *p = (*it);

		if(m_provider_index.contains(pid(p)))
		{
		}

		m_provider_cache.append(p);
		m_provider_index[pid(p)] = p;
	}
}

void Engine::mergeEntries(Entry::List entries)
{
	for(Entry::List::Iterator it = entries.begin(); it != entries.end(); it++)
	{
		// TODO: find entry in entrycache, replace if needed
		// don't forget marking as 'updateable'
		Entry *e = (*it);

		if(m_entry_index.contains(id(e)))
		{
		}

		m_entry_cache.append(e);
		m_entry_index[id(e)] = e;
	}
}

QString Engine::id(Entry *e)
{
	// This is the primary key of an entry:
	// A lookup on the untranslated original name, which must exist
	return e->name().translated(QString());
}

QString Engine::pid(Provider *p)
{
	// This is the primary key of a provider:
	// The download URL
	return p->downloadUrl().url();
}

#include "engine.moc"
