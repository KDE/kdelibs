/* This file is part of the KDE libraries
   Copyright (C) 1996-1998 Martin R. Jones <mjones@kde.org>
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>

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

#ifndef __kbookmarkexporter_h
#define __kbookmarkexporter_h

#include <qtextstream.h>
#include <kbookmark.h>

// Note - KNSBookmarkImporter can be used to get the path to the filename


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

class KBookmarkExporterBase
{
public:
    KBookmarkExporterBase(KBookmarkManager* mgr, const QString & fileName)
        : m_fileName(fileName), m_pManager(mgr) 
    { ; }
    virtual ~KBookmarkExporterBase() {}
    virtual void write(bool utf8 /* true for mozilla */, KBookmarkGroup) = 0;
protected:
    virtual const QString folderAsString(KBookmarkGroup) = 0;
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
    virtual void write(bool utf8, KBookmarkGroup);
protected:
    virtual const QString folderAsString(KBookmarkGroup);
};

#endif
