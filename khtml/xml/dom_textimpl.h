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
#ifndef _DOM_CharacterDataImpl_h_
#define _DOM_CharacterDataImpl_h_

#include "dom_nodeimpl.h"
#include "dom_string.h"

namespace khtml {
    class RenderStyle;
}

namespace DOM {

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

class CommentImpl : public CharacterDataImpl
{
public:
    CommentImpl(DocumentImpl *doc, const DOMString &_text);
    CommentImpl(DocumentImpl *doc);
    virtual ~CommentImpl();

    virtual const DOMString nodeName() const;
    virtual DOMString nodeValue() const;
    virtual unsigned short nodeType() const;
    virtual ushort id() const;
    virtual NodeImpl *cloneNode(bool deep);
};

// ----------------------------------------------------------------------------

class TextImpl : public CharacterDataImpl
{
public:
    TextImpl(DocumentImpl *impl, const DOMString &_text);
    TextImpl(DocumentImpl *impl);
    virtual ~TextImpl();

    virtual void setStyle(khtml::RenderStyle *) {}
    virtual khtml::RenderStyle *style() { return _parent->style(); }

    virtual const DOMString nodeName() const;
    virtual DOMString nodeValue() const;
    virtual unsigned short nodeType() const;
    virtual bool isTextNode() const { return true; }

    TextImpl *splitText ( const unsigned long offset );

    virtual ushort id() const;

    virtual void attach(KHTMLView *);
    virtual void detach();
    virtual void applyChanges(bool top=true, bool force=true);

    virtual bool mouseEvent( int _x, int _y, int, MouseEventType,
			     int _tx, int _ty, DOMString &,
                             NodeImpl *& innerNode, long &offset);
    virtual NodeImpl *cloneNode(bool deep);
    virtual void recalcStyle();

    khtml::RenderStyle *m_style;
};

}; //namespace
#endif
