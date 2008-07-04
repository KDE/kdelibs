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
    static QHash<QString, QStringList> parseGlobFile(QIODevice* file);

private:
    KMimeTypeFactory* m_mimeTypeFactory;

};

#endif /* KMIMEFILEPARSER_H */

