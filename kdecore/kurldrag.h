/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __KURLDRAG_H
#define __KURLDRAG_H

#include <qstringlist.h>
#include <kurl.h>
class QMimeSource;

class KURLDrag
{
public:
  
  /**
   * Convenience method that decodes the contents of @p e
   * into a list of KURLs.
   * Do not use QUriDrag::decodeToUnicodeUris (see below why).
   */
  static bool decode( const QMimeSource *e, KURL::List &uris );

  /**
   * QUriDrag assumes that URLs contain paths encoded in UTF-8.
   * This is just not true (they contain paths in locale-8-bit), so please
   * use KURLDrag::decodeLocalFiles instead of QUriDrag::decodeLocalFiles.
   */
  static bool decodeLocalFiles( const QMimeSource* e, QStringList& l );

};

#endif

