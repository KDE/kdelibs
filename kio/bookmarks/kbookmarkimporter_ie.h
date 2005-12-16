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

#ifndef __kbookmarkimporter_ie_h
#define __kbookmarkimporter_ie_h

#include <qdom.h>
#include <qstringlist.h>
#include <ksimpleconfig.h>
#include <kdemacros.h>

#include <kbookmarkimporter.h>

/**
 * A class for importing IE bookmarks
 * @deprecated
 */
class KIO_EXPORT_DEPRECATED KIEBookmarkImporter : public QObject
{
    Q_OBJECT
public:
    KIEBookmarkImporter( const QString & fileName ) : m_fileName(fileName) {}
    ~KIEBookmarkImporter() {}

    void parseIEBookmarks();

    // Usual place for IE bookmarks
    static QString IEBookmarksDir();

signals:
    void newBookmark( const QString & text, const QString & url, const QString & additionalInfo );
    void newFolder( const QString & text, bool open, const QString & additionalInfo );
    void newSeparator();
    void endFolder();

protected:
    void parseIEBookmarks_dir( const QString &dirname, const QString &name = QString::null );
    void parseIEBookmarks_url_file( const QString &filename, const QString &name );

    QString m_fileName;
};

/**
 * A class for importing IE bookmarks
 * @since 3.2
 */
class KIO_EXPORT KIEBookmarkImporterImpl : public KBookmarkImporterBase
{
public:
    KIEBookmarkImporterImpl() { }
    virtual void parse();
    virtual QString findDefaultLocation(bool forSaving = false) const;
private:
    class KIEBookmarkImporterImplPrivate *d;
};

/*
 * @since 3.2
 */
class KIO_EXPORT KIEBookmarkExporterImpl : public KBookmarkExporterBase
{
public:
    KIEBookmarkExporterImpl(KBookmarkManager* mgr, const QString & path)
      : KBookmarkExporterBase(mgr, path)
    { ; }
    virtual ~KIEBookmarkExporterImpl() {}
    virtual void write(KBookmarkGroup);
private:
    class KIEBookmarkExporterImplPrivate *d;
};

#endif
