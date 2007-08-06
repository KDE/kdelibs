/* This file is part of the KDE libraries
   Copyright (C) 1999 Daniel M. Duley <mosfet@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef __KDRAWUTIL_H
#define __KDRAWUTIL_H

#include <kdefx_export.h>

#include <QtCore/QCOORD>
#include <QtGui/QPainter>
#include <QtGui/QBitmap>
#include <QtGui/QPalette>

/*
 * Various drawing routines. Also see Qt's qdrawutil.h for some more routines
 * contained in Qt.
 *
 * (C) Daniel M. Duley <mosfet@kde.org>
 */

/**
 * @relates KStyle
 * @c \#include @c <kdrawutil.h>
 *
 * Paints the provided bitmaps in the painter, using the supplied colorgroup for
 * the foreground colors. There's one bitmap for each color. If you want to skip
 * a color, pass @a null for the corresponding bitmap.
 *
 * @note The bitmaps will be self-masked automatically if not masked
 *       prior to calling this routine.
 *
 * @param p             The painter to use for drawing the bitmaps.
 * @param pal           Specifies the shading colors.
 * @param x             The X coordinate at which to draw the bitmaps.
 * @param y             The Y coordinate at which to draw the bitmaps.
 * @param lightColor    The bitmap to use for the light part.
 * @param midColor      The bitmap to use for the mid part.
 * @param midlightColor The bitmap to use for the midlight part.
 * @param darkColor     The bitmap to use for the dark part.
 * @param blackColor    The bitmap to use for the black part.
 * @param whiteColor    The bitmap to use for the white part.
 *
 * @see QPalette
 */
KDEFX_EXPORT void kColorBitmaps(QPainter *p, const QPalette &pal, int x, int y,
                   QBitmap *lightColor=0, QBitmap *midColor=0,
                   QBitmap *midlightColor=0, QBitmap *darkColor=0,
                   QBitmap *blackColor=0, QBitmap *whiteColor=0);

/**
 * @relates KStyle
 * @c \#include @c <kdrawutil.h>
 * @overload
 */
 KDEFX_EXPORT void kColorBitmaps(QPainter *p, const QPalette &pal, int x, int y, int w,
                   int h, bool isXBitmaps=true, const uchar *lightColor = 0,
                   const uchar *midColor=0, const uchar *midlightColor=0,
                   const uchar *darkColor=0, const uchar *blackColor=0,
                   const uchar *whiteColor=0);

#endif
