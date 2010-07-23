/*  This file is part of the KDE libraries
 *  Copyright 2007, 2010 David Faure <faure@kde.org>
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

#ifndef KMIMEGLOBSFILEPARSER_P_H
#define KMIMEGLOBSFILEPARSER_P_H

#include <kdebug.h>
#include <QtCore/QStringList>
#include <QtCore/QHash>
class QIODevice;
class QString;

/**
 * @internal
 */
class KMimeGlobsFileParser
{
public:
    explicit KMimeGlobsFileParser();

    class AllGlobs;

    // Read globs (patterns) files
    AllGlobs parseGlobs();

    // Separate method, for unit test
    AllGlobs parseGlobs(const QStringList&);

    struct Glob {
        Glob(const QString& mime, int w = 50, const QString& pat = QString(), int _flags = 0)
            : weight(w), flags(_flags), pattern(pat), mimeType(mime) {}
        int weight;
        int flags;
        QString pattern;
        QString mimeType;
    };

    class GlobList : public QList<Glob>
    {
    public:
        bool hasPattern(const QString& mime, const QString& pattern) const {
            const_iterator it = begin();
            const const_iterator myend = end();
            for (; it != myend; ++it)
                if ((*it).pattern == pattern && (*it).mimeType == mime)
                    return true;
            return false;
        }
        // "noglobs" is very rare occurence, so it's ok if it's slow
        void removeMime(const QString& mime) {
            QMutableListIterator<Glob> it(*this);
            while (it.hasNext()) {
                if (it.next().mimeType == mime)
                    it.remove();
            }
        }
    };

    typedef QHash<QString, QStringList> PatternsMap; // mimetype -> patterns

    /**
     * Result of the globs parsing, as data structures ready for efficient mimetype matching.
     * This contains:
     * 1) a map of fast regular patterns (e.g. *.txt is stored as "txt" in a qhash's key)
     * 2) a linear list of high-weight globs
     * 3) a linear list of low-weight globs
     * The mime-matching algorithms on top of these data structures are in KMimeTypeFactory.
     */
    class AllGlobs
    {
    public:
        void addGlob(const Glob& glob);
        void removeMime(const QString& mime);
        PatternsMap patternsMap() const; // for KMimeTypeFactory

        QHash<QString, QStringList> m_fastPatterns; // example: "doc" -> "application/msword", "text/plain"
        GlobList m_highWeightGlobs;
        GlobList m_lowWeightGlobs; // <= 50, including the non-fast 50 patterns
    };
    enum Format { OldGlobs, Globs2WithWeight };

    static AllGlobs parseGlobFiles(const QStringList& globFiles, QStringList& parsedFiles);
    static bool parseGlobFile(QIODevice* file, Format format, AllGlobs& globs);

private:
    AllGlobs m_mimeTypeGlobs;
};

#endif /* KMIMEFILEPARSER_H */

