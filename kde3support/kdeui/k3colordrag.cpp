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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qpainter.h>
#include "k3colordrag.h"

static const char * const color_mime_string = "application/x-color";
static const char * const text_mime_string = "text/plain";

K3ColorDrag::K3ColorDrag( const QColor &color, QWidget *dragsource,
			const char *name)
     : Q3StoredDrag( color_mime_string, dragsource, name)
{
     setColor( color);
}

K3ColorDrag::K3ColorDrag( QWidget *dragsource, const char *name)
     : Q3StoredDrag( color_mime_string, dragsource, name)
{
     setColor( Qt::white );
}

void
K3ColorDrag::setColor( const QColor &color)
{
     Q3ColorDrag tmp(color, 0, 0);
     setEncodedData(tmp.encodedData(color_mime_string));

     QPixmap colorpix( 25, 20);
     colorpix.fill( color);
     QPainter p( &colorpix );
     p.setPen( Qt::black );
     p.drawRect(0,0,25,20);
     p.end();
     setPixmap(colorpix, QPoint(-5,-7));
}

const char *K3ColorDrag::format(int i) const
{
     if (i==1)
        return text_mime_string;
     else
        return Q3StoredDrag::format(i);
}

QByteArray K3ColorDrag::encodedData ( const char * m ) const
{
     if (!qstrcmp(m, text_mime_string) )
     {
        QColor color;
        Q3ColorDrag::decode(const_cast<K3ColorDrag *>(this), color);
        QByteArray result = color.name().latin1();
        result.resize(result.length());
        return result;
     }
     return Q3StoredDrag::encodedData(m);
}

bool
K3ColorDrag::canDecode( QMimeSource *e)
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
K3ColorDrag::decode( QMimeSource *e, QColor &color)
{
     if (Q3ColorDrag::decode(e, color))
        return true;

     QByteArray data = e->encodedData( text_mime_string);
     QString colorName = QString::fromLatin1(data.data(), data.size());
     if ((colorName.length() < 4) || (colorName[0] != '#'))
        return false;
     color.setNamedColor(colorName);
     return color.isValid();
}


K3ColorDrag*
K3ColorDrag::makeDrag( const QColor &color,QWidget *dragsource)
{
     return new K3ColorDrag( color, dragsource);
}

void K3ColorDrag::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "k3colordrag.moc"
