
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
inline kdbgstream operator<<( kdbgstream str, const QRegion & reg ) {
  str = str.operator<<("[ ");
  QMemArray<QRect>rs=reg.rects();
  for (uint i=0;i<rs.size();++i)
    str = str.operator<<( QString("[%1, %2 - %3, %4] ").arg(rs[i].left()).arg(rs[i].top()).arg(rs[i].right()).arg(rs[i].bottom()));
  str = str.operator<<("]");
  return str;
}

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
inline kdbgstream operator<<( kdbgstream str, const QColor &c )  {
    if ( c.isValid() )
        str = str.operator<<(c.name());
    else
        str = str.operator<<("(invalid/default)");
    return str;
}
inline kdbgstream operator<<( kdbgstream str, const QBrush &b )  {
    static const char* const s_brushStyles[] = {
        "NoBrush", "SolidPattern", "Dense1Pattern", "Dense2Pattern", "Dense3Pattern",
        "Dense4Pattern", "Dense5Pattern", "Dense6Pattern", "Dense7Pattern",
        "HorPattern", "VerPattern", "CrossPattern", "BDiagPattern", "FDiagPattern",
        "DiagCrossPattern" };
    str = str.operator<<("[ style: ");
    str = str.operator<<(s_brushStyles[ b.style() ]);
    str = str.operator<<(" color: ");
    // can't use operator<<(str, b.color()) because that terminates a kdbgstream (flushes)
    if ( b.color().isValid() )
        str = str.operator<<(b.color().name());
    else
        str = str.operator<<("(invalid/default)");
    if ( b.pixmap() )
        str = str.operator<<(" has a pixmap");
    str = str.operator<<(" ]");
    return str;
}

inline kndbgstream operator<<( kndbgstream str, const QPoint & )  { return str; }
inline kndbgstream operator<<( kndbgstream str, const QSize & )  { return str; }
inline kndbgstream operator<<( kndbgstream str, const QRect & )  { return str; }
inline kndbgstream operator<<( kndbgstream str, const QRegion & ) { return str; }
inline kndbgstream operator<<( kndbgstream str, const KURL & )  { return str; }
inline kndbgstream operator<<( kndbgstream str, const QStringList & ) { return str; }
inline kndbgstream operator<<( kndbgstream str, const QColor & ) { return str; }
inline kndbgstream operator<<( kndbgstream str, const QBrush & ) { return str; }

#endif

