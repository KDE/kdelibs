/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
                  2002 Holger Freyther (freyther@kde.org)

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef QT_NO_CAST_FROM_ASCII
#define QT_NO_CAST_FROM_ASCII
#endif
#ifndef QT_NO_CAST_TO_ASCII
#define QT_NO_CAST_TO_ASCII
#endif

#include "kdebug.h"

#include <qwidget.h>
#include <qregion.h>
#include <qpen.h>
#include <qbrush.h>

// This file contains kdebug operators that cannot be in kdecore because of ui dependency.

kdbgstream& operator << (kdbgstream& stream, const QWidget* widget)
{
  QString output;
  if(widget==0) {
      output += QLatin1String("[Null pointer]");
  } else {
      output += QString::fromAscii("[%1 pointer(0x%2)")
                         .arg(QString::fromUtf8(widget->metaObject()->className()))
                         .arg(QString::number(ulong(widget), 16)
		              .rightJustified(8, QLatin1Char('0')));
      if(widget->objectName().isEmpty()) {
	  output += QLatin1String( " to unnamed widget, " );
      } else {
	  output += QString::fromAscii(" to widget %1, ")
	                    .arg(widget->objectName());
      }
      output += QString::fromAscii("geometry=%1x%2+%3+%4]")
                       .arg(widget->width()).arg(widget->height())
                       .arg(widget->x()).arg(widget->y());
  }
  return stream << output;
}

static QString s_makeColorName(const QColor& c) {
    QString s = QLatin1String("(invalid/default)");
    if ( c.isValid() )
        s = c.name();
    return s;
}
kdbgstream& operator<<( kdbgstream& stream, const QColor& c ) {
    return stream << s_makeColorName( c );
}
kdbgstream& operator<<( kdbgstream& stream, const QPen& p ) {
    static const char* const s_penStyles[] = {
        "NoPen", "SolidLine", "DashLine", "DotLine", "DashDotLine",
        "DashDotDotLine" };
    static const char* const s_capStyles[] = {
        "FlatCap", "SquareCap", "RoundCap" };

    QString output;
    output += QLatin1String("[ style:");
    output += QLatin1String(s_penStyles[ p.style() ]);
    output += QString::fromAscii(" width:%1").arg(p.width());
    output += QLatin1String(" color:") + s_makeColorName( p.color() );
    if ( p.width() > 0 ) // cap style doesn't matter, otherwise
    {
        output += QLatin1String(" capstyle:") +
	             QLatin1String(s_capStyles[ p.capStyle() >> 4 ]);
        // join style omitted
    }
    output += QLatin1String(" ]");
    return stream << output;
}
kdbgstream& operator<<( kdbgstream& stream, const QBrush& b) {
    static const char* const s_brushStyles[] = {
        "NoBrush", "SolidPattern", "Dense1Pattern", "Dense2Pattern", "Dense3Pattern",
        "Dense4Pattern", "Dense5Pattern", "Dense6Pattern", "Dense7Pattern",
        "HorPattern", "VerPattern", "CrossPattern", "BDiagPattern", "FDiagPattern",
        "DiagCrossPattern", "LinearGradientPattern", "ConicalGradientPattern",
        "RadialGradientPattern", "TexturePattern"
    };

    QString output;
    output += QLatin1String("[ style: ");
    output += QLatin1String(s_brushStyles[ b.style() ]);
    output += QLatin1String(" color: ");
    output += s_makeColorName( b.color() );
    if ( !b.texture().isNull() )
        output += QLatin1String(" has a pixmap");
    output += QLatin1String(" ]");
    return stream << output;
}
template <class Rect>
static QString s_rectString(const Rect& r)
{
    QString str = QString::fromAscii("[%1,%2 - %3x%4]");
    return str.arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height());
}
kdbgstream& operator<<( kdbgstream& stream, const QRegion& reg ) {
    QString output;
    output += QLatin1String( "[ " );

    QVector<QRect>rs=reg.rects();
    for (int i=0;i<rs.size();++i)
        output += s_rectString( rs[i] ) + QLatin1Char( ' ' );

    output += QLatin1String( "]" );

    return stream << output;
}
