/* This file is part of the KDE libraries
    Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include "kpixmapsplitter.h"

class KPixmapSplitter::KPixmapSplitterPrivate
{
public:
    QPixmap pixmap;
    QSize itemSize;

    int vSpacing;
    int hSpacing;

    int numCols;
    int numRows;

    bool dirty;

    KPixmapSplitterPrivate()
     : itemSize( 4, 7 ),
       vSpacing( 0 ),
       hSpacing( 0 ),
       numCols( 0 ),
       numRows( 0 ),
       dirty( false )
    {
    }
};

KPixmapSplitter::KPixmapSplitter()
 : d(new KPixmapSplitterPrivate)
{
}

KPixmapSplitter::~KPixmapSplitter()
{
    delete d;
}

void KPixmapSplitter::setPixmap( const QPixmap& pixmap )
{
    d->pixmap = pixmap;
    d->dirty = true;
}

const QPixmap &KPixmapSplitter::pixmap() const
{
    return d->pixmap;
}

void KPixmapSplitter::setItemSize( const QSize& size )
{
    if ( size != d->itemSize ) {
	d->itemSize = size;
	d->dirty = true;
    }
}

const QSize &KPixmapSplitter::itemSize() const
{
    return d->itemSize;
}

void KPixmapSplitter::setVSpacing( int spacing )
{
    if ( spacing != d->vSpacing ) {
	d->vSpacing = spacing;
	d->dirty = true;
    }
}

void KPixmapSplitter::setHSpacing( int spacing )
{
    if ( spacing != d->hSpacing ) {
	d->hSpacing = spacing;
	d->dirty = true;
    }
}


QRect KPixmapSplitter::coordinates( int pos )
{
    if ( pos < 0 || d->pixmap.isNull() )
	return QRect();

    if ( d->dirty ) {
	d->numCols = d->pixmap.width() / ( d->itemSize.width() + d->hSpacing );
	d->numRows = d->pixmap.height() / ( d->itemSize.height() + d->vSpacing );
	d->dirty = false;
	// qDebug("cols: %i, rows: %i (pixmap: %i, %i)", m_numCols, m_numRows, m_pixmap.width(), m_pixmap.height());
    }

    if ( d->numCols == 0 || d->numRows == 0 )
	return QRect();

    int row = pos / d->numCols;
    int col = pos - (row * d->numCols);

    return QRect( col * (d->itemSize.width() + d->hSpacing),
		  row * (d->itemSize.height() + d->vSpacing),
		  d->itemSize.width(),
		  d->itemSize.height() );
}

QRect KPixmapSplitter::coordinates( const QChar& ch )
{
    return coordinates( (unsigned char) ch.toLatin1() );
}

