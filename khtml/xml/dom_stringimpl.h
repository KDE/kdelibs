/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
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
#ifndef _DOM_DOMStringImpl_h_
#define _DOM_DOMStringImpl_h_

#include <qstring.h>
#include <dom_misc.h>
#include <khtmllayout.h>
#include <qlist.h>

namespace DOM {

class DOMStringImpl : public DomShared
{
protected:
    DOMStringImpl() { s = 0, l = 0; }
public:
    DOMStringImpl(QChar *str, uint len);
    virtual ~DOMStringImpl() { if(s) delete [] s; }

    void append(DOMStringImpl *str);
    void insert(DOMStringImpl *str, uint pos);
    void truncate(int len);
    void remove(uint pos, int len=1);
    DOMStringImpl *split(uint pos);
    DOMStringImpl *copy() const;
    DOMStringImpl *substring(uint pos, uint len);

    const QChar &operator [] (int pos)
	{ return *(s+pos); }

    khtml::Length toLength() const;
    int toInt() const;
    QList<khtml::Length> *toLengthList() const;

    uint l;
    QChar *s;
};

};
#endif
