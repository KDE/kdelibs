/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 2006-2007 David Faure <faure@kde.org>
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

#include <QtCore/QStringList>

#include "ksycocafactory.h"
#include "kmimetype.h"
#include "kmimemagicrule_p.h"

class KSycoca;

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
     * Returns the list of parents for a given mimetype
     */
    QStringList parents(const QString& mime);

    enum GlobMatchingFlag {
        NoFlag = 0,
        CaseSensitive = 0x1
    };

private: // only for KMimeType
    friend class KMimeType;
    friend class KMimeFileParserTest;

    /**
     * Find a mimetype from a filename (using the pattern list)
     * @param filename filename to check.
     * @param match if provided, returns the extension that matched.
     *
     * This is internal API, use KMimeType::findByUrl instead.
     */
    QList<KMimeType::Ptr> findFromFileName(const QString &filename, QString *matchingExtension = 0);

    enum WhichPriority { LowPriorityRules, HighPriorityRules, AllRules };
    /**
     * Find a mimetype from the content of a file or buffer
     * @param device the file or buffer. Must be open.
     * @param whichPriority whether to use only low (<80) or high priority (>=80) rules, or all.
     * @param accuracy returns the priority of the rule that matched
     * @param beginning will contain the first N bytes of the device; used as cache to avoid repeated seeks
     *
     * This is internal API, use KMimeType::findByUrl instead.
     */
    KMimeType::Ptr findFromContent(QIODevice* device, WhichPriority whichPriority, int* accuracy, QByteArray& beginning);

    /**
     * @return true if at least one mimetype is present
     * Safety test
     */
    bool checkMimeTypes();

protected:
    typedef QHash<QString, QString> AliasesMap;
    const AliasesMap& aliases();

public:
    /**
     * @return all mimetypes
     * Slow and memory consuming, avoid using
     */
    KMimeType::List allMimeTypes();

    /**
     * @return the unique mimetype factory, creating it if necessary
     */
    static KMimeTypeFactory * self();

    /**
     * @internal (public for unit tests only)
     */
    QList<KMimeMagicRule> parseMagicFile(QIODevice* file, const QString& fileName) const;

    /**
     * @internal (public for unit tests only)
     */
    static bool matchFileName( const QString &filename, const QString &pattern );

protected:
    virtual KMimeType *createEntry(int offset) const;

protected: // accessed by KBuildMimeTypeFactory
    /// @internal
    int m_fastPatternOffset;
    /// @internal
    int m_highWeightPatternOffset;
    /// @internal
    int m_lowWeightPatternOffset;
    /// @internal
    //int m_parentsMapOffset;

    KSycocaDict* m_fastPatternDict;

    struct OtherPattern
    {
        OtherPattern(const QString& pat, qint32 off, qint32 w, qint32 fl)
            : pattern(pat), offset(off), weight(w), flags(fl) {}
        QString pattern;
        qint32 offset;
        qint32 weight;
        qint32 flags;
    };
    typedef QList<OtherPattern> OtherPatternList;

private:
    // Read magic files
    void parseMagic();

    QList<KMimeType::Ptr> findFromFastPatternDict(const QString &extension);
    /**
     * Look into either the high-weight patterns or the low-weight patterns.
     * @param matchingMimeTypes in/out parameter. In: the already found mimetypes;
     * this is only set when the fast pattern dict found matches (i.e. weight 50)
     * and we want to check if there are other, longer, weight 50 matches.
     * @param filename the filename we are trying to match
     * @param foundExt in/out parameter, the recognized extension of the match
     * @param highWeight whether to look into >50 or <=50 patterns.
     */
    void findFromOtherPatternList(QList<KMimeType::Ptr>& matchingMimeTypes,
                                  const QString &filename,
                                  QString& foundExt,
                                  bool highWeight);

    OtherPatternList m_highWeightPatterns;
    OtherPatternList m_lowWeightPatterns;

    mutable AliasesMap m_aliases; // alias -> canonicalName

    typedef QHash<QString, QStringList> ParentsMap;
    ParentsMap m_parents;

    bool m_highWeightPatternsLoaded;
    bool m_lowWeightPatternsLoaded;
    bool m_parentsMapLoaded;
    bool m_magicFilesParsed;
    mutable bool m_aliasFilesParsed;
    QList<KMimeMagicRule> m_magicRules;

private:
    // d pointer: useless since this header is not installed
    //class KMimeTypeFactoryPrivate* d;
};

#endif
