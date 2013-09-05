/* This file is part of the KDE libraries
   Copyright (C) 2002 Alexander Kellett <lypanov@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KBOOKMARKIMPORTER_OPERA_P_H
#define KBOOKMARKIMPORTER_OPERA_P_H

/**
 * A class for importing Opera bookmarks
 * @internal
 */
class KOperaBookmarkImporter : public QObject
{
    Q_OBJECT
public:
    KOperaBookmarkImporter( const QString & fileName ) : m_fileName(fileName) {}
    ~KOperaBookmarkImporter() {}

    void parseOperaBookmarks();

    // Usual place for Opera bookmarks
    static QString operaBookmarksFile();

Q_SIGNALS:
    void newBookmark( const QString & text, const QString & url, const QString & additionalInfo );
    void newFolder( const QString & text, bool open, const QString & additionalInfo );
    void newSeparator();
    void endFolder();

protected:
    QString m_fileName;
};

#endif /* KBOOKMARKIMPORTER_OPERA_P_H */

