
/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
                  2000-2002 Stephan Kulow (coolo@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef _KDEBUG_CLASSES_H_
#define _KDEBUG_CLASSES_H_ "$Id$"

#include "kdebug.h"
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstringlist.h>

#include "kurl.h"

inline kdbgstream operator<<( kdbgstream str, const QPoint & p )  { str << "(" << p.x() << ", " << p.y() << ")"; return str; }
inline kdbgstream operator<<( kdbgstream str, const QSize & s )  { str << "[" << s.width() << "x" << s.height() << "]"; return str; }
inline kdbgstream operator<<( kdbgstream str, const QRect & r )  { str << "[" << r.left() << ", " << r.top() << " - " << r.right() << ", " << r.bottom() << "]"; return str; }

inline kdbgstream operator<<( kdbgstream str, const KURL & u )  { str << u.prettyURL(); return str; }

inline kdbgstream operator<<( kdbgstream str, const QStringList & l )  { 
  str = str.operator<<("("); 
  for (QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) { 
    str = str.operator<<( *it); 
    if (it != l.fromLast())
      str = str.operator<<( ", "); 
  }
  str = str.operator<<(")"); 
  return str;
}

inline kndbgstream operator<<( kndbgstream str, const QPoint & )  { return str; }
inline kndbgstream operator<<( kndbgstream str, const QSize & )  { return str; }
inline kndbgstream operator<<( kndbgstream str, const QRect & )  { return str; }
inline kndbgstream operator<<( kndbgstream str, const KURL & )  { return str; }
inline kndbgstream operator<<( kndbgstream str, const QStringList & ) { return str; }

#endif

