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

#ifndef KPIXMAPSPLITTER_H
#define KPIXMAPSPLITTER_H

#include <config.h>

#include <qpixmap.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>

class KPixmapSplitterPrivate;
/**
 * If you have a pixmap containing several items (icons), you can use this
 * class to get the coordinates of each item.
 *
 * For example, if you have a pixmap with 25 items and you want to get the
 * 4th item as a pixmap (every item being 20x10 pixels):
 * <pre>
 * KPixmapSplitter splitter;
 * splitter.setPixmap( somePixmap );
 * splitter.setItemSize( QSize( 20, 10 ));
 *
 * QPixmap item( 20, 10 );
 * item.fill( Qt::white );
 * QRect rect = splitter.coordinates( 4 );
 * if ( !rect.isEmpty() )
 *     bitBlt( &item, QPoint(0,0), &somePixmap, rect, CopyROP );
 * </pre>
 *
 * @short A class to split a pixmap into several items.
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KPixmapSplitter
{
public:
    /**
     * Constructor, does nothing but initializing some default-values.
     */
    KPixmapSplitter();
    ~KPixmapSplitter();

    /**
     * Sets the pixmap to be splitted.
     */
    void setPixmap( const QPixmap& pixmap );

    /**
     * @returns the pixmap that has been set via @ref setPixmap().
     */
    const QPixmap& pixmap() const { return m_pixmap; }

    /**
     * Sets the size of the items you want to get out of the given pixmap.
     * The QRect of #coordinates(int) will have the width and height of exactly
     * this @p size.
     */
    void setItemSize( const QSize& size );

    /**
     * @returns the set size of the items (coordinates) you want to get
     * out of the given pixmap.
     */
    QSize itemSize() const { return m_itemSize; }

    /**
     * If there is space between rows in the given pixmap, you have to specify
     * how many pixels there are.
     */
    void setVSpacing( int spacing );

    /**
     * If there is space between columns in the given pixmap, you have to
     * specify how many pixels there are.
     */
    void setHSpacing( int spacing );

    /**
     * @returns the coordinates of the item at position pos in the given
     * pixmap.
     */
    QRect coordinates( int pos );

    /**
     * Overloaded for convenience. Returns the item at the position of the
     * given character (when using a latin1 font-pixmap)
     */
    QRect coordinates( const QChar& ch );

private:
    QPixmap m_pixmap;
    QSize m_itemSize;

    int m_vSpacing;
    int m_hSpacing;

    int m_numCols;
    int m_numRows;

    bool m_dirty;
    KPixmapSplitterPrivate* d;
};

#endif // KPIXMAPSPLITTER_H
