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

#include "dom_exception.h"
#include "dom_string.h"

#include "dom_text.h"
#include "dom_textimpl.h"
using namespace DOM;


CharacterData::CharacterData() : Node()
{
}

CharacterData::CharacterData(const CharacterData &other) : Node(other)
{
}

CharacterData &CharacterData::operator = (const CharacterData &other)
{
    Node::operator =(other);
    return *this;
}

CharacterData::~CharacterData()
{
}

DOMString CharacterData::data() const
{
    if(!impl) return 0;
    return ((CharacterDataImpl *)impl)->data();
}

void CharacterData::setData( const DOMString &str )
{
    ((CharacterDataImpl *)impl)->setData(str);
}

unsigned long CharacterData::length() const
{
    return ((CharacterDataImpl *)impl)->length();
}

DOMString CharacterData::substringData( const unsigned long &offset, const unsigned long &count )
{
    return ((CharacterDataImpl *)impl)->substringData(offset, count);
}

void CharacterData::appendData( const DOMString &arg )
{
    ((CharacterDataImpl *)impl)->appendData(arg);
}

void CharacterData::insertData( const unsigned long &offset, const DOMString &arg )
{
    ((CharacterDataImpl *)impl)->insertData(offset, arg);
}

void CharacterData::deleteData( const unsigned long &offset, const unsigned long &count )
{
    ((CharacterDataImpl *)impl)->deleteData(offset, count);
}

void CharacterData::replaceData( const unsigned long &offset, const unsigned long &count, const DOMString &arg )
{
    ((CharacterDataImpl *)impl)->replaceData(offset, count, arg);
}

CharacterData::CharacterData(CharacterDataImpl *i) : Node(i)
{
}

// ---------------------------------------------------------------------------

Comment::Comment() : CharacterData()
{
}

Comment::Comment(const Comment &other) : CharacterData(other)
{
}

Comment &Comment::operator = (const Comment &other)
{
    CharacterData::operator =(other);
    return *this;
}

Comment::~Comment()
{
}

// ----------------------------------------------------------------------------

Text::Text()
{
}

Text::Text(const Text &other) : CharacterData(other)
{
}

Text &Text::operator = (const Text &other)
{
    CharacterData::operator =(other);
    return *this;
}

Text::~Text()
{
}

Text Text::splitText( const unsigned long &offset )
{
    return ((TextImpl *)impl)->splitText(offset);
}

Text::Text(TextImpl *i) : CharacterData(i)
{
}
