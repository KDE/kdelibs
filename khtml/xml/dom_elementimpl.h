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
#ifndef _DOM_ELEMENTImpl_h_
#define _DOM_ELEMENTImpl_h_

#include "dom_nodeimpl.h"

#include "htmltoken.h"
#include "dtd.h"

namespace DOM {

class ElementImpl;
class DocumentImpl;

class AttrImpl : public NodeImpl
{
    friend class ElementImpl;

public:

    AttrImpl(DocumentImpl *doc, const DOMString &name);
    AttrImpl(const AttrImpl &other);

    AttrImpl &operator = (const AttrImpl &other);
    ~AttrImpl();

    virtual const DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual bool isAttributeNode() { return true; }

    DOMString name() const;
    bool specified() const;

    virtual DOMString value() const;
    virtual void setValue( const DOMString &v );

    virtual DOMString nodeValue() const;

    virtual void setNodeValue( const DOMString & );

    virtual NodeImpl *parentNode() const;

    virtual void setParent(NodeImpl *parent);

protected:
    AttrImpl(const DOMString &, const DOMString &, DocumentImpl *, bool);

    int attrId() { return attr.id; }

    khtml::Attribute attr;
    bool _specified;

    NodeImpl *_parent;
};


class ElementImpl : public NodeBaseImpl
{
    friend class DocumentImpl;

public:
    ElementImpl(DocumentImpl *doc);
    ~ElementImpl();

    virtual bool isInline();

    virtual unsigned short nodeType() const;
    virtual bool isElementNode() { return true; }

    virtual bool isHTMLElement() { return false; }

    DOMString tagName() const;

    DOMString getAttribute ( const DOMString &name );

    void setAttribute ( const DOMString &name, const DOMString &value );

    void removeAttribute ( const DOMString &name );

    AttrImpl *getAttributeNode ( const DOMString &name );

    AttrImpl *setAttributeNode ( AttrImpl *newAttr );

    AttrImpl *removeAttributeNode ( AttrImpl *oldAttr );

    NodeListImpl *getElementsByTagName ( const DOMString &name );

    void normalize (  );

    virtual void applyChanges(bool = true);

    virtual void setStyle(khtml::RenderStyle *style) { m_style = style; }
    virtual khtml::RenderStyle *style() { return m_style; }

    /**
     * override this in subclasses if you need to parse
     * attributes. This is always called, whenever an attribute changed
      */
    virtual void parseAttribute(khtml::Attribute *) {}

    virtual tagStatus startTag() { return DOM::REQUIRED; }
    virtual tagStatus endTag() { return DOM::REQUIRED; }

    // not part of the DOM

    DOMString getAttribute ( int id );
    void setAttribute ( int id, const DOMString &value );
    void setAttribute ( khtml::AttributeList list );

    // State of the element.
    virtual QString state() { return QString::null; }

    virtual void attach(KHTMLView *w);

    virtual DOMString toHTML(DOMString _string);

protected: // member variables

    khtml::AttributeList attributeMap;

    // map of default attributes. derived element classes are responsible
    // for setting this according to the corresponding element description
    // in the DTD
    virtual khtml::AttributeList *defaultMap() const;

    khtml::RenderStyle *m_style;
};

}; //namespace

#endif
