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
#include "khtmlstyle.h"

#include "dom_exception.h"
#include "dom_string.h"
#include "dom_stringimpl.h"

#include "dom_textimpl.h"
#include "dom_node.h"

#include <stdio.h>

using namespace DOM;


CharacterDataImpl::CharacterDataImpl(DocumentImpl *doc) : NodeWParentImpl(doc) 
{
    str = 0;
}

CharacterDataImpl::CharacterDataImpl(DocumentImpl *doc, const DOMString &_text) 
    : NodeWParentImpl(doc) 
{
    str = _text.impl;
    str->ref();
}

CharacterDataImpl::~CharacterDataImpl()
{
    if(str) str->deref();
}

DOMString CharacterDataImpl::data() const
{
    return str;
}

void CharacterDataImpl::setData( const DOMString &newStr )
{
    if(str == newStr.impl) return;
    if(str) str->deref();
    str = newStr.impl;
    if(str) str->ref();
}

unsigned long CharacterDataImpl::length() const
{
    return str->l;
}

DOMString CharacterDataImpl::substringData( const unsigned long /*offset*/, const unsigned long /*count*/ )
{
    // ###
    return 0;
}

void CharacterDataImpl::appendData( const DOMString &arg )
{
    str->append(arg.impl);
}

void CharacterDataImpl::insertData( const unsigned long offset, const DOMString &arg )
{
    str->insert(arg.impl, offset);
}

void CharacterDataImpl::deleteData( const unsigned long /*offset*/, const unsigned long /*count */)
{
    // ###
}

void CharacterDataImpl::replaceData( const unsigned long /*offset*/, const unsigned long /*count*/, const DOMString &/*arg*/ )
{
    // ###
}

// ---------------------------------------------------------------------------

TextImpl::TextImpl(DocumentImpl *doc, const DOMString &_text) 
    : CharacterDataImpl(doc, _text)
{
    first = 0;
    minWidth = -1;
    maxWidth = -1;
    font = 0;
}

TextImpl::TextImpl(DocumentImpl *doc) 
    : CharacterDataImpl(doc)
{
}

TextImpl::~TextImpl()
{
    // delete all slaves
    TextSlave *s = first;
    while(s)
    {
	TextSlave *next = s->next();
	delete s;
	s = next;
    }
}

TextImpl *TextImpl::splitText( const unsigned long /*offset*/ )
{
    // ####
    return 0;
}

const DOMString TextImpl::nodeName() const
{
  return "#text";
}

unsigned short TextImpl::nodeType() const
{
    return Node::TEXT_NODE;
}


void TextImpl::printObject( QPainter *p, int x, int y, int w, int h, 
		      int tx, int ty)
{
#ifdef DEBUG_LAYOUT
    printf("Text::printObject(long)\n");
#endif

    TextSlave *s = first;
    p->setFont(*font);
    p->setPen( font->textColor() );

    while(s)
    {
	s->print(p, x, y, w, h, tx, ty);
	s=s->next();
    }
}

void TextImpl::print( QPainter *p, int x, int y, int w, int h, 
		      int tx, int ty)
{
    printObject(p, x, y, w, h, tx, ty);
}

void TextImpl::calcMinMaxWidth()
{
#ifdef DEBUG_LAYOUT
    printf("Text::calcMinMaxWidth(): known=%d\n", minMaxKnown());
#endif

    if(minMaxKnown()) return;

    // ### this is not really the right place for this call...
    if(!font) font = parentNode()->getFont();
    QFontMetrics fm(*font);

    // ### calc Min and Max width...
    minWidth = 0;
    maxWidth = 0;

    int currMinWidth = 0;
    int currMaxWidth = 0;

    int len = str->l;
    for(int i = 0; i < len; i++)
    {
	const QChar c = *(str->s+i);
	if( c == QChar(' '))
	{
	    if(currMinWidth > minWidth) minWidth = currMinWidth;
	    currMinWidth = 0;
	    currMaxWidth += fm.width(c);
	}
	else if( c == QChar('\n'))
	{
	    if(currMinWidth > minWidth) minWidth = currMinWidth;
	    currMinWidth = 0;
	    if(currMaxWidth > maxWidth) maxWidth = currMaxWidth;
	    currMaxWidth = 0;
	}
	else
	{
	    int w = fm.width(c);
	    currMinWidth += w;
	    currMaxWidth += w;
	}
    }
    if(currMinWidth > minWidth) minWidth = currMinWidth;
    currMinWidth = 0;
    if(currMaxWidth > maxWidth) maxWidth = currMaxWidth;
    currMaxWidth = 0;

    setMinMaxKnown();
}

bool TextImpl::mouseEvent( int _x, int _y, int, MouseEventType, 
			   int _tx, int _ty, DOMString &)
{
    //printf("Text::mouseEvent\n");
    bool inside = false;

    TextSlave *s = first;
    QFontMetrics fm(*font);
    while(s)
    {
	if(s->checkPoint(_x, _y, _tx, _ty, fm)) 
	{
	    inside = true;
	    break;
	}
	s=s->next();
    }
    //if(inside) printf(" Text  --> inside\n");

    return inside;
}
