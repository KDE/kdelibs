/*
Copyright 2008  Roland Harnau <tau@gmx.eu>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef HOSTINFO_H_
#define HOSTINFO_H_

#include <QtCore/QString>
#include <QtCore/QObject>
#include "kio_export.h"

class QHostInfo;

namespace KIO
{
    namespace HostInfo
    {
        KIO_EXPORT void lookupHost(const QString& hostName, QObject* receiver, const char* member);
        KIO_EXPORT QHostInfo lookupHost(const QString& hostName, unsigned long timeout);
        KIO_EXPORT QHostInfo lookupCachedHostInfoFor(const QString& hostName);
        KIO_EXPORT void cacheLookup(const QHostInfo& info);

        // used by khtml's DNS prefetching feature
        KIO_EXPORT void prefetchHost(const QString& hostName);
        KIO_EXPORT void setCacheSize( int s );
        KIO_EXPORT void setTTL( int ttl );
    }
}

#endif
