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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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

// Collapses white-space according to CSS 2.1 rules
DOMStringImpl *DOMStringImpl::collapseWhiteSpace(bool preserveLF, bool preserveWS)
{
    if (preserveLF && preserveWS) return this;

    // Notice we are likely allocating more space than needed (worst case)
    QChar *n = QT_ALLOC_QCHAR_VEC(l);

    unsigned int pos = 0;
    bool collapsing = false;   // collapsing white-space
    bool collapsingLF = false; // collapsing around linefeed
    bool changedLF = false;
    for(unsigned int i=0; i<l; i++) {
        QChar ch = s[i];
        if (!preserveLF && (ch == '\n' || ch == '\r')) {
            // ### Not strictly correct according to CSS3 text-module.
            // - In ideographic languages linefeed should be ignored
            // - and in Thai and Khmer it should be treated as a zero-width space
            ch = ' '; // Treat as space
            changedLF = true;
        }

        if (collapsing) {
            if (ch == ' ')
                continue;
            // We act on \r as we would on \n because CSS uses it to indicate new-line
            if (ch == '\n' || ch == '\r') {
                collapsingLF = true;
                continue;
            }

            n[pos++] = (collapsingLF) ? '\n' : ' ';
            collapsing = false;
            collapsingLF = false;
        }
        else
        if (!preserveWS && ch == ' ') {
            collapsing = true;
            continue;
        }
        else
        if (!preserveWS && (ch == '\n' || ch == '\r')) {
            collapsing = true;
            collapsingLF = true;
            continue;
        }

        n[pos++] = ch;
    }
    if (collapsing)
        n[pos++] = ((collapsingLF) ? '\n' : ' ');

    if (pos == l && !changedLF) {
        QT_DELETE_QCHAR_VEC(n);
        return this;
    }
    else {
        DOMStringImpl* out = new DOMStringImpl();
        out->s = n;
        out->l = pos;

        return out;
    }
}

static Length parseLength(const QChar *s, unsigned int l)
{
    if (l == 0) {
        return Length(1, Relative);
    }

    unsigned i = 0;
    while (i < l && s[i].isSpace())
        ++i;
    if (i < l && (s[i] == '+' || s[i] == '-'))
        ++i;
    while (i < l && s[i].isDigit())
        ++i;

    bool ok;
    int r = QConstString(s, i).string().toInt(&ok);

    /* Skip over any remaining digits, we are not that accurate (5.5% => 5%) */
    while (i < l && (s[i].isDigit() || s[i] == '.'))
        ++i;

    /* IE Quirk: Skip any whitespace (20 % => 20%) */
    while (i < l && s[i].isSpace())
        ++i;

    if (ok) {
        if (i == l) {
            return Length(r, Fixed);
        } else {
            const QChar* next = s+i;

            if (*next == '%')
                return Length(r, Percent);

            if (*next == '*') 
                return Length(r, Relative);
        }
        return Length(r, Fixed);
    } else {
        if (i < l) {
            const QChar* next = s+i;
            
            if (*next == '*') 
                return Length(1, Relative);

            if (*next == '%')
                return Length(1, Relative);
        }
    }
    return Length(0, Relative);
}

khtml::Length* DOMStringImpl::toCoordsArray(int& len) const
{
    QString str(s, l);
    for(unsigned int i=0; i < l; i++) {
        QChar cc = s[i];
        if (cc > '9' || (cc < '0' && cc != '-' && cc != '*' && cc != '.'))
            str[i] = ' ';
    }
    str = str.simplified();

    len = str.count(' ') + 1;
    khtml::Length* r = new khtml::Length[len];

    int i = 0;
    int pos = 0;
    int pos2;

    while((pos2 = str.find(QLatin1Char(' '), pos)) != -1) {
        r[i++] = parseLength((QChar *) str.unicode()+pos, pos2-pos);
        pos = pos2+1;
    }
    r[i] = parseLength((QChar *) str.unicode()+pos, str.length()-pos);

    return r;
}

khtml::Length* DOMStringImpl::toLengthArray(int& len) const
{
    QString str(s, l);
    str = str.simplified();

    len = str.count(QLatin1Char(',')) + 1;
    khtml::Length* r = new khtml::Length[len];

    int i = 0;
    int pos = 0;
    int pos2;

    while((pos2 = str.find(QLatin1Char(','), pos)) != -1) {
        r[i++] = parseLength((QChar *) str.unicode()+pos, pos2-pos);
        pos = pos2+1;
    }

    /* IE Quirk: If the last comma is the last char skip it and reduce len by one */
    if (str.length()-pos > 0)
        r[i] = parseLength((QChar *) str.unicode()+pos, str.length()-pos);
    else
        len--;

    return r;
}

bool DOMStringImpl::isLower() const
{
    unsigned int i;
    for (i = 0; i < l; i++)
	if (s[i].toLower() != s[i])
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
	c->s[i] = s[i].toLower();

    return c;
}

DOMStringImpl *DOMStringImpl::upper() const
{
    DOMStringImpl *c = new DOMStringImpl;
    if(!l) return c;

    c->s = QT_ALLOC_QCHAR_VEC(l);
    c->l = l;

    for (unsigned int i = 0; i < l; i++)
	c->s[i] = s[i].toUpper();

    return c;
}

DOMStringImpl *DOMStringImpl::capitalize() const
{
    bool canCapitalize=true;
    DOMStringImpl *c = new DOMStringImpl;
    if(!l) return c;
    
    c->s = QT_ALLOC_QCHAR_VEC(l);
    c->l = l;

    for (unsigned int i=0; i<l; i++)
    {
        if (s[i].isLetterOrNumber() && canCapitalize)
        {
            c->s[i]=s[i].toUpper();
            canCapitalize=false;
        }
        else
        {
            c->s[i]=s[i];
            if (s[i].isSpace())
                canCapitalize=true;
        }
    }
    
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



