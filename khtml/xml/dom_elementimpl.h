/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Peter Kelly (pmk@post.com)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
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
 */
#ifndef _DOM_ELEMENTImpl_h_
#define _DOM_ELEMENTImpl_h_

#include "dom_nodeimpl.h"
#include "dom/dom_element.h"
#include "xml/dom_stringimpl.h"
#include "misc/shared.h"

namespace khtml {
    class CSSStyleSelector;
}

namespace DOM {

class ElementImpl;
class DocumentImpl;
class NamedAttrMapImpl;

// Attr can have Text and EntityReference children
// therefore it has to be a fullblown Node. The plan
// is to dynamically allocate a textchild and store the
// resulting nodevalue in the AttributeImpl upon
// destruction. however, this is not yet implemented.
class AttrImpl : public NodeBaseImpl
{
    friend class ElementImpl;
    friend class NamedAttrMapImpl;

public:
    AttrImpl(ElementImpl* element, DocumentPtr* docPtr, NodeImpl::Id attrId,
	     DOMStringImpl *value, DOMStringImpl *prefix);
    ~AttrImpl();

private:
    AttrImpl(const AttrImpl &other);
    AttrImpl &operator = (const AttrImpl &other);
public:

    // DOM methods & attributes for Attr
    bool specified() const { return m_specified; }
    ElementImpl* ownerElement() const { return m_element; }
    NodeImpl::Id attrId() const { return m_attrId; }

    //DOMString value() const;
    void setValue( const DOMString &v, int &exceptioncode );

    // DOM methods overridden from  parent classes
    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual DOMString prefix() const;
    virtual void setPrefix(const DOMString &_prefix, int &exceptioncode );

    virtual DOMString nodeValue() const;
    virtual void setNodeValue( const DOMString &, int &exceptioncode );
    virtual NodeImpl *cloneNode ( bool deep );

    // Other methods (not part of DOM)
    virtual bool isAttributeNode() const { return true; }
    virtual bool childAllowed( NodeImpl *newChild );
    virtual bool childTypeAllowed( unsigned short type );

    void setElement(ElementImpl *element);
    DOMStringImpl *val() { return m_value; }

protected:
    ElementImpl *m_element;
    NodeImpl::Id m_attrId;
    DOMStringImpl *m_value;
    DOMStringImpl *m_prefix;
};

// Mini version of AttrImpl. Stores either the id and value of an attribute
// (in the case of m_attrId != 0), or a pointer to an AttrImpl (if m_attrId == 0)
// The latter case only happens when the Attr node is requested by some DOM
// code. In most cases the id and value is all we need to store, which is more
// memory efficient.
struct AttributeImpl
{
    NodeImpl::Id id() const { return m_attrId ? m_attrId : m_data.attr->attrId(); }
    DOMStringImpl *val() const { return m_attrId ? m_data.value : m_data.attr->val(); }
    DOMString value() const { return val(); }
    AttrImpl *attr() const { return m_attrId ? 0 : m_data.attr; }

    void setValue(DOMStringImpl *value, ElementImpl *element);
    AttrImpl *createAttr(ElementImpl *element, DocumentPtr *docPtr);
    void free();

    NodeImpl::Id m_attrId;
    union {
	DOMStringImpl *value;
	AttrImpl *attr;
    } m_data;
};

class ElementImpl : public NodeBaseImpl
{
    friend class DocumentImpl;
    friend class NamedAttrMapImpl;
    friend class AttrImpl;
    friend class NodeImpl;
    friend class khtml::CSSStyleSelector;
public:
    ElementImpl(DocumentPtr *doc);
    ~ElementImpl();

    DOMString getAttribute( NodeImpl::Id id ) const;
    void setAttribute( NodeImpl::Id id, DOMStringImpl* value, int &exceptioncode );
    void removeAttribute( NodeImpl::Id id, int &exceptioncode );

    DOMString prefix() const { return m_prefix; }
    void setPrefix(const DOMString &_prefix, int &exceptioncode );

    // DOM methods overridden from  parent classes
    virtual DOMString tagName() const;
    virtual unsigned short nodeType() const;
    virtual NodeImpl *cloneNode ( bool deep );
    virtual DOMString nodeName() const;
    virtual bool isElementNode() const { return true; }

    // convenience methods which ignore exceptions
    void setAttribute (NodeImpl::Id id, const DOMString &value);

    NamedAttrMapImpl* attributes(bool readonly = false) const
    {
        if (!readonly && !namedAttrMap) createAttributeMap();
        return namedAttrMap;
    }

    //This is always called, whenever an attribute changed
    virtual void parseAttribute(AttributeImpl *) {}
    void parseAttribute(NodeImpl::Id attrId, DOMStringImpl *value) {
	AttributeImpl aimpl;
	aimpl.m_attrId = attrId;
	aimpl.m_data.value = value;
	parseAttribute(&aimpl);
    }

    // not part of the DOM
    void setAttributeMap ( NamedAttrMapImpl* list );

    // State of the element.
    virtual QString state() { return QString::null; }

    virtual void attach();
    virtual void recalcStyle( StyleChange = NoChange );

    virtual void mouseEventHandler( MouseEvent* /*ev*/, bool /*inside*/ ) {};
    virtual bool isSelectable() const;
    virtual bool childAllowed( NodeImpl *newChild );
    virtual bool childTypeAllowed( unsigned short type );

    DOM::CSSStyleDeclarationImpl *styleRules() {
      if (!m_styleDecls) createDecl();
      return m_styleDecls;
    }

    void dispatchAttrRemovalEvent(NodeImpl::Id id, DOMStringImpl *value);
    void dispatchAttrAdditionEvent(NodeImpl::Id id, DOMStringImpl *value);

protected:
    void createAttributeMap() const;
    void createDecl();

private:
    // map of default attributes. derived element classes are responsible
    // for setting this according to the corresponding element description
    // in the DTD
    virtual NamedAttrMapImpl* defaultMap() const;

protected: // member variables
    mutable NamedAttrMapImpl *namedAttrMap;

    DOM::CSSStyleDeclarationImpl *m_styleDecls;
    DOMStringImpl *m_prefix;
};


class XMLElementImpl : public ElementImpl
{

public:
    XMLElementImpl(DocumentPtr *doc, DOMStringImpl *_tagName);
    XMLElementImpl(DocumentPtr *doc, DOMStringImpl *_qualifiedName, DOMStringImpl *_namespaceURI);
    ~XMLElementImpl();

    // DOM methods overridden from  parent classes

    virtual DOMString localName() const;
    virtual NodeImpl *cloneNode ( bool deep );

    // Other methods (not part of DOM)
    virtual bool isXMLElementNode() const { return true; }
    virtual Id id() const { return m_id; }

protected:
    Id m_id;
};

// the map of attributes of an element
class NamedAttrMapImpl : public NamedNodeMapImpl
{
    friend class ElementImpl;
public:
    NamedAttrMapImpl(ElementImpl *element);
    virtual ~NamedAttrMapImpl();

    // ### are the non-NS methods equivalent to NS methods with a namespace of ""/null, or
    // should they match attributes in any namespace?

    // DOM methods & attributes for NamedNodeMap
    virtual NodeImpl *getNamedItem ( NodeImpl::Id id ) const;
    virtual Node removeNamedItem ( NodeImpl::Id id, int &exceptioncode );
    virtual Node setNamedItem ( NodeImpl* arg, int &exceptioncode );

    virtual NodeImpl *item ( unsigned long index ) const;
    virtual unsigned long length(  ) const;

    // Other methods (not part of DOM)
    virtual bool isReadOnly() { return false; }
    NodeImpl::Id idAt(unsigned long index) const;
    DOMStringImpl *valueAt(unsigned long index) const;
    DOMStringImpl *getValue(NodeImpl::Id id) const;
    void setValue(NodeImpl::Id id, DOMStringImpl *value);
    NodeImpl::Id mapId(const DOMString& namespaceURI,  const DOMString& localName,  bool readonly);
    void copyAttributes(NamedAttrMapImpl *other);
    void setElement(ElementImpl *element);
    void detachFromElement();

protected:
    ElementImpl *m_element;
    AttributeImpl *m_attrs;
    unsigned long m_attrCount;
};

}; //namespace

#endif
