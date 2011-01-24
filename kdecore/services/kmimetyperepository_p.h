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

#include "kmimemagicrule_p.h"
#include "kmimeglobsfileparser_p.h"
#include "kmimetype.h"
#include <QReadWriteLock>

/**
 * @internal  - this header is not installed
 * Can create KMimeTypes and holds all the extra information about mimetypes
 * (aliases, parents, etc.)
 *
 * Exported for kbuildsycoca, for now
 */
class KDECORE_EXPORT KMimeTypeRepository
{
public:
    /**
     * @return the unique mimetype factory, creating it if necessary
     */
    static KMimeTypeRepository * self();

    /**
     * Creates a KMimeType
     */
    KMimeType::Ptr findMimeTypeByName(const QString &_name, KMimeType::FindByNameOption options = KMimeType::DontResolveAlias);

    /**
     * Check if mime is an alias, and return the canonical name for it if it is, otherwise empty.
     */
    QString resolveAlias(const QString& mime);

    /**
     * Resolve mime if it's an alias, and return it otherwise.
     */
    QString canonicalName(const QString& mime);

    /**
     * Returns the list of parents for a given mimetype
     */
    QStringList parents(const QString& mime);

    enum GlobMatchingFlag {
        NoFlag = 0,
        CaseSensitive = 0x1
    };

    /**
     * Return the patterns (globs) for a given mimetype
     * TEMPORARY method, it will go away once we can require shared-mime-info >= 0.70.
     */
    QStringList patternsForMimetype(const QString& mimeType);

    /**
     * This function makes sure that vital mime types are installed.
     */
    void checkEssentialMimeTypes();

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

private: // only for KMimeType and unittests
    friend class KMimeType;
    friend class KMimeFileParserTest;
    friend class KMimeTypeTest;

    /**
     * Find a mimetype from a filename (using the pattern list)
     * @param filename filename to check.
     * @param match if provided, returns the extension that matched.
     *
     * This is internal API, use KMimeType::findByUrl instead.
     */
    QStringList findFromFileName(const QString &filename, QString *matchingExtension = 0);

    /**
     * Find a mimetype from the content of a file or buffer
     * @param device the file or buffer. Must be open.
     * @param accuracy returns the priority of the rule that matched
     * @param beginning will contain the first N bytes of the device; used as cache to avoid repeated seeks
     *
     * This is internal API, use KMimeType::findByUrl instead.
     */
    KMimeType::Ptr findFromContent(QIODevice* device, int* accuracy, QByteArray& beginning);

    /**
     * @return true if at least one mimetype is present
     * Safety test
     */
    bool checkMimeTypes();

private:
    KMimeTypeRepository();
    ~KMimeTypeRepository();

    typedef QHash<QString, QString> AliasesMap;
    const AliasesMap& aliases();

    /**
     * @internal (public for unit tests only)
     */
    QList<KMimeMagicRule> parseMagicFile(QIODevice* file, const QString& fileName) const;

    // Read magic files
    void parseMagic();

    void parseGlobs();

    /**
     * Look into either the high-weight patterns or the low-weight patterns.
     * @param matchingMimeTypes in/out parameter. In: the already found mimetypes;
     * this is only set when the fast pattern dict found matches (i.e. weight 50)
     * and we want to check if there are other, longer, weight 50 matches.
     * @param filename the filename we are trying to match
     * @param foundExt in/out parameter, the recognized extension of the match
     * @param highWeight whether to look into >50 or <=50 patterns.
     */
    void findFromOtherPatternList(QStringList& matchingMimeTypes,
                                  const QString &filename,
                                  QString& foundExt,
                                  bool highWeight);

    mutable AliasesMap m_aliases; // alias -> canonicalName

    typedef QHash<QString, QStringList> ParentsMap;
    ParentsMap m_parents;

    typedef QHash<QString, QStringList> PatternsMap;
    PatternsMap m_patterns;

    bool m_parentsMapLoaded;
    bool m_magicFilesParsed;
    mutable bool m_aliasFilesParsed;
    bool m_globsFilesParsed;
    bool m_patternsMapCalculated;
    bool m_mimeTypesChecked;
    bool m_useFavIcons;
    bool m_useFavIconsChecked;
    int m_sharedMimeInfoVersion;
    QList<KMimeMagicRule> m_magicRules;
    KMimeGlobsFileParser::AllGlobs m_globs;
    KMimeType::Ptr m_defaultMimeType;
    QReadWriteLock m_mutex;
};

#endif /* KMIMETYPEREPOSITORY_H */

