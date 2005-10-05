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
#include <qdrawutil.h>
#include <QPolygon>

KDEFX_EXPORT void kDrawNextButton(QPainter *p, int x, int y, int w, int h,
                     const QColorGroup &g, bool sunken,
                     const QBrush *fill)
{
    QPen oldPen = p->pen();
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->fillRect(x+1, y+1, w-2, h-2,
                fill ? *fill : g.brush(QColorGroup::Button));
    p->setPen(sunken ? Qt::black : g.light());
    p->drawLine(x, y, x2-1, y);
    p->drawLine(x, y, x, y2-1);
    p->setPen(sunken ? g.midlight() : g.mid());
    p->drawLine(x+1, y2-1, x2-1, y2-1);
    p->drawLine(x2-1, y+1, x2-1, y2-1);
    p->setPen(sunken ? g.light() : Qt::black);
    p->drawLine(x, y2, x2, y2);
    p->drawLine(x2, y, x2, y2);
    p->setPen(oldPen);
}


KDEFX_EXPORT void kDrawNextButton(QPainter *p, const QRect &r, const QColorGroup &g,
                     bool sunken, const QBrush *fill)
{
    kDrawNextButton(p, r.x(), r.y(), r.width(), r.height(), g, sunken, fill);
}

KDEFX_EXPORT void kDrawBeButton(QPainter *p, int x, int y, int w, int h,
                   const QColorGroup &g, bool sunken, const QBrush *fill)
{
    QPen oldPen = p->pen();
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->setPen(g.dark());
    p->drawLine(x+1, y, x2-1, y);
    p->drawLine(x, y+1, x, y2-1);
    p->drawLine(x+1, y2, x2-1, y2);
    p->drawLine(x2, y+1, x2, y2-1);


    if(!sunken){
        p->setPen(g.light());
        p->drawLine(x+2, y+2, x2-1, y+2);
        p->drawLine(x+2, y+3, x2-2, y+3);
        p->drawLine(x+2, y+4, x+2, y2-1);
        p->drawLine(x+3, y+4, x+3, y2-2);
    }
    else{
        p->setPen(g.mid());
        p->drawLine(x+2, y+2, x2-1, y+2);
        p->drawLine(x+2, y+3, x2-2, y+3);
        p->drawLine(x+2, y+4, x+2, y2-1);
        p->drawLine(x+3, y+4, x+3, y2-2);
    }


    p->setPen(sunken? g.light() : g.mid());
    p->drawLine(x2-1, y+2, x2-1, y2-1);
    p->drawLine(x+2, y2-1, x2-1, y2-1);

    p->setPen(g.mid());
    p->drawLine(x+1, y+1, x2-1, y+1);
    p->drawLine(x+1, y+2, x+1, y2-1);
    p->drawLine(x2-2, y+3, x2-2, y2-2);

    if(fill)
        p->fillRect(x+4, y+4, w-6, h-6, *fill);
    
    p->setPen(oldPen);
}

KDEFX_EXPORT void kDrawBeButton(QPainter *p, QRect &r, const QColorGroup &g, bool sunken,
                   const QBrush *fill)
{
    kDrawBeButton(p, r.x(), r.y(), r.width(), r.height(), g, sunken, fill);
}

KDEFX_EXPORT void kDrawRoundButton(QPainter *p, const QRect &r, const QColorGroup &g,
                      bool sunken)
{
    int x, y, x2, y2;
    r.coords(&x, &y, &x2, &y2);
    if(r.width() > 16 && r.height() > 16){
        QPen oldPen = p->pen();
        QPolygon hPntArray, lPntArray;
        hPntArray.putPoints(0, 12, x+4,y+1, x+5,y+1, // top left
                            x+3,y+2, x+2,y+3, x+1,y+4, x+1,y+5,
                            x+1,y2-5, x+1,y2-4, x+2,y2-3, // half corners
                            x2-5,y+1, x2-4,y+1, x2-3,y+2);

        lPntArray.putPoints(0, 17, x2-5,y2-1, x2-4,y2-1, // btm right
                            x2-3,y2-2, x2-2,y2-3, x2-1,y2-5, x2-1,y2-4,
 
                            x+3,y2-2, x+4,y2-1, x+5,y2-1, //half corners
                            x2-2,y+3, x2-1,y+4, x2-1,y+5,

                            x2-5,y2-2, x2-4,y2-2, // testing
                            x2-3,y2-3,
                            x2-2,y2-5, x2-2,y2-4);

        p->setPen(sunken ? g.dark() : g.light());
        p->drawLine(x+6, y, x2-6, y);
        p->drawLine(0, y+6, 0, y2-6);
        p->drawPoints(hPntArray);

        p->setPen(sunken ? g.light() : g.dark());
        p->drawLine(x+6, y2, x2-6, y2);
        p->drawLine(x+6, y2-1, x2-6, y2-1);
        p->drawLine(x2, y+6, x2, y2-6);
        p->drawLine(x2-1, y+6, x2-1, y2-6);
        p->drawPoints(lPntArray);
        p->setPen(oldPen);
    }
    else
        qDrawWinPanel(p, x, y, r.width(), r.height(), g, sunken);
}

KDEFX_EXPORT void kDrawRoundButton(QPainter *p, int x, int y, int w, int h,
                      const QColorGroup &g, bool sunken)
{
    QRect r(x, y, w, h);
    kDrawRoundButton(p, r, g, sunken);
}

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)

KDEFX_EXPORT void kDrawRoundMask(QPainter *p, int x, int y, int w, int h, bool clear)
{
    // round edge fills
    static const QCOORD btm_left_fill[]={ 0,0,1,0,2,0,3,0,4,0,0,1,1,1,2,1,3,1,4,1,
    1,2,2,2,3,2,4,2,2,3,3,3,4,3,3,4,4,4 };

    static const QCOORD btm_right_fill[]={ 0,0,1,0,2,0,3,0,4,0,0,1,1,1,2,1,3,1,4,
    1,0,2,1,2,2,2,3,2,0,3,1,3,2,3,0,4,1,4 };

    static const QCOORD top_left_fill[]={ 3,0,4,0,2,1,3,1,4,1,1,2,2,2,3,2,4,2,0,3,
    1,3,2,3,3,3,4,3,0,4,1,4,2,4,3,4,4,4 };

    static const QCOORD top_right_fill[]={ 0,0,1,0,0,1,1,1,2,1,0,2,1,2,2,2,3,2,0,
    3,1,3,2,3,3,3,4,3,0,4,1,4,2,4,3,4,4,4 };

    if(clear)
        p->fillRect(x, y, w, h, QBrush(Qt::color0, Qt::SolidPattern));
    
    QBrush fillBrush(Qt::color1, Qt::SolidPattern);
    p->setPen(Qt::color1);
    if(w > 16 && h > 16){
        int x2 = x+w-1;
        int y2 = y+h-1;
        QPolygon a(QCOORDARRLEN(top_left_fill), top_left_fill);
        a.translate(1, 1);
        p->drawPoints(a);
        a.setPoints(QCOORDARRLEN(btm_left_fill), btm_left_fill);
        a.translate(1, h-6);
        p->drawPoints(a);
        a.setPoints(QCOORDARRLEN(top_right_fill), top_right_fill);
        a.translate(w-6, 1);
        p->drawPoints(a);
        a.setPoints(QCOORDARRLEN(btm_right_fill), btm_right_fill);
        a.translate(w-6, h-6);
        p->drawPoints(a);

        p->fillRect(x+6, y, w-12, h, fillBrush);
        p->fillRect(x, y+6, x+6, h-12, fillBrush);
        p->fillRect(x2-6, y+6, x2, h-12, fillBrush);
        p->drawLine(x+6, y, x2-6, y);
        p->drawLine(x+6, y2, x2-6, y2);
        p->drawLine(x, y+6, x, y2-6);
        p->drawLine(x2, y+6, x2, y2-6);

    }
    else
        p->fillRect(x, y, w, h, fillBrush);
}

KDEFX_EXPORT void kRoundMaskRegion(QRegion &r, int x, int y, int w, int h)
{
    // using a bunch of QRect lines seems much more efficient than bitmaps or
    // point arrays, even tho it uses more statements
    r += QRect(x+6, y+0, w-12, h);
    r += QRect(x+5, y+1, 1, h-2); // left
    r += QRect(x+4, y+1, 1, h-2);
    r += QRect(x+3, y+2, 1, h-4);
    r += QRect(x+2, y+3, 1, h-6);
    r += QRect(x+1, y+4, 1, h-8);
    r += QRect(x, y+6, 1, h-12);
    int x2 = x+w-1;
    r += QRect(x2-5, y+1, 1, h-2); // right
    r += QRect(x2-4, y+1, 1, h-2);
    r += QRect(x2-3, y+2, 1, h-4);
    r += QRect(x2-2, y+3, 1, h-6);
    r += QRect(x2-1, y+4, 1, h-8);
    r += QRect(x2, y+6, 1, h-12);
}

KDEFX_EXPORT void kColorBitmaps(QPainter *p, const QColorGroup &g, int x, int y,
                   QBitmap *lightColor, QBitmap *midColor,
                   QBitmap *midlightColor, QBitmap *darkColor,
                   QBitmap *blackColor, QBitmap *whiteColor)
{
    QBitmap *bitmaps[]={lightColor, midColor, midlightColor, darkColor,
        blackColor, whiteColor};

    QColor colors[]={g.light(), g.mid(), g.midlight(), g.dark(),
        Qt::black, Qt::white};

    int i;
    for(i=0; i < 6; ++i){
        if(bitmaps[i]){
            if(!bitmaps[i]->mask())
                bitmaps[i]->setMask(*bitmaps[i]);
            p->setPen(colors[i]);
            p->drawPixmap(x, y, *bitmaps[i]);
        }
    }
}

KDEFX_EXPORT void kColorBitmaps(QPainter *p, const QColorGroup &g, int x, int y, int w,
                   int h, bool isXBitmaps, const uchar *lightColor,
                   const uchar *midColor, const uchar *midlightColor,
                   const uchar *darkColor, const uchar *blackColor,
                   const uchar *whiteColor)
{
    const uchar *data[]={lightColor, midColor, midlightColor, darkColor,
        blackColor, whiteColor};

    QColor colors[]={g.light(), g.mid(), g.midlight(), g.dark(),
        Qt::black, Qt::white};

    int i;
    QBitmap b;
    for(i=0; i < 6; ++i){
        if(data[i]){
            b = QBitmap(w, h, data[i], isXBitmaps);
            b.setMask(b);
            p->setPen(colors[i]);
            p->drawPixmap(x, y, b);
        }
    }
}



