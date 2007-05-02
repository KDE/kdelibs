//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
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

#ifndef __kbookmarkimporter_crash_h
#define __kbookmarkimporter_crash_h

#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <kconfig.h>
#include <kdemacros.h>

#include "kbookmarkimporter.h"

/**
 * A class for importing all crash sessions as bookmarks
 * @deprecated
 */
class KIO_EXPORT_DEPRECATED KCrashBookmarkImporter : public QObject
{
    Q_OBJECT
public:
    KCrashBookmarkImporter( const QString & fileName ) : m_fileName(fileName) {}
    ~KCrashBookmarkImporter() {}
    void parseCrashBookmarks( bool del = true );
    static QString crashBookmarksDir( );
Q_SIGNALS:
    void newBookmark( const QString & text, const QString & url, const QString & additionalInfo );
    void newFolder( const QString & text, bool open, const QString & additionalInfo );
    void newSeparator();
    void endFolder();
protected:
    QString m_fileName;
};

/**
 * A class for importing all crash sessions as bookmarks
 */
class KIO_EXPORT KCrashBookmarkImporterImpl : public KBookmarkImporterBase
{
public:
    KCrashBookmarkImporterImpl() : m_shouldDelete(false) { }
    void setShouldDelete(bool);
    virtual void parse();
    virtual QString findDefaultLocation(bool forSaving = false) const;
    static QStringList getCrashLogs();
private:
    bool m_shouldDelete;
    QMap<QString, QString> parseCrashLog_noemit( const QString & filename, bool del );
    class KCrashBookmarkImporterImplPrivate *d;
};

#endif
