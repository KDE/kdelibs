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
    const QStringList globFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", "globs");
    //kDebug() << globFiles;
    QListIterator<QString> globIter(globFiles);
    globIter.toBack();
    while (globIter.hasPrevious()) { // global first, then local. Turns out it doesn't matter though.
        const QString fileName = globIter.previous();
        QFile globFile(fileName);
        kDebug() << "Now parsing" << fileName;
        parseGlobFile(&globFile, fileName);
    }
}

// uses a QIODevice to make unit tests possible
// the filename is only there for the kWarning
void KMimeFileParser::parseGlobFile(QIODevice* file, const QString& fileName)
{
    if (!file->open(QIODevice::ReadOnly))
        return;
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
        KMimeType::Ptr mimeType = m_mimeTypeFactory->findMimeTypeByName(mimeTypeName);
        if (!mimeType)
            kWarning(7012) << fileName << "refers to unknown mimetype" << mimeTypeName;
        else {
            const QString pattern = line.mid(pos+1);
            Q_ASSERT(!pattern.isEmpty());
            mimeType->addPattern(pattern);
        }
    }
}
