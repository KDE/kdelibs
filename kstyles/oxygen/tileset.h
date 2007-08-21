/* Oxygen widget style for KDE 4
   Copyright (C) 2006-2007 Thomas Luebking <thomas.luebking@web.de>

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

#ifndef TILESET_H
#define TILESET_H

#include <QBitmap>
#include <QRect>
#include <QHash>


class TileSet
{
public:
   TileSet(const QPixmap &pix, int xOff, int yOff, int width, int height, int rx = 0, int ry = 0);
   TileSet(){}

    /// Flags for what part to render. They can be |'ed together. To paint corners you specify two sides
   enum Position
   {
      Top = 0x1, Left=0x2, Bottom=0x4, Right=0x8,
      Ring=0xf, Center=0x10, Full=0x1f
   };

   typedef uint PosFlags;
   void render(const QRect &rect, QPainter *p, PosFlags pf = Ring) const;
   QRect rect(const QRect &rect, PosFlags pf) const;

protected:

    inline static bool matches(PosFlags This, PosFlags That){return (This & That) == This;}
    QPixmap pixmap[9];

private:
   enum Section
   {
      TopLeft = 0, TopRight, BtmLeft, BtmRight,
      TopMid, BtmMid, MidLeft, MidMid, MidRight
   };
   inline int width(Section sect) const {return pixmap[sect].width();}
   inline int height(Section sect) const {return pixmap[sect].height();}

   int rxf, ryf;
   bool _isBitmap;
};

#endif //TILESET_H
