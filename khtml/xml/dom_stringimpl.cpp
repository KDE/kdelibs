/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001-2003 Dirk Mueller ( mueller@kde.org )
 *           (C) 2002 Apple Computer, Inc.
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
 */

#include "dom_stringimpl.h"

#include <kdebug.h>

#include <string.h>
#include <qstringlist.h>

using namespace DOM;
using namespace khtml;


DOMStringImpl::DOMStringImpl(const char *str)
{
    if(str && *str)
    {
        l = strlen(str);
        s = QT_ALLOC_QCHAR_VEC( l );
        int i = l;
        QChar* ptr = s;
        while( i-- )
            *ptr++ = *str++;
    }
    else
    {
        s = QT_ALLOC_QCHAR_VEC( 1 );  // crash protection
        s[0] = 0x0; // == QChar::null;
        l = 0;
    }
}

// FIXME: should be a cached flag maybe.
bool DOMStringImpl::containsOnlyWhitespace() const
{
    if (!s)
        return true;

    for (uint i = 0; i < l; i++) {
        QChar c = s[i];
        if (c.unicode() <= 0x7F) {
            if (c.unicode() > ' ')
                return false;
        } else {
            if (c.direction() != QChar::DirWS)
                return false;
        }
    }
    return true;
}


void DOMStringImpl::append(DOMStringImpl *str)
{
    if(str && str->l != 0)
    {
        int newlen = l+str->l;
        QChar *c = QT_ALLOC_QCHAR_VEC(newlen);
        memcpy(c, s, l*sizeof(QChar));
        memcpy(c+l, str->s, str->l*sizeof(QChar));
        if(s) QT_DELETE_QCHAR_VEC(s);
        s = c;
        l = newlen;
    }
}

void DOMStringImpl::insert(DOMStringImpl *str, uint pos)
{
    if(pos > l)
    {
        append(str);
        return;
    }
    if(str && str->l != 0)
    {
        int newlen = l+str->l;
        QChar *c = QT_ALLOC_QCHAR_VEC(newlen);
        memcpy(c, s, pos*sizeof(QChar));
        memcpy(c+pos, str->s, str->l*sizeof(QChar));
        memcpy(c+pos+str->l, s+pos, (l-pos)*sizeof(QChar));
        if(s) QT_DELETE_QCHAR_VEC(s);
        s = c;
        l = newlen;
    }
}

void DOMStringImpl::truncate(int len)
{
    if(len > (int)l) return;

    int nl = len < 1 ? 1 : len;
    QChar *c = QT_ALLOC_QCHAR_VEC(nl);
    memcpy(c, s, nl*sizeof(QChar));
    if(s) QT_DELETE_QCHAR_VEC(s);
    s = c;
    l = len;
}

void DOMStringImpl::remove(uint pos, int len)
{
  if(pos >= l ) return;
  if(pos+len > l)
    len = l - pos;

  uint newLen = l-len;
  QChar *c = QT_ALLOC_QCHAR_VEC(newLen);
  memcpy(c, s, pos*sizeof(QChar));
  memcpy(c+pos, s+pos+len, (l-len-pos)*sizeof(QChar));
  if(s) QT_DELETE_QCHAR_VEC(s);
  s = c;
  l = newLen;
}

DOMStringImpl *DOMStringImpl::split(uint pos)
{
  if( pos >=l ) return new DOMStringImpl();

  uint newLen = l-pos;
  QChar *c = QT_ALLOC_QCHAR_VEC(newLen);
  memcpy(c, s+pos, newLen*sizeof(QChar));

  DOMStringImpl *str = new DOMStringImpl(s + pos, newLen);
  truncate(pos);
  return str;
}

DOMStringImpl *DOMStringImpl::substring(uint pos, uint len)
{
  if( pos >=l ) return new DOMStringImpl();
  if(pos+len > l)
    len = l - pos;

  return new DOMStringImpl(s + pos, len);
}

static Length parseLength(const QChar *s, unsigned int l)
{
    const QChar* last = s+l-1;
    if (l && *last == QChar('%')) {
        // CSS allows one decimal after the point, like
        //  42.2%, but not 42.22%
        // we ignore the non-integer part for speed/space reasons
        int i = QConstString(s, l).string().findRev('.');
        if ( i >= 0 && i < (int)l-1 )
            l = i + 1;

        bool ok;
        i = QConstString(s, l-1).string().toInt(&ok);

        if (ok)
            return Length(i, Percent);

        // in case of weird constructs like 5*%
        last--;
        l--;
    }

    if (l == 0)
        return Length(0, Variable);

    if ( *last == '*') {
        if(last == s)
            return Length(1, Relative);
        else
            return Length(QConstString(s, l-1).string().toInt(), Relative);
    }

    // should we strip of the non-integer part here also?
    // CSS says no, all important browsers do so, including Mozilla. sigh.
    bool ok;
    // this ugly construct helps in case someone specifies a length as "100."
    int v = (int) QConstString(s, l).string().toFloat(&ok);

    if(ok)
        return Length(v, Fixed);

    return Length(0, Variable);
}

khtml::Length* DOMStringImpl::toLengthArray(int& len) const
{
    QString str(s, l);
    int pos = 0;
    int pos2;

    // web authors are so stupid. This is a workaround
    // to fix lists like "1,2px 3 ,4"
    // make sure not to break percentage or relative widths
    // ### what about "auto" ?
    QChar space(' ');
    for(unsigned int i=0; i < l; i++) {
        char cc = str[i].latin1();
        if ( cc > '9' || ( cc < '0' && cc != '-' && cc != '*' && cc != '%' && cc != '.') )
            str[i] = space;
    }
    str = str.simplifyWhiteSpace();

    len = str.contains(' ') + 1;
    khtml::Length* r = new khtml::Length[len];
    int i = 0;
    while((pos2 = str.find(' ', pos)) != -1)
    {
        r[i++] = parseLength((QChar *) str.unicode()+pos, pos2-pos);
        pos = pos2+1;
    }
    r[i] = parseLength((QChar *) str.unicode()+pos, str.length()-pos);

    return r;
}

bool DOMStringImpl::isLower() const
{
    unsigned int i;
    for (i = 0; i < l; i++)
	if (s[i].lower() != s[i])
	    return false;
    return true;
}

DOMStringImpl *DOMStringImpl::lower() const
{
    DOMStringImpl *c = new DOMStringImpl;
    if(!l) return c;

    c->s = QT_ALLOC_QCHAR_VEC(l);
    c->l = l;

    for (unsigned int i = 0; i < l; i++)
	c->s[i] = s[i].lower();

    return c;
}

DOMStringImpl *DOMStringImpl::upper() const
{
    DOMStringImpl *c = new DOMStringImpl;
    if(!l) return c;

    c->s = QT_ALLOC_QCHAR_VEC(l);
    c->l = l;

    for (unsigned int i = 0; i < l; i++)
	c->s[i] = s[i].upper();

    return c;
}

DOMStringImpl *DOMStringImpl::capitalize() const
{
    DOMStringImpl *c = new DOMStringImpl;
    if(!l) return c;

    c->s = QT_ALLOC_QCHAR_VEC(l);
    c->l = l;

    if ( l ) c->s[0] = s[0].upper();
    for (unsigned int i = 1; i < l; i++)
	c->s[i] = s[i-1].isLetterOrNumber() ? s[i] : s[i].upper();

    return c;
}

int DOMStringImpl::toInt(bool* ok) const
{
    // match \s*[+-]?\d*
    unsigned i = 0;
    while (i < l && s[i].isSpace())
        ++i;
    if (i < l && (s[i] == '+' || s[i] == '-'))
        ++i;
    while (i < l && s[i].isDigit())
        ++i;

    return QConstString(s, i).string().toInt(ok);
}



