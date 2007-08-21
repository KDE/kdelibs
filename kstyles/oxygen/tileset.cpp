/* Oxygen widget style for KDE 4
   Copyright (C) 2006-2007 Thomas Luebking <thomas.luebking@web.de>
   Copyright (C) 2007 Casper Boemann <cbr@boemann.dk>

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

#include <QPainter>
#include <cmath>

#include "tileset.h"

TileSet::TileSet(const QPixmap &pix, int xOff, int yOff, int width, int height, int rx, int ry)
{
    if (pix.isNull())
    {
        _isBitmap = false;
        return;
    }
    _isBitmap = pix.isQBitmap();
    rxf = pix.width()*rx;
    ryf = pix.height()*ry;
    
    int rOff = pix.width() - xOff - width;
    int bOff = pix.height() - yOff - height;
    int amount = 32/width+1;
    int amount2 = 32/height+1;
    int i;
    
    QPainter p;
   
#define initPixmap(_SECTION_,_WIDTH_,_HEIGHT_)\
   pixmap[_SECTION_] = QPixmap(_WIDTH_, _HEIGHT_);\
   pixmap[_SECTION_].fill(Qt::transparent); p.begin(&pixmap[_SECTION_])

    initPixmap(TopLeft, xOff, yOff);
    p.drawPixmap(0, 0, pix, 0, 0, xOff, yOff);
    p.end();

    initPixmap(TopMid, amount*width, yOff);
    for (i = 0; i < amount; i++)
         p.drawPixmap(i*width, 0, pix, xOff, 0, width, yOff);
    p.end();

    initPixmap(TopRight, rOff, yOff);
    p.drawPixmap(0, 0, pix, xOff+width, 0, rOff, yOff);
    p.end();

    //----------------------------------
    initPixmap(MidLeft, xOff, amount2*height);
    for (i = 0; i < amount2; i++)
         p.drawPixmap(0, i*height, pix, 0, yOff, xOff, height);
    p.end();

    initPixmap(MidMid, amount*width, amount2*height);
    for (i = 0; i < amount; i++)
         for (int j = 0; j < amount2; j++)
              p.drawPixmap(i*width, j*height, pix, xOff, yOff, width, height);
    p.end();

    initPixmap(MidRight, rOff, amount2*height);
    for (i = 0; i < amount2; i++)
         p.drawPixmap(0, i*height, pix, xOff+width, yOff, rOff, height);
    p.end();

    //----------------------------------
    initPixmap(BtmLeft, xOff, bOff);
    p.drawPixmap(0, 0, pix, 0, yOff+height, xOff, bOff);
    p.end();

    initPixmap(BtmMid, amount*width, bOff);
    for (i = 0; i < amount; i++)
         p.drawPixmap(i*width, 0, pix, xOff, yOff+height, width, bOff);
    p.end();

    initPixmap(BtmRight, rOff, bOff);
    p.drawPixmap(0, 0, pix, xOff+width, yOff+height, rOff, bOff);
    p.end();
}

void TileSet::render(const QRect &r, QPainter *p, PosFlags pf) const
{
    int xOff, yOff, midw, midh;

    r.getRect(&xOff, &yOff, &midw, &midh);
    int tlh = height(TopLeft), blh = height(BtmLeft),
        trh = height(TopRight), brh = height(BtmLeft),
        tlw = width(TopLeft), blw = width(BtmLeft),
        trw = width(TopRight), brw = width(BtmRight);

    // Figure out how much space we have for the middle part and fix corner sizes at the same time
    if(pf & Top)
        midh -= tlh;
    else
        tlh = trh = 0;
    if(pf & Bottom)
        midh -= brh;
    else
        blh = brh = 0;
    if(pf & Left)
        midw -= tlw;
    else
        tlw = blw = 0;
    if(pf & Right)
        midw -= brw;
    else
        trw = brw = 0;

    p->setClipRect(r);
    // Paint the corners
    if((pf & Top) && (pf & Left))
        p->drawPixmap(r.x(), r.y(), pixmap[TopLeft], 0, 0, tlw, tlh);
    if((pf & Top) && (pf & Right))
        p->drawPixmap(r.right()-trw+1, r.y(), pixmap[TopRight], 0, 0, trw, trh);
    if((pf & Bottom) && (pf & Left))
        p->drawPixmap(r.x(), r.bottom()-blh+1, pixmap[BtmLeft], 0, 0, blw, blh);
    if((pf & Bottom) && (pf & Right))
        p->drawPixmap(r.right()-trw+1, r.bottom()-brh+1, pixmap[BtmRight], 0, 0, brw, brh);

    //Paint the sides
    if(pf & Top && midw >0)
        p->drawTiledPixmap(r.x()+tlw, r.y(), midw, tlh, pixmap[TopMid], 0, 0);
    if(pf & Bottom && midw >0)
        p->drawTiledPixmap(r.x()+tlw, r.bottom()-blh+1, midw, blh, pixmap[BtmMid], 0, 0);
    if(pf & Left && midh >0)
        p->drawTiledPixmap(r.x(), r.y()+tlh, tlw, midh, pixmap[MidLeft], 0, 0);
    if(pf & Right && midh >0)
        p->drawTiledPixmap(r.right()-trw+1, r.y()+trh, trw, midh, pixmap[MidRight], 0, 0);

    //Paint the center
    if(pf & Center && midw>0 && midh>0)
        p->drawTiledPixmap(r.x()+trw, r.y()+trh, midw, midh, pixmap[MidMid], 0, 0);

    p->setClipping(false);
}
