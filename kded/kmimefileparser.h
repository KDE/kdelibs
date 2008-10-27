/*  This file is part of the KDE libraries
 *  Copyright 2007 David Faure <faure@kde.org>
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

#ifndef KDED_KMIMEFILEPARSER_H
#define KDED_KMIMEFILEPARSER_H

#include <QtCore/QStringList>
#include <QtCore/QHash>
class QIODevice;
class QString;
class KMimeTypeFactory;

class KMimeFileParser
{
public:
    explicit KMimeFileParser( KMimeTypeFactory* mimeTypeFactory );

    // Read globs (patterns) files
    void parseGlobs();

    // Separate method, for unit test
    void parseGlobs(const QStringList&);

    struct Glob {
        Glob(int w = 50, const QString& pat = QString() ) : weight(w), pattern(pat) {}
        int weight;
        QString pattern;
    };
    class GlobList : public QList<Glob>
    {
    public:
        bool containsPattern(const QString& pattern) const {
            const_iterator it = begin();
            const const_iterator myend = end();
            for (; it != myend; ++it)
                if ((*it).pattern == pattern)
                    return true;
            return false;
        }
    };
    enum Format { OldGlobs, Globs2WithWeight };

    typedef QHash<QString, GlobList> AllGlobs;
    typedef QHashIterator<QString, GlobList> AllGlobsIterator;

    static AllGlobs parseGlobFiles(const QStringList& globFiles, QStringList& parsedFiles);
    static AllGlobs parseGlobFile(QIODevice* file, Format format);

    // Retrieve the result of the parsing
    const AllGlobs& mimeTypeGlobs() const { return m_mimeTypeGlobs; }
    const QStringList& allMimeTypes() const { return m_allMimeTypes; }

private:
    KMimeTypeFactory* m_mimeTypeFactory;
    AllGlobs m_mimeTypeGlobs;
    QStringList m_allMimeTypes;
};

#endif /* KMIMEFILEPARSER_H */

