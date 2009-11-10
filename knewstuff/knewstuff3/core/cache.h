/*
    Copyright (c) 2009 Frederik Gladhorn <gladhorn@kde.org>

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
#include "engine.h"
#include "core/entry.h"

namespace KNS3 {
    
class Cache : public QObject
{
    Q_OBJECT
public:
    Cache(QObject* parent = 0);
    /// The file name of the cache - this is usally the application name
    void setCacheFileName(const QString& file);
    /// The cache policy (what to save)
    void setPolicy(Engine::CachePolicy policy);
    /// The cache policy (what to save)
    Engine::CachePolicy policy() const;

    /// Read the cache file (e.g. on startup)
    void readCache();
    Entry::List cacheForProvider(const QString& providerId);

    /// Save the list of entries to the cache
    void writeCache();

    void insert(const QList<Entry>& entries);
    
    
private:
    Engine::CachePolicy cachePolicy;
    QString cacheFile;

    QSet<Entry> cache;
    
};

}

#endif
