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
#ifndef _DOM_CharacterDataImpl_h_
#define _DOM_CharacterDataImpl_h_

#include "dom_nodeimpl.h"
#include "dom_string.h"
#include "khtmltags.h"

namespace khtml
{
    class TextSlave;
};


namespace DOM {

class DOMString;
class DocumentImpl;

class CharacterDataImpl : public NodeWParentImpl
{
public:
    CharacterDataImpl(DocumentImpl *doc, const DOMString &_text);
    CharacterDataImpl(DocumentImpl *doc);
    virtual ~CharacterDataImpl();

    DOMString data() const;

    void setData( const DOMString & );

    unsigned long length() const;

    DOMString substringData ( const unsigned long offset, const unsigned long count );

    void appendData ( const DOMString &arg );

    void insertData ( const unsigned long offset, const DOMString &arg );

    void deleteData ( const unsigned long offset, const unsigned long count );

    void replaceData ( const unsigned long offset, const unsigned long count, const DOMString &arg );

    DOMStringImpl *string() { return str; }
protected:
    DOMStringImpl *str;
};

// ----------------------------------------------------------------------------

class TextImpl : public CharacterDataImpl
{
public:
    TextImpl(DocumentImpl *impl, const DOMString &_text);
    TextImpl(DocumentImpl *impl);
    virtual ~TextImpl();

    virtual const DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual bool isTextNode() { return true; }

    TextImpl *splitText ( const unsigned long offset );

    // overrides NodeImpl

    virtual bool isRendered() { return true; }
    virtual ushort id() const { return ID_TEXT; }

    virtual void print( QPainter *, int x, int y, int w, int h, 
			int tx, int ty);
    virtual void printObject( QPainter *, int x, int y, int w, int h, 
			int tx, int ty);

    khtml::TextSlave *first;

    // overrides
    virtual void calcMinMaxWidth();
    virtual short getMinWidth() const { return minWidth; }
    virtual short getMaxWidth() const { return maxWidth; }
    virtual const HTMLFont *getFont() { return font; }
    virtual bool mouseEvent( int x, int y, int button, 
			     DOM::NodeImpl::MouseEventType, 
			     int _tx, int _ty, DOMString &url);
    virtual int getXPos() const;
    virtual int getYPos() const;
    virtual int getAscent() const;

protected:
    short minWidth;
    short maxWidth;
    const HTMLFont *font;
};

}; //namespace
#endif
