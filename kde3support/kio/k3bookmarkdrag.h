//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef K3BOOKMARKDRAG_H
#define K3BOOKMARKDRAG_H

#include <kde3support_export.h>

#include <kbookmark.h>
#include <Qt3Support/Q3ColorDrag>
#include <Qt3Support/Q3ValueList>

// Clipboard/dnd data : URLs + XML for bookmarks
class KDE3SUPPORT_EXPORT K3BookmarkDrag : public Q3UriDrag
{
public:
    static K3BookmarkDrag * newDrag( const Q3ValueList<KBookmark> & bookmarks,
                                    QWidget * dragSource = 0,
                                    const char * name = 0 );
    static K3BookmarkDrag * newDrag( const KBookmark & bookmark,
                                    QWidget * dragSource = 0,
                                    const char * name = 0 );
protected:
    K3BookmarkDrag( const Q3ValueList<KBookmark> & bookmarks,
                   const Q3StrList & urls,
                   QWidget * dragSource,
                   const char * name );
public:
    virtual ~K3BookmarkDrag() {}

    virtual const char* format( int i ) const;
    virtual QByteArray encodedData( const char* mime ) const;

    static bool canDecode( const QMimeSource * e );
    static Q3ValueList<KBookmark> decode( const QMimeSource * e );

protected:
    Q3ValueList<KBookmark> m_bookmarks;
    QDomDocument m_doc;
};
#endif
