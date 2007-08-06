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
#include "kdrawutil.h"
#include <QtGui/qdrawutil.h>
#include <QtGui/QPolygon>

KDEFX_EXPORT void kColorBitmaps(QPainter *p, const QPalette &pal, int x, int y,
                   QBitmap *lightColor, QBitmap *midColor,
                   QBitmap *midlightColor, QBitmap *darkColor,
                   QBitmap *blackColor, QBitmap *whiteColor)
{
    QBitmap *bitmaps[]={lightColor, midColor, midlightColor, darkColor,
        blackColor, whiteColor};

    QColor colors[]={pal.color(QPalette::Light), pal.color(QPalette::Mid),
        pal.color(QPalette::Midlight), pal.color(QPalette::Dark),
        Qt::black, Qt::white};

    int i;
    for(i=0; i < 6; ++i){
        if(bitmaps[i]){
            if (bitmaps[i]->mask().isNull())
                bitmaps[i]->setMask(*bitmaps[i]);
            p->setPen(colors[i]);
            p->drawPixmap(x, y, *bitmaps[i]);
        }
    }
}

KDEFX_EXPORT void kColorBitmaps(QPainter *p, const QPalette &pal, int x, int y, int w,
                   int h, bool isXBitmaps, const uchar *lightColor,
                   const uchar *midColor, const uchar *midlightColor,
                   const uchar *darkColor, const uchar *blackColor,
                   const uchar *whiteColor)
{
    const uchar *data[]={lightColor, midColor, midlightColor, darkColor,
        blackColor, whiteColor};

    QColor colors[]={pal.color(QPalette::Light), pal.color(QPalette::Mid),
        pal.color(QPalette::Midlight), pal.color(QPalette::Dark),
        Qt::black, Qt::white};

    int i;
    QBitmap b;
    for(i=0; i < 6; ++i){
        if(data[i]){
            QSize s(w,h);
            b = QBitmap::fromData(s, data[i], isXBitmaps ? QImage::Format_MonoLSB : QImage::Format_Mono );
            b.setMask(b);
            p->setPen(colors[i]);
            p->drawPixmap(x, y, b);
        }
    }
}



