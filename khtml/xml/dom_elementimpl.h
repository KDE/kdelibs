/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Peter Kelly (pmk@post.com)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 *           (C) 2003, 2004, 2005, 2006, 2007 Apple Inc. All rights reserved.
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
#ifndef _DOM_ELEMENTImpl_h_
#define _DOM_ELEMENTImpl_h_

#include "dom_nodeimpl.h"
#include "dom/dom_exception.h"
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
class ElementRareDataImpl;

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
    AttrImpl(ElementImpl* element, DocumentImpl* docPtr, NodeImpl::Id attrId,
	     DOMStringImpl *value, DOMStringImpl *prefix = 0);
    ~AttrImpl();

private:
    AttrImpl(const AttrImpl &other);
    AttrImpl &operator = (const AttrImpl &other);
    void createTextChild();
public:

    // DOM methods & attributes for Attr
    bool specified() const { return true; /* we don't yet support default attributes*/ }
    ElementImpl* ownerElement() const { return m_element; }
    void setOwnerElement( ElementImpl* impl ) { m_element = impl; }
    DOMString name() const;

    //DOMString value() const;
    void setValue( const DOMString &v, int &exceptioncode );

    // DOM methods overridden from  parent classes
    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual DOMString prefix() const;
    virtual void setPrefix(const DOMString &_prefix, int &exceptioncode );
    virtual DOMString namespaceURI() const;
    virtual DOMString localName() const;

    virtual DOMString nodeValue() const;
    virtual void setNodeValue( const DOMString &, int &exceptioncode );
    virtual WTF::PassRefPtr<NodeImpl> cloneNode ( bool deep );

    // Other methods (not part of DOM)
    virtual bool isAttributeNode() const { return true; }
    virtual bool childAllowed( NodeImpl *newChild );
    virtual bool childTypeAllowed( unsigned short type );
    virtual NodeImpl::Id id() const { return m_attrId; }
    virtual void childrenChanged();

    // This is used for when the value is normalized on setting by
    // parseAttribute; it silently updates it w/o issuing any events, etc.
    // Doesn't work for ATTR_ID!
    void rewriteValue( const DOMString& newValue );

    virtual DOMString toString() const;

    void setElement(ElementImpl *element);
    DOMStringImpl *val() { return m_value; }

protected:
    ElementImpl *m_element;
    NodeImpl::Id m_attrId;
    DOMStringImpl *m_value;
    DOMStringImpl *m_prefix;
    DOMStringImpl *m_localName;
};

// Mini version of AttrImpl internal to NamedAttrMapImpl.
// Stores either the id and value of an attribute
// (in the case of m_attrId != 0), or a pointer to an AttrImpl (if m_attrId == 0)
// The latter case only happens when the Attr node is requested by some DOM
// code or is an XML attribute.
// In most cases the id and value is all we need to store, which is more
// memory efficient.
struct AttributeImpl
{
    NodeImpl::Id id() const { return m_attrId ? m_attrId : m_data.attr->id(); }
    DOMStringImpl *val() const { if (m_attrId) return m_data.value; else return m_data.attr->val(); }
    DOMString value() const { return val(); }
    AttrImpl *attr() const { return m_attrId ? 0 : m_data.attr; }
    DOMString namespaceURI() { return m_attrId ? DOMString() : m_data.attr->namespaceURI(); }
    DOMString prefix() { return m_attrId ? DOMString() : m_data.attr->prefix(); }
    DOMString localName() { return m_attrId ? DOMString() : m_data.attr->localName(); }
    DOMString name() { return m_attrId ? DOMString() : m_data.attr->name(); }

    void setValue(DOMStringImpl *value, ElementImpl *element);
    AttrImpl *createAttr(ElementImpl *element, DocumentImpl *docPtr);
    void free();

    // See the description for AttrImpl.
    void rewriteValue( const DOMString& newValue );

    NodeImpl::Id m_attrId;
    union {
        DOMStringImpl *value;
        AttrImpl *attr;
    } m_data;
};

struct CombinedStyleDecl
{
    DOM::CSSStyleDeclarationImpl *inlineDecls;
    DOM::CSSStyleDeclarationImpl *nonCSSDecls;
};

class ElementImpl : public NodeBaseImpl
{
    friend class DocumentImpl;
    friend class NamedAttrMapImpl;
    friend class AttrImpl;
    friend class NodeImpl;
    friend class khtml::CSSStyleSelector;
public:
    ElementImpl(DocumentImpl *doc);
    ~ElementImpl();
    
    //Higher-level DOM stuff
    virtual bool hasAttributes() const;
    bool hasAttribute( const DOMString& name ) const;
    bool hasAttributeNS( const DOMString &namespaceURI, const DOMString &localName ) const;
    DOMString getAttribute( const DOMString &name );
    void setAttribute( const DOMString &name, const DOMString &value, int& exceptioncode );
    void removeAttribute( const DOMString &name, int& exceptioncode );
    void removeAttribute( NodeImpl::Id id, int &exceptioncode );
    AttrImpl* getAttributeNode( const DOMString &name );
    Attr setAttributeNode( AttrImpl* newAttr, int& exceptioncode );
    Attr removeAttributeNode( AttrImpl* oldAttr, int& exceptioncode );
    
    DOMString getAttributeNS( const DOMString &namespaceURI, const DOMString &localName, int& exceptioncode );
    void removeAttributeNS( const DOMString &namespaceURI, const DOMString &localName, int& exceptioncode );
    AttrImpl* getAttributeNodeNS( const DOMString &namespaceURI, const DOMString &localName, int& exceptioncode );
    Attr setAttributeNodeNS( AttrImpl* newAttr, int& exceptioncode );

    //Lower-level implementation primitives
    DOMString getAttribute( NodeImpl::Id id, bool nsAware = false, const DOMString& qName = DOMString() ) const;
    DOMStringImpl* getAttributeImpl( NodeImpl::Id id, bool nsAware = false, DOMStringImpl* qName = 0 ) const;
    bool hasAttribute( NodeImpl::Id id, bool nsAware = false, DOMStringImpl* qName = 0 ) const {
        return getAttributeImpl( id, nsAware, qName ) != 0;
    }
    void setAttribute( NodeImpl::Id id, const DOMString &value, const DOMString &qName,
                       int &exceptioncode );
    void setAttributeNS( const DOMString &namespaceURI, const DOMString &qualifiedName,
                         const DOMString& value, int &exceptioncode );
    virtual DOMString prefix() const;
    void setPrefix(const DOMString &_prefix, int &exceptioncode );
    virtual DOMString namespaceURI() const;

    // DOM methods overridden from  parent classes
    virtual DOMString tagName() const = 0;
    virtual unsigned short nodeType() const;
    virtual WTF::PassRefPtr<NodeImpl> cloneNode ( bool deep );
    virtual DOMString nodeName() const;
    virtual NodeImpl::Id id() const = 0;
    virtual bool isElementNode() const { return true; }
    virtual void insertedIntoDocument();
    virtual void removedFromDocument();

    // convenience methods which ignore exceptions
    void setAttribute (NodeImpl::Id id, const DOMString &value);

    NamedAttrMapImpl* attributes(bool readonly = false) const
    {
        if (!readonly && !namedAttrMap) createAttributeMap();
        return namedAttrMap;
    }

    //This is always called, whenever an attribute changed
    virtual void parseAttribute(AttributeImpl *) {}

    void parseNullAttribute(NodeImpl::Id attrId) {
	AttributeImpl aimpl;
	aimpl.m_attrId = attrId;
	aimpl.m_data.value = 0;
	parseAttribute(&aimpl);
    }

    void parseAttribute(AttrImpl* fullAttr) {
	AttributeImpl aimpl;
	aimpl.m_attrId = 0;
	aimpl.m_data.attr = fullAttr;
	parseAttribute(&aimpl);
    }

    // not part of the DOM
    void setAttributeMap ( NamedAttrMapImpl* list );

    // State of the element.
    virtual QString state() { return QString(); }

    virtual void copyNonAttributeProperties(const ElementImpl* /*source*/) {}

    virtual void attach();
    virtual void close();
    virtual void detach();
    virtual void structureChanged();
    virtual void backwardsStructureChanged();
    virtual void attributeChanged(NodeImpl::Id attrId);
    
    virtual void defaultEventHandler(EventImpl *evt);

    virtual khtml::RenderStyle *styleForRenderer(khtml::RenderObject *parent);
    virtual khtml::RenderObject *createRenderer(khtml::RenderArena *, khtml::RenderStyle *);
    virtual void recalcStyle( StyleChange = NoChange );
    virtual khtml::RenderStyle* computedStyle();

    virtual void mouseEventHandler( MouseEvent* /*ev*/, bool /*inside*/ ) {}

    virtual bool childAllowed( NodeImpl *newChild );
    virtual bool childTypeAllowed( unsigned short type );
    DOM::CSSStyleDeclarationImpl *inlineStyleDecls() const { return m_hasCombinedStyle ? m_style.combinedDecls->inlineDecls : m_style.inlineDecls; }
    DOM::CSSStyleDeclarationImpl *nonCSSStyleDecls() const { return m_hasCombinedStyle ? m_style.combinedDecls->nonCSSDecls : 0; }
    DOM::CSSStyleDeclarationImpl *getInlineStyleDecls();

    void dispatchAttrRemovalEvent(NodeImpl::Id id, DOMStringImpl *value);
    void dispatchAttrAdditionEvent(NodeImpl::Id id, DOMStringImpl *value);

    virtual DOMString toString() const;
    virtual DOMString selectionToString(NodeImpl *selectionStart, NodeImpl *selectionEnd, int startOffset, int endOffset, bool &found) const;

    virtual bool isFocusable() const;	    
    virtual bool isContentEditable() const;
    void setContentEditable(bool enabled);

    void scrollIntoView(bool alignToTop);

    /** Returns the opening tag and properties.
     *  Examples:  '<b', '<img alt="hello" src="image.png"
     *
     *  For security reasons, passwords are stripped out of all src= and
     *  href=  tags if expandurls is turned on.
     *
     *  @param expandurls If this is set then in the above example, it would give
     *                    src="http://website.com/image.png".  Note that the password
     *                    is stripped out of the url.
     *
     *  DOM::RangeImpl uses this which is why it is public.
     */
    DOMString openTagStartToString(bool expandurls = false) const;
    
    void updateId(DOMStringImpl* oldId, DOMStringImpl* newId);
    //Called when mapping from id to this node in document should be removed
    virtual void removeId(const QString& id);
    //Called when mapping from id to this node in document should be added
    virtual void addId   (const QString& id);

protected:
    void createAttributeMap() const;
    void createInlineDecl(); // for inline styles
    void createNonCSSDecl(); // for presentational styles
    void finishCloneNode( ElementImpl *clone, bool deep );

private:
    // map of default attributes. derived element classes are responsible
    // for setting this according to the corresponding element description
    // in the DTD
    virtual NamedAttrMapImpl* defaultMap() const;

    // There is some information such as computed style for display:none
    // elements that is needed only for a few elements. We store it
    // in one of these. 
    ElementRareDataImpl* rareData();
    const ElementRareDataImpl* rareData() const;
    ElementRareDataImpl* createRareData();


protected: // member variables
    mutable NamedAttrMapImpl *namedAttrMap;

    union {
        DOM::CSSStyleDeclarationImpl *inlineDecls;
        CombinedStyleDecl *combinedDecls;
    } m_style;
    DOMStringImpl *m_prefix;
};


class XMLElementImpl : public ElementImpl
{

public:
    XMLElementImpl(DocumentImpl *doc, NodeImpl::Id id);
    XMLElementImpl(DocumentImpl *doc, NodeImpl::Id id, DOMStringImpl *_qualifiedName);
    ~XMLElementImpl();

    // DOM methods overridden from  parent classes
    virtual DOMString tagName() const;
    virtual DOMString localName() const;
    virtual WTF::PassRefPtr<NodeImpl> cloneNode ( bool deep );

    // Other methods (not part of DOM)
    virtual bool isXMLElementNode() const { return true; }
    virtual Id id() const { return m_id; }

protected:
    Id  m_id;
};

// the map of attributes of an element
class NamedAttrMapImpl : public NamedNodeMapImpl
{
    friend class ElementImpl;
public:
    NamedAttrMapImpl(ElementImpl *element);
    virtual ~NamedAttrMapImpl();

    // DOM methods & attributes for NamedNodeMap
    virtual NodeImpl *getNamedItem ( NodeImpl::Id id, bool nsAware = false, DOMStringImpl* qName = 0 ) const;
    virtual Node removeNamedItem ( NodeImpl::Id id, bool nsAware, DOMStringImpl* qName, int &exceptioncode );
    virtual Node setNamedItem ( NodeImpl* arg, bool nsAware, DOMStringImpl* qName, int &exceptioncode );

    virtual NodeImpl *item ( unsigned long index ) const;
    virtual unsigned long length(  ) const;

    // Other methods (not part of DOM)
    virtual bool isReadOnly() { return false; }

    AttributeImpl *attrAt(unsigned long index) const { return &m_attrs[index]; }
    // ### replace idAt and getValueAt with attrAt
    NodeImpl::Id idAt(unsigned long index) const;
    DOMStringImpl *valueAt(unsigned long index) const;
    DOMStringImpl *getValue(NodeImpl::Id id, bool nsAware = false, DOMStringImpl* qName = 0) const;
    void setValue(NodeImpl::Id id, DOMStringImpl *value, DOMStringImpl* qName = 0,
                  DOMStringImpl *prefix = 0, bool nsAware = false, bool hasNS = false);
    Attr removeAttr(AttrImpl *attr);
    NodeImpl::Id mapId(DOMStringImpl* namespaceURI, DOMStringImpl* localName, bool readonly);
    void copyAttributes(NamedAttrMapImpl *other);
    void setElement(ElementImpl *element);
    void detachFromElement();

protected:
    ElementImpl *m_element;
    AttributeImpl *m_attrs;
    unsigned long m_attrCount;
};

// ------------  inline DOM helper functions ---------------

inline bool checkQualifiedName(const DOMString &qualifiedName, const DOMString &namespaceURI, int *colonPos,
                               bool nameCanBeNull, bool nameCanBeEmpty, int *pExceptioncode)
{

    // Not mentioned in spec: throw NAMESPACE_ERR if no qualifiedName supplied
    if (!nameCanBeNull && qualifiedName.isNull()) {
        if (pExceptioncode)
            *pExceptioncode = DOMException::NAMESPACE_ERR;
        return false;
    }

    // INVALID_CHARACTER_ERR: Raised if the specified qualified name contains an illegal character.
    if (!qualifiedName.isNull() && !Element::khtmlValidQualifiedName(qualifiedName)
        && ( !qualifiedName.isEmpty() || !nameCanBeEmpty ) ) {
        if (pExceptioncode)
            *pExceptioncode = DOMException::INVALID_CHARACTER_ERR;
        return false;
    }

    // NAMESPACE_ERR:
    // - Raised if the qualifiedName is malformed,
    // - if the qualifiedName has a prefix and the namespaceURI is null, or
    // - if the qualifiedName is null and the namespaceURI is different from null
    // - if the qualifiedName has a prefix that is "xml" and the namespaceURI is different
    //   from "http://www.w3.org/XML/1998/namespace" [Namespaces].
    int colonpos = -1;
    uint i;
    DOMStringImpl *qname = qualifiedName.implementation();
    for (i = 0 ; i < (qname ? qname->l : 0); i++) {
        if ((*qname)[i] == QLatin1Char(':')) {
            colonpos = i;
            break;
        }
    }

    bool hasXMLPrefix = colonpos == 3 && (*qname)[0] == QLatin1Char('x') &&
      (*qname)[1] == QLatin1Char('m') && (*qname)[2] == QLatin1Char('l');
    bool hasXMLNSPrefix = colonpos == 5 && (*qname)[0] == QLatin1Char('x') &&
      (*qname)[1] == QLatin1Char('m') && (*qname)[2] == QLatin1Char('l') &&
      (*qname)[3] == QLatin1Char('n') && (*qname)[4] == QLatin1Char('s');

    if ((!qualifiedName.isNull() && Element::khtmlMalformedQualifiedName(qualifiedName)) ||
        (colonpos >= 0 && namespaceURI.isNull()) ||
        colonpos == 0 || // prefix has to consist of at least a letter
        (qualifiedName.isNull() && !namespaceURI.isNull()) ||
        (hasXMLPrefix &&
         namespaceURI != "http://www.w3.org/XML/1998/namespace") ||
        (hasXMLNSPrefix &&
         namespaceURI != "http://www.w3.org/2000/xmlns/") ||
        (namespaceURI == "http://www.w3.org/2000/xmlns/" &&
         !hasXMLNSPrefix && qualifiedName != "xmlns")) {
        if (pExceptioncode)
            *pExceptioncode = DOMException::NAMESPACE_ERR;
        return false;
    }
    if(colonPos)
        *colonPos = colonpos;
    return true;
}

inline void splitPrefixLocalName(DOMStringImpl *qualifiedName, DOMString &prefix, DOMString &localName, int colonPos = -2)
{
    if (colonPos == -2)
        for (uint i = 0 ; i < qualifiedName->l ; ++i)
            if (qualifiedName->s[i] == QLatin1Char(':')) {
                colonPos = i;
                break;
            }
    if (colonPos >= 0) {
        prefix = qualifiedName->copy();
        localName = prefix.split(colonPos+1);
        prefix.implementation()->truncate(colonPos);
    } else
        localName = qualifiedName->copy();
}

} //namespace

#endif
