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

#include "dom_stringimpl.h"
using namespace DOM;
#include <qregexp.h>

#include <stdio.h>

DOMStringImpl::DOMStringImpl(QChar *str, uint len)
{
    s = str, l = len;
}

void DOMStringImpl::append(DOMStringImpl *str)
{
    if(str && str->l != 0)
    {
	int newlen = l+str->l;
	QChar *c = new QChar[newlen];
	memcpy(c, s, l*sizeof(QChar));
	memcpy(c+l, str->s, str->l*sizeof(QChar));
	if(s) delete [] s;
	s = c;
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
	QChar *c = new QChar[newlen];
	memcpy(c, s, pos*sizeof(QChar));
	memcpy(c+pos, str->s, str->l*sizeof(QChar));
	memcpy(c+pos+str->l, s+pos, (l-pos)*sizeof(QChar));
	if(s) delete [] s;
	s = c;
    }
}

void DOMStringImpl::truncate(int len)
{
    if(len > (int)l) return;

    QChar *c = new QChar[len];
    memcpy(c, s, len*sizeof(QChar));
    if(s) delete [] s;
    s = c;
}

DOMStringImpl *DOMStringImpl::copy() const
{
    QChar *c = new QChar[l];
    memcpy(c, s, l*sizeof(QChar));
    return new DOMStringImpl(c, l);
}

static Length parseLength(QChar *s, unsigned int l)
{
    if ( *(s+l-1) == QChar('%'))
	return Length(QConstString(s, l-1).string().toInt(), Percent);

    if ( *(s+l-1) == QChar('*'))
    {
	if(l == 1)
	    return Length(1, Relative);
	else
	    return Length(QConstString(s, l-1).string().toInt(), Relative);
    }
    
    int v = QConstString(s, l).string().toInt();
    if(v)
	return Length(v, Fixed);
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
    str.replace(QRegExp(" "),"");

    QList<Length> *list = new QList<Length>;
    list->setAutoDelete(true);
    while((pos2 = str.find(',', pos)) != -1)
    {
	Length *l = new Length(parseLength(s+pos, pos2-pos));
	printf("got length %d, type=%d\n", l->value, l->type);
	list->append(l);
	pos = pos2+1;
    }

    Length *l = new Length(parseLength(s+pos, str.length()-pos));
    printf("got length %d, type=%d\n", l->value, l->type);
    list->append(l);

    return list;
}


