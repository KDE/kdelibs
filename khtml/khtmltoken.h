/*
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)
              (C) 1998 Waldo Bastian (bastian@kde.org)
              (C) 1999 Lars Knoll (knoll@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
//----------------------------------------------------------------------------
//
// KDE HTML Widget -- Tokenizers
// $Id$

#ifndef HTMLTOKEN_H
#define HTMLTOKEN_H

#include <qstring.h>

#include "khtmlstring.h"
#include "dom_stringimpl.h"

class Attribute
{
public:
    Attribute() { id = 0, v = 0; }
    Attribute(const DOMString &name, const DOMString &val);
    Attribute(const DOMStringImpl *name, DOMStringImpl *val);
    Attribute(int _id, const DOMString &val)
	{
	    _id = id;
	    v = val.implementation();
	    v->ref();
	}

    Attribute(int _id, DOMStringImpl *val)
	{ id = _id, v = val; v->ref(); }
    Attribute( const Attribute &other) 
	{ 
	    id = other.id; 
	    v = other.v; 
	    v->ref();
	}

    ~Attribute()
	{
	    if(v) v->deref();
	}

    Attribute &operator = (const Attribute &other) { 
	id = other.id;
	if(v == other.v) return *this;
	if(v) v->deref();
	v = other.v;
	if(v) v->ref();
	return *this;
    }

    void setValue(QChar *_s, int _l) { 
	if(v) v->deref();
	QChar *c = new QChar[_l];
	memcpy(c, _s, _l*sizeof(QChar));
 	v = new DOMStringImpl(c, _l);
	v->ref();
    }
    void setValue(const DOMString &s) {
	if(v) v->deref();
	v = s.implementation();
	if(v) v->ref();
    }
    void setValue(DOMStringImpl *s) {
	if(v == s) return;
	if(v) v->deref();
	v = s;
	if(v) v->ref();
    }
    DOMString value() const { return v; }
    DOMStringImpl *val() const { return v; }

    const DOMString &name() const;
    void setName(const DOMString &name);
    void setName(const DOMStringImpl *name);

    ushort id;

protected:
    DOMStringImpl *v;
};

class AttributeList
{
public:
    AttributeList();
    AttributeList( Attribute *list, unsigned int length);
    AttributeList( const AttributeList &other);
    virtual ~AttributeList();

    AttributeList &operator = (const AttributeList &other );

    ushort id(uint index);
    DOMString name(uint index);
    DOMString value(uint index);
    DOMString valueForId(uint id)
	{ 
	    int i = find(id);
	    if(i == 1) return 0;
	    return value(i);
	}
    // adds the attribute. If it exists, replaces it.
    void add(const Attribute &attr);

    unsigned int length();

    // returns -1 if not found
    int find(const DOMString &name);
    int find(ushort id);

    DOMString operator [] (const DOMString &name); 
    Attribute *operator [] (uint index); 

    void remove(DOMString name);
    void remove(uint index);

protected:
    Attribute *_list;
    uint _len;
};
   

class Token
{
public:
    Token() { id = 0; }
    // contructor for a text token
    Token(QChar *text, uint len);
    // tag without attributes
    Token(ushort id);
    // tag with attributes
    Token(ushort id, Attribute *lst, uint len);

    ushort id;
    AttributeList attrs;
    DOMString text;
};


// makes code in the parser nicer
inline bool operator==( const Attribute &a, const int &i )
{ return a.id == i; }

inline bool operator==( const Attribute &a, const QString &s )
{ return a.value() == s; }

#endif // HTMLTOKEN

