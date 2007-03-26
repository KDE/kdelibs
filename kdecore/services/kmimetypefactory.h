/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 2006-2007 David Faure <bastian@kde.org>
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
#include "kmimemagicrule.h"

class KSycoca;
class KSycocaDict;

class KServiceType;
class KFolderType;
class KDEDesktopMimeType;
class KExecMimeType;

/**
 * @internal  - this header is not installed
 *
 * A sycoca factory for mimetypes
 * It loads the mime types from parsing directories (e.g. mimelnk/)
 * but can also create mime types from data streams or single config files
 * @see KMimeType
 */
class KDECORE_EXPORT KMimeTypeFactory : public KSycocaFactory
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
    virtual KSycocaEntry *createEntry(const QString &, const char *) const
    { assert(0); return 0; }

    /**
     * Find a mime type in the database file (allocates it)
     * Overloaded by KBuildMimeTypeFactory to return a memory one.
     */
    virtual KMimeType::Ptr findMimeTypeByName(const QString &_name, KMimeType::FindByNameOption options = KMimeType::DontResolveAlias);

    /**
     * Check if mime is an alias, and return the canonical name for it if it is.
     */
    QString resolveAlias(const QString& mime);

    /**
     * Find a mimetype from a filename (using the pattern list)
     * @param filename filename to check.
     * @param match if provided, returns the pattern that matched.
     *
     * This is internal API, use KMimeType::findByUrl instead.
     */
    KMimeType::Ptr findFromPattern(const QString &filename, QString *match = 0);

    enum WhichPriority { LowPriorityRules, HighPriorityRules, AllRules };
    /**
     * Find a mimetype from the content of a file or buffer
     * @param device the file or buffer. open or not.
     * @param whichPriority whether to use only low (<80) or high priority (>=80) rules, or all.
     * @param accuracy returns the priority of the rule that matched
     * @param beginning the first N bytes of the device, to avoid repeated seeks
     *
     * This is internal API, use KMimeType::findByUrl instead.
     */
    KMimeType::Ptr findFromContent(QIODevice* device, WhichPriority whichPriority, int* accuracy = 0, const QByteArray& beginning = QByteArray());

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

    /**
     * @internal for kbuildsycoca
     */
    QMap<QString, QString>& aliases();

    /**
     * @internal (public for unit tests only)
     */
    QList<KMimeMagicRule> parseMagicFile(QIODevice* file, const QString& fileName) const;

protected:
    virtual KMimeType *createEntry(int offset) const;

protected: // accessed by KBuildMimeTypeFactory
    /// @internal
    int m_fastPatternOffset;
    /// @internal
    int m_otherPatternOffset;

private:
    // Read magic files
    void parseMagic();
    KMimeType::Ptr findFromPatternHelper(const QString &filename, QString *match = 0);

    QStringList m_patterns;
    QList<qint32> m_pattern_offsets;
    QMap<QString, QString> m_aliases; // alias -> canonicalName

    bool m_magicFilesParsed;
    QList<KMimeMagicRule> m_magicRules;

    static KMimeTypeFactory *_self;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    // d pointer: useless since this header is not installed
    class KMimeTypeFactoryPrivate* d;
};

#endif
