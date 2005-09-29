/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 2001-2003 Dirk Mueller (mueller@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2003 Apple Computer, Inc
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
#ifndef _DOM_CharacterDataImpl_h_
#define _DOM_CharacterDataImpl_h_

#include "xml/dom_nodeimpl.h"
#include "dom/dom_string.h"

namespace DOM {

    class DocumentImpl;
    class CharacterData;
    class Text;

class CharacterDataImpl : public NodeImpl
{
public:
    CharacterDataImpl(DocumentPtr *doc, DOMStringImpl* _text);
    CharacterDataImpl(DocumentPtr *doc)
        : NodeImpl(doc), str(0) {}

    virtual ~CharacterDataImpl();

    // DOM methods & attributes for CharacterData

    virtual void setData( const DOMString &_data, int &exceptioncode );
    virtual unsigned long length (  ) const;
    virtual DOMString substringData ( const unsigned long offset, const unsigned long count, int &exceptioncode );
    virtual void appendData ( const DOMString &arg, int &exceptioncode );
    virtual void insertData ( const unsigned long offset, const DOMString &arg, int &exceptioncode );
    virtual void deleteData ( const unsigned long offset, const unsigned long count, int &exceptioncode );
    virtual void replaceData ( const unsigned long offset, const unsigned long count, const DOMString &arg, int &exceptioncode );

    virtual bool containsOnlyWhitespace() const;

    // DOM methods overridden from  parent classes

    virtual DOMString nodeValue() const;
    virtual void setNodeValue( const DOMString &_nodeValue, int &exceptioncode );

    // Other methods (not part of DOM)

    DOMStringImpl *string() const { return str; }
    DOMString data() const { return str; }

    virtual void checkCharDataOperation( const unsigned long offset, int &exceptioncode );

    virtual long minOffset() const;
    virtual long maxOffset() const;

protected:
    // note: since DOMStrings are shared, str should always be copied when making
    // a change or returning a string
    DOMStringImpl *str;

    void dispatchModifiedEvent(DOMStringImpl *prevValue);
};

// ----------------------------------------------------------------------------

class CommentImpl : public CharacterDataImpl
{
public:
    CommentImpl(DocumentPtr *doc, DOMStringImpl* _text)
        : CharacterDataImpl(doc, _text) {}
    CommentImpl(DocumentPtr *doc)
        : CharacterDataImpl(doc) {}
    // DOM methods overridden from  parent classes
    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual NodeImpl *cloneNode(bool deep);

    // Other methods (not part of DOM)

    virtual Id id() const;
    virtual bool childTypeAllowed( unsigned short type );

    virtual DOMString toString() const;
};

// ----------------------------------------------------------------------------

class TextImpl : public CharacterDataImpl
{
public:
    TextImpl(DocumentPtr *impl, DOMStringImpl* _text)
        : CharacterDataImpl(impl, _text) {}
    TextImpl(DocumentPtr *impl)
        : CharacterDataImpl(impl) {}

    // DOM methods & attributes for CharacterData

    TextImpl *splitText ( const unsigned long offset, int &exceptioncode );

    // DOM methods overridden from  parent classes
    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual NodeImpl *cloneNode(bool deep);

    // Other methods (not part of DOM)

    virtual bool isTextNode() const { return true; }
    virtual Id id() const;
    virtual void attach();
    virtual bool rendererIsNeeded(khtml::RenderStyle *);
    virtual khtml::RenderObject *createRenderer(khtml::RenderArena *, khtml::RenderStyle *);
    virtual void recalcStyle( StyleChange = NoChange );
    virtual bool childTypeAllowed( unsigned short type );

    DOMStringImpl *renderString() const;
    
    virtual DOMString toString() const;
    /** Return the text for the node, with < replaced with &lt; and so on.
     *  @param startOffset The number of characters counted from the left, zero indexed, counting "<" as one character, to start from.  Use -1 to start from 0.
     *  @param endOffset The number of characters counted from the left, zero indexed, counting "<" as one character, to end on.  Use -1 to end at the end of the string.
     *  @return An html escaped version of the substring.
     */
    DOMString toString(long long startOffset, long long endOffset) const;
protected:
    virtual TextImpl *createNew(DOMStringImpl *_str);
};

// ----------------------------------------------------------------------------

class CDATASectionImpl : public TextImpl
{
public:
    CDATASectionImpl(DocumentPtr *impl, DOMStringImpl* _text)
        : TextImpl(impl, _text) {}
    CDATASectionImpl(DocumentPtr *impl)
        : TextImpl(impl) {}

    // DOM methods overridden from  parent classes
    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual NodeImpl *cloneNode(bool deep);

    // Other methods (not part of DOM)

    virtual bool childTypeAllowed( unsigned short type );

    virtual DOMString toString() const;

protected:
    virtual TextImpl *createNew(DOMStringImpl *_str);
};



} //namespace
#endif
