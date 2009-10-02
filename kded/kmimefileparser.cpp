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

#include "kmimefileparser.h"
#include <kglobal.h>
#include <kdeversion.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <kmimetypefactory.h>
#include <kdebug.h>
#include <QtCore/QTextStream>
#include <QtCore/QFile>

KMimeFileParser::KMimeFileParser(KMimeTypeFactory* mimeTypeFactory)
    : m_mimeTypeFactory(mimeTypeFactory)
{
}

void KMimeFileParser::setParsedPatternMap(const ParsedPatternMap& parsedPatternMap)
{
    m_parsedPatternMap = parsedPatternMap;
}

void KMimeFileParser::parseGlobs()
{
    const QStringList globFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", "globs");
    //kDebug() << globFiles;
    parseGlobs(globFiles);
}

void KMimeFileParser::parseGlobs(const QStringList& globFiles)
{
    QStringList parsedFiles;
    m_mimeTypeGlobs = parseGlobFiles(globFiles, parsedFiles);
    m_allMimeTypes = m_mimeTypeGlobs.uniqueKeys();

    // This is just to fill in KMimeType::patterns. This has no real effect
    // on the actual mimetype matching.
    // We only do it for those mimetypes were we just parsed the xml,
    // not those mimetypes we loaded in incremental mode.
    Q_FOREACH(const QString& mimeTypeName, m_allMimeTypes) {
        if (m_parsedPatternMap.contains(mimeTypeName)) {
            KMimeType::Ptr mimeType = m_mimeTypeFactory->findMimeTypeByName(mimeTypeName, KMimeType::DontResolveAlias);
            if (!mimeType) {
                kWarning(7012) << "one of glob files in" << parsedFiles << "refers to unknown mimetype" << mimeTypeName;
                m_mimeTypeGlobs.remove(mimeTypeName);
            } else {
                const GlobList globs = m_mimeTypeGlobs.value(mimeTypeName);
                const QString mainPattern = m_parsedPatternMap.value(mimeTypeName);
                QStringList patterns;
                Q_FOREACH(const Glob& glob, globs) {
                    if (glob.pattern == mainPattern)
                        patterns.prepend(glob.pattern);
                    else
                        patterns.append(glob.pattern);
                }
                mimeType->setPatterns(patterns);
            }
        }
    }
}

KMimeFileParser::AllGlobs KMimeFileParser::parseGlobFiles(const QStringList& globFiles, QStringList& parsedFiles)
{
    KMimeFileParser::AllGlobs allGlobs;
    QListIterator<QString> globIter(globFiles);
    globIter.toBack();
    // At each level, we must be able to override (not just add to) the information that we read at higher levels
    // (if glob-deleteall is used).
    // This is why we don't directly call mimetype->addPattern, nor can we use the same qhash for everything.
    while (globIter.hasPrevious()) { // global first, then local
        Format format = OldGlobs;
        QString fileName = globIter.previous();
        QString fileNamev2 = fileName + '2'; // NOTE: this relies on u-m-d always generating the old globs file
        if (QFile::exists(fileNamev2)) {
            fileName = fileNamev2;
            format = Globs2WithWeight;
        }
        parsedFiles << fileName;
        QFile globFile(fileName);
        //kDebug(7021) << "Now parsing" << fileName;
        parseGlobFile(&globFile, format, allGlobs);
    }
    return allGlobs;
}

// uses a QIODevice to make unit tests possible
bool KMimeFileParser::parseGlobFile(QIODevice* file, Format format, AllGlobs& globs)
{
    if (!file->open(QIODevice::ReadOnly))
        return false;

    // If we're not going to get the "cs" flag because smi is too old, then we need to emulate it for *.C at least.
    const bool caseSensitiveHackNeeded = (KMimeType::sharedMimeInfoVersion() <= KDE_MAKE_VERSION(0, 60, 0));

    QTextStream stream(file);
    //stream.setCodec("UTF-8"); // should be all latin1
    QString line;
    while (!stream.atEnd()) {
        line = stream.readLine();
        if (line.isEmpty() || line.startsWith('#'))
            continue;

        const QStringList fields = line.split(':', QString::KeepEmptyParts);
        if (fields.count() < 2) // syntax error
            continue;

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
            flagList = flagsStr.split(',', QString::SkipEmptyParts);
        } else {
            mimeTypeName = fields[0];
            pattern = fields[1];
        }
        Q_ASSERT(!pattern.isEmpty());
        Q_ASSERT(!pattern.contains(':'));
        bool caseSensitive = flagList.contains("cs");

        if (caseSensitiveHackNeeded && (pattern == "*.C" || pattern == "*.c" || pattern == "core"))
            caseSensitive = true;

        GlobList& globList = globs[mimeTypeName]; // find or create entry
        if (pattern == "__NOGLOBS__") {
            globList.clear();
        } else {
            int flags = 0;
            if (caseSensitive)
                flags = KMimeTypeFactory::CaseSensitive;

            // Check for duplicates, like when installing kde.xml and freedesktop.org.xml
            // in the same prefix, and they both have text/plain:*.txt
            GlobList::iterator existingEntry = globList.findPattern(pattern);
            if (existingEntry == globList.end()) {
                //if (mimeTypeName == "text/plain")
                //    kDebug() << "Adding pattern" << pattern << "to mimetype" << mimeTypeName << "from globs file, with weight" << weight;
                globList.append(Glob(weight, pattern, flags));
            }
        }
    }
    return true;
}
