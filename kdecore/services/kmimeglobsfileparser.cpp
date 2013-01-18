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

#include "kmimeglobsfileparser_p.h"
#include <kglobal.h>
#include <kdeversion.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include "kmimetyperepository_p.h"
#include <kdebug.h>
#include <QtCore/QTextStream>
#include <QtCore/QFile>

KMimeGlobsFileParser::KMimeGlobsFileParser()
{
}

KMimeGlobsFileParser::AllGlobs KMimeGlobsFileParser::parseGlobs()
{
    const QStringList globFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", QString::fromLatin1("globs"));
    //kDebug() << globFiles;
    return parseGlobs(globFiles);
}

KMimeGlobsFileParser::AllGlobs KMimeGlobsFileParser::parseGlobs(const QStringList& globFiles)
{
    QStringList parsedFiles;
    return parseGlobFiles(globFiles, parsedFiles);
}

KMimeGlobsFileParser::AllGlobs KMimeGlobsFileParser::parseGlobFiles(const QStringList& globFiles, QStringList& parsedFiles)
{
    KMimeGlobsFileParser::AllGlobs allGlobs;
    QListIterator<QString> globIter(globFiles);
    globIter.toBack();
    // At each level, we must be able to override (not just add to) the information that we read at higher levels
    // (if glob-deleteall is used).
    while (globIter.hasPrevious()) { // global first, then local
        Format format = OldGlobs;
        QString fileName = globIter.previous();
        QString fileNamev2 = fileName + QLatin1Char('2'); // NOTE: this relies on u-m-d always generating the old globs file
        if (QFile::exists(fileNamev2)) {
            fileName = fileNamev2;
            format = Globs2WithWeight;
        }
        parsedFiles << fileName;
        QFile globFile(fileName);
        //kDebug() << "Now parsing" << fileName;
        parseGlobFile(&globFile, format, allGlobs);
    }
    return allGlobs;
}

// uses a QIODevice to make unit tests possible
bool KMimeGlobsFileParser::parseGlobFile(QIODevice* file, Format format, AllGlobs& globs)
{
    if (!file->open(QIODevice::ReadOnly))
        return false;

    // If we're not going to get the "cs" flag because smi is too old, then we need to emulate it for *.C at least.
    const bool caseSensitiveHackNeeded = (KMimeType::sharedMimeInfoVersion() <= KDE_MAKE_VERSION(0, 60, 0));

    QTextStream stream(file);
    //stream.setCodec("UTF-8"); // should be all latin1
    QString lastMime, lastPattern;
    QString line;
    while (!stream.atEnd()) {
        line = stream.readLine();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
            continue;

        const QStringList fields = line.split(QLatin1Char(':'), QString::KeepEmptyParts);
        if (fields.count() < 2) // syntax error
            continue;

        //kDebug() << "line=" << line;

        QString mimeTypeName, pattern;
        QStringList flagList;
        int weight = 50;
        if (format == Globs2WithWeight) {
            if (fields.count() < 3) // syntax error
                continue;
            weight = fields[0].toInt();
            mimeTypeName = fields[1];
            pattern = fields[2];
            const QString flagsStr = fields.value(3); // could be empty
            flagList = flagsStr.split(QLatin1Char(','), QString::SkipEmptyParts);
        } else {
            mimeTypeName = fields[0];
            pattern = fields[1];
        }
        Q_ASSERT(!pattern.isEmpty());
        Q_ASSERT(!pattern.contains(QLatin1Char(':')));

        //kDebug() << " got:" << mimeTypeName << pattern;

        if (lastMime == mimeTypeName && lastPattern == pattern) {
            // Ignore duplicates, especially important for those with no flags after a line with flags:
            // 50:text/x-csrc:*.c:cs
            // 50:text/x-csrc:*.c
            continue;
        }

        bool caseSensitive = flagList.contains(QLatin1String("cs"));

        if (caseSensitiveHackNeeded && (pattern == QLatin1String("*.C") || pattern == QLatin1String("*.c") || pattern == QLatin1String("core")))
            caseSensitive = true;

        if (pattern == QLatin1String("__NOGLOBS__")) {
            //kDebug() << "removing" << mimeTypeName;
            globs.removeMime(mimeTypeName);
            lastMime.clear();
        } else {
            int flags = 0;
            if (caseSensitive)
                flags = KMimeTypeRepository::CaseSensitive;

            //if (mimeTypeName == "text/plain")
            //    kDebug() << "Adding pattern" << pattern << "to mimetype" << mimeTypeName << "from globs file, with weight" << weight;
            //if (pattern.toLower() == "*.c")
            //    kDebug() << " Adding pattern" << pattern << "to mimetype" << mimeTypeName << "from globs file, with weight" << weight << "flags" << flags;
            globs.addGlob(Glob(mimeTypeName, weight, pattern, flags));
            lastMime = mimeTypeName;
            lastPattern = pattern;
        }
    }
    return true;
}

static bool isFastPattern(const QString& pattern)
{
   // starts with "*.", has no other '*' and no other '.'
   return pattern.lastIndexOf(QLatin1Char('*')) == 0
      && pattern.lastIndexOf(QLatin1Char('.')) == 1
      // and contains no other special character
      && !pattern.contains(QLatin1Char('?'))
      && !pattern.contains(QLatin1Char('['))
      ;
}

void KMimeGlobsFileParser::AllGlobs::addGlob(const Glob& glob)
{
    // Note that in each case, we check for duplicates to avoid inserting duplicated patterns.
    // This can happen when installing kde.xml and freedesktop.org.xml
    // in the same prefix, and they both have text/plain:*.txt

    const QString &pattern = glob.pattern;
    Q_ASSERT(!pattern.isEmpty());

    //kDebug() << "pattern" << pattern << "glob.weight=" << glob.weight << "isFast=" << isFastPattern(pattern) << glob.flags;

    // Store each patterns into either m_fastPatternDict (*.txt, *.html etc. with default weight 50)
    // or for the rest, like core.*, *.tar.bz2, *~, into highWeightPatternOffset (>50)
    // or lowWeightPatternOffset (<=50)

    if (glob.weight == 50 && isFastPattern(pattern) && ((glob.flags & KMimeTypeRepository::CaseSensitive) == 0)) {
        // The bulk of the patterns is *.foo with weight 50 --> those go into the fast patterns hash.
        const QString extension = pattern.mid(2).toLower();
        QStringList& patterns = m_fastPatterns[extension]; // find or create
        if (!patterns.contains(glob.mimeType))
            patterns.append(glob.mimeType);
    } else {
        Glob adjustedGlob(glob);
        if ((adjustedGlob.flags & KMimeTypeRepository::CaseSensitive) == 0)
            adjustedGlob.pattern = adjustedGlob.pattern.toLower();
        if (adjustedGlob.weight > 50) {
            if (!m_highWeightGlobs.hasPattern(adjustedGlob.mimeType, adjustedGlob.pattern))
                m_highWeightGlobs.append(adjustedGlob);
        } else {
            if (!m_lowWeightGlobs.hasPattern(adjustedGlob.mimeType, adjustedGlob.pattern))
                m_lowWeightGlobs.append(adjustedGlob);
        }
    }
}

KMimeGlobsFileParser::PatternsMap KMimeGlobsFileParser::AllGlobs::patternsMap() const
{
    PatternsMap patMap;

    // This is just to fill in KMimeType::patterns. This has no real effect
    // on the actual mimetype matching.

    QHash<QString, QStringList>::const_iterator it = m_fastPatterns.begin();
    const QHash<QString, QStringList>::const_iterator end = m_fastPatterns.end();
    for (; it != end; ++it) {
        Q_FOREACH(const QString& mime, it.value())
            patMap[mime].append(QString::fromLatin1("*.") + it.key());
    }

    Q_FOREACH(const Glob& glob, m_highWeightGlobs)
        patMap[glob.mimeType].append(glob.pattern);

    Q_FOREACH(const Glob& glob, m_lowWeightGlobs)
        patMap[glob.mimeType].append(glob.pattern);

    return patMap;
}

void KMimeGlobsFileParser::AllGlobs::removeMime(const QString& mime)
{
    QMutableHashIterator<QString, QStringList> it(m_fastPatterns);
    while (it.hasNext()) {
        it.next().value().removeAll(mime);
    }
    m_highWeightGlobs.removeMime(mime);
    m_lowWeightGlobs.removeMime(mime);
}
