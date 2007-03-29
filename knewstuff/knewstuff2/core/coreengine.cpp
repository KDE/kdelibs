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

#include "coreengine.h"

#include "entryhandler.h"
#include "providerhandler.h"
#include "entryloader.h"
#include "providerloader.h"
#include "installation.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include <kio/job.h>
#include <krandom.h>
#include <ktar.h>
#include <kzip.h>

#include <qdir.h>
#include <qdom.h>
#include <qprocess.h>

using namespace KNS;

CoreEngine::CoreEngine()
{
	m_provider_loader = NULL;
	m_entry_loader = NULL;

	m_uploadedentry = NULL;
	m_uploadprovider = NULL;

	m_installation = NULL;
}

CoreEngine::~CoreEngine()
{
	shutdown();
}

bool CoreEngine::init(const QString &configfile)
{
	KConfig conf(configfile);
	if(conf.getConfigState() == KConfig::NoAccess)
	{
		kError(550) << "No knsrc file named '" << configfile << "' was found." << endl;
		return false;
	}
	// FIXME: getConfigState() doesn't return NoAccess for non-existing files
	// - bug in kdecore?
	// - this needs to be looked at again until KConfig backend changes for KDE 4
	// the check below is a workaround
	if(KStandardDirs::locate("config", configfile).isEmpty())
	{
		kError(550) << "No knsrc file named '" << configfile << "' was found." << endl;
		return false;
	}

	if(!conf.hasGroup("KNewStuff2"))
	{
		kError(550) << "A knsrc file was found but it doesn't contain a KNewStuff2 section." << endl;
		return false;
	}

	KConfigGroup group = conf.group("KNewStuff2");
	m_providersurl = group.readEntry("ProvidersUrl", QString());
	m_localregistrydir = group.readEntry("LocalRegistryDir", QString());

	// FIXME: add support for several categories later on
	// FIXME: read out only when actually installing as a performance improvement?
	m_installation = new Installation();
	m_installation->setUncompression(group.readEntry("Uncompress", QString()));
	m_installation->setCommand(group.readEntry("InstallationCommand", QString()));
	m_installation->setStandardResourceDir(group.readEntry("StandardResource", QString()));
	m_installation->setTargetDir(group.readEntry("TargetDir", QString()));
	m_installation->setInstallPath(group.readEntry("InstallPath", QString()));

	QString checksumpolicy = group.readEntry("ChecksumPolicy", QString());
	QString signaturepolicy = group.readEntry("SignaturePolicy", QString());
	if(!checksumpolicy.isEmpty())
	{
		if(checksumpolicy == "never")
			m_installation->setChecksumPolicy(Installation::CheckNever);
		else if(checksumpolicy == "ifpossible")
			m_installation->setChecksumPolicy(Installation::CheckIfPossible);
		else if(checksumpolicy == "always")
			m_installation->setChecksumPolicy(Installation::CheckAlways);
		else
		{
			kError(550) << "The checksum policy '" + checksumpolicy + "' is unknown." << endl;
			return false;
		}
	}
	if(!signaturepolicy.isEmpty())
	{
		if(signaturepolicy == "never")
			m_installation->setSignaturePolicy(Installation::CheckNever);
		else if(signaturepolicy == "ifpossible")
			m_installation->setSignaturePolicy(Installation::CheckIfPossible);
		else if(signaturepolicy == "always")
			m_installation->setSignaturePolicy(Installation::CheckAlways);
		else
		{
			kError(550) << "The signature policy '" + signaturepolicy + "' is unknown." << endl;
			return false;
		}
	}

	return true;
}

void CoreEngine::start(bool localonly)
{
	loadProvidersCache();
	loadEntryCache();

	// FIXME: LocalRegistryDir must be created in $KDEHOME if missing?
	// FIXME: rename registry to cache?

	if(!m_localregistrydir.isEmpty())
	{
		loadRegistry(m_localregistrydir);
	}

	if(localonly)
	{
		emit signalEntriesFinished();
		return;
	}

	m_provider_loader = new ProviderLoader();
	m_provider_loader->load(m_providersurl);

	// FIXME: I think we need a slot so we can delete the loader again
	// we could have one for the entire lifetime, but for entry loaders this
	// would result in too many objects active at once
	connect(m_provider_loader,
		SIGNAL(signalProvidersLoaded(KNS::Provider::List*)),
		SLOT(slotProvidersLoaded(KNS::Provider::List*)));
	connect(m_provider_loader,
		SIGNAL(signalProvidersFailed()),
		SLOT(slotProvidersFailed()));
}

void CoreEngine::loadEntries(Provider *provider)
{
	EntryLoader *entry_loader = new EntryLoader();
	entry_loader->load(provider->downloadUrl().url());
	// FIXME: loaders should probably accept KUrl directly

	// FIXME: the engine should do this... for all feeds!
	connect(entry_loader,
		SIGNAL(signalEntriesLoaded(KNS::Entry::List*)),
		SLOT(slotEntriesLoaded(KNS::Entry::List*)));
	connect(entry_loader,
		SIGNAL(signalEntriesFailed()),
		SLOT(slotEntriesFailed()));
}

void CoreEngine::downloadPreview(Entry *entry)
{
	if(m_previewfiles.contains(entry))
	{
		// FIXME: ensure somewhere else that preview file even exists
		kDebug(550) << "Reusing preview from '" << m_previewfiles[entry] << "'" << endl;
		emit signalPreviewLoaded(KUrl::fromPath(m_previewfiles[entry]));
		return;
	}

	KUrl source = KUrl(entry->preview().representation());
	KUrl destination = KGlobal::dirs()->saveLocation("tmp") + KRandom::randomString(10);
	kDebug(550) << "Downloading preview '" << source << "' to '" << destination << "'" << endl;

	// FIXME: check for validity
	KIO::FileCopyJob *job = KIO::file_copy(source, destination, -1, true, false, false);
	connect(job,
		SIGNAL(result(KJob*)),
		SLOT(slotPreviewResult(KJob*)));

	m_entry_jobs[job] = entry;
}

void CoreEngine::downloadPayload(Entry *entry)
{
	KUrl source = KUrl(entry->payload().representation());

	if(m_installation->isRemote())
	{
		// Remote resource
		kDebug(550) << "Relaying remote payload '" << source << "'" << endl;
		entry->setStatus(Entry::Installed);
		emit signalPayloadLoaded(source);
		// FIXME: we still need registration for eventual deletion
		return;
	}

	KUrl destination = KGlobal::dirs()->saveLocation("tmp") + KRandom::randomString(10);
	kDebug(550) << "Downloading payload '" << source << "' to '" << destination << "'" << endl;

	// FIXME: check for validity
	KIO::FileCopyJob *job = KIO::file_copy(source, destination, -1, true, false, false);
	connect(job,
		SIGNAL(result(KJob*)),
		SLOT(slotPayloadResult(KJob*)));

	m_entry_jobs[job] = entry;
}

bool CoreEngine::uploadEntry(Provider *provider, Entry *entry)
{
	kDebug(550) << "Uploading " << entry->name().representation() << "..." << endl;

	if(m_uploadedentry)
	{
		kError(550) << "Another upload is in progress!" << endl;
		return false;
	}

	if(!provider->uploadUrl().isValid())
	{
		kError(550) << "The provider doesn't support uploads." << endl;
		return false;

		// FIXME: support for <noupload> will go here (file bundle creation etc.)
	}

	// FIXME: validate files etc.

	m_uploadedentry = entry;

	KUrl sourcepayload = KUrl(entry->payload().representation());
	KUrl destfolder = provider->uploadUrl();

	destfolder.setFileName(sourcepayload.fileName());

	KIO::FileCopyJob *fcjob = KIO::file_copy(sourcepayload, destfolder, -1, true, false, false);
	connect(fcjob,
		SIGNAL(result(KJob*)),
		SLOT(slotUploadPayloadResult(KJob*)));

	return true;
}

void CoreEngine::slotProvidersLoaded(KNS::Provider::List *list)
{
	mergeProviders(list);

	/*for(Provider::List::Iterator it = list->begin(); it != list->end(); it++)
	{
		Provider *provider = (*it);
		emit signalProviderLoaded(provider);
	}*/
	// FIXME: cleanup provider loader
}

void CoreEngine::slotProvidersFailed()
{
	emit signalProvidersFailed();
	// FIXME: cleanup provider loader
}

void CoreEngine::slotEntriesLoaded(KNS::Entry::List *list)
{
	mergeEntries(list);

	/*for(Entry::List::Iterator it = list->begin(); it != list->end(); it++)
	{
		Entry *entry = (*it);
		emit signalEntryLoaded(entry);
	}*/
	// FIXME: cleanup entry loader
}

void CoreEngine::slotEntriesFailed()
{
	emit signalEntriesFailed();
	// FIXME: cleanup entry loader
}

void CoreEngine::slotPayloadResult(KJob *job)
{
	if(job->error())
	{
		kError(550) << "Cannot load payload file." << endl;
		kError(550) << job->errorString() << endl;

		m_entry_jobs.remove(job);
		emit signalPayloadFailed();
	}
	else
	{
		KIO::FileCopyJob *fcjob = static_cast<KIO::FileCopyJob*>(job);

		if(m_entry_jobs.contains(job))
		{
			// FIXME: this is only so exposing the KUrl suffices for downloaded entries
			Entry *entry = m_entry_jobs[job];
			entry->setStatus(Entry::Installed);
			m_entry_jobs.remove(job);
			m_payloadfiles[entry] = fcjob->destUrl().path();
		}
		// FIXME: ignore if not? shouldn't happen...

		emit signalPayloadLoaded(fcjob->destUrl());
	}
}

// FIXME: this should be handled more internally to return a (cached) preview image
void CoreEngine::slotPreviewResult(KJob *job)
{
	if(job->error())
	{
		kError(550) << "Cannot load preview file." << endl;
		kError(550) << job->errorString() << endl;

		m_entry_jobs.remove(job);
		emit signalPreviewFailed();
	}
	else
	{
		KIO::FileCopyJob *fcjob = static_cast<KIO::FileCopyJob*>(job);

		if(m_entry_jobs.contains(job))
		{
			// now, assign temporary filename to entry and update entry cache
			Entry *entry = m_entry_jobs[job];
			m_entry_jobs.remove(job);
			m_previewfiles[entry] = fcjob->destUrl().path();
			cacheEntry(entry);
		}
		// FIXME: ignore if not? shouldn't happen...

		emit signalPreviewLoaded(fcjob->destUrl());
	}
}

void CoreEngine::slotUploadPayloadResult(KJob *job)
{
	if(job->error())
	{
		kError(550) << "Cannot upload payload file." << endl;
		kError(550) << job->errorString() << endl;

		m_uploadedentry = NULL;
		m_uploadprovider = NULL;

		emit signalEntryFailed();
		return;
	}

	if(m_uploadedentry->preview().isEmpty())
	{
		// FIXME: we abuse 'job' here for the shortcut if there's no preview
		slotUploadPreviewResult(job);
		return;
	}

	KUrl sourcepreview = KUrl(m_uploadedentry->preview().representation());
	KUrl destfolder = m_uploadprovider->uploadUrl();

	KIO::FileCopyJob *fcjob = KIO::file_copy(sourcepreview, destfolder, -1, true, false, false);
	connect(fcjob,
		SIGNAL(result(KJob*)),
		SLOT(slotUploadPreviewResult(KJob*)));
}

void CoreEngine::slotUploadPreviewResult(KJob *job)
{
	if(job->error())
	{
		kError(550) << "Cannot upload preview file." << endl;
		kError(550) << job->errorString() << endl;

		m_uploadedentry = NULL;
		m_uploadprovider = NULL;

		emit signalEntryFailed();
		return;
	}

	// FIXME: the following save code is also in cacheEntry()
	// when we upload, the entry should probably be cached!

	// FIXME: adhere to meta naming rules as discussed
	KUrl sourcemeta = KGlobal::dirs()->saveLocation("tmp") + KRandom::randomString(10) + ".meta";
	KUrl destfolder = m_uploadprovider->uploadUrl();

	EntryHandler eh(*m_uploadedentry);
	QDomElement exml = eh.entryXML();

	QFile f(sourcemeta.path());
	if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		kError(550) << "Cannot write meta information to '" << sourcemeta << "'." << endl;

		m_uploadedentry = NULL;
		m_uploadprovider = NULL;

		emit signalEntryFailed();
		return;
	}
	QTextStream metastream(&f);
	metastream << exml;
	f.close();

	KIO::FileCopyJob *fcjob = KIO::file_copy(sourcemeta, destfolder, -1, true, false, false);
	connect(fcjob,
		SIGNAL(result(KJob*)),
		SLOT(slotUploadMetaResult(KJob*)));
}

void CoreEngine::slotUploadMetaResult(KJob *job)
{
	if(job->error())
	{
		kError(550) << "Cannot upload meta file." << endl;
		kError(550) << job->errorString() << endl;

		m_uploadedentry = NULL;
		m_uploadprovider = NULL;

		emit signalEntryFailed();
		return;
	}
	else
	{
		m_uploadedentry = NULL;
		m_uploadprovider = NULL;

		//KIO::FileCopyJob *fcjob = static_cast<KIO::FileCopyJob*>(job);
		emit signalEntryUploaded();
	}
}

void CoreEngine::loadRegistry(const QString &registrydir)
{
	KStandardDirs d;

	kDebug(550) << "Loading registry in all directories named '" + registrydir + "'." << endl;

	QStringList dirs = d.findDirs("data", registrydir);
	for(QStringList::Iterator it = dirs.begin(); it != dirs.end(); it++)
	{
		kDebug(550) << " + Load from directory '" + (*it) + "'." << endl;
		QDir dir((*it));
		QStringList files = dir.entryList(QDir::Files | QDir::Readable);
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
			e->setStatus(Entry::Installed);
			m_entry_cache.append(e);
			m_entry_index[id(e)] = e;

			// FIXME: we must overwrite cache entries with registered entries
			// and not just append the latter ones
			// (m_entry_index is correct here but m_entry_cache not yet)
		}
	}
}

void CoreEngine::loadProvidersCache()
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

		emit signalProviderLoaded(p);

		provider = root.nextSiblingElement("provider");
	}
}

void CoreEngine::loadEntryCache()
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
	QStringList files = dir.entryList(QDir::Files | QDir::Readable);
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
		if(root.tagName() != "ghnscache")
		{
			kWarning(550) << "The file doesn't seem to be of interest." << endl;
			continue;
		}

		QDomElement stuff = root.firstChildElement("stuff");
		if(stuff.isNull())
		{
			kWarning(550) << "Missing GHNS cache metadata." << endl;
			continue;
		}

		EntryHandler handler(stuff);
		if(!handler.isValid())
		{
			kWarning(550) << "Invalid GHNS installation metadata." << endl;
			continue;
		}

		Entry *e = handler.entryptr();
		e->setStatus(Entry::Downloadable);
		m_entry_cache.append(e);
		m_entry_index[id(e)] = e;

		if(root.hasAttribute("previewfile"))
		{
			m_previewfiles[e] = root.attribute("previewfile");
			// FIXME: check here for a [ -f previewfile ]
		}
		if(root.hasAttribute("payloadfile"))
		{
			m_payloadfiles[e] = root.attribute("payloadfile");
			// FIXME: check here for a [ -f payloadfile ]
		}

		emit signalEntryLoaded(e);
	}
}

void CoreEngine::shutdown()
{
	m_entry_index.clear();
	m_provider_index.clear();

	qDeleteAll(m_entry_cache);
	qDeleteAll(m_provider_cache);

	m_entry_cache.clear();
	m_provider_cache.clear();

	delete m_installation;

	delete m_provider_loader;
	delete m_entry_loader;
	// FIXME: entry loader object not used yet - must be a list of those
}

void CoreEngine::mergeProviders(Provider::List *providers)
{
	for(Provider::List::Iterator it = providers->begin(); it != providers->end(); it++)
	{
		// TODO: find entry in providercache, replace if needed
		Provider *p = (*it);

		if(m_provider_index.contains(pid(p)))
		{
			kDebug(550) << "CACHE: hit provider " << p->name().representation() << endl;
			// FIXME: see mergeEntries for the hit case
			Provider *oldprovider = m_provider_index[pid(p)];
			if(p->downloadUrl() != oldprovider->downloadUrl())
			{
				kDebug(550) << "CACHE: update provider" << endl;
				cacheProvider(p);
				emit signalProviderChanged(p);
			}
		}
		else
		{
			kDebug(550) << "CACHE: miss provider " << p->name().representation() << endl;
			cacheProvider(p);
			emit signalProviderLoaded(p);
		}

		m_provider_cache.append(p);
		m_provider_index[pid(p)] = p;
	}

	emit signalProvidersFinished();
}

void CoreEngine::mergeEntries(Entry::List *entries)
{
	for(Entry::List::Iterator it = entries->begin(); it != entries->end(); it++)
	{
		// TODO: find entry in entrycache, replace if needed
		// don't forget marking as 'updateable'
		Entry *e = (*it);
		e->setStatus(Entry::Downloadable);

		if(m_entry_index.contains(id(e)))
		{
			kDebug(550) << "CACHE: hit entry " << e->name().representation() << endl;
			// FIXME: if changed, emit signalEntryChanged()
			// we might have a cache on the whole content (e.g. base64) for that matter
			// more robust than comparing all attributes? (-> xml infoset)
			// FIXME: separate version updated from server-side translation updates
			Entry *oldentry = m_entry_index[id(e)];
			if(e->releaseDate() > oldentry->releaseDate())
			{
				kDebug(550) << "CACHE: update entry" << endl;
				e->setStatus(Entry::Updateable);
				// entry has changed
				// FIXME: important: for cache filename, whole-content comparison
				// is harmful, still needs id-based one!
				cacheEntry(e);
				emit signalEntryChanged(e);
				// FIXME: oldentry can now be deleted, but it's still in the list!
				// FIXME: better: assigne all values to 'e', keeps refs intact
			}
		}
		else
		{
			kDebug(550) << "CACHE: miss entry " << e->name().representation() << endl;
			cacheEntry(e);
			emit signalEntryLoaded(e);
		}

		m_entry_cache.append(e);
		m_entry_index[id(e)] = e;
	}

	emit signalEntriesFinished();
}

void CoreEngine::cacheProvider(Provider *provider)
{
	KStandardDirs d;

	kDebug(550) << "Caching provider." << endl;

	QString cachedir = d.saveLocation("cache");
	QString cachefile = cachedir + "knewstuff2-providers.cache.xml";

	kDebug(550) << " + Save to file '" + cachefile + "'." << endl;

	QDomDocument doc;
	QDomElement root = doc.createElement("ghnsproviders");

	for(Provider::List::Iterator it = m_provider_cache.begin(); it != m_provider_cache.end(); it++)
	{
		Provider *p = (*it);
		ProviderHandler ph(*p);
		QDomElement pxml = ph.providerXML();
		root.appendChild(pxml);
	}
	ProviderHandler ph(*provider);
	QDomElement pxml = ph.providerXML();
	root.appendChild(pxml);

	QFile f(cachefile);
	if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		kError(550) << "Cannot write meta information to '" << cachedir << "'." << endl;
		// FIXME: ignore?
		return;
	}
	QTextStream metastream(&f);
	metastream << root;
	f.close();
}

void CoreEngine::cacheEntry(Entry *entry)
{
	KStandardDirs d;

	kDebug(550) << "Caching entry." << endl;

	QString cachedir = d.saveLocation("cache", "knewstuff2-entries.cache");

	kDebug(550) << " + Save to directory '" + cachedir + "'." << endl;

	//QString cachefile = KRandom::randomString(10) + ".meta";
	//FIXME: this must be deterministic, but it could also be an OOB random string
	//which gets stored into <ghnscache> just like preview...
	QString cachefile = id(entry) + ".meta";

	kDebug(550) << " + Save to file '" + cachefile + "'." << endl;

	// FIXME: adhere to meta naming rules as discussed
	// FIXME: maybe related filename to base64-encoded id(), or the reverse?

	EntryHandler eh(*entry);
	QDomElement exml = eh.entryXML();

	QDomDocument doc;
	QDomElement root = doc.createElement("ghnscache");
	root.appendChild(exml);

	if(m_previewfiles.contains(entry))
	{
		root.setAttribute("previewfile", m_previewfiles[entry]);
	}
	/*if(m_payloadfiles.contains(entry))
	{
		root.setAttribute("payloadfile", m_payloadfiles[entry]);
	}*/

	QFile f(cachedir + cachefile);
	if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		kError(550) << "Cannot write meta information to '" << cachedir + cachefile << "'." << endl;
		// FIXME: ignore?
		return;
	}
	QTextStream metastream(&f);
	metastream << root;
	f.close();
}

void CoreEngine::registerEntry(Entry *entry)
{
	KStandardDirs d;

	kDebug(550) << "Registering entry." << endl;

	// FIXME: this directory must match loadRegistry!
	QString registrydir = d.saveLocation("data", "knewstuff2-entries.registry");

	kDebug(550) << " + Save to directory '" + registrydir + "'." << endl;

	// FIXME: see cacheEntry() for naming-related discussion
	QString registryfile = id(entry) + ".meta";

	kDebug(550) << " + Save to file '" + registryfile + "'." << endl;

	EntryHandler eh(*entry);
	QDomElement exml = eh.entryXML();

	QDomDocument doc;
	QDomElement root = doc.createElement("ghnsinstall");
	root.appendChild(exml);

	if(m_payloadfiles.contains(entry))
	{
		root.setAttribute("payloadfile", m_payloadfiles[entry]);
	}

	QFile f(registrydir + registryfile);
	if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		kError(550) << "Cannot write meta information to '" << registrydir + registryfile << "'." << endl;
		// FIXME: ignore?
		return;
	}
	QTextStream metastream(&f);
	metastream << root;
	f.close();
}

QString CoreEngine::id(Entry *e)
{
	// This is the primary key of an entry:
	// A lookup on the untranslated original name, which must exist
	// FIXME: this is not a valid assumption per GHNS spec!
	return e->name().translated(QString());
}

QString CoreEngine::pid(Provider *p)
{
	// This is the primary key of a provider:
	// The download URL
	return p->downloadUrl().url();
}

bool CoreEngine::install(QString payloadfile)
{
	QList<Entry*> entries = m_payloadfiles.keys(payloadfile);
	if(entries.size() != 1)
	{
		// FIXME: shouldn't ever happen - make this an assertion?
		kError(550) << "ASSERT: payloadfile is not associated" << endl;
		return false;
	}

	Entry *entry = entries.first();

	// FIXME: first of all, do the security stuff here
	// this means check sum comparison and signature verification
	// signature verification might take a long time - make async?!

	if(m_installation->checksumPolicy() != Installation::CheckNever)
	{
		if(entry->checksum().isEmpty())
		{
			if(m_installation->checksumPolicy() == Installation::CheckIfPossible)
			{
				kDebug(550) << "Skip checksum verification" << endl;
			}
			else
			{
				kError(550) << "Checksum verification not possible" << endl;
				return false;
			}
		}
		else
		{
			kDebug(550) << "Verify checksum..." << endl;
		}
	}

	if(m_installation->signaturePolicy() != Installation::CheckNever)
	{
		if(entry->signature().isEmpty())
		{
			if(m_installation->signaturePolicy() == Installation::CheckIfPossible)
			{
				kDebug(550) << "Skip signature verification" << endl;
			}
			else
			{
				kError(550) << "Signature verification not possible" << endl;
				return false;
			}
		}
		else
		{
			kDebug(550) << "Verify signature..." << endl;
		}
	}

	kDebug(550) << "INSTALL resourceDir " << m_installation->standardResourceDir() << endl;
	kDebug(550) << "INSTALL targetDir " << m_installation->targetDir() << endl;
	kDebug(550) << "INSTALL installPath " << m_installation->installPath() << endl;
	kDebug(550) << "INSTALL + uncompression " << m_installation->uncompression() << endl;
	kDebug(550) << "INSTALL + command " << m_installation->command() << endl;

	QString ext = payloadfile.section('.', 1);
	QString installfile = entry->name().representation();
	installfile += "-" + entry->version();
	if(!ext.isEmpty()) installfile += "." + ext;

	QString installpath, installdir;
	int pathcounter = 0;
	if(!m_installation->standardResourceDir().isEmpty())
	{
		installdir = KStandardDirs::locateLocal(m_installation->standardResourceDir().toUtf8(), "/");
		pathcounter++;
	}
	if(!m_installation->targetDir().isEmpty())
	{
		installdir = KStandardDirs::locateLocal("data", m_installation->targetDir() + "/");
		pathcounter++;
	}
	if(!m_installation->installPath().isEmpty())
	{
		installdir = QDir::home().path() + "/" + m_installation->installPath() + "/";
		pathcounter++;
	}
	installpath = installdir + "/" + installfile;

	if(pathcounter != 1)
	{
		kError(550) << "Wrong number of installation directories given." << endl;
		return false;
	}

	kDebug(550) << "Install to file " << installpath << endl;
	// FIXME: copy goes here (including overwrite checking)
	// FIXME: what must be done now is to update the cache *again*
	//        in order to set the new payload filename (on root tag only)
	//        - this might or might not need to take uncompression into account
	// FIXME: for updates, we might need to force an overwrite (that is, deleting before)
	QFile file(payloadfile);
	bool success = file.rename(installpath);
	if(!success)
	{
		kError(550) << "Cannot move file to destination" << endl;
		return false;
	}

	m_payloadfiles[entry] = installpath;
	registerEntry(entry);
	// FIXME: hm, do we need to update the cache really?
	// only registration is probably be needed here

	if(!m_installation->uncompression().isEmpty())
	{
		kDebug(550) << "Postinstallation: uncompress the file" << endl;

		// FIXME: check for overwriting, malicious archive entries (../foo) etc.
		// FIXME: KArchive should provide "safe mode" for this!
		// FIXME: value for uncompression was application/x-gzip etc. - and now?

		if(ext == "zip")
		{
			KZip zip(installpath);
			bool success = zip.open(QIODevice::ReadOnly);
			if(!success)
			{
				kError(550) << "Cannot open archive file '" << installpath << "'" << endl;
				return false;
			}
			const KArchiveDirectory *dir = zip.directory();
			dir->copyTo(installdir);
			zip.close();
			QFile::remove(installpath);
		}
		else if((ext == "tar") || (ext == "gz") || (ext == "bz2"))
		{
			KTar tar(installpath);
			bool success = tar.open(QIODevice::ReadOnly);
			if(!success)
			{
				kError(550) << "Cannot open archive file '" << installpath << "'" << endl;
				return false;
			}
			const KArchiveDirectory *dir = tar.directory();
			dir->copyTo(installdir);
			tar.close();
			QFile::remove(installpath);
		}
		else
		{
			kError(550) << "Unknown uncompression method " << ext << endl;
			return false;
		}
	}

	if(!m_installation->command().isEmpty())
	{
		kDebug(550) << "Postinstallation: execute command" << endl;
		kDebug(550) << "Command is: " << m_installation->command() << endl;

		// FIXME: knewstuff1 comment mentions kmacroexpander and kshell
		//        but how would they help much here?
		QStringList args;
		QStringList list = m_installation->command().split(" ");
		for(QStringList::iterator it = list.begin(); it != list.end(); it++)
		{
			args << (*it).replace("%f", installpath);
		}
		QString exe(args.takeFirst());
		int exitcode = QProcess::execute(exe, args);

		if(exitcode)
		{
			kError(550) << "Command failed" << endl;
		}
		else
		{
			kDebug(550) << "Command executed successfully" << endl;
		}
	}

	return true;
}

bool CoreEngine::uninstall(KNS::Entry *entry)
{
	entry->setStatus(Entry::Deleted);
	// FIXME: remove payload file, and maybe unpacked files
	return true;
}

#include "coreengine.moc"
