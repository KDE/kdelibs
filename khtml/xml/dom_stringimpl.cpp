/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
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

#include "dom_stringimpl.h"

#include <qstring.h>
#include <qlist.h>
#include <kdebug.h>

#include <string.h>

// avoid crashes if this is not defined
#undef CORRECT_BEHAVIOUR
//#define CORRECT_BEHAVIOUR 1

using namespace DOM;
using namespace khtml;

#define QT_ALLOC_QCHAR_VEC( N ) (QChar*) new char[ sizeof(QChar)*( N ) ]
#define QT_DELETE_QCHAR_VEC( P ) delete[] ((char*)( P ))

template class QList<Length>;

DOMStringImpl::DOMStringImpl(const QChar *str, uint len)
{
    if(str && len)
    {
        s = QT_ALLOC_QCHAR_VEC( len );
        memcpy( s, str, len * sizeof(QChar) );
        l = len;
    }
#ifndef CORRECT_BEHAVIOUR
    else if(len == 0)
    {
        kdDebug( 6030 ) << "DOMStringImpl::DOMStringImpl(const QChar, len): incorrect behaviour!" << endl;
        s = new QChar;
        l = 0;
    }
#endif
    else
    {
        s = 0; l = 0;
    }
}

DOMStringImpl::DOMStringImpl(const char *str)
{
    if(str)
    {
        l = strlen(str);
        s = QT_ALLOC_QCHAR_VEC( l );
        int i = l;
        QChar* ptr = s;
        while( i-- )
            *ptr++ = *str++;
    }
#ifndef CORRECT_BEHAVIOUR
    else if(str && strlen(str) == 0)
    {
        kdDebug( 6030 ) << "DOMStringImpl::DOMStringImpl(const char*): incorrect behaviour!" << endl;
        s = new QChar;
        l = 0;
    }
#endif
    else
    {
        s = 0; l = 0;
    }
}

DOMStringImpl::DOMStringImpl(const QChar &ch)
{
    s = QT_ALLOC_QCHAR_VEC( 1 );
    memcpy(s, &ch, sizeof(QChar));
    l = 1;
}

DOMStringImpl::~DOMStringImpl()
{
    if(s) QT_DELETE_QCHAR_VEC(s);
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

    QChar *c = QT_ALLOC_QCHAR_VEC(len);
    memcpy(c, s, len*sizeof(QChar));
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

DOMStringImpl *DOMStringImpl::copy() const
{
    return new DOMStringImpl(s, l);
}

DOMStringImpl *DOMStringImpl::substring(uint pos, uint len)
{
  if( pos >=l ) return new DOMStringImpl();
  if(pos+len > l)
    len = l - pos;

  return new DOMStringImpl(s + pos, len);
}

static Length parseLength(QChar *s, unsigned int l)
{
    const QChar* last = s+l-1;

    if ( *last == QChar('%')) {
        // CSS allows one decimal after the point, like
        //  42.2%, but not 42.22%
        // we ignore the non-integer part for speed/space reasons
        int i = QConstString(s, l).string().findRev('.');
        if ( i >= 0 && i < (int)l-1 )
            l = i + 1;

        return Length(QConstString(s, l-1).string().toInt(), Percent);
    }

    if ( *last == QChar('*'))
    {
        if(l == 1)
            return Length(1, Relative);
        else
            return Length(QConstString(s, l-1).string().toInt(), Relative);
    }

    // should we strip of the non-integer part here also?
    // CSS says no, all important browsers do so, including Mozilla. sigh.
    bool ok;
    // this ugly construct helps in case someone specifies a length as "100."
    int v = (int) QConstString(s, l).string().toFloat(&ok);
    if(ok) {
        return Length(v, Fixed);
    }
    if(l == 4 && QConstString(s, l).string().contains("auto", false))
        return Length(0, Variable);

    return Length(0, Undefined);
}

Length DOMStringImpl::toLength() const
{
    return parseLength(s, l);
}

int DOMStringImpl::toInt() const
{
    return QConstString(s, l).string().toInt();
}

QList<Length> *DOMStringImpl::toLengthList() const
{
    QString str(s, l);
    int pos = 0;
    int pos2;

    // web authors are so stupid. This is a workaround
    // to fix lists like "1,2  3,4"
    QChar space(' ');
    for(int i=0; i < l; i++) if(str[i].latin1() == ',') str[i] = space;
    str = str.simplifyWhiteSpace();

    QList<Length> *list = new QList<Length>;
    list->setAutoDelete(true);
    while((pos2 = str.find(' ', pos)) != -1)
    {
        Length *l = new Length(parseLength((QChar *) str.unicode()+pos, pos2-pos));
        list->append(l);
        pos = pos2+1;
    }

    Length *l = new Length(parseLength((QChar *) str.unicode()+pos, str.length()-pos));
    list->append(l);

    return list;
}


