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
#ifndef _DOM_NodeImpl_h_
#define _DOM_NodeImpl_h_

#include "dom/dom_misc.h"
#include "dom/dom_string.h"

class QPainter;
class KHTMLView;
class QRect;

namespace khtml {
    class RenderStyle;
    class RenderObject;
};

namespace DOM {

class NodeListImpl;
class NamedNodeMapImpl;
class DocumentImpl;
class CSSStyleDeclarationImpl;

//enumerator for the keyboardfocus-state of an object.
enum ActivationState { ActivationOff,
		       ActivationOffbutVisited,
		       ActivationPassive,
		       ActivationActive };

// Skeleton of a node. No children and no parents are allowed.
// We use this class as a basic Node Implementation, and derive all other
// Node classes from it. This is done to reduce memory overhead.
// Derived classes will only implement the functionality needed, and only
// use as much storage as they really need; i.e. if a node has no children
// or even no parent; it does not need to store null pointers.
// (Such nodes could be attributes or readonly nodes at the end of the
// tree)
class NodeImpl : public DomShared
{
public:
    NodeImpl(DocumentImpl *doc);
    virtual ~NodeImpl();

    virtual const DOMString nodeName() const;

    virtual DOMString nodeValue() const;

    virtual void setNodeValue( const DOMString & );

    virtual unsigned short nodeType() const;

    virtual bool isElementNode() const { return false; }
    virtual bool isAttributeNode() const { return false; }
    virtual bool isTextNode() const { return false; }
    virtual bool isDocumentNode() const { return false; }

    virtual NodeImpl *parentNode() const;

    virtual NodeListImpl *childNodes();

    virtual NodeImpl *firstChild() const;

    virtual NodeImpl *lastChild() const;

    virtual NodeImpl *previousSibling() const;

    virtual NodeImpl *nextSibling() const;

    virtual NamedNodeMapImpl *attributes() const;

    DocumentImpl *ownerDocument() const
	{ return document; }

    virtual NodeImpl *insertBefore ( NodeImpl *newChild, NodeImpl *refChild );

    virtual NodeImpl *replaceChild ( NodeImpl *newChild, NodeImpl *oldChild );

    virtual NodeImpl *removeChild ( NodeImpl *oldChild );

    virtual NodeImpl *appendChild ( NodeImpl *newChild );

    virtual bool hasChildNodes (  );

    virtual NodeImpl *cloneNode ( bool deep );

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

    virtual unsigned short id() const { return 0; };

    enum MouseEventType {
	MousePress,
	MouseRelease,
	MouseClick,
	MouseDblClick,
	MouseMove
    };
    /*
     * generic handler for mouse events. goes through the doucment
     * tree and triggers the corresponding events for all elements
     * where the mouse is inside.
     *
     * @param x,y is the mouse position
     * @param _tx, _ty are helper variables needed (set to 0 if you call
     *  this in the body element
     * @param url returns the url under the mouse, or an empty string otherwise
     */
    virtual bool mouseEvent( int /*x*/, int /*y*/, int /*button*/,
			     MouseEventType /*type*/, int /*_tx*/, int /*_ty*/,
			     DOMString &/*url*/,
                             NodeImpl *&/*innerNode*/, long &/*offset*/) { return false; }

    virtual void setStyle(khtml::RenderStyle *) {}
    virtual khtml::RenderStyle *style() { return 0; }

    virtual void setRenderer(khtml::RenderObject *object) { m_render = object; }
    virtual khtml::RenderObject *renderer() { return m_render; }

    virtual DOM::CSSStyleDeclarationImpl *styleRules() { return 0; }

    // for LINK and STYLE
    virtual void sheetLoaded() {}

    bool hasEvents() const  { return m_hasEvents; }
    bool hasID() const      { return m_hasId; }
    bool hasClass() const   { return m_hasClass; }
    bool hasStyle() const   { return m_hasStyle; }
    bool hasTooltip() const { return m_hasTooltip; }
    bool pressed() const    { return m_pressed; }
    bool mouseInside() const { return m_mouseInside; }
    bool attached() const   { return m_attached; }
    bool changed() const    { return m_changed; }
    bool complexText() const { return m_complexText; }
    void setComplexText(bool b=true) { m_complexText = b; }
    void setHasEvents(bool b=true) { m_hasEvents = b; }
    void setHasID(bool b=true) { m_hasId = b; }
    void setHasClass(bool b=true) { m_hasClass = b; }
    void setHasStyle(bool b=true) { m_hasStyle = b; }
    void setHasTooltip(bool b=true) { m_hasTooltip = b; }
    void setPressed(bool b=true) { m_pressed = b; }
    void setMouseInside(bool b=true) { m_mouseInside = b; }
    void setAttached(bool b=true) { m_attached = b; }
    virtual void setChanged(bool b=true); 

    /**
     * attach to a KHTMLView. Additional info (like style information,
     * frames, etc...) will only get loaded, when attached to a widget.
     */
    virtual void attach(KHTMLView *) { setAttached(true); }
    /**
     * detach from a HTMLWidget
     */
    virtual void detach() { setAttached(false); }

    // ### check if this function is still needed at all...
    virtual bool isInline() { return true; }

    virtual QString toHTML();
    virtual void recursive( QChar *&htmlText, long &currentLength, long &offset, int stdInc );
    virtual bool increaseStringLength( QChar *&htmlText, long &currentLength, long offset, int stdInc );

    virtual void applyChanges(bool top = true, bool force = true);
    virtual void getCursor(int offset, int &_x, int &_y, int &height);
    virtual QRect getRect();

    void setKeyboardFocus(ActivationState b=ActivationPassive);
    virtual void recalcStyle() {}

protected:
    DocumentImpl *document;
    khtml::RenderObject *m_render;
    bool m_complexText : 1;
    bool m_hasEvents : 1;
    bool m_hasId : 1;
    bool m_hasClass : 1;
    bool m_hasStyle : 1;
    bool m_hasTooltip : 1;
    bool m_pressed : 1;
    bool m_mouseInside : 1;
    bool m_attached : 1;
    bool m_changed : 1;
    bool m_specified : 1; // used in AttrImpl. Accessor functions there
private:
    static const QChar LESSTHAN;
    static const QChar MORETHAN;
    static const QChar SLASH;
    static const QChar SPACE;
    static const QChar EQUALS;
    static const QChar QUOTE;
};

// this class implements nodes, which can have a parent but no children:
class NodeWParentImpl : public NodeImpl
{
public:
    NodeWParentImpl(DocumentImpl *doc);

    virtual ~NodeWParentImpl();

    virtual NodeImpl *parentNode() const;

    virtual NodeImpl *previousSibling() const;

    virtual NodeImpl *nextSibling() const;

    virtual NodeImpl *cloneNode ( bool deep );

    // helper functions not being part of the DOM
    virtual void setParent(NodeImpl *parent);
    virtual bool deleteMe();

    virtual void setPreviousSibling(NodeImpl *);
    virtual void setNextSibling(NodeImpl *);

protected:
    NodeImpl *_parent;
    NodeImpl *_previous;
    NodeImpl *_next;

    // helper function; throws exception if modifying a readonly node
    void checkReadOnly();
};

// this is the full Node Implementation with parents and children.
class NodeBaseImpl : public NodeWParentImpl
{
public:
    NodeBaseImpl(DocumentImpl *doc);

    virtual ~NodeBaseImpl();

    virtual NodeImpl *firstChild() const;

    virtual NodeImpl *lastChild() const;

    virtual NodeImpl *insertBefore ( NodeImpl *newChild, NodeImpl *refChild );

    virtual NodeImpl *replaceChild ( NodeImpl *newChild, NodeImpl *oldChild );

    virtual NodeImpl *removeChild ( NodeImpl *oldChild );

    virtual NodeImpl *appendChild ( NodeImpl *newChild );

    virtual bool hasChildNodes (  );

    virtual NodeImpl *cloneNode ( bool deep );

    // not part of the DOM
    virtual void setFirstChild(NodeImpl *child);
    virtual void setLastChild(NodeImpl *child);
    virtual NodeImpl *addChild(NodeImpl *newChild);
    virtual void attach(KHTMLView *w);
    virtual void detach();

protected:
    NodeImpl *_first;
    NodeImpl *_last;

    // helper functions for inserting children:

    // check for same source document:
    void checkSameDocument( NodeImpl *newchild );
    // check for being (grand-..)father:
    void checkNoOwner( NodeImpl *other );
    // check for being child:
    void checkIsChild( NodeImpl *oldchild );
    // find out if a node is allowed to be our child
    virtual bool childAllowed( NodeImpl *newChild );
};

// --------------------------------------------------------------------------
class Node;
class NodeImpl;

class NodeListImpl : public DomShared
{
public:
    virtual unsigned long length() const;

    virtual NodeImpl *item ( unsigned long index ) const;

protected:
    // helper functions for searching all ElementImpls in a tree
    unsigned long recursiveLength(NodeImpl *start) const;

    NodeImpl *recursiveItem ( NodeImpl *start, unsigned long &offset ) const;

    virtual bool nodeMatches( NodeImpl *testNode ) const;
};

class ChildNodeListImpl : public NodeListImpl
{
public:
    ChildNodeListImpl( NodeImpl *n);

    virtual ~ChildNodeListImpl();

    virtual unsigned long length() const;

    virtual NodeImpl *item ( unsigned long index ) const;

protected:
    NodeImpl *refNode;
};


/**
 * NodeList which lists all Nodes in a document with a given tag name
 */
class TagNodeListImpl : public NodeListImpl
{
public:
    TagNodeListImpl( NodeImpl *n, const DOMString &t );

    virtual ~TagNodeListImpl();

    virtual unsigned long length() const;

    virtual NodeImpl *item ( unsigned long index ) const;

protected:
    virtual bool nodeMatches( NodeImpl *testNode ) const;

    NodeImpl *refNode;
    DOMString tagName;
    bool allElements;
};


/**
 * NodeList which lists all Nodes in a document with a given "name=" tag
 */
class NameNodeListImpl : public NodeListImpl
{
public:
    NameNodeListImpl( DocumentImpl *doc, const DOMString &t );

    virtual ~NameNodeListImpl();

    virtual unsigned long length() const;

    virtual NodeImpl *item ( unsigned long index ) const;

protected:
    virtual bool nodeMatches( NodeImpl *testNode ) const;

    DocumentImpl *refDoc;
    DOMString nodeName;
};

class NamedNodeMapImpl : public DomShared
{
public:
    NamedNodeMapImpl();
    virtual ~NamedNodeMapImpl();

    virtual unsigned long length() const;

    virtual NodeImpl *getNamedItem ( const DOMString &name ) const;

    virtual NodeImpl *setNamedItem ( const Node &arg );

    virtual NodeImpl *removeNamedItem ( const DOMString &name );

    virtual NodeImpl *item ( unsigned long index ) const;
};


}; //namespace
#endif
