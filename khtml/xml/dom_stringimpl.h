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

template<class type> class QList;
class QChar;

namespace DOM {

class DOMStringImpl : public DomShared
{
protected:
    DOMStringImpl() { s = 0, l = 0; }
public:
    DOMStringImpl(const QChar *str, unsigned int len);
    DOMStringImpl(const char *str);
    DOMStringImpl(const QChar &ch);
    virtual ~DOMStringImpl();

    void append(DOMStringImpl *str);
    void insert(DOMStringImpl *str, unsigned int pos);
    void truncate(int len);
    void remove(unsigned int pos, int len=1);
    DOMStringImpl *split(unsigned int pos);
    DOMStringImpl *copy() const {
        return new DOMStringImpl(s,l);
    };

    DOMStringImpl *substring(unsigned int pos, unsigned int len);

    const QChar &operator [] (int pos)
	{ return *(s+pos); }

    khtml::Length toLength() const;

    int toInt() const {
        return QConstString(s,l).string().toInt();
    };

    QList<khtml::Length> *toLengthList() const;
    bool isLower() const;
    DOMStringImpl *lower();

    unsigned int l;
    QChar *s;

    // returns the (new) lenght of 'value' having stripped away
    // anything after [0-9.%*]
    static unsigned int stripAttributeGarbage( QChar *s, unsigned int l );
};

};
#endif
