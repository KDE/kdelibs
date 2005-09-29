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

#ifndef __kbookmarkimporter_kde1_h
#define __kbookmarkimporter_kde1_h

#include <qdom.h>
#include <qcstring.h>
#include <qstringlist.h>
#include <ksimpleconfig.h>

/**
 * A class for importing the previous bookmarks (desktop files)
 * Separated from KBookmarkManager to save memory (we throw this one
 * out once the import is done)
 */
class KIO_EXPORT KBookmarkImporter
{
public:
    KBookmarkImporter( QDomDocument * doc ) : m_pDoc(doc) {}

    void import( const QString & path );

private:
    void scanIntern( QDomElement & parentElem, const QString & _path );
    void parseBookmark( QDomElement & parentElem, QCString _text,
                        KSimpleConfig& _cfg, const QString &_group );
    QDomDocument * m_pDoc;
    QStringList m_lstParsedDirs;
};

#endif
