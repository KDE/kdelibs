/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __kbookmarkimporter_ns_h
#define __kbookmarkimporter_ns_h

#include <qdom.h>
#include <qcstring.h>
#include <qstringlist.h>
#include <ksimpleconfig.h>

#include "kbookmarkimporter.h"
#include "kbookmarkexporter.h"

/**
 * A class for importing NS bookmarks
 * @deprecated
 */
class KNSBookmarkImporter : public QObject
{
    Q_OBJECT
public:
    KNSBookmarkImporter( const QString & fileName ) : m_fileName(fileName) {}
    ~KNSBookmarkImporter() {}

    // for compat reasons only
    void parseNSBookmarks() { parseNSBookmarks(false); }
    // go for it. Set utf8 to true for Mozilla, false for Netscape.
    void parseNSBookmarks( bool utf8 );

    static QString netscapeBookmarksFile( bool forSaving=false );
    static QString mozillaBookmarksFile( bool forSaving=false );

signals:
    void newBookmark( const QString & text, const QCString & url, const QString & additionalInfo );
    void newFolder( const QString & text, bool open, const QString & additionalInfo );
    void newSeparator();
    void endFolder();

protected:
    QString m_fileName;
};

/**
 * A class for importing NS bookmarks
 * utf8 defaults to off
 * @since 3.2
 */
class KNSBookmarkImporterImpl : public KBookmarkImporterBase
{
public:
    KNSBookmarkImporterImpl() : m_utf8(false) { }
    void setUtf8(bool utf8) { m_utf8 = utf8; }
    virtual void parse();
    virtual QString findDefaultLocation(bool forSaving = false) const;
private:
    bool m_utf8;
};

/**
 * A class for importing Mozilla bookmarks
 * utf8 defaults to on
 * @since 3.2
 */
class KMozillaBookmarkImporterImpl : public KNSBookmarkImporterImpl
{
public:
    KMozillaBookmarkImporterImpl() { setUtf8(true); }
};

/**
 * A class that exports all the current bookmarks to Netscape/Mozilla bookmarks
 * Warning, it overwrites the existing bookmarks.html file !
 *
 * only for BC - remove for KDE 4.0
 */
class KNSBookmarkExporter
{
public:
    KNSBookmarkExporter(KBookmarkManager* mgr, const QString & fileName)
      : m_fileName(fileName), m_pManager(mgr) { }
    ~KNSBookmarkExporter() {}

    void write() { write(false); } // deprecated
    void write( bool utf8 );

protected:
    void writeFolder(QTextStream &stream, KBookmarkGroup parent);
    QString m_fileName;
    KBookmarkManager* m_pManager;
};

class KNSBookmarkExporterImpl : public KBookmarkExporterBase
{
public:
    KNSBookmarkExporterImpl(KBookmarkManager* mgr, const QString & fileName)
      : KBookmarkExporterBase(mgr, fileName) 
    { ; }
    virtual ~KNSBookmarkExporterImpl() {}
    virtual void write(KBookmarkGroup);
    void setUtf8(bool);
protected:
    virtual const QString folderAsString(KBookmarkGroup);
private:
    bool m_utf8;
};

#endif
