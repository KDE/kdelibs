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

void KMimeFileParser::parseGlobs()
{
    // TODO parse globs2 file instead if it exists.
    // This will fix http://bugs.freedesktop.org/show_bug.cgi?id=15436
    const QStringList globFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", "globs");
    //kDebug() << globFiles;
    parseGlobs(globFiles);
}

void KMimeFileParser::parseGlobs(const QStringList& globFiles)
{
    QListIterator<QString> globIter(globFiles);
    globIter.toBack();
    // At each level, we must be able to override (not just add to) the information that we read at higher levels.
    // This is why we don't directly call mimetype->addPattern, nor can we use the same qhash for everything.
    QHash<QString, QStringList> mimeTypeGlobs;
    while (globIter.hasPrevious()) { // global first, then local
        const QString fileName = globIter.previous();
        QFile globFile(fileName);
        kDebug(7021) << "Now parsing" << fileName;
        const QHash<QString, QStringList> thisLevelGlobs = parseGlobFile(&globFile);
        if (mimeTypeGlobs.isEmpty())
            mimeTypeGlobs = thisLevelGlobs;
        else {
            // We insert stuff multiple times into the hash, and we only look at the last inserted later on.
            mimeTypeGlobs.unite(thisLevelGlobs);
        }
    }

    const QStringList allMimes = mimeTypeGlobs.uniqueKeys();
    Q_FOREACH(const QString& mimeTypeName, allMimes) {
        KMimeType::Ptr mimeType = m_mimeTypeFactory->findMimeTypeByName(mimeTypeName);
        if (!mimeType) {
            kWarning(7012) << "one of glob files in" << globFiles << "refers to unknown mimetype" << mimeTypeName;
        } else {
            mimeType->setPatterns(mimeTypeGlobs.value(mimeTypeName));
        }
    }
}

// uses a QIODevice to make unit tests possible
QHash<QString, QStringList> KMimeFileParser::parseGlobFile(QIODevice* file)
{
    QHash<QString, QStringList> globs;
    if (!file->open(QIODevice::ReadOnly))
        return globs;
    QTextStream stream(file);
    //stream.setCodec("UTF-8"); // should be all latin1
    QString line;
    while (!stream.atEnd()) {
        line = stream.readLine();
        if (line.isEmpty() || line.startsWith('#'))
            continue;
        const int pos = line.indexOf(':');
        if (pos == -1) // syntax error
            continue;
        const QString mimeTypeName = line.left(pos);
        const QString pattern = line.mid(pos+1);
        Q_ASSERT(!pattern.isEmpty());
        //if (mimeTypeName == "text/plain")
        //    kDebug() << "Adding pattern" << pattern << "to mimetype" << mimeTypeName << "from globs file";
        QStringList& patterns = globs[mimeTypeName]; // find or create entry
        if (!patterns.contains(pattern)) // ### I miss a QStringList::makeUnique or something...
            patterns.append(pattern);
    }
    return globs;
}
