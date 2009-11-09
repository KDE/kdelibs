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
    void setPolicy(Engine::CachePolicy policy);
    Engine::CachePolicy policy() const;
    
    void readCache();

    void writeCache();

    Entry::List entriesForProvider(const QString& providerId);
    
private:
    Engine::CachePolicy cachePolicy;
    
};

}

#endif
