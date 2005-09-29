/* This file is part of the KDE libraries
   Copyright (C) 1999 Steffen Hansen (hansen@kde.org)

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

#include <qpainter.h>
#include "kcolordrag.h"

static const char * const color_mime_string = "application/x-color";
static const char * const text_mime_string = "text/plain";

KColorDrag::KColorDrag( const QColor &color, QWidget *dragsource,
			const char *name)
     : QStoredDrag( color_mime_string, dragsource, name)
{
     setColor( color);
}

KColorDrag::KColorDrag( QWidget *dragsource, const char *name)
     : QStoredDrag( color_mime_string, dragsource, name)
{
     setColor( white );
}

void
KColorDrag::setColor( const QColor &color)
{
     QColorDrag tmp(color, 0, 0);
     setEncodedData(tmp.encodedData(color_mime_string));

     QPixmap colorpix( 25, 20);
     colorpix.fill( color);
     QPainter p( &colorpix );
     p.setPen( black );
     p.drawRect(0,0,25,20);
     p.end();
     setPixmap(colorpix, QPoint(-5,-7));
}

const char *KColorDrag::format(int i) const
{
     if (i==1)
        return text_mime_string;
     else
        return QStoredDrag::format(i);
}

QByteArray KColorDrag::encodedData ( const char * m ) const
{
     if (!qstrcmp(m, text_mime_string) )
     {
        QColor color;
        QColorDrag::decode(const_cast<KColorDrag *>(this), color);
        QCString result = color.name().latin1();
        ((QByteArray&)result).resize(result.length());
        return result;
     }
     return QStoredDrag::encodedData(m);
}

bool
KColorDrag::canDecode( QMimeSource *e)
{
     if (e->provides(color_mime_string))
        return true;
     if (e->provides(text_mime_string))
     {
        QColor dummy;
        return decode(e, dummy);
     }
     return false;
}

bool
KColorDrag::decode( QMimeSource *e, QColor &color)
{
     if (QColorDrag::decode(e, color))
        return true;

     QByteArray data = e->encodedData( text_mime_string);
     QString colorName = QString::fromLatin1(data.data(), data.size());
     if ((colorName.length() < 4) || (colorName[0] != '#'))
        return false;
     color.setNamedColor(colorName);
     return color.isValid();
}


KColorDrag*
KColorDrag::makeDrag( const QColor &color,QWidget *dragsource)
{
     return new KColorDrag( color, dragsource);
}

void KColorDrag::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kcolordrag.moc"
