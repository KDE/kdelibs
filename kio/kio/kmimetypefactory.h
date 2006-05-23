/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 2006 David Faure <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
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

#ifndef KMIMETYPEFACTORY_H
#define KMIMETYPEFACTORY_H

#include <assert.h>

#include <qstringlist.h>

#include "ksycocafactory.h"
#include "kmimetype.h"

class KSycoca;
class KSycocaDict;

class KServiceType;
class KFolderType;
class KDEDesktopMimeType;
class KExecMimeType;

/**
 * @internal
 * A sycoca factory for mimetypes
 * It loads the mime types from parsing directories (e.g. mimelnk/)
 * but can also create mime types from data streams or single config files
 * @see KMimeType
 */
class KIO_EXPORT KMimeTypeFactory : public KSycocaFactory
{
    K_SYCOCAFACTORY( KST_KMimeTypeFactory )
        public:
    /**
     * Create factory
     */
    KMimeTypeFactory();

    virtual ~KMimeTypeFactory();

    /**
     * Not meant to be called at this level
     */
    virtual KSycocaEntry *createEntry(const QString &, const char *)
    { assert(0); return 0; }

    /**
     * Find a mime type in the database file (allocates it)
     * Overloaded by KBuildMimeTypeFactory to return a memory one.
     */
    virtual KMimeType::Ptr findMimeTypeByName(const QString &_name);

    /**
     * Find a mimetype from a filename (using the pattern list)
     * @param filename filename to check.
     * @param match if provided, returns the pattern that matched.
     */
    KMimeType::Ptr findFromPattern(const QString &filename, QString *match = 0);

    /**
     * @return all mimetypes
     * Slow and memory consuming, avoid using
     */
    KMimeType::List allMimeTypes();

    /**
     * @return true if at least one mimetype is present
     * Safety test
     */
    bool checkMimeTypes();

    /**
     * @return the unique mimetype factory, creating it if necessary
     */
    static KMimeTypeFactory * self();

protected:
    virtual KMimeType *createEntry(int offset);

private:
    static KMimeTypeFactory *_self;

protected: // accessed by KBuildMimeTypeFactory
    int m_fastPatternOffset;
    int m_otherPatternOffset;

private:
    QStringList m_patterns;
    QList<qint32> m_pattern_offsets;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KMimeTypeFactoryPrivate* d;
};

#endif
