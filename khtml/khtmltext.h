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
#ifndef KHTMLTEXT_H
#define KHTMLTEXT_H

#include <qpainter.h>
#include <qfontmetrics.h>
#include "dom_stringimpl.h"

namespace khtml
{

class TextSlave
{
public:
    TextSlave(int _x, int _y, int _from, int _len, DOM::DOMStringImpl *_str,
	      int _ascent, int _descent)
	{
	    x = _x;
	    y = _y;
	    from = _from;
	    len = _len;
	    str = _str;
	    str->ref();
	    n = 0;
	    ascent = _ascent;
	    descent = _descent;
	}
    ~TextSlave() { str->deref(); }

    void print( QPainter *p, int _x, int _y, int _w, int _h, 
		int _tx, int _ty);
    bool checkPoint(int _x, int _y, int _tx, int _ty, const QFontMetrics &fm);


    void setNext(TextSlave *_n) { n = _n; }
    TextSlave *next() { return n; }

    int x;
    int y;
    int from;
    int len;
    DOM::DOMStringImpl *str;
    TextSlave *n;
    int ascent;
    int descent;
};

};
using namespace khtml;
#endif
