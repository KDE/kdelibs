/*
    Copyright (c) 2009 Frederik Gladhorn <gladhorn@kde.org>
    Copyright (c) 2010 Matthias Fuchs <mat69@gmx.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CACHE_H
#define CACHE_H

#include <QtCore/QObject>
#include <QtCore/QSet>

#include "engine_p.h"
#include "entryinternal_p.h"

namespace KNS3 {

class Cache : public QObject
{
    Q_OBJECT

public:
    /**
     * Returns an instance of a shared cache for appName
     * That way it is made sure, that there do not exist different
     * instances of cache, with different contents
     * @param appName The file name of the registry - this is usually
     * the application name, it will be stored in "apps/knewstuff3/appname.knsregistry"
     */
    static QSharedPointer<Cache> getCache(const QString &appName);

    ~Cache();

    /// Read the installed entries (on startup)
    void readRegistry();
    /// All entries that have been installed by a certain provider
    EntryInternal::List registryForProvider(const QString& providerId);

    /// Save the list of installed entries
    void writeRegistry();

    void insertRequest(const KNS3::Provider::SearchRequest&, const KNS3::EntryInternal::List& entries);
    EntryInternal::List requestFromCache(const KNS3::Provider::SearchRequest&);

public Q_SLOTS:
    void registerChangedEntry(const KNS3::EntryInternal& entry);

private:
    Q_DISABLE_COPY(Cache)
    Cache(const QString& appName);

    // compatibility with KNS2
    void readKns2MetaFiles();

private:
    // The file that is used to keep track of downloaded entries
    QString registryFile;

    // The component name that was used in KNS2 to keep track of .meta files
    // This is only for compatibility with the former version - KNewStuff2.
    QString m_kns2ComponentName;

    QSet<EntryInternal> cache;
    QHash<QString, EntryInternal::List> requestCache;
};

}

#endif
