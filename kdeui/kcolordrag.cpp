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
/*
 * $Id$
 *
 * $Log$
 * Revision 1.3  2001/08/26 20:14:19  lunakl
 * Ok, watch closely :
 * const is your friend !
 *
 * static int blah[] = { 1, 2, .... }
 * should be
 * static const int blah[] = { 1, 2, ... }
 *
 * static const char* txt[] = { "blah", "foo", ... }
 * should be
 * static const char* const txt[] = { "blah", "foo", ... }
 *
 * And just in case you wonder about those const_cast< const char** >, that's
 * because QPixmap( const char** xpm ) and QImage( const char** xpm ) got it
 * wrong too. Everybody guessing correctly where the const is missing wins
 * a free cvs update.
 *
 * Revision 1.2  2000/02/12 12:46:41  espen
 * Added a black frame around the drag pixmap. Looks better.
 *
 * Revision 1.1  1999/05/06 02:46:13  steffen
 * Drag&drop for colors. Qt drag&drop is really easy to use. We should have stuff like this all over KDE.
 *
 *
 */

#include <qpainter.h> 
#include "kcolordrag.h"

static const char * const color_mime_string = "application/x-color";

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
     QByteArray data;
     // A short int for each of R G B Alpha (compatible with gtk);
     unsigned short int  rgba[4];
     data.resize( sizeof(rgba));
     rgba[0] = color.red()   * 0xFF;
     rgba[1] = color.green() * 0xFF;
     rgba[2] = color.blue()  * 0xFF;
     rgba[3] = 0xFFFF; // Alpha not supported yet.
     memcpy( data.data(), rgba, sizeof( rgba));
     setEncodedData( data);

     QPixmap colorpix( 25, 20);
     colorpix.fill( color);
     QPainter p( &colorpix );
     p.setPen( black );
     p.drawRect(0,0,25,20);
     p.end();
     setPixmap(colorpix, QPoint(-5,-7));
}

bool 
KColorDrag::canDecode( QMimeSource *e)
{
     return e->provides( color_mime_string);
}

bool
KColorDrag::decode( QMimeSource *e, QColor &color)
{
     QByteArray data = e->encodedData( color_mime_string);
     unsigned short int rgba[4];
     if( data.size() != sizeof(rgba)) return false;
     memcpy( rgba, data.data(), sizeof( rgba));
     color.setRgb( rgba[0] / 0xFF, rgba[1] / 0xFF, rgba[2] / 0xFF); 
     return true;
}


KColorDrag* 
KColorDrag::makeDrag( const QColor &color,QWidget *dragsource)
{
     return new KColorDrag( color, dragsource);
}
#include "kcolordrag.moc"
