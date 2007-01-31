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
#ifndef KNEWSTUFF2_ENGINE_H
#define KNEWSTUFF2_ENGINE_H

#include <knewstuff2/provider.h>
#include <knewstuff2/entry.h>

#include <kdelibs_export.h>

#include <qobject.h>
#include <qstring.h>
#include <qmap.h>

namespace KNS {

class ProviderLoader;
class EntryLoader;

/**
 * KNewStuff engine.
 * An engine keeps track of data which is available locally and remote
 * and offers high-level synchronization calls which make use of an
 * underlying GHNS or DXS infrastructure.
 */
class KDE_EXPORT Engine : public QObject
{
    Q_OBJECT
  public:
    /**
     * Constructor.
     */
    Engine();

    /**
     * Destructor. Frees up all the memory again which might be taken
     * by cached entries and providers.
     */
    ~Engine();

    /**
     * Initializes the engine. This step is application-specific and relies
     * on an external configuration file, which determines all the details
     * about the initialization.
     *
     * @param configfile KNewStuff2 configuration file (*.knsrc)
     * @return \b true if any configuration was found, \b false otherwise
     */
    bool init(const QString &configfile);

  signals:
    /**
     * Indicates that the list of providers has been successfully loaded.
     * This signal might occur twice, for the local cache and for updated provider
     * information from the ProvidersUrl.
     */
    void signalProvidersLoaded(KNS::Provider::List *list);
    void signalProvidersFailed();

  private:
    void loadRegistry(const QString &registrydir);
    void loadProvidersCache();
    void loadEntryCache();
    void mergeProviders(Provider::List providers);
    void mergeEntries(Entry::List entries);
    void shutdown();

    QString id(Entry *e);
    QString pid(Provider *p);

    QList<Provider*> m_provider_cache;
    QList<Entry*> m_entry_cache;

    QMap<QString, Provider*> m_provider_index;
    QMap<QString, Entry*> m_entry_index;

    ProviderLoader *m_provider_loader;
    EntryLoader *m_entry_loader;
};

}

#endif
