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

#include "kurldrag.h"
#include <qstrlist.h>
#include <qdragobject.h>

bool KURLDrag::decode( const QMimeSource *e, KURL::List &uris )
{
    QStrList lst;
    bool ret = QUriDrag::decode( e, lst );
    for (QStrListIterator it(lst); *it; ++it)
      uris.append(KURL(*it)); // *it is encoded already
    return ret;
}

bool KURLDrag::decodeLocalFiles( const QMimeSource* e, QStringList& l )
{
  QStrList uris;
  if ( QUriDrag::decode( e, uris ) )
  {
    for (QStrListIterator it(uris); *it; ++it)
    {
      KURL u = *it;
      if (u.isLocalFile())
        l.append(u.path());
    }
    return true;
  }
  return false;
}

