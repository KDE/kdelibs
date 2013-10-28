/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 2006-2007 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
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
 */

#ifndef KMIMETYPEREPOSITORY_H
#define KMIMETYPEREPOSITORY_H

#include "kmimetype.h"
#include "qmimedatabase.h"
#include <QReadWriteLock>

/**
 * @internal  - this header is not installed
 * Can create KMimeTypes and holds all the extra information about mimetypes
 * (aliases, parents, etc.)
 *
 * Exported for kbuildsycoca, for now
 */
class KDE4SUPPORT_DEPRECATED_EXPORT KMimeTypeRepository
{
public:
    /**
     * @return the unique mimetype factory, creating it if necessary
     */
    static KMimeTypeRepository * self();

    /**
     * Return the patterns (globs) for a given mimetype
     * TEMPORARY method, it will go away once we can require shared-mime-info >= 0.70.
     */
    QStringList patternsForMimetype(const QString& mimeType);

    KMimeType::Ptr defaultMimeTypePtr();

    /**
     * Returns true if KMimeType::favIconForUrl should talk to kded's favicons module.
     */
    bool useFavIcons();

    int sharedMimeInfoVersion();

    /**
     * @internal
     */
    static bool matchFileName( const QString &filename, const QString &pattern );

private:
    KMimeTypeRepository();
    ~KMimeTypeRepository();

    QMimeDatabase m_mimeDb;

    bool m_useFavIcons;
    bool m_useFavIconsChecked;
    int m_sharedMimeInfoVersion;
    KMimeType::Ptr m_defaultMimeType;

    QReadWriteLock m_mutex;

    friend class KMimeTypeRepositorySingleton;
};

#endif /* KMIMETYPEREPOSITORY_H */

