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
class NamedAttrMapImpl;

class AttrImpl : public NodeImpl
{
    friend class ElementImpl;
    friend class NamedAttrMapImpl;

public:
    AttrImpl();
    AttrImpl(DocumentImpl *doc, const DOMString &name);
    AttrImpl(DocumentImpl *doc, int id);
    AttrImpl(const AttrImpl &other);

    AttrImpl &operator = (const AttrImpl &other);
    ~AttrImpl();

    virtual const DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual bool isAttributeNode() const { return true; }

    DOMString name() const;
    bool specified() const { return _specified; }

    virtual DOMString value() const;
    virtual void setValue( const DOMString &v );

    virtual DOMString nodeValue() const { return value(); }

    virtual void setNodeValue( const DOMString & );

    virtual NodeImpl *parentNode() const;
    virtual NodeImpl *previousSibling() const;
    virtual NodeImpl *nextSibling() const;
    virtual NodeImpl *cloneNode ( bool deep );

    virtual bool deleteMe();
    DOMStringImpl *val() { return _value; }

    unsigned char attrId;
protected:
    AttrImpl(const DOMString &name, const DOMString &value, DocumentImpl *doc, bool specified);
    AttrImpl(const khtml::Attribute *attr, DocumentImpl *doc, ElementImpl *element);
    AttrImpl(const DOMString &name, const DOMString &value, DocumentImpl *doc);
    AttrImpl(int _id, const DOMString &value, DocumentImpl *doc);

    void setName(const DOMString &n);

    bool _specified;

    DOMStringImpl *_name;
    DOMStringImpl *_value;

    ElementImpl *_element;

};


class ElementImpl : public NodeBaseImpl
{
    friend class DocumentImpl;
    friend class NamedAttrMapImpl;
    friend class AttrImpl;

public:
    ElementImpl(DocumentImpl *doc);
    ~ElementImpl();

    virtual bool isInline();

    virtual unsigned short nodeType() const;
    virtual bool isElementNode() const { return true; }

    virtual bool isHTMLElement() const { return false; }

    DOMString tagName() const;

    DOMString getAttribute ( const DOMString &name );

    void setAttribute ( const DOMString &name, const DOMString &value );

    void removeAttribute ( const DOMString &name );

    AttrImpl *getAttributeNode ( const DOMString &name );

    AttrImpl *setAttributeNode ( AttrImpl *newAttr );

    AttrImpl *removeAttributeNode ( AttrImpl *oldAttr );

    NodeListImpl *getElementsByTagName ( const DOMString &name );

    void normalize (  );

    virtual void applyChanges(bool = true, bool = true);

    virtual void setStyle(khtml::RenderStyle *style) { m_style = style; }
    virtual khtml::RenderStyle *style() { return m_style; }
    virtual NodeImpl *cloneNode ( bool deep );
    virtual NamedNodeMapImpl *attributes() const;

    /**
     * override this in subclasses if you need to parse
     * attributes. This is always called, whenever an attribute changed
      */
    virtual void parseAttribute(AttrImpl *) {}

    virtual tagStatus startTag() { return DOM::REQUIRED; }
    virtual tagStatus endTag() { return DOM::REQUIRED; }

    // not part of the DOM

    DOMString getAttribute ( int id );
    AttrImpl *getAttributeNode ( int index );
    int getAttributeCount();
    void setAttribute ( int id, const DOMString &value );
    void setAttribute ( khtml::AttributeList list );

    // State of the element.
    virtual QString state() { return QString::null; }

    virtual void attach(KHTMLView *w);
    virtual void detach();
    virtual void recalcStyle();
    virtual void saveDefaults() {}

protected: // member variables

    NamedAttrMapImpl *namedAttrMap;

    // map of default attributes. derived element classes are responsible
    // for setting this according to the corresponding element description
    // in the DTD
    virtual khtml::AttributeList *defaultMap() const;

    khtml::RenderStyle *m_style;
};

class NamedAttrMapImpl : public NamedNodeMapImpl
{
public:
    NamedAttrMapImpl(ElementImpl *e);
    virtual ~NamedAttrMapImpl();
    void fromAttributeList(const khtml::AttributeList list);
    void fromNamedAttrMapImpl(const NamedAttrMapImpl *other);

    unsigned long length() const;

    NodeImpl *getNamedItem ( const DOMString &name ) const;
    AttrImpl *getIdItem ( int id ) const;

    NodeImpl *setNamedItem ( const Node &arg );
    AttrImpl *setIdItem ( AttrImpl *attr );

    NodeImpl *removeNamedItem ( const DOMString &name );
    AttrImpl *removeIdItem ( int id );

    NodeImpl *item ( unsigned long index ) const;
    AttrImpl *removeAttr( AttrImpl *oldAttr );
    void detachFromElement();

protected:
    ElementImpl *element;
    AttrImpl **attrs;
    uint len;
    void clearAttrs();

};

}; //namespace

#endif
