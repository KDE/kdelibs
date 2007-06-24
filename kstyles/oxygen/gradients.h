/***************************************************************************
 *   Copyright (C) 2006-2007 by Thomas Lübking                             *
 *   thomas.luebking@web.de                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef GRADIENTS_H
#define GRADIENTS_H

#include <QBrush>
#include <QColor>
#include <QPixmap>

namespace Gradients {

enum Type {
   None = 0, Simple, Sunken, Gloss, Glass, Button, RadialGloss,
   TypeAmount
};

enum BgMode {
   BevelV = 3, BevelH, LightV, LightH
};


/** use only if sure you're not requesting Type::None */
const QPixmap& pix(const QColor &c,
                   int size,
                   Qt::Orientation o,
                   Type type = Simple);

/** wrapper to support Type::None */
inline const QBrush
brush(const QColor &c, int size, Qt::Orientation o, Type type  = Simple) {
   if (type == None)
      return QBrush(c);
   return QBrush(pix(c, size, o, type));
}

/** a diagonal NW -> SE light */
const QPixmap &shadow(int height, bool bottom = false);

/** a diagonal 16:9 SE -> NW light */
const QPixmap &ambient(int height);

/** a vertical N -> S light */
const QPixmap &light(int height);

const QPixmap &bg(const QColor &c, bool other = false);

void init(BgMode mode);
void wipe();

}

#endif //GRADIENTS_H
