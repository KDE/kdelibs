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

#include <qnamespace.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qpalette.h>

#include <kdelibs_export.h>

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
 * Draws a Next-style button (solid black shadow with light and midlight highlight).
 * 
 * @param p       The painter to use for drawing the button.
 * @param r       Specifies the rect in which to draw the button.
 * @param g       Specifies the shading colors.
 * @param sunken  Whether to draw the button as sunken (pressed) or not.
 * @param fill    The brush to use for filling the interior of the button.
 *                Pass @a null to prevent the button from being filled.
 */
KDEFX_EXPORT void kDrawNextButton(QPainter *p, const QRect &r, const QColorGroup &g,
                     bool sunken=false, const QBrush *fill=0);

/**
 * @relates KStyle
 * @overload
 */
KDEFX_EXPORT void kDrawNextButton(QPainter *p, int x, int y, int w, int h,
                     const QColorGroup &g, bool sunken=false, 
                     const QBrush *fill=0);

/**
 * @relates KStyle
 * @c \#include @c <kdrawutil.h>
 *
 * Draws a Be-style button.
 *
 * @param p       The painter to use for drawing the button.
 * @param r       Specifies the rect in which to draw the button.
 * @param g       Specifies the shading colors.
 * @param sunken  Whether to draw the button as sunken (pressed) or not.
 * @param fill    The brush to use for filling the interior of the button.
 *                Pass @a null to prevent the button from being filled.
 */
KDEFX_EXPORT void kDrawBeButton(QPainter *p, QRect &r, const QColorGroup &g,
                   bool sunken=false, const QBrush *fill=0);

/**
 * @relates KStyle
 * @c \#include @c <kdrawutil.h>
 * @overload
 */
KDEFX_EXPORT void kDrawBeButton(QPainter *p, int x, int y, int w, int h,
                   const QColorGroup &g, bool sunken=false, 
                   const QBrush *fill=0);

/**
 * @relates KStyle
 * @c \#include @c <kdrawutil.h>
 *
 * Draws a rounded oval button. This function doesn't fill the button.
 * See kRoundMaskRegion() for setting masks for fills.
 *
 * @param p       The painter to use for drawing the button.
 * @param r       Specifies the rect in which to draw the button.
 * @param g       Specifies the shading colors.
 * @param sunken  Whether to draw the button as sunken (pressed) or not.
 */
KDEFX_EXPORT void kDrawRoundButton(QPainter *p, const QRect &r, const QColorGroup &g,
                      bool sunken=false);

/**
 * @relates KStyle
 * @overload
 */
KDEFX_EXPORT void kDrawRoundButton(QPainter *p, int x, int y, int w, int h,
                      const QColorGroup &g, bool sunken=false);

/**
 * @relates KStyle
 * @c \#include @c <kdrawutil.h>
 *
 * Sets a region to the pixels covered by a round button of the given
 * size. You can use this to set clipping regions.
 * 
 * @param r  Reference to the region to set.
 * @param x  The X coordinate of the button.
 * @param y  The Y coordinate of the button.
 * @param w  The width of the button.
 * @param h  The height of the button.
 *
 * @see kDrawRoundButton() and kDrawRoundMask()
 */
KDEFX_EXPORT void kRoundMaskRegion(QRegion &r, int x, int y, int w, int h);

/**
 * @relates KStyle
 * @c \#include @c <kdrawutil.h>
 *
 * Paints the pixels covered by a round button of the given size with
 * Qt::color1. This function is useful in QStyle::drawControlMask().
 *
 * @param p      The painter to use for drawing the button.
 * @param x      The X coordinate of the button.
 * @param y      The Y coordinate of the button.
 * @param w      The width of the button.
 * @param h      The height of the button.
 * @param clear  Whether to clear the rectangle specified by @p (x, y, w, h) to
 *               Qt::color0 before drawing the mask.
 */
KDEFX_EXPORT void kDrawRoundMask(QPainter *p, int x, int y, int w, int h, bool clear=false);

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
 * @param g             Specifies the shading colors.
 * @param x             The X coordinate at which to draw the bitmaps.
 * @param y             The Y coordinate at which to draw the bitmaps.
 * @param lightColor    The bitmap to use for the light part.
 * @param midColor      The bitmap to use for the mid part.
 * @param midlightColor The bitmap to use for the midlight part.
 * @param darkColor     The bitmap to use for the dark part.
 * @param blackColor    The bitmap to use for the black part.
 * @param whiteColor    The bitmap to use for the white part.
 *
 * @see QColorGroup
 */
KDEFX_EXPORT void kColorBitmaps(QPainter *p, const QColorGroup &g, int x, int y,
                   QBitmap *lightColor=0, QBitmap *midColor=0,
                   QBitmap *midlightColor=0, QBitmap *darkColor=0,
                   QBitmap *blackColor=0, QBitmap *whiteColor=0);

/**
 * @relates KStyle
 * @c \#include @c <kdrawutil.h>
 * @overload
 */
 KDEFX_EXPORT void kColorBitmaps(QPainter *p, const QColorGroup &g, int x, int y, int w,
                   int h, bool isXBitmaps=true, const uchar *lightColor = 0,
                   const uchar *midColor=0, const uchar *midlightColor=0,
                   const uchar *darkColor=0, const uchar *blackColor=0,
                   const uchar *whiteColor=0);

#endif
