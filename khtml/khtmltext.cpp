/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */
#include "khtmltext.h"
#include "dom_stringimpl.h"

void TextSlave::print( QPainter *p, int _x, int _y, int _w, int _h, 
		       int _tx, int _ty)
{
    if((_ty + y - ascent > _y + _h) || (_ty + y + descent < _y)) return;

    QConstString s(str->s+from , len);
    p->drawText(x + _tx, y + _ty, s.string());
}

bool TextSlave::checkPoint(int _x, int _y, int _tx, int _ty, 
			   const QFontMetrics &fm)
{
    if((_ty + y - ascent > _y) || (_ty + y + descent < _y)) return false;
    QConstString s(str->s+from , len);
    int width = fm.width(s.string());
    if((_tx + x > _x) || (_tx + x + width < _x)) return false;
    return true;
}

