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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kpixmapsplitter.h"

KPixmapSplitter::KPixmapSplitter()
    : m_itemSize( 4, 7 ),
      m_vSpacing( 0 ),
      m_hSpacing( 0 ),
      m_numCols( 0 ),
      m_numRows( 0 ),
      m_dirty( false )
{
}

KPixmapSplitter::~KPixmapSplitter()
{
}

void KPixmapSplitter::setPixmap( const QPixmap& pixmap )
{
    m_pixmap = pixmap;
    m_dirty = true;
}

void KPixmapSplitter::setItemSize( const QSize& size )
{
    if ( size != m_itemSize ) {
	m_itemSize = size;
	m_dirty = true;
    }
}

void KPixmapSplitter::setVSpacing( int spacing )
{
    if ( spacing != m_vSpacing ) {
	m_vSpacing = spacing;
	m_dirty = true;
    }
}

void KPixmapSplitter::setHSpacing( int spacing )
{
    if ( spacing != m_hSpacing ) {
	m_hSpacing = spacing;
	m_dirty = true;
    }
}


QRect KPixmapSplitter::coordinates( int pos )
{
    if ( pos < 0 || m_pixmap.isNull() )
	return QRect();

    if ( m_dirty ) {
	m_numCols = m_pixmap.width() / ( m_itemSize.width() + m_hSpacing );
	m_numRows = m_pixmap.height() / ( m_itemSize.height() + m_vSpacing );
	m_dirty = false;
	// qDebug("cols: %i, rows: %i (pixmap: %i, %i)", m_numCols, m_numRows, m_pixmap.width(), m_pixmap.height());
    }

    if ( m_numCols == 0 || m_numRows == 0 )
	return QRect();

    int row = pos / m_numCols;
    int col = pos - (row * m_numCols);

    return QRect( col * (m_itemSize.width() + m_hSpacing),
		  row * (m_itemSize.height() + m_vSpacing),
		  m_itemSize.width(),
		  m_itemSize.height() );
}

QRect KPixmapSplitter::coordinates( const QChar& ch )
{
    return coordinates( (unsigned char) ch.latin1() );
}

