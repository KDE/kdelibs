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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __KDRAWUTIL_H
#define __KDRAWUTIL_H

#include <config.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qpalette.h>

/*
 * Various drawing routines. Also see Qt's qdrawutil.h for some more routines
 * contained in Qt.
 *
 * (C) Daniel M. Duley <mosfet@kde.org>
 */

/**
 * Draw a Next-style button (solid black shadow with light and midlight
 * highlight).
 */
void kDrawNextButton(QPainter *p, const QRect &r, const QColorGroup &g,
                     bool sunken=false, const QBrush *fill=0);

void kDrawNextButton(QPainter *p, int x, int y, int w, int h,
                     const QColorGroup &g, bool sunken=false, 
                     const QBrush *fill=0);

/**
 * Draw a Be-like style button.
 */
void kDrawBeButton(QPainter *p, QRect &r, const QColorGroup &g,
                   bool sunken=false, const QBrush *fill=0);

void kDrawBeButton(QPainter *p, int x, int y, int w, int h,
                   const QColorGroup &g, bool sunken=false, 
                   const QBrush *fill=0);

/**
 * Draw a rounded oval button. This does not fill the button, see
 * @ref kRoundMask() and @ref kRoundMaskRegion() for setting masks for fills.
 */
void kDrawRoundButton(QPainter *p, const QRect &r, const QColorGroup &g,
                      bool sunken=false);

void kDrawRoundButton(QPainter *p, int x, int y, int w, int h,
                      const QColorGroup &g, bool sunken=false);

/**
 * Set the region to the pixels contained in a round button of the given
 * size. You can use this to set clipping regions.
 */
void kRoundMaskRegion(QRegion &r, int x, int y, int w, int h);

/**
 * Paint the bitmap with the pixels contained in a round button of the given
 * size. This is mostly useful inside @ref QStyle routines.
 */
void kDrawRoundMask(QPainter *p, int x, int y, int w, int h, bool clear=false);

/**
 * Paint the supplied bitmaps onto the @ref QPainter using the colorgroup for
 * the foreground colors. Note: The bitmaps will be self-masked automatically
 * if not masked prior to calling this routine.
 *
 */
void kColorBitmaps(QPainter *p, const QColorGroup &g, int x, int y,
                   QBitmap *lightColor=0, QBitmap *midColor=0,
                   QBitmap *midlightColor=0, QBitmap *darkColor=0,
                   QBitmap *blackColor=0, QBitmap *whiteColor=0);

void kColorBitmaps(QPainter *p, const QColorGroup &g, int x, int y, int w,
                   int h, bool isXBitmaps=true, const uchar *lightColor = 0,
                   const uchar *midColor=0, const uchar *midlightColor=0,
                   const uchar *darkColor=0, const uchar *blackColor=0,
                   const uchar *whiteColor=0);

#endif
