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
#include "misc/shared.h"

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

class DocumentPtr : public khtml::Shared<DocumentPtr>
{
public:
    DocumentImpl *document() const { return doc; }
private:
    DocumentPtr() { doc = 0; }
    friend class DocumentImpl;
    friend class DOMImplementationImpl;

    DocumentImpl *doc;
};

// this class implements nodes, which can have a parent but no children:
class NodeImpl : public khtml::TreeShared<NodeImpl>
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
    NodeImpl *parentNode() const { return m_parent; }
    NodeImpl *previousSibling() const { return m_previous; }
    NodeImpl *nextSibling() const { return m_next; }
    virtual NodeListImpl *childNodes();
    virtual NodeImpl *firstChild() const;
    virtual NodeImpl *lastChild() const;
    virtual NodeImpl *insertBefore ( NodeImpl *newChild, NodeImpl *refChild, int &exceptioncode );
    virtual NodeImpl *replaceChild ( NodeImpl *newChild, NodeImpl *oldChild, int &exceptioncode );
    virtual NodeImpl *removeChild ( NodeImpl *oldChild, int &exceptioncode );
    virtual NodeImpl *appendChild ( NodeImpl *newChild, int &exceptioncode );
    virtual bool hasChildNodes (  ) const;
    virtual NodeImpl *cloneNode ( bool deep ) = 0;
    virtual DOMString localName() const;
    virtual DOMString prefix() const;
    virtual void setPrefix(const DOMString &_prefix, int &exceptioncode );
    void normalize ();

    // Other methods (not part of DOM)
    virtual bool isElementNode() const { return false; }
    virtual bool isHTMLElement() const { return false; }
    virtual bool isAttributeNode() const { return false; }
    virtual bool isTextNode() const { return false; }
    virtual bool isDocumentNode() const { return false; }
    virtual bool isXMLElementNode() const { return false; }

    // helper functions not being part of the DOM
    // Attention: they assume that the caller did the consistency checking!
    void setPreviousSibling(NodeImpl *previous) { m_previous = previous; }
    void setNextSibling(NodeImpl *next) { m_next = next; }

    virtual void setFirstChild(NodeImpl *child);
    virtual void setLastChild(NodeImpl *child);

    // used by the parser. Doesn't do as many error checkings as
    // appendChild(), and returns the node into which will be parsed next.
    virtual NodeImpl *addChild(NodeImpl *newChild);

    typedef Q_UINT32 Id;
    static const Q_UINT32 IdNSMask;
    static const Q_UINT32 IdLocalMask;
    // id() is used to easily and exactly identify a node. It
    // is optimized for quick comparison and low memory consumption.
    // its value depends on the owner document of the node and is
    // categorized in the following way:
    // 1..ID_LAST_TAG: the node inherits HTMLElementImpl and is
    //                 part of the HTML namespace.
    //                 The HTML namespace is either the global
    //                 one (no namespace) or the XHTML namespace
    //                 depending on the owner document's doctype
    // ID_LAST_TAG+1..0xffff: non-HTML elements in the global namespace
    // others       non-HTML elements in a namespace.
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
                    const DOMString &_url = DOMString(), const DOMString& _target = DOMString(),
                    NodeImpl *_innerNode = 0)
            {
                button = _button; type = _type;
                url = _url; target = _target;
                innerNode = _innerNode;
            }

        int button;
        MouseEventType type;
        DOMString url; // url under mouse or empty
        DOMString target;
        Node innerNode;
    };

    // for LINK and STYLE
    virtual void sheetLoaded() {}

    bool hasID() const      { return m_hasId; }
    bool hasClass() const   { return m_hasClass; }
    bool hasStyle() const   { return m_hasStyle; }
    bool pressed() const    { return m_pressed; }
    bool active() const     { return m_active; }
    bool focused() const { return m_focused; }
    bool attached() const   { return m_attached; }
    bool changed() const    { return m_changed; }
    bool hasChangedChild() const { return m_hasChangedChild; }
    bool hasAnchor() const { return m_hasAnchor; }
    bool inDocument() const { return m_inDocument; }
    bool styleElement() const { return m_styleElement; }
    void setHasID(bool b=true) { m_hasId = b; }
    void setHasClass(bool b=true) { m_hasClass = b; }
    void setHasStyle(bool b=true) { m_hasStyle = b; }
    void setPressed(bool b=true) { m_pressed = b; }
    void setHasChangedChild( bool b = true ) { m_hasChangedChild = b; }
    void setInDocument(bool b=true) { m_inDocument = b; }
    virtual void setFocus(bool b=true) { m_focused = b; }
    virtual void setActive(bool b=true) { m_active = b; }
    virtual void setChanged(bool b=true);

    unsigned short tabIndex() const { return m_tabIndex; }
    void setTabIndex(unsigned short _tabIndex) { m_tabIndex = _tabIndex; }

    /**
     * whether this node can receive the keyboard focus.
     */
    virtual bool isSelectable() const { return false; };

    bool isInline() const;
    virtual QString toHTML() const;
    QString recursive_toHTML(bool start = false) const;

    virtual void getCursor(int offset, int &_x, int &_y, int &height);
    virtual QRect getRect() const;

    enum StyleChange { NoChange, NoInherit, Inherit, Force };
    virtual void recalcStyle( StyleChange = NoChange ) {}
    StyleChange diff( khtml::RenderStyle *s1, khtml::RenderStyle *s2 ) const;

    unsigned long nodeIndex() const;
    // Returns the document that this node is associated with. This is guaranteed to always be non-null, as opposed to
    // DOM's ownerDocument() which is null for Document nodes (and sometimes DocumentType nodes).
    DocumentImpl* getDocument() const { return document->document(); }

    void addEventListener(int id, EventListener *listener, const bool useCapture);
    void removeEventListener(int id, EventListener *listener, bool useCapture);
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

    virtual bool isReadOnly();
    virtual bool childTypeAllowed( unsigned short /*type*/ ) { return false; }
    virtual unsigned long childNodeCount();
    virtual NodeImpl *childNode(unsigned long index);

    /**
     * Does a pre-order traversal of the tree to find the node next node after this one. This uses the same order that
     * the tags appear in the source file.
     *
     * @param stayWithin If not null, the traversal will stop once the specified node is reached. This can be used to
     * restrict traversal to a particular sub-tree.
     *
     * @return The next node, in document order
     *
     * see @ref traversePreviousNode()
     */
    NodeImpl *traverseNextNode(NodeImpl *stayWithin = 0) const;

    /**
     * Does a reverse pre-order traversal to find the node that comes before the current one in document order
     *
     * see @ref traverseNextNode()
     */
    NodeImpl *traversePreviousNode() const;

    DocumentPtr *docPtr() const { return document; }

    khtml::RenderObject *renderer() const { return m_render; }
    khtml::RenderObject *nextRenderer();

    void checkSetPrefix(const DOMString &_prefix, int &exceptioncode);
    void checkAddChild(NodeImpl *newChild, int &exceptioncode);
    bool isAncestor( NodeImpl *other );
    virtual bool childAllowed( NodeImpl *newChild );
#ifndef NDEBUG
    virtual void dump(QTextStream *stream, QString ind = "") const;
#endif

    /**
     * Performs any necessary initialization of the element based on attributes that have been set during parsing. This
     * method is called after the parser has finished parsing the element and inserted it into the tree, but before it
     * is attached to the rendering tree.
     *
     * This method will eventually be removed once all of the initialization that currently only works during parsing is
     * fixed to work with dynamic changes.
     */
    virtual void init();

    // -----------------------------------------------------------------------------
    // Integration with rendering tree

    /**
     * Attaches this node to the rendering tree. This calculates the style to be applied to the node and creates an
     * appropriate RenderObject which will be inserted into the tree (except when the style has display: none). This
     * makes the node visible in the KHTMLView.
     */
    virtual void attach();

    /**
     * Detaches the node from the rendering tree, making it invisible in the rendered view. This method will remove
     * the node's rendering object from the rendering tree and delete it.
     */
    virtual void detach();

    // -----------------------------------------------------------------------------
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

    // -----------------------------------------------------------------------------
    // Notification of document stucture changes

    /**
     * Notifies the node that it has been inserted into the document. This is called during document parsing, and also
     * when a node is added through the DOM methods insertBefore(), appendChild() or replaceChild(). Note that this only
     * happens when the node becomes part of the document tree, i.e. only when the document is actually an ancestor of
     * the node. The call happens _after_ the node has been added to the tree.
     *
     * This is similar to the DOMNodeInsertedIntoDocument DOM event, but does not require the overhead of event
     * dispatching.
     */
    virtual void insertedIntoDocument();

    /**
     * Notifies the node that it is no longer part of the document tree, i.e. when the document is no longer an ancestor
     * node.
     *
     * This is similar to the DOMNodeRemovedFromDocument DOM event, but does not require the overhead of event
     * dispatching, and is called _after_ the node is removed from the tree.
     */
    virtual void removedFromDocument();

    /**
     * Notifies the node that it's list of children have changed (either by adding or removing child nodes), or a child
     * node that is of the type CDATA_SECTION_NODE, TEXT_NODE or COMMENT_NODE has changed it's value.
     */
    virtual void childrenChanged();

private: // members
    DocumentPtr *document;
    NodeImpl *m_previous;
    NodeImpl *m_next;
protected:
    khtml::RenderObject *m_render;
    QPtrList<RegisteredEventListener> *m_regdListeners;

    unsigned short m_tabIndex : 15;
    bool m_hasTabIndex  : 1;

    bool m_hasId : 1;
    bool m_hasClass : 1;
    bool m_hasStyle : 1;
    bool m_pressed : 1;
    bool m_attached : 1;
    bool m_changed : 1;
    bool m_hasChangedChild : 1;
    bool m_inDocument : 1;

    bool m_hasAnchor : 1;
    bool m_specified : 1; // used in AttrImpl. Accessor functions there
    bool m_focused : 1;
    bool m_active : 1;
    bool m_styleElement : 1; // contains stylesheet text

    // 3 bits unused
};

// this is the full Node Implementation with parents and children.
class NodeBaseImpl : public NodeImpl
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
    void cloneChildNodes(NodeImpl *clone);

    virtual void setFirstChild(NodeImpl *child);
    virtual void setLastChild(NodeImpl *child);
    virtual NodeImpl *addChild(NodeImpl *newChild);
    virtual void attach();
    virtual void detach();

    virtual NodeListImpl *getElementsByTagNameNS ( DOMStringImpl* namespaceURI,
                                                   DOMStringImpl* localName );

    virtual QRect getRect() const;
    bool getUpperLeftCorner(int &xPos, int &yPos) const;
    bool getLowerRightCorner(int &xPos, int &yPos) const;

    virtual void setFocus(bool=true);
    virtual void setActive(bool=true);
    virtual unsigned long childNodeCount();
    virtual NodeImpl *childNode(unsigned long index);

    // check for being (grand-..)father:
    bool checkNoOwner( NodeImpl *other, int &exceptioncode );

protected:
    NodeImpl *_first;
    NodeImpl *_last;

    // helper functions for inserting children:

    // ### this should vanish. do it in dom/ !
    // check for same source document:
    bool checkSameDocument( NodeImpl *newchild, int &exceptioncode );
    // check for being child:
    bool checkIsChild( NodeImpl *oldchild, int &exceptioncode );
    // ###

    // find out if a node is allowed to be our child
    void dispatchChildInsertedEvents( NodeImpl *child, int &exceptioncode );
    void dispatchChildRemovalEvents( NodeImpl *child, int &exceptioncode );
};

// --------------------------------------------------------------------------
class Node;
class NodeImpl;

class NodeListImpl : public khtml::Shared<NodeListImpl>
{
public:
    virtual ~NodeListImpl() {}

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
// of an element
class NamedNodeMapImpl : public khtml::Shared<NamedNodeMapImpl>
{
public:
    NamedNodeMapImpl();
    virtual ~NamedNodeMapImpl();

    // DOM methods & attributes for NamedNodeMap
    virtual NodeImpl *getNamedItem ( NodeImpl::Id id ) const = 0;
    virtual Node removeNamedItem ( NodeImpl::Id id, int &exceptioncode ) = 0;
    virtual Node setNamedItem ( NodeImpl* arg, int &exceptioncode ) = 0;

    virtual NodeImpl *item ( unsigned long index ) const = 0;
    virtual unsigned long length(  ) const = 0;

    // Other methods (not part of DOM)
    virtual NodeImpl::Id mapId(const DOMString& namespaceURI,  const DOMString& localName,  bool readonly) = 0;
    virtual bool isReadOnly() { return false; }
};


// ### fixme
#if 0
// Generic read-only NamedNodeMap implementation
// You can add items using the internal function addItem()
class GenericRONamedNodeMapImpl : public NamedNodeMapImpl
{
public:
    GenericRONamedNodeMapImpl(DocumentPtr* doc);
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
    DocumentImpl* m_doc;
    QPtrList<NodeImpl> *m_contents;
};
#endif

}; //namespace
#endif
