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

#include <kconfig.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include <qdir.h>
#include <qdom.h>

using namespace KNS;

Engine::Engine()
{
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

	// FIXME: LocalRegistryDir must be created in $KDEHOME if missing?
	// FIXME: rename registry to cache?

	if(!localregistrydir.isEmpty())
	{
		loadRegistry(localregistrydir);
	}

	loadProvidersCache();

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
		QDir d((*it));
		QStringList files = d.entryList();
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

			Entry e = handler.entry();
			Q_UNUSED(e);
			// TODO: store into runtime cache
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

		Provider p = handler.provider();
		Q_UNUSED(p);
		// TODO: store into runtime cache

		provider = root.nextSiblingElement("provider");
	}
}

#include "engine.moc"
