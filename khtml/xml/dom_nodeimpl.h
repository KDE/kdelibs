/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
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
 * $Id$
 */
#ifndef _DOM_NodeImpl_h_
#define _DOM_NodeImpl_h_

#include "dom/dom_misc.h"
#include "dom/dom_string.h"
#include "dom/dom_node.h"
#include "misc/helper.h"

// The namespace used for XHTML elements
#define XHTML_NAMESPACE "http://www.w3.org/1999/xhtml"

class QPainter;
template <class type> class QPtrList;
class KHTMLView;
class QRect;
class QMouseEvent;
class QKeyEvent;
class QTextStream;

namespace khtml {
    class RenderStyle;
    class RenderObject;
};

namespace DOM {

class NodeListImpl;
class NamedNodeMapImpl;
class DocumentImpl;
class CSSStyleDeclarationImpl;
class RegisteredEventListener;
class EventImpl;

    class DocumentPtr : public DomShared
    {
    public:
        DocumentImpl *document() const { return doc; }
    private:
        DocumentPtr() { doc = 0; }
        void resetDocument() { doc = 0; }
        friend class DocumentImpl;
        friend class DOMImplementationImpl;

        DocumentImpl *doc;
    };

/**
 * @internal
 *
 * Skeleton of a node. No children and no parents are allowed. We use this class as a basic Node Implementation, and
 * derive all other Node classes from it. This is done to reduce memory overhead. Derived classes will only implement
 * the functionality needed, and only use as much storage as they really need; i.e. if a node has no children or even
 * no parent; it does not need to store null pointers. (Such nodes could be attributes or readonly nodes at the end of
 * the tree)
 */
class NodeImpl : public DomShared
{
    friend class DocumentImpl;
public:
    NodeImpl(DocumentPtr *doc);
    virtual ~NodeImpl();

    // DOM methods & attributes for Node
    virtual DOMString nodeName() const;
    virtual DOMString nodeValue() const;
    virtual void setNodeValue( const DOMString &_nodeValue, int &exceptioncode );
    virtual unsigned short nodeType() const;
    virtual NodeImpl *parentNode() const;
    virtual NodeListImpl *childNodes();
    virtual NodeImpl *firstChild() const;
    virtual NodeImpl *lastChild() const;
    virtual NodeImpl *previousSibling() const;
    virtual NodeImpl *nextSibling() const;
    virtual NamedNodeMapImpl *attributes() const;
    virtual DocumentImpl *ownerDocument() const { return document->document(); }
    virtual NodeImpl *insertBefore ( NodeImpl *newChild, NodeImpl *refChild, int &exceptioncode );
    virtual NodeImpl *replaceChild ( NodeImpl *newChild, NodeImpl *oldChild, int &exceptioncode );
    virtual NodeImpl *removeChild ( NodeImpl *oldChild, int &exceptioncode );
    virtual NodeImpl *appendChild ( NodeImpl *newChild, int &exceptioncode );
    virtual bool hasChildNodes (  ) const;
    virtual NodeImpl *cloneNode ( bool deep, int &exceptioncode ) = 0;
    virtual void normalize ( int &exceptioncode );
    virtual bool isSupported( const DOMString &feature, const DOMString &version, int &exceptioncode ) const;
    virtual DOMString namespaceURI() const = 0;
    virtual DOMString prefix() const;
    virtual void setPrefix(const DOMString &_prefix, int &exceptioncode );
    virtual DOMString localName() const;
    virtual bool hasAttributes (  ) const;

    // Other methods (not part of DOM)

    virtual bool isElementNode() const { return false; }
    virtual bool isAttributeNode() const { return false; }
    virtual bool isTextNode() const { return false; }
    virtual bool isDocumentNode() const { return false; }
    virtual bool isXMLElementNode() const { return false; }

    // helper functions not being part of the DOM
    // Attention: all these functions assume the caller did
    //            the consistency checking!!!!
    virtual void setParent(NodeImpl *parent);

    virtual void setPreviousSibling(NodeImpl *);
    virtual void setNextSibling(NodeImpl *);

    virtual void setFirstChild(NodeImpl *child);
    virtual void setLastChild(NodeImpl *child);

    // used by the parser. Doesn't do as many error checkings as
    // appendChild(), and returns the node into which will be parsed next.
    virtual NodeImpl *addChild(NodeImpl *newChild);

    typedef Q_UINT32 Id;
    static const Q_UINT32 IdNSMask    = 0xffff0000;
    static const Q_UINT32 IdLocalMask = 0x0000ffff;
    // id() is used to easily and exactly identify a node. It
    // is optimized for quick comparison and low memory consumption.
    // its value depends on the ownerDocument() of the node and is
    // categorized in the following way:
    // 1..ID_LAST_TAG: the node inherits HTMLElementImpl and is
    //                 part of the HTML namespace.
    //                 The HTML namespace is either the global
    //                 one (no namespace) or the XHTML namespace
    //                 depending on the ownerDocument's doctype
    // ID_LAST_TAG..0xffff: non-HTML elements in the global namespace
    // others          non-HTML elements in a namespace.
    //                 the upper 16 bit identify the namespace
    //                 the lower 16 bit identify the local part of the
    //                 qualified element name.
    virtual Id id() const { return 0; };

    enum MouseEventType {
        MousePress,
        MouseRelease,
        MouseClick,
        MouseDblClick,
        MouseMove
    };

    struct MouseEvent
    {
        MouseEvent( int _button, MouseEventType _type,
                    const DOMString &_url = DOMString(), NodeImpl *_innerNode = 0, bool _noHref = false )
            {
                button = _button; type = _type;
                url = _url;
                innerNode = _innerNode;
                currentZIndex = -1;
                zIndex = -1;
                noHref = _noHref;
            }

        int button;
        MouseEventType type;
        DOMString url; // url under mouse or empty
        Node innerNode;
        int zIndex; // used to select the one most on top
        int currentZIndex; // temporary variable to avoid passing a param around
        bool noHref; // whether anchor should be blocked by a map element
        int nodeAbsX, nodeAbsY; // temporary hack for selection stuff
    };

    /*
     * Called whenever we are getting ready to handle a mouse event.
     * Does not actually perform any actions e.g. rollovers - this
     * is handled in eventDefault()
     *
     * This fills in ev with appropriate info such as innerNode, url etc.
     *
     * Returns true if the mouse is within this node (or it's children),
     * false otherwise
     */
    virtual bool prepareMouseEvent( int /*_x*/, int /*_y*/,
                                    int /*_tx*/, int /*_ty*/,
                                    MouseEvent */*ev*/ ) { return false; }

    virtual khtml::FindSelectionResult findSelectionNode( int /*_x*/, int /*_y*/, int /*_tx*/, int /*_ty*/,
                                                   DOM::Node & /*node*/, int & /*offset*/ )
                                                   { return khtml::SelectionPointBefore; }

    virtual void setStyle(khtml::RenderStyle *) {}
    virtual khtml::RenderStyle *style() const { return 0; }

    virtual void setRenderer(khtml::RenderObject *object) { m_render = object; }
    virtual khtml::RenderObject *renderer() const { return m_render; }

    virtual DOM::CSSStyleDeclarationImpl *styleRules() { return 0; }

    // for LINK and STYLE
    virtual void sheetLoaded() {}

    bool hasEvents() const  { return m_hasEvents; }
    bool hasID() const      { return m_hasId; }
    bool hasClass() const   { return m_hasClass; }
    bool hasStyle() const   { return m_hasStyle; }
    bool pressed() const    { return m_pressed; }
    bool active() const     { return m_active; }
    bool mouseInside() const { return m_mouseInside; }
    bool attached() const   { return m_attached; }
    bool changed() const    { return m_changed; }
    bool hasChangedChild() const { return m_hasChangedChild; }
    bool styleElement() const { return m_styleElement; }
    void setHasEvents(bool b=true) { m_hasEvents = b; }
    void setHasID(bool b=true) { m_hasId = b; }
    void setHasClass(bool b=true) { m_hasClass = b; }
    void setHasStyle(bool b=true) { m_hasStyle = b; }
    void setPressed(bool b=true) { m_pressed = b; }
    void setMouseInside(bool b=true) { m_mouseInside = b; }
    void setAttached(bool b=true) { m_attached = b; }
    void setHasChangedChild( bool b = true ) { m_hasChangedChild = b; }
    virtual void setFocus(bool b=true) { m_focused = b; }
    virtual void setActive(bool b=true) { m_active = b; }
    virtual void setChanged(bool b=true);

    unsigned short tabIndex() const { return m_tabIndex; }
    void setTabIndex(unsigned short _tabIndex) { m_tabIndex = _tabIndex; }

    /**
     * whether this node can receive the keyboard focus.
     */
    virtual bool isSelectable() const { return false; };

    /**
     * attach to a KHTMLView. Additional info (like style information,
     * frames, etc...) will only get loaded, when attached to a widget.
     */
    virtual void attach() { setAttached(true); }
    /**
     * detach from a HTMLWidget
     */
    virtual void detach() { setAttached(false); }

    // ### check if this function is still needed at all...
    virtual bool isInline() const { return true; }
    virtual void printTree(int indent=0);
    virtual QString toHTML() const;
    QString recursive_toHTML(bool start = false) const;

    virtual void getCursor(int offset, int &_x, int &_y, int &height);
    virtual QRect getRect() const;

    enum StyleChange { NoChange, NoInherit, Inherit, Force };
    virtual void recalcStyle( StyleChange = NoChange ) {}
    StyleChange diff( khtml::RenderStyle *s1, khtml::RenderStyle *s2 ) const;
    
    virtual unsigned long nodeIndex() const;

    virtual DocumentImpl* getDocument()
        { return document->document(); } // different from ownerDocument() in that it is never null

    void addEventListener(int id, EventListener *listener, const bool useCapture);
    void addEventListener(const DOMString &type, EventListener *listener,
                                  const bool useCapture, int &exceptioncode);
    void removeEventListener(int id, EventListener *listener, bool useCapture);
    void removeEventListener(const DOMString &type, EventListener *listener,
                                     bool useCapture,int &exceptioncode);
    void removeHTMLEventListener(int id);
    void setHTMLEventListener(int id, EventListener *listener);
    EventListener *getHTMLEventListener(int id);

    bool dispatchEvent(EventImpl *evt, int &exceptioncode, bool tempEvent = false);
    bool dispatchGenericEvent( EventImpl *evt, int &exceptioncode);
    bool dispatchHTMLEvent(int _id, bool canBubbleArg, bool cancelableArg);
    bool dispatchWindowEvent(int _id, bool canBubbleArg, bool cancelableArg);
    bool dispatchMouseEvent(QMouseEvent *e, int overrideId = 0, int overrideDetail = 0);
    bool dispatchUIEvent(int _id, int detail = 0);
    bool dispatchSubtreeModifiedEvent();
    bool dispatchKeyEvent(QKeyEvent *key);

    void handleLocalEvents(EventImpl *evt, bool useCapture);

    /**
     * Perform the default action for an event e.g. submitting a form
     */
    virtual void defaultEventHandler(EventImpl *evt);

    virtual bool isReadOnly() { return false; }
    virtual bool childTypeAllowed( unsigned short /*type*/ ) { return false; }
    virtual unsigned long childNodeCount();
    virtual NodeImpl *childNode(unsigned long index);
    NodeImpl *traverseNextNode(NodeImpl *stayWithin = 0) const;
    NodeImpl *traversePreviousNode() const;

    DocumentPtr *docPtr() const { return document; }

    virtual khtml::RenderObject *nextRenderer();

    void checkSetPrefix(const DOMString &_prefix, int &exceptioncode);
    void checkAddChild(NodeImpl *newChild, int &exceptioncode);
    bool isAncestor( NodeImpl *other );
    virtual bool childAllowed( NodeImpl *newChild );

    static bool validAttrName(const DOMString &/*name*/) { return true; }
    static bool validPrefix(const DOMString &/*prefix*/) { return true; }
    static bool vaildQualifiedName(const DOMString &/*qualifiedName*/) { return true; }
    static bool malformedPrefix(const DOMString &/*prefix*/) { return false; }
    static bool malformedQualifiedName(const DOMString &/*qualifiedName*/) { return false; }

    virtual void dump(QTextStream *stream, QString ind = "") const;

    // Methods for maintaining the state of the element between history navigation

    /**
     * Indicates whether or not this type of node maintains it's state. If so, the state of the node will be stored when
     * the user goes to a different page using the state() method, and restored using the restoreState() method if the
     * user returns (e.g. using the back button). This is used to ensure that user-changeable elements such as form
     * controls maintain their contents when the user returns to a previous page in the history.
     */
    virtual bool maintainsState();

    /**
     * Returns the state of this node represented as a string. This string will be passed to restoreState() if the user
     * returns to the page.
     *
     * @return State information about the node represented as a string
     */
    virtual QString state();

    /**
     * Sets the state of the element based on a string previosuly returned by state(). This is used to initialize form
     * controls with their old values when the user returns to the page in their history.
     *
     * @param state A string representation of the node's previously-stored state
     */
    virtual void restoreState(const QString &state);

protected:
    DocumentPtr *document;
    khtml::RenderObject *m_render;
    QPtrList<RegisteredEventListener> *m_regdListeners;

    unsigned short m_tabIndex : 15;
    bool m_hasTabIndex  : 1;

    bool m_hasEvents : 1;
    bool m_hasId : 1;
    bool m_hasClass : 1;
    bool m_hasStyle : 1;
    bool m_pressed : 1;
    bool m_mouseInside : 1;
    bool m_attached : 1;
    bool m_changed : 1;
    bool m_hasChangedChild : 1;
    bool m_specified : 1; // used in AttrImpl. Accessor functions there
    bool m_focused : 1;
    bool m_active : 1;
    bool m_styleElement : 1; // contains stylesheet text
};

/**
 * @internal
 *
 * This class implements nodes, which can have a parent but no children:
 */
class NodeWParentImpl : public NodeImpl
{
public:
    NodeWParentImpl(DocumentPtr *doc);

    virtual ~NodeWParentImpl();

    // DOM methods overridden from  parent classes
    virtual NodeImpl *parentNode() const;
    virtual NodeImpl *previousSibling() const;
    virtual NodeImpl *nextSibling() const;

    // helper functions not being part of the DOM
    virtual void setParent(NodeImpl *parent);
    virtual bool deleteMe();

    virtual void setPreviousSibling(NodeImpl *);
    virtual void setNextSibling(NodeImpl *);
    virtual unsigned long nodeIndex() const;
    virtual bool isReadOnly();
    virtual khtml::RenderObject *nextRenderer();

    virtual bool prepareMouseEvent( int x, int y,
                                    int _tx, int _ty,
                                    MouseEvent *ev);

protected:
    NodeImpl *_parent;
    NodeImpl *_previous;
    NodeImpl *_next;

    // helper function; throws exception if modifying a readonly node
    bool checkReadOnly() const;
};

/**
 * @internal
 *
 * This is the full Node Implementation with parents and children.
 */
class NodeBaseImpl : public NodeWParentImpl
{
public:
    NodeBaseImpl(DocumentPtr *doc);
    virtual ~NodeBaseImpl();

    // DOM methods overridden from  parent classes
    virtual NodeImpl *firstChild() const;
    virtual NodeImpl *lastChild() const;
    virtual NodeImpl *insertBefore ( NodeImpl *newChild, NodeImpl *refChild, int &exceptioncode );
    virtual NodeImpl *replaceChild ( NodeImpl *newChild, NodeImpl *oldChild, int &exceptioncode );
    virtual NodeImpl *removeChild ( NodeImpl *oldChild, int &exceptioncode );
    virtual NodeImpl *appendChild ( NodeImpl *newChild, int &exceptioncode );
    virtual bool hasChildNodes (  ) const;

    // Other methods (not part of DOM)
    void removeChildren();
    virtual void setFirstChild(NodeImpl *child);
    virtual void setLastChild(NodeImpl *child);
    virtual NodeImpl *addChild(NodeImpl *newChild);
    virtual void attach();
    virtual void detach();
    virtual void cloneChildNodes(NodeImpl *clone, int &exceptioncode);

    virtual NodeListImpl *getElementsByTagNameNS ( DOMStringImpl* namespaceURI,
                                                   DOMStringImpl* localName );

    virtual void setStyle(khtml::RenderStyle *style);
    virtual khtml::RenderStyle *style() const { return m_style; }

    virtual QRect getRect() const;
    bool getUpperLeftCorner(int &xPos, int &yPos) const;
    bool getLowerRightCorner(int &xPos, int &yPos) const;

    virtual bool prepareMouseEvent( int x, int y,
                                    int _tx, int _ty,
                                    MouseEvent *ev);

    virtual void setFocus(bool=true);
    virtual void setActive(bool=true);
    virtual unsigned long childNodeCount();
    virtual NodeImpl *childNode(unsigned long index);

    // check for being (grand-..)father:
    bool checkNoOwner( NodeImpl *other, int &exceptioncode );

protected:
    NodeImpl *_first;
    NodeImpl *_last;
    khtml::RenderStyle* m_style;

    // helper functions for inserting children:

    // check for same source document:
    bool checkSameDocument( NodeImpl *newchild, int &exceptioncode );
    // check for being child:
    bool checkIsChild( NodeImpl *oldchild, int &exceptioncode );
    // find out if a node is allowed to be our child
    void dispatchChildInsertedEvents( NodeImpl *child, int &exceptioncode );
};

// --------------------------------------------------------------------------
class Node;
class NodeImpl;

class NodeListImpl : public DomShared
{
public:

    // DOM methods & attributes for NodeList
    virtual unsigned long length() const;
    virtual NodeImpl *item ( unsigned long index ) const;

    // Other methods (not part of DOM)

protected:
    // helper functions for searching all ElementImpls in a tree
    unsigned long recursiveLength(NodeImpl *start) const;
    NodeImpl *recursiveItem ( NodeImpl *start, unsigned long &offset ) const;
    virtual bool nodeMatches( NodeImpl *testNode ) const = 0;
};

class ChildNodeListImpl : public NodeListImpl
{
public:
    ChildNodeListImpl( NodeImpl *n);
    virtual ~ChildNodeListImpl();

    // DOM methods overridden from  parent classes

    virtual unsigned long length() const;
    virtual NodeImpl *item ( unsigned long index ) const;

protected:
    virtual bool nodeMatches( NodeImpl *testNode ) const;

    NodeImpl *refNode;
};


/**
 * NodeList which lists all Nodes in a document with a given tag name
 */
class TagNodeListImpl : public NodeListImpl
{
public:
    TagNodeListImpl( NodeImpl *n, NodeImpl::Id tagId, NodeImpl::Id tagIdMask );
    virtual ~TagNodeListImpl();

    // DOM methods overridden from  parent classes

    virtual unsigned long length() const;
    virtual NodeImpl *item ( unsigned long index ) const;

    // Other methods (not part of DOM)

protected:
    virtual bool nodeMatches( NodeImpl *testNode ) const;

    NodeImpl *refNode;
    NodeImpl::Id m_id;
    NodeImpl::Id m_idMask;
};


/**
 * NodeList which lists all Nodes in a Element with a given "name=" tag
 */
class NameNodeListImpl : public NodeListImpl
{
public:
    NameNodeListImpl( NodeImpl *doc, const DOMString &t );
    virtual ~NameNodeListImpl();

    // DOM methods overridden from  parent classes

    virtual unsigned long length() const;
    virtual NodeImpl *item ( unsigned long index ) const;

    // Other methods (not part of DOM)

protected:
    virtual bool nodeMatches( NodeImpl *testNode ) const;

    NodeImpl *refNode;
    DOMString nodeName;
};


// Generic NamedNodeMap interface
// Other classes implement this for more specific situations e.g. attributes
// of an elemenet
class NamedNodeMapImpl : public DomShared
{
public:
    NamedNodeMapImpl();
    virtual ~NamedNodeMapImpl();

    // DOM methods & attributes for NamedNodeMap

    virtual NodeImpl *getNamedItem ( const DOMString &name, int &exceptioncode ) const = 0;
    virtual Node setNamedItem ( const Node &arg, int &exceptioncode ) = 0;
    virtual Node removeNamedItem ( const DOMString &name, int &exceptioncode ) = 0;
    virtual NodeImpl *item ( unsigned long index ) const = 0;
    virtual unsigned long length(  ) const = 0;
    virtual NodeImpl *getNamedItemNS( const DOMString &namespaceURI, const DOMString &localName,
                                      int &exceptioncode ) const = 0;
    virtual NodeImpl *setNamedItemNS( NodeImpl *arg, int &exceptioncode ) = 0;
    virtual NodeImpl *removeNamedItemNS( const DOMString &namespaceURI, const DOMString &localName,
                                         int &exceptioncode ) = 0;

    // Other methods (not part of DOM)

    virtual bool isReadOnly() { return false; }
};


// Generic read-only NamedNodeMap implementation
// You can add items using the internal function addItem()
class GenericRONamedNodeMapImpl : public NamedNodeMapImpl
{
public:
    GenericRONamedNodeMapImpl();
    virtual ~GenericRONamedNodeMapImpl();

    // DOM methods & attributes for NamedNodeMap

    virtual NodeImpl *getNamedItem ( const DOMString &name, int &exceptioncode ) const;
    virtual Node setNamedItem ( const Node &arg, int &exceptioncode );
    virtual Node removeNamedItem ( const DOMString &name, int &exceptioncode );
    virtual NodeImpl *item ( unsigned long index ) const;
    virtual unsigned long length(  ) const;
    virtual NodeImpl *getNamedItemNS( const DOMString &namespaceURI, const DOMString &localName,
                                      int &exceptioncode ) const;
    virtual NodeImpl *setNamedItemNS( NodeImpl *arg, int &exceptioncode );
    virtual NodeImpl *removeNamedItemNS( const DOMString &namespaceURI, const DOMString &localName,
                                         int &exceptioncode );

    // Other methods (not part of DOM)

    virtual bool isReadOnly() { return true; }

    void addNode(NodeImpl *n);

protected:
    QPtrList<NodeImpl> *m_contents;
};

}; //namespace
#endif
