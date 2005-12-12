/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 *           (C) 2003 Apple Computer, Inc.
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
 */

#include "dom/dom_exception.h"
#include "misc/htmlattrs.h"
#include "misc/htmltags.h"
#include "xml/dom_elementimpl.h"
#include "xml/dom_textimpl.h"
#include "xml/dom2_eventsimpl.h"
#include "xml/dom_docimpl.h"
#include "xml/dom_nodeimpl.h"

#include <kglobal.h>
#include <kdebug.h>

#include "rendering/render_text.h"
#include "rendering/render_flow.h"
#include "rendering/render_line.h"

#include "ecma/kjs_proxy.h"
#include "khtmlview.h"
#include "khtml_part.h"
#include "dom_nodeimpl.h"
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <Q3PtrList>

// from khtml_caret_p.h
namespace khtml {
void /*KDE_NO_EXPORT*/ mapDOMPosToRenderPos(DOM::NodeImpl *node, long offset,
		khtml::RenderObject *&r, long &r_ofs, bool &outside, bool &outsideEnd);
}

using namespace DOM;
using namespace khtml;

NodeImpl::NodeImpl(DocumentPtr *doc)
    : document(doc),
      m_previous(0),
      m_next(0),
      m_render(0),
      m_tabIndex( 0 ),
      m_hasId( false ),
      m_hasStyle( false ),
      m_attached(false),
      m_closed(false),
      m_changed( false ),
      m_hasChangedChild( false ),
      m_inDocument( false ),
      m_hasAnchor( false ),
      m_specified( false ),
      m_focused( false ),
      m_active( false ),
      m_styleElement( false ),
      m_implicit( false ),
      m_rendererNeedsClose( false ),
      m_htmlCompat( false )
{
    if (document)
        document->ref();
}

NodeImpl::~NodeImpl()
{
    if (m_render)
        detach();
    if (document)
        document->deref();
    if (m_previous)
        m_previous->setNextSibling(0);
    if (m_next)
        m_next->setPreviousSibling(0);
}

DOMString NodeImpl::nodeValue() const
{
  return DOMString();
}

void NodeImpl::setNodeValue( const DOMString &/*_nodeValue*/, int &/*exceptioncode*/ )
{
    // by default nodeValue is null, so setting it has no effect
    // don't throw NO_MODIFICATION_ALLOWED_ERR from here, DOMTS-Core-Level1's hc_nodevalue03
    // (createEntityReference().setNodeValue())) says it would be wrong.
    // This must be done by subclasses instead.
}

DOMString NodeImpl::nodeName() const
{
  return DOMString();
}

unsigned short NodeImpl::nodeType() const
{
  return 0;
}

NodeListImpl *NodeImpl::childNodes()
{
  return new ChildNodeListImpl(this);
}

NodeImpl *NodeImpl::firstChild() const
{
  return 0;
}

NodeImpl *NodeImpl::lastChild() const
{
  return 0;
}

NodeImpl *NodeImpl::insertBefore( NodeImpl *, NodeImpl *, int &exceptioncode )
{
    exceptioncode = DOMException::HIERARCHY_REQUEST_ERR;
    return 0;
}

NodeImpl *NodeImpl::replaceChild( NodeImpl *, NodeImpl *, int &exceptioncode )
{
  exceptioncode = DOMException::HIERARCHY_REQUEST_ERR;
  return 0;
}

NodeImpl *NodeImpl::removeChild( NodeImpl *, int &exceptioncode )
{
  exceptioncode = DOMException::NOT_FOUND_ERR;
  return 0;
}

NodeImpl *NodeImpl::appendChild( NodeImpl *, int &exceptioncode )
{
  exceptioncode = DOMException::HIERARCHY_REQUEST_ERR;
  return 0;
}

bool NodeImpl::hasChildNodes(  ) const
{
  return false;
}

void NodeImpl::normalize ()
{
    // ### normalize attributes? (when we store attributes using child nodes)
    int exceptioncode = 0;
    NodeImpl *child = firstChild();

    // Recursively go through the subtree beneath us, normalizing all nodes. In the case
    // where there are two adjacent text nodes, they are merged together
    while (child) {
        NodeImpl *nextChild = child->nextSibling();

        if (nextChild && child->nodeType() == Node::TEXT_NODE && nextChild->nodeType() == Node::TEXT_NODE) {
            // Current child and the next one are both text nodes... merge them
            TextImpl *currentText = static_cast<TextImpl*>(child);
            TextImpl *nextText = static_cast<TextImpl*>(nextChild);

            currentText->appendData(nextText->data(),exceptioncode);
            if (exceptioncode)
                return;

            removeChild(nextChild,exceptioncode);
            if (exceptioncode)
                return;
        }
        else {
            child->normalize();
            child = nextChild;
        }
    }
}

DOMString NodeImpl::prefix() const
{
    // For nodes other than elements and attributes, the prefix is always null
    return DOMString();
}

DOMString NodeImpl::namespaceURI() const
{
    return DOMString();
}

void NodeImpl::setPrefix(const DOMString &/*_prefix*/, int &exceptioncode )
{
    // The spec says that for nodes other than elements and attributes, prefix is always null.
    // It does not say what to do when the user tries to set the prefix on another type of
    // node, however mozilla throws a NAMESPACE_ERR exception
    exceptioncode = DOMException::NAMESPACE_ERR;
}

DOMString NodeImpl::localName() const
{
    return DOMString();
}

void NodeImpl::setFirstChild(NodeImpl *)
{
}

void NodeImpl::setLastChild(NodeImpl *)
{
}

NodeImpl *NodeImpl::addChild(NodeImpl *)
{
  return 0;
}

void NodeImpl::getCaret(int offset, bool override, int &_x, int &_y, int &width, int &height)
{
    if (m_render) {
        RenderObject *r;
	long r_ofs;
	bool outside, outsideEnd;
#if 0
kdDebug(6200) << "getCaret: node " << this << " " << nodeName().string() << " offset: " << offset << endl;
#endif
        mapDOMPosToRenderPos(this, offset, r, r_ofs, outside, outsideEnd);
#if 0
kdDebug(6200) << "getCaret: r " << r << " " << (r?r->renderName():QString::null) << " r_ofs: " << r_ofs << " outside " << outside << " outsideEnd " << outsideEnd << endl;
#endif
	if (r) {
            r->caretPos(r_ofs, override*RenderObject::CFOverride
		    + outside*RenderObject::CFOutside
		    + outsideEnd*RenderObject::CFOutsideEnd, _x, _y, width, height);
	} else
	    _x = _y = height = -1, width = 1;
    } else _x = _y = height = -1, width = 1;
}

QRect NodeImpl::getRect() const
{
    int _x, _y;
    if(m_render && m_render->absolutePosition(_x, _y))
        return QRect( _x + m_render->inlineXPos(), _y + m_render->inlineYPos(),
        		m_render->width(), m_render->height() );

    return QRect();
}

void NodeImpl::setChanged(bool b)
{
    if (b && !attached()) // changed compared to what?
        return;

    m_changed = b;
    if ( b ) {
	NodeImpl *p = parentNode();
	while ( p ) {
	    p->setHasChangedChild( true );
	    p = p->parentNode();
	}
        getDocument()->setDocumentChanged();
    }
}

bool NodeImpl::isInline() const
{
    if (m_render) return m_render->style()->display() == khtml::INLINE;
    return !isElementNode();
}


unsigned long NodeImpl::nodeIndex() const
{
    NodeImpl *_tempNode = previousSibling();
    unsigned long count=0;
    for( count=0; _tempNode; count++ )
        _tempNode = _tempNode->previousSibling();
    return count;
}

void NodeImpl::addEventListener(int id, EventListener *listener, const bool useCapture)
{
    switch (id) {
	case EventImpl::DOMSUBTREEMODIFIED_EVENT:
	    getDocument()->addListenerType(DocumentImpl::DOMSUBTREEMODIFIED_LISTENER);
	    break;
	case EventImpl::DOMNODEINSERTED_EVENT:
	    getDocument()->addListenerType(DocumentImpl::DOMNODEINSERTED_LISTENER);
	    break;
	case EventImpl::DOMNODEREMOVED_EVENT:
	    getDocument()->addListenerType(DocumentImpl::DOMNODEREMOVED_LISTENER);
	    break;
        case EventImpl::DOMNODEREMOVEDFROMDOCUMENT_EVENT:
	    getDocument()->addListenerType(DocumentImpl::DOMNODEREMOVEDFROMDOCUMENT_LISTENER);
	    break;
        case EventImpl::DOMNODEINSERTEDINTODOCUMENT_EVENT:
	    getDocument()->addListenerType(DocumentImpl::DOMNODEINSERTEDINTODOCUMENT_LISTENER);
	    break;
        case EventImpl::DOMATTRMODIFIED_EVENT:
	    getDocument()->addListenerType(DocumentImpl::DOMATTRMODIFIED_LISTENER);
	    break;
        case EventImpl::DOMCHARACTERDATAMODIFIED_EVENT:
	    getDocument()->addListenerType(DocumentImpl::DOMCHARACTERDATAMODIFIED_LISTENER);
	    break;
	default:
	    break;
    }

    m_regdListeners.addEventListener(id, listener, useCapture);
}

void NodeImpl::removeEventListener(int id, EventListener *listener, bool useCapture)
{
    m_regdListeners.removeEventListener(id, listener, useCapture);
}

void NodeImpl::setHTMLEventListener(int id, EventListener *listener)
{
    m_regdListeners.setHTMLEventListener(id, listener);
}

EventListener *NodeImpl::getHTMLEventListener(int id)
{
    return m_regdListeners.getHTMLEventListener(id);
}

void NodeImpl::dispatchEvent(EventImpl *evt, int &exceptioncode, bool tempEvent)
{
    evt->setTarget(this);

    // Since event handling code could cause this object to be deleted, grab a reference to the view now
    KHTMLView *view = document->document()->view();

    dispatchGenericEvent( evt, exceptioncode );

    // If tempEvent is true, this means that the DOM implementation will not be storing a reference to the event, i.e.
    // there is no way to retrieve it from javascript if a script does not already have a reference to it in a variable.
    // So there is no need for the interpreter to keep the event in its cache
    if (tempEvent && view && view->part() && view->part()->jScript())
        view->part()->jScript()->finishedWithEvent(evt);
}

void NodeImpl::dispatchGenericEvent( EventImpl *evt, int &/*exceptioncode */)
{
    // ### check that type specified

    // work out what nodes to send event to
    Q3PtrList<NodeImpl> nodeChain;
    NodeImpl *n;
    for (n = this; n; n = n->parentNode()) {
        n->ref();
        nodeChain.prepend(n);
    }

    // trigger any capturing event handlers on our way down
    evt->setEventPhase(Event::CAPTURING_PHASE);
    Q3PtrListIterator<NodeImpl> it(nodeChain);
    for (; it.current() && it.current() != this && !evt->propagationStopped(); ++it) {
        evt->setCurrentTarget(it.current());
        it.current()->handleLocalEvents(evt,true);
    }

    // dispatch to the actual target node
    it.toLast();
    NodeImpl* propagationSentinel = 0;
    if (!evt->propagationStopped()) {
        evt->setEventPhase(Event::AT_TARGET);
        evt->setCurrentTarget(it.current());
        it.current()->handleLocalEvents(evt, true);
        if (!evt->propagationStopped())
            it.current()->handleLocalEvents(evt,false);
        else
            propagationSentinel = it.current();
    }
    --it;

    if (evt->bubbles()) {
        evt->setEventPhase(Event::BUBBLING_PHASE);
        for (; it.current() && !evt->propagationStopped(); --it) {
            if (evt->propagationStopped()) propagationSentinel = it.current();
            evt->setCurrentTarget(it.current());
            it.current()->handleLocalEvents(evt,false);
        }

        // now we call all default event handlers (this is not part of DOM - it is internal to khtml)
        evt->setCurrentTarget(0);
        evt->setEventPhase(0); // I guess this is correct, the spec does not seem to say
        for (it.toLast(); it.current() && it.current() != propagationSentinel &&
                 !evt->defaultPrevented() && !evt->defaultHandled(); --it)
            it.current()->defaultEventHandler(evt);

        if (evt->id() == EventImpl::CLICK_EVENT && !evt->defaultPrevented() &&
             static_cast<MouseEventImpl*>( evt )->button() == 0) // LMB click
            dispatchUIEvent(EventImpl::DOMACTIVATE_EVENT, static_cast<UIEventImpl*>(evt)->detail());
    }

    // copy this over into a local variable, as the following deref() calls might cause this to be deleted.
    DocumentPtr *doc = document;
    doc->ref();

    // deref all nodes in chain
    it.toFirst();
    for (; it.current(); ++it)
        it.current()->deref(); // this may delete us

    DocumentImpl::updateDocumentsRendering();
    doc->deref();
}

bool NodeImpl::dispatchHTMLEvent(int _id, bool canBubbleArg, bool cancelableArg)
{
    int exceptioncode = 0;
    EventImpl* const evt = new EventImpl(static_cast<EventImpl::EventId>(_id),canBubbleArg,cancelableArg);
    evt->ref();
    dispatchEvent(evt,exceptioncode,true);
    bool ret = !evt->defaultPrevented();
    evt->deref();
    return ret;
}

void NodeImpl::dispatchWindowEvent(int _id, bool canBubbleArg, bool cancelableArg)
{
    int exceptioncode = 0;
    EventImpl* const evt = new EventImpl(static_cast<EventImpl::EventId>(_id),canBubbleArg,cancelableArg);
    evt->setTarget( 0 );
    evt->ref();
    DocumentPtr *doc = document;
    doc->ref();
    dispatchGenericEvent( evt, exceptioncode );
    if (!evt->defaultPrevented() && doc->document())
	doc->document()->defaultEventHandler(evt);

    if (_id == EventImpl::LOAD_EVENT && !evt->propagationStopped() && doc->document()) {
        // For onload events, send them to the enclosing frame only.
        // This is a DOM extension and is independent of bubbling/capturing rules of
        // the DOM.  You send the event only to the enclosing frame.  It does not
        // bubble through the parent document.
        DOM::ElementImpl* elt = doc->document()->ownerElement();
        if (elt && (elt->getDocument()->domain().isNull() ||
                    elt->getDocument()->domain() == doc->document()->domain())) {
            // We also do a security check, since we don't want to allow the enclosing
            // iframe to see loads of child documents in other domains.
            evt->setCurrentTarget(elt);

            // Capturing first.
            elt->handleLocalEvents(evt,true);

            // Bubbling second.
            if (!evt->propagationStopped())
                elt->handleLocalEvents(evt,false);
        }
    }

    doc->deref();
    evt->deref();
}

void NodeImpl::dispatchMouseEvent(QMouseEvent *_mouse, int overrideId, int overrideDetail)
{
    bool cancelable = true;
    int detail = overrideDetail; // defaults to 0
    EventImpl::EventId evtId = EventImpl::UNKNOWN_EVENT;
    if (overrideId) {
        evtId = static_cast<EventImpl::EventId>(overrideId);
    }
    else {
        switch (_mouse->type()) {
            case QEvent::MouseButtonPress:
                evtId = EventImpl::MOUSEDOWN_EVENT;
                break;
            case QEvent::MouseButtonRelease:
                evtId = EventImpl::MOUSEUP_EVENT;
                break;
            case QEvent::MouseButtonDblClick:
                evtId = EventImpl::CLICK_EVENT;
                detail = 1; // ### support for multiple double clicks
                break;
            case QEvent::MouseMove:
                evtId = EventImpl::MOUSEMOVE_EVENT;
                cancelable = false;
                break;
            default:
                break;
        }
    }
    if (evtId == EventImpl::UNKNOWN_EVENT)
        return; // shouldn't happen


    int exceptioncode = 0;
    int pageX = _mouse->x();
    int pageY = _mouse->y();
    int clientX = pageX;
    int clientY = pageY;
    if ( getDocument()->view() )
        getDocument()->view()->viewportToContents( clientX, clientY, pageX, pageY );

    int screenX = _mouse->globalX();
    int screenY = _mouse->globalY();

    int button = -1;
    switch (_mouse->button()) {
        case Qt::LeftButton:
            button = 0;
            break;
        case Qt::MidButton:
            button = 1;
            break;
        case Qt::RightButton:
            button = 2;
            break;
        default:
            break;
    }
    bool ctrlKey = (_mouse->state() & Qt::ControlModifier);
    bool altKey = (_mouse->state() & Qt::AltModifier);
    bool shiftKey = (_mouse->state() & Qt::ShiftModifier);
    bool metaKey = false; // ### qt support?

    EventImpl* const evt = new MouseEventImpl(evtId,true,cancelable,getDocument()->defaultView(),
                   detail,screenX,screenY,clientX,clientY,pageX,pageY,ctrlKey,altKey,shiftKey,metaKey,
                   button,0);
    evt->ref();
    dispatchEvent(evt,exceptioncode,true);
    evt->deref();
}

void NodeImpl::dispatchUIEvent(int _id, int detail)
{
    assert (!( (_id != EventImpl::DOMFOCUSIN_EVENT &&
        _id != EventImpl::DOMFOCUSOUT_EVENT &&
                _id != EventImpl::DOMACTIVATE_EVENT)));

    bool cancelable = false;
    if (_id == EventImpl::DOMACTIVATE_EVENT)
        cancelable = true;

    int exceptioncode = 0;
    UIEventImpl* const evt = new UIEventImpl(static_cast<EventImpl::EventId>(_id),true,
                                       cancelable,getDocument()->defaultView(),detail);
    evt->ref();
    dispatchEvent(evt,exceptioncode,true);
    evt->deref();
}

void NodeImpl::dispatchSubtreeModifiedEvent()
{
    childrenChanged();
    getDocument()->incDOMTreeVersion();
    if (!getDocument()->hasListenerType(DocumentImpl::DOMSUBTREEMODIFIED_LISTENER))
        return;
    int exceptioncode = 0;
    MutationEventImpl* const evt = new MutationEventImpl(EventImpl::DOMSUBTREEMODIFIED_EVENT,true,
                                                         false,0,DOMString(),DOMString(),DOMString(),0);
    evt->ref();
    dispatchEvent(evt,exceptioncode,true);
    evt->deref();
}

bool NodeImpl::dispatchKeyEvent(QKeyEvent *key, bool keypress)
{
    int exceptioncode = 0;
    //kdDebug(6010) << "DOM::NodeImpl: dispatching keyboard event" << endl;
    TextEventImpl* const keyEventImpl = new TextEventImpl(key, keypress, getDocument()->defaultView());
    keyEventImpl->ref();
    dispatchEvent(keyEventImpl,exceptioncode,true);
    bool r = keyEventImpl->defaultHandled() || keyEventImpl->defaultPrevented();
    keyEventImpl->deref();
    return r;
}

void NodeImpl::handleLocalEvents(EventImpl *evt, bool useCapture)
{
    if (!m_regdListeners.listeners)
        return;

    Event ev = evt;
    // removeEventListener (e.g. called from a JS event listener) might
    // invalidate the item after the current iterator (which "it" is pointing to).
    // So we make a copy of the list.
    Q3ValueList<RegisteredEventListener> listeners = *m_regdListeners.listeners;
    Q3ValueList<RegisteredEventListener>::iterator it;
    for (it = listeners.begin(); it != listeners.end(); ++it) {
        //Check whether this got removed...KDE4: use Java-style iterators
        if (!m_regdListeners.stillContainsListener(*it))
            continue;

        RegisteredEventListener& current = (*it);
        if (current.id == evt->id() && current.useCapture == useCapture)
            current.listener->handleEvent(ev);

        // ECMA legacy hack
        if (current.useCapture == useCapture && evt->id() == EventImpl::CLICK_EVENT) {
            MouseEventImpl* me = static_cast<MouseEventImpl*>(evt);
            if (me->button() == 0) {
                // To find whether to call onclick or ondblclick, we can't
                // * use me->detail(), it's 2 when clicking twice w/o moving, even very slowly
                // * use me->qEvent(), it's not available when using initMouseEvent/dispatchEvent
                // So we currently store a bool in MouseEventImpl. If anyone needs to trigger
                // dblclicks from the DOM API, we'll need a timer here (well in the doc).
                if ( ( !me->isDoubleClick() && current.id == EventImpl::KHTML_ECMA_CLICK_EVENT) ||
                  ( me->isDoubleClick() && current.id == EventImpl::KHTML_ECMA_DBLCLICK_EVENT) )
                    current.listener->handleEvent(ev);
            }
        }
    }
}

void NodeImpl::defaultEventHandler(EventImpl *)
{
}

unsigned long NodeImpl::childNodeCount()
{
    return 0;
}

NodeImpl *NodeImpl::childNode(unsigned long /*index*/)
{
    return 0;
}

NodeImpl *NodeImpl::traverseNextNode(NodeImpl *stayWithin) const
{
    if (firstChild() || stayWithin == this)
	return firstChild();
    else if (nextSibling())
	return nextSibling();
    else {
	const NodeImpl *n = this;
	while (n && !n->nextSibling() && (!stayWithin || n->parentNode() != stayWithin))
	    n = n->parentNode();
	if (n)
	    return n->nextSibling();
    }
    return 0;
}

NodeImpl *NodeImpl::traversePreviousNode() const
{
    if (previousSibling()) {
        NodeImpl *n = previousSibling();
        while (n->lastChild())
            n = n->lastChild();
        return n;
    }
    else if (parentNode()) {
        return parentNode();
    }
    else {
        return 0;
    }
}

void NodeImpl::checkSetPrefix(const DOMString &_prefix, int &exceptioncode)
{
    // Perform error checking as required by spec for setting Node.prefix. Used by
    // ElementImpl::setPrefix() and AttrImpl::setPrefix()

    // INVALID_CHARACTER_ERR: Raised if the specified prefix contains an illegal character.
    if (!Element::khtmlValidPrefix(_prefix)) {
        exceptioncode = DOMException::INVALID_CHARACTER_ERR;
        return;
    }

    // NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return;
    }

    // NAMESPACE_ERR: - Raised if the specified prefix is malformed
    // - if the namespaceURI of this node is null,
    // - if the specified prefix is "xml" and the namespaceURI of this node is different from
    //   "http://www.w3.org/XML/1998/namespace",
    // - if this node is an attribute and the specified prefix is "xmlns" and
    //   the namespaceURI of this node is different from "http://www.w3.org/2000/xmlns/",
    // - or if this node is an attribute and the qualifiedName of this node is "xmlns" [Namespaces].
    if (Element::khtmlMalformedPrefix(_prefix) || (!(id() & NodeImpl_IdNSMask) && id() > ID_LAST_TAG) ||
        (_prefix == "xml" && namespaceURI() != "http://www.w3.org/XML/1998/namespace")) {
        exceptioncode = DOMException::NAMESPACE_ERR;
        return;
    }
}

void NodeImpl::checkAddChild(NodeImpl *newChild, int &exceptioncode)
{
    // Perform error checking as required by spec for adding a new child. Used by
    // appendChild(), replaceChild() and insertBefore()

    // Not mentioned in spec: throw NOT_FOUND_ERR if newChild is null
    if (!newChild) {
        exceptioncode = DOMException::NOT_FOUND_ERR;
        return;
    }

    // NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return;
    }

    // WRONG_DOCUMENT_ERR: Raised if newChild was created from a different document than the one that
    // created this node.
    // We assume that if newChild is a DocumentFragment, all children are created from the same document
    // as the fragment itself (otherwise they could not have been added as children)
    if (newChild->getDocument() != getDocument()) {
        exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
        return;
    }

    // HIERARCHY_REQUEST_ERR: Raised if this node is of a type that does not allow children of the type of the
    // newChild node, or if the node to append is one of this node's ancestors.

    // check for ancestor/same node
    if (isAncestor(newChild)) {
        exceptioncode = DOMException::HIERARCHY_REQUEST_ERR;
        return;
    }

    // check node allowed
    if (newChild->nodeType() == Node::DOCUMENT_FRAGMENT_NODE) {
        // newChild is a DocumentFragment... check all its children instead of newChild itself
        NodeImpl *child;
        for (child = newChild->firstChild(); child; child = child->nextSibling()) {
            if (!childAllowed(child)) {
                exceptioncode = DOMException::HIERARCHY_REQUEST_ERR;
                return;
            }
        }
    }
    else {
        // newChild is not a DocumentFragment... check if it's allowed directly
        if(!childAllowed(newChild)) {
            exceptioncode = DOMException::HIERARCHY_REQUEST_ERR;
            return;
        }
    }
}

bool NodeImpl::isAncestor( NodeImpl *other )
{
    // Return true if other is the same as this node or an ancestor of it, otherwise false
    NodeImpl *n;
    for (n = this; n; n = n->parentNode()) {
        if (n == other)
            return true;
    }
    return false;
}

bool NodeImpl::childAllowed( NodeImpl *newChild )
{
    return childTypeAllowed(newChild->nodeType());
}

NodeImpl::StyleChange NodeImpl::diff( khtml::RenderStyle *s1, khtml::RenderStyle *s2 ) const
{
    StyleChange ch = NoInherit;
    if ( !s1 || !s2 )
	ch = Inherit;
    else if ( *s1 == *s2 )
 	ch = NoChange;
    else if ( s1->inheritedNotEqual( s2 ) )
	ch = Inherit;
    return ch;
}

void NodeImpl::close()
{
    closeRenderer();
    m_closed = true;
}

void NodeImpl::closeRenderer()
{
    // It's important that we close the renderer, even if it hasn't been
    // created yet. This happens even more because of the FOUC fixes we did
    // at Apple, which prevent renderers from being created until the stylesheets
    // are all loaded. If the renderer is not here to be closed, we set a flag,
    // then close it later when it's attached.
    assert(!m_rendererNeedsClose);
    if (m_render)
        m_render->close();
    else
        m_rendererNeedsClose = true;
}

void NodeImpl::attach()
{
    assert(!attached());
    assert(!m_render || (m_render->style() && m_render->parent()));
    if (m_render && m_rendererNeedsClose) {
        m_render->close();
        m_rendererNeedsClose = false;
    }
    getDocument()->incDOMTreeVersion();
    m_attached = true;
}

void NodeImpl::detach()
{
//    assert(m_attached);

    if ( m_render )
        m_render->detach();

    m_render = 0;
    getDocument()->incDOMTreeVersion();
    m_attached = false;
}

bool NodeImpl::maintainsState()
{
    return false;
}

QString NodeImpl::state()
{
    return QString::null;
}

void NodeImpl::restoreState(const QString &/*state*/)
{
}

void NodeImpl::insertedIntoDocument()
{
    setInDocument(true);
}

void NodeImpl::removedFromDocument()
{
    setInDocument(false);
}

void NodeImpl::childrenChanged()
{
    if (parentNode())
        parentNode()->childrenChanged();
}

bool NodeImpl::isReadOnly()
{
    // Entity & Entity Reference nodes and their descendants are read-only
    NodeImpl *n = this;
    while (n) {
	if (n->nodeType() == Node::ENTITY_NODE ||
	    n->nodeType() == Node::ENTITY_REFERENCE_NODE)
	    return true;
	n = n->parentNode();
    }
    return false;
}

RenderObject * NodeImpl::previousRenderer()
{
    for (NodeImpl *n = previousSibling(); n; n = n->previousSibling()) {
        if (n->renderer())
            return n->renderer();
    }
    return 0;
}

RenderObject * NodeImpl::nextRenderer()
{
    for (NodeImpl *n = nextSibling(); n; n = n->nextSibling()) {
        if (n->renderer())
            return n->renderer();
    }
    return 0;
}

void NodeImpl::createRendererIfNeeded()
{
#ifdef APPLE_CHANGES
    if (!getDocument()->shouldCreateRenderers())
        return;
#endif

    assert(!attached());
    assert(!m_render);

    NodeImpl *parent = parentNode();
    assert(parent);

    RenderObject *parentRenderer = parent->renderer();
    if (parentRenderer && parentRenderer->childAllowed()) {
        RenderStyle *style = styleForRenderer(parentRenderer);
        style->ref();
        if (rendererIsNeeded(style)) {
            m_render = createRenderer(getDocument()->renderArena(), style);
            m_render->setStyle(style);
            parentRenderer->addChild(m_render, nextRenderer());
        }
        style->deref();
    }
}

RenderStyle *NodeImpl::styleForRenderer(RenderObject *parent)
{
    return parent->style();
}

bool NodeImpl::rendererIsNeeded(RenderStyle *style)
{
    return (getDocument()->documentElement() == this) || (style->display() != NONE);
}

RenderObject *NodeImpl::createRenderer(RenderArena* /*arena*/, RenderStyle* /*style*/)
{
    assert(false);
    return 0;
}

bool NodeImpl::contentEditable() const
{
    RenderObject *r = renderer();
    if (!r) return false;
    return r->style()->userInput() == UI_ENABLED;
}

long NodeImpl::minOffset() const
{
  // Arrgh! You'd think *every* offset starts at zero, but loo,
  // therefore we need this method
  return renderer() ? renderer()->minOffset() : 0;
}

long NodeImpl::maxOffset() const
{
  return const_cast<NodeImpl *>(this)->childNodeCount();
//  return renderer() ? renderer()->maxOffset() : 1;
}

NodeListImpl* NodeImpl::getElementsByTagName( const DOMString &tagName )
{
    NodeImpl::Id id;
    if ( tagName == "*" )
        id = 0;
    else
        id = getDocument()->getId(NodeImpl::ElementId, tagName.implementation(), false, true);
    return new TagNodeListImpl( this, id );
}

NodeListImpl* NodeImpl::getElementsByTagNameNS( const DOMString &namespaceURI, const DOMString &localName )
{
    return new TagNodeListImpl( this, namespaceURI, localName );
}


bool NodeImpl::hasAttributes() const
{
    return false;
}

bool NodeImpl::isSupported(const DOMString &feature, const DOMString &version)
{
    return DOMImplementationImpl::instance()->hasFeature(feature, version);
}

DocumentImpl* NodeImpl::ownerDocument() const
{
    // braindead DOM spec says that ownerDocument
    // should return null if called on the document node
    // we thus have our nicer getDocument, and hack it here
    // for DOMy clients in one central place
    DocumentImpl* doc = getDocument();
    if (doc == this)
        return 0;
    else
        return doc;
}


//-------------------------------------------------------------------------

NodeBaseImpl::~NodeBaseImpl()
{
    //kdDebug( 6020 ) << "NodeBaseImpl destructor" << endl;
    // we have to tell all children, that the parent has died...
    NodeImpl *n;
    NodeImpl *next;

    for( n = _first; n != 0; n = next ) {
        next = n->nextSibling();
        n->setPreviousSibling(0);
        n->setNextSibling(0);
        n->setParent(0);
	if ( !n->refCount() )
            delete n;
    }
}


NodeImpl *NodeBaseImpl::firstChild() const
{
    return _first;
}

NodeImpl *NodeBaseImpl::lastChild() const
{
    return _last;
}

NodeImpl *NodeBaseImpl::insertBefore ( NodeImpl *newChild, NodeImpl *refChild, int &exceptioncode )
{
    exceptioncode = 0;

    // insertBefore(...,null) is equivalent to appendChild()
    if(!refChild)
        return appendChild(newChild, exceptioncode);

    // Make sure adding the new child is ok
    checkAddChild(newChild, exceptioncode);
    if (exceptioncode)
        return 0;

    // NOT_FOUND_ERR: Raised if refChild is not a child of this node
    if (refChild->parentNode() != this) {
        exceptioncode = DOMException::NOT_FOUND_ERR;
        return 0;
    }

    bool isFragment = newChild->nodeType() == Node::DOCUMENT_FRAGMENT_NODE;

    // If newChild is a DocumentFragment with no children.... there's nothing to do.
    // Just return the document fragment
    if (isFragment && !newChild->firstChild())
        return newChild;

    // Now actually add the child(ren)
    NodeImpl *nextChild;
    NodeImpl *child = isFragment ? newChild->firstChild() : newChild;

    NodeImpl *prev = refChild->previousSibling();
    if ( prev == newChild || refChild == newChild ) // nothing to do
	return newChild;

    while (child) {
        nextChild = isFragment ? child->nextSibling() : 0;

        // If child is already present in the tree, first remove it
        NodeImpl *newParent = child->parentNode();
        if(newParent)
            newParent->removeChild( child, exceptioncode );
        if ( exceptioncode )
            return 0;

        // Add child in the correct position
        if (prev)
            prev->setNextSibling(child);
        else
            _first = child;
        refChild->setPreviousSibling(child);
        child->setParent(this);
        child->setPreviousSibling(prev);
        child->setNextSibling(refChild);

        // Add child to the rendering tree
        // ### should we detach() it first if it's already attached?
        if (attached() && !child->attached())
            child->attach();

        // Dispatch the mutation events
        dispatchChildInsertedEvents(child,exceptioncode);

        prev = child;
        child = nextChild;
    }

    // ### set style in case it's attached
    setChanged(true);
    dispatchSubtreeModifiedEvent();
    return newChild;
}

NodeImpl *NodeBaseImpl::replaceChild ( NodeImpl *newChild, NodeImpl *oldChild, int &exceptioncode )
{
    exceptioncode = 0;

    if ( oldChild == newChild ) // nothing to do
	return oldChild;

    // Make sure adding the new child is ok
    checkAddChild(newChild, exceptioncode);
    if (exceptioncode)
        return 0;

    // NOT_FOUND_ERR: Raised if oldChild is not a child of this node.
    if (!oldChild || oldChild->parentNode() != this) {
        exceptioncode = DOMException::NOT_FOUND_ERR;
        return 0;
    }

    bool isFragment = newChild->nodeType() == Node::DOCUMENT_FRAGMENT_NODE;
    NodeImpl *nextChild;
    NodeImpl *child = isFragment ? newChild->firstChild() : newChild;


    // Remove the old child
    NodeImpl *prev = oldChild->previousSibling();
    NodeImpl *next = oldChild->nextSibling();

    removeChild(oldChild, exceptioncode);
    if (exceptioncode)
        return 0;

    // Add the new child(ren)
    while (child) {
        nextChild = isFragment ? child->nextSibling() : 0;

        // If child is already present in the tree, first remove it
        NodeImpl *newParent = child->parentNode();
	if ( child == next )
	    next = child->nextSibling();
	if ( child == prev )
	    prev = child->previousSibling();
        if(newParent)
            newParent->removeChild( child, exceptioncode );
        if (exceptioncode)
            return 0;

        // Add child in the correct position
        if (prev) prev->setNextSibling(child);
        if (next) next->setPreviousSibling(child);
        if(!prev) _first = child;
        if(!next) _last = child;
        child->setParent(this);
        child->setPreviousSibling(prev);
        child->setNextSibling(next);

        // Add child to the rendering tree
        // ### should we detach() it first if it's already attached?
        if (attached() && !child->attached())
            child->attach();

        // Dispatch the mutation events
        dispatchChildInsertedEvents(child,exceptioncode);

        prev = child;
        child = nextChild;
    }

    // ### set style in case it's attached
    setChanged(true);
    dispatchSubtreeModifiedEvent();
    return oldChild;
}

NodeImpl *NodeBaseImpl::removeChild ( NodeImpl *oldChild, int &exceptioncode )
{
    exceptioncode = 0;

    // NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return 0;
    }

    // NOT_FOUND_ERR: Raised if oldChild is not a child of this node.
    if (!oldChild || oldChild->parentNode() != this) {
        exceptioncode = DOMException::NOT_FOUND_ERR;
        return 0;
    }

    dispatchChildRemovalEvents(oldChild,exceptioncode);
    if (exceptioncode)
        return 0;

    // Remove from rendering tree
    if (oldChild->attached())
        oldChild->detach();

    // Remove the child
    NodeImpl *prev, *next;
    prev = oldChild->previousSibling();
    next = oldChild->nextSibling();

    if(next) next->setPreviousSibling(prev);
    if(prev) prev->setNextSibling(next);
    if(_first == oldChild) _first = next;
    if(_last == oldChild) _last = prev;

    oldChild->setPreviousSibling(0);
    oldChild->setNextSibling(0);
    oldChild->setParent(0);

    setChanged(true);

    // Dispatch post-removal mutation events
    dispatchSubtreeModifiedEvent();

    NodeImpl *p = this;
    while (p->parentNode())
	p = p->parentNode();
    if (p->nodeType() == Node::DOCUMENT_NODE) {
	for (NodeImpl *c = oldChild; c; c = c->traverseNextNode(oldChild))
	    c->removedFromDocument();
    }

    return oldChild;
}

void NodeBaseImpl::removeChildren()
{
    NodeImpl *n, *next;
    for( n = _first, _first = 0; n; n = next )
    {
        next = n->nextSibling();
        if (n->attached())
	    n->detach();
        n->setPreviousSibling(0);
        n->setNextSibling(0);
        n->setParent(0);
        if( !n->refCount() )
            delete n;
        else
            for ( NodeImpl* c = n; c; c = c->traverseNextNode( n ) )
                c->removedFromDocument();
    }
    _last = 0;
}


NodeImpl *NodeBaseImpl::appendChild ( NodeImpl *newChild, int &exceptioncode )
{
    exceptioncode = 0;

    // Make sure adding the new child is ok
    checkAddChild(newChild, exceptioncode);
    if (exceptioncode)
        return 0;

    if ( newChild == _last ) // nothing to do
	return newChild;

    bool isFragment = newChild->nodeType() == Node::DOCUMENT_FRAGMENT_NODE;

    // If newChild is a DocumentFragment with no children.... there's nothing to do.
    // Just return the document fragment
    if (isFragment && !newChild->firstChild())
        return newChild;

    // Now actually add the child(ren)
    NodeImpl *nextChild;
    NodeImpl *child = isFragment ? newChild->firstChild() : newChild;

    while (child) {
        nextChild = isFragment ? child->nextSibling() : 0;

        // If child is already present in the tree, first remove it
        NodeImpl *oldParent = child->parentNode();
        if(oldParent) {
            oldParent->removeChild( child, exceptioncode );
            if (exceptioncode)
                return 0;
        }

        // Append child to the end of the list
        child->setParent(this);

        if(_last)
        {
            child->setPreviousSibling(_last);
            _last->setNextSibling(child);
            _last = child;
        }
        else
        {
            _first = _last = child;
        }

        // Add child to the rendering tree
        // ### should we detach() it first if it's already attached?
        if (attached() && !child->attached())
            child->attach();

        // Dispatch the mutation events
        dispatchChildInsertedEvents(child,exceptioncode);

        child = nextChild;
    }

    setChanged(true);
    // ### set style in case it's attached
    dispatchSubtreeModifiedEvent();
    return newChild;
}

bool NodeBaseImpl::hasChildNodes (  ) const
{
    return _first != 0;
}

// not part of the DOM
void NodeBaseImpl::setFirstChild(NodeImpl *child)
{
    _first = child;
}

void NodeBaseImpl::setLastChild(NodeImpl *child)
{
    _last = child;
}

// check for same source document:
bool NodeBaseImpl::checkSameDocument( NodeImpl *newChild, int &exceptioncode )
{
    exceptioncode = 0;
    DocumentImpl *ownerDocThis = getDocument();
    DocumentImpl *ownerDocNew = getDocument();
    if(ownerDocThis != ownerDocNew) {
        kdDebug(6010)<< "not same document, newChild = " << newChild << "document = " << getDocument() << endl;
        exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
        return true;
    }
    return false;
}

// check for being child:
bool NodeBaseImpl::checkIsChild( NodeImpl *oldChild, int &exceptioncode )
{
    if(!oldChild || oldChild->parentNode() != this) {
        exceptioncode = DOMException::NOT_FOUND_ERR;
        return true;
    }
    return false;
}

NodeImpl *NodeBaseImpl::addChild(NodeImpl *newChild)
{
    // do not add applyChanges here! This function is only used during parsing

    // short check for consistency with DTD
    if(!isXMLElementNode() && !newChild->isXMLElementNode() && !childAllowed(newChild))
    {
        //kdDebug( 6020 ) << "AddChild failed! id=" << id() << ", child->id=" << newChild->id() << endl;
        return 0;
    }

    // just add it...
    newChild->setParent(this);

    if(_last)
    {
        newChild->setPreviousSibling(_last);
        _last->setNextSibling(newChild);
        _last = newChild;
    }
    else
    {
        _first = _last = newChild;
    }

    newChild->insertedIntoDocument();
    childrenChanged();

    if(newChild->nodeType() == Node::ELEMENT_NODE)
        return newChild;
    return this;
}

void NodeBaseImpl::attach()
{
    NodeImpl *child = _first;
    while(child != 0)
    {
        child->attach();
        child = child->nextSibling();
    }
    NodeImpl::attach();
}

void NodeBaseImpl::detach()
{
    NodeImpl *child = _first;
    while(child != 0)
    {
        NodeImpl* prev = child;
        child = child->nextSibling();
        prev->detach();
    }
    NodeImpl::detach();
}

void NodeBaseImpl::cloneChildNodes(NodeImpl *clone)
{
    int exceptioncode = 0;
    NodeImpl *n;
    for(n = firstChild(); n && !exceptioncode; n = n->nextSibling())
    {
        clone->appendChild(n->cloneNode(true),exceptioncode);
    }
}

// I don't like this way of implementing the method, but I didn't find any
// other way. Lars
bool NodeBaseImpl::getUpperLeftCorner(int &xPos, int &yPos) const
{
    if (!m_render)
        return false;
    RenderObject *o = m_render;
    xPos = yPos = 0;
    if ( !o->isInline() || o->isReplaced() ) {
        o->absolutePosition( xPos, yPos );
        return true;
    }

    // find the next text/image child, to get a position
    while(o) {
        if(o->firstChild())
            o = o->firstChild();
        else if(o->nextSibling())
            o = o->nextSibling();
        else {
            RenderObject *next = 0;
            while(!next) {
                o = o->parent();
                if(!o) return false;
                next = o->nextSibling();
            }
            o = next;
        }
        if((o->isText() && !o->isBR()) || o->isReplaced()) {
            o->container()->absolutePosition( xPos, yPos );
            if (o->isText()) {
                xPos += o->inlineXPos();
                yPos += o->inlineYPos();
            } else {
                xPos += o->xPos();
                yPos += o->yPos();
            }
            return true;
        }
    }
    return true;
}

bool NodeBaseImpl::getLowerRightCorner(int &xPos, int &yPos) const
{
    if (!m_render)
        return false;

    RenderObject *o = m_render;
    xPos = yPos = 0;
    if (!o->isInline() || o->isReplaced())
    {
        o->absolutePosition( xPos, yPos );
        xPos += o->width();
        yPos += o->height();
        return true;
    }
    // find the last text/image child, to get a position
    while(o) {
        if(o->lastChild())
            o = o->lastChild();
        else if(o->previousSibling())
            o = o->previousSibling();
        else {
            RenderObject *prev = 0;
            while(!prev) {
                o = o->parent();
                if(!o) return false;
                prev = o->previousSibling();
            }
            o = prev;
        }
        if((o->isText() && !o->isBR()) || o->isReplaced()) {
            o->container()->absolutePosition(xPos, yPos);
            if (o->isText()) {
                xPos += o->inlineXPos() + o->width();
                yPos += o->inlineYPos() + o->height();
            } else {
                xPos += o->xPos() + o->width();
                yPos += o->yPos() + o->height();
            }
            return true;
        }
    }
    return true;
}

void NodeBaseImpl::setFocus(bool received)
{
    if (m_focused == received) return;

    NodeImpl::setFocus(received);

    // note that we need to recalc the style
    setChanged();
}

void NodeBaseImpl::setActive(bool down)
{
    if (down == active()) return;

    NodeImpl::setActive(down);

    // note that we need to recalc the style
    if (m_render && m_render->style()->affectedByActiveRules())
        setChanged();
}

unsigned long NodeBaseImpl::childNodeCount()
{
    unsigned long count = 0;
    NodeImpl *n;
    for (n = firstChild(); n; n = n->nextSibling())
        count++;
    return count;
}

NodeImpl *NodeBaseImpl::childNode(unsigned long index)
{
    unsigned long i;
    NodeImpl *n = firstChild();
    for (i = 0; n && i < index; i++)
        n = n->nextSibling();
    return n;
}

void NodeBaseImpl::dispatchChildInsertedEvents( NodeImpl *child, int &exceptioncode )
{
    if (getDocument()->hasListenerType(DocumentImpl::DOMNODEINSERTED_LISTENER)) {
        MutationEventImpl* const evt = new MutationEventImpl(EventImpl::DOMNODEINSERTED_EVENT,true,false,this,DOMString(),DOMString(),DOMString(),0);
        evt->ref();
        child->dispatchEvent(evt,exceptioncode,true);
        evt->deref();
        if (exceptioncode)
            return;
    }

    // dispatch the DOMNodeInsertedIntoDocument event to all descendants
    bool hasInsertedListeners = getDocument()->hasListenerType(DocumentImpl::DOMNODEINSERTEDINTODOCUMENT_LISTENER);
    NodeImpl *p = this;
    while (p->parentNode())
        p = p->parentNode();
    if (p->nodeType() == Node::DOCUMENT_NODE) {
        for (NodeImpl *c = child; c; c = c->traverseNextNode(child)) {
            c->insertedIntoDocument();

            if (hasInsertedListeners) {
                MutationEventImpl* const evt = new MutationEventImpl(EventImpl::DOMNODEINSERTEDINTODOCUMENT_EVENT,false,false,0,DOMString(),DOMString(),DOMString(),0);
                evt->ref();
                c->dispatchEvent(evt,exceptioncode,true);
                evt->deref();
                if (exceptioncode)
                    return;
            }
        }
    }
}

void NodeBaseImpl::dispatchChildRemovalEvents( NodeImpl *child, int &exceptioncode )
{
    // Dispatch pre-removal mutation events
    getDocument()->notifyBeforeNodeRemoval(child); // ### use events instead
    if (getDocument()->hasListenerType(DocumentImpl::DOMNODEREMOVED_LISTENER)) {
        MutationEventImpl* const evt = new MutationEventImpl(EventImpl::DOMNODEREMOVED_EVENT,true,false,this,DOMString(),DOMString(),DOMString(),0);
        evt->ref();
        child->dispatchEvent(evt,exceptioncode,true);
        evt->deref();
        if (exceptioncode)
            return;
    }

    bool hasRemovalListeners = getDocument()->hasListenerType(DocumentImpl::DOMNODEREMOVEDFROMDOCUMENT_LISTENER);

    // dispatch the DOMNodeRemovedFromDocument event to all descendants
    NodeImpl *p = this;
    while (p->parentNode())
        p = p->parentNode();
    if (p->nodeType() == Node::DOCUMENT_NODE) {
        for (NodeImpl *c = child; c; c = c->traverseNextNode(child)) {
            if (hasRemovalListeners) {
                MutationEventImpl* const evt = new MutationEventImpl(EventImpl::DOMNODEREMOVEDFROMDOCUMENT_EVENT,false,false,0,DOMString(),DOMString(),DOMString(),0);
                evt->ref();
                c->dispatchEvent(evt,exceptioncode,true);
                evt->deref();
                if (exceptioncode)
                    return;
            }
        }
    }
}

// ---------------------------------------------------------------------------
NodeImpl *NodeListImpl::item( unsigned long index ) const
{
    unsigned long requestIndex = index;

    m_cache->updateNodeListInfo(m_refNode->getDocument());

    NodeImpl* n;
    bool usedCache = false;
    if (m_cache->current.node) {
        //Compute distance from the requested index to the cache node
        unsigned long cacheDist = QABS(long(index) - long(m_cache->position));

        if (cacheDist < index) { //Closer to the cached position
            usedCache = true;
            if (index >= m_cache->position) { //Go ahead
                unsigned long relIndex = index - m_cache->position;
                n = recursiveItem(m_refNode, m_cache->current.node, relIndex);
            } else { //Go backwards
                unsigned long relIndex = m_cache->position - index;
                n = recursiveItemBack(m_refNode, m_cache->current.node, relIndex);
            }
        }
    }

    if (!usedCache)
        n = recursiveItem(m_refNode, m_refNode->firstChild(), index);

    //We always update the cache state, to make starting iteration
    //where it was left off easy.
    m_cache->current.node  = n;
    m_cache->position = requestIndex;
    return n;
}

unsigned long NodeListImpl::length() const
{
    m_cache->updateNodeListInfo(m_refNode->getDocument());
    if (!m_cache->hasLength) {
        m_cache->length    = recursiveLength( m_refNode );
        m_cache->hasLength = true;
    }
    return m_cache->length;
}

unsigned long NodeListImpl::recursiveLength(NodeImpl *start) const
{
    unsigned long len = 0;
    for(NodeImpl *n = start->firstChild(); n != 0; n = n->nextSibling()) {
        bool recurse = true;
        if (nodeMatches(n, recurse))
                len++;
        if (recurse)
            len+= recursiveLength(n);
        }

    return len;
}

NodeListImpl::NodeListImpl( NodeImpl *n, int type, CacheFactory* factory )
{
    m_refNode = n;
    m_refNode->ref();

    m_cache = m_refNode->getDocument()->acquireCachedNodeListInfo(
                  factory ? factory : Cache::make,
                  n, type );
}

NodeListImpl::~NodeListImpl()
{
    m_refNode->getDocument()->releaseCachedNodeListInfo(m_cache);
    m_refNode->deref();
}


/**
 Next item in the pre-order walk of tree from node, but not going outside 
 absStart
*/
static NodeImpl* helperNext(NodeImpl* node, NodeImpl* absStart)
{
    //Walk up until we wind a sibling to go to.
    while (!node->nextSibling() && node != absStart)
        node = node->parentNode();

    if (node != absStart)
        return node->nextSibling();
    else
        return 0;
}

NodeImpl *NodeListImpl::recursiveItem ( NodeImpl* absStart, NodeImpl *start, unsigned long &offset ) const
{
    for(NodeImpl *n = start; n != 0; n = helperNext(n, absStart)) {
        bool recurse = true;
        if (nodeMatches(n, recurse))
            if (!offset--)
                return n;

        NodeImpl *depthSearch = recurse ? recursiveItem(n, n->firstChild(), offset) : 0;
        if (depthSearch)
            return depthSearch;
    }

    return 0; // no matching node in this subtree
}


NodeImpl *NodeListImpl::recursiveItemBack ( NodeImpl* absStart, NodeImpl *start, unsigned long &offset ) const
{
    //### it might be cleaner/faster to split nodeMatches and recursion
    //filtering.
    bool dummy   = true;
    NodeImpl* n  = start;

    do {
        bool recurse = true;

        //Check whether the current node matches.
        if (nodeMatches(n, dummy))
            if (!offset--)
                return n;

        if (n->previousSibling()) {
            //Move to the last node of this whole subtree that we should recurse into
            n       = n->previousSibling();
            recurse = true;

            while (n->lastChild()) {
                (void)nodeMatches(n, recurse);
                if (!recurse)
                   break; //Don't go there
                n = n->lastChild();
            }
        } else {
            //We're done with this whole subtree, so move up
            n = n->parentNode();
        }
    }
    while (n && n != absStart);

    return 0;
}


NodeListImpl::Cache::~Cache()
{}

void NodeListImpl::Cache::clear(DocumentImpl* doc)
{
   hasLength = false;
   current.node = 0;
   version   = doc->domTreeVersion();
}

void NodeListImpl::Cache::updateNodeListInfo(DocumentImpl* doc)
{
    //If version doesn't match, clear
    if (doc->domTreeVersion() != version)
        clear(doc);
}

ChildNodeListImpl::ChildNodeListImpl( NodeImpl *n ): NodeListImpl(n, CHILD_NODES)
{}

bool ChildNodeListImpl::nodeMatches( NodeImpl* /*testNode*/, bool& doRecurse ) const
{
    doRecurse = false;
    return true;
}

TagNodeListImpl::TagNodeListImpl( NodeImpl *n, NodeImpl::Id id )
  : NodeListImpl(n, UNCACHEABLE),
    m_id(id),
    m_namespaceAware(false)
{
    // An id of 0 here means "*" (match all nodes)
    m_matchAllNames = (id == 0);
    m_matchAllNamespaces = false;
}

TagNodeListImpl::TagNodeListImpl( NodeImpl *n, const DOMString &namespaceURI, const DOMString &localName )
  : NodeListImpl(n, UNCACHEABLE),
    m_id(0),
    m_namespaceURI(namespaceURI),
    m_localName(localName),
    m_namespaceAware(true)
{
    m_matchAllNames = (localName == "*");
    m_matchAllNamespaces = (namespaceURI == "*");
}


bool TagNodeListImpl::nodeMatches( NodeImpl *testNode, bool& /*doRecurse*/ ) const
{
    if ( testNode->nodeType() != Node::ELEMENT_NODE ) return false;
    if (m_namespaceAware)
	return (m_matchAllNamespaces || testNode->namespaceURI() == m_namespaceURI) &&
	       (m_matchAllNames || testNode->localName() == m_localName);
    else {
        NodeImpl::Id testId = testNode->id();
        //we have to strip the namespaces if we compare in a namespace unaware fashion
        if ( !m_namespaceAware )
            testId &= ~NodeImpl_IdNSMask;
	return (m_id == 0 || m_id == testId);
    }
}

NameNodeListImpl::NameNodeListImpl(NodeImpl *n, const DOMString &t )
  : NodeListImpl(n, UNCACHEABLE),
    nodeName(t)
{}

bool NameNodeListImpl::nodeMatches( NodeImpl *testNode, bool& /*doRecurse*/ ) const
{
    if ( testNode->nodeType() != Node::ELEMENT_NODE ) return false;
    return static_cast<ElementImpl *>(testNode)->getAttribute(ATTR_NAME) == nodeName;
}

NamedTagNodeListImpl::NamedTagNodeListImpl( NodeImpl *n, NodeImpl::Id tagId, const DOMString& name )
    : TagNodeListImpl( n, tagId ), nodeName( name )
{
}

bool NamedTagNodeListImpl::nodeMatches( NodeImpl *testNode, bool& doRecurse ) const
{
    if ( testNode->nodeType() != Node::ELEMENT_NODE ) return false;
    return TagNodeListImpl::nodeMatches( testNode, doRecurse )
        && static_cast<ElementImpl *>(testNode)->getAttribute(ATTR_NAME) == nodeName;
}


// ---------------------------------------------------------------------------

NamedNodeMapImpl::NamedNodeMapImpl()
{
}

NamedNodeMapImpl::~NamedNodeMapImpl()
{
}

NodeImpl* NamedNodeMapImpl::getNamedItem( const DOMString &name )
{
    NodeImpl::Id nid = mapId(0, name.implementation(), true);
    if (!nid) return 0;
    return getNamedItem(nid, false, name.implementation());
}

Node NamedNodeMapImpl::setNamedItem( const Node &arg, int& exceptioncode )
{
    if (!arg.handle()) {
      exceptioncode = DOMException::NOT_FOUND_ERR;
      return 0;
    }
    
    Node r = setNamedItem(arg.handle(), false,
                       arg.handle()->nodeName().implementation(), exceptioncode);
    return r;
}

Node NamedNodeMapImpl::removeNamedItem( const DOMString &name, int& exceptioncode )
{
    Node r = removeNamedItem(mapId(0, name.implementation(), false),
                                   false, name.implementation(), exceptioncode);
    return r;
}

Node NamedNodeMapImpl::getNamedItemNS( const DOMString &namespaceURI, const DOMString &localName )
{
    NodeImpl::Id nid = mapId( namespaceURI.implementation(), localName.implementation(), true );
    return getNamedItem(nid, true);
}

Node NamedNodeMapImpl::setNamedItemNS( const Node &arg, int& exceptioncode )
{
    Node r = setNamedItem(arg.handle(), true, 0, exceptioncode);
    return r;
}

Node NamedNodeMapImpl::removeNamedItemNS( const DOMString &namespaceURI, const DOMString &localName, int& exceptioncode )
{
    NodeImpl::Id nid = mapId( namespaceURI.implementation(), localName.implementation(), false );
    Node r = removeNamedItem(nid, true, 0, exceptioncode);
    return r;
}


// ----------------------------------------------------------------------------

GenericRONamedNodeMapImpl::GenericRONamedNodeMapImpl(DocumentPtr* doc)
    : NamedNodeMapImpl()
{
    m_doc = doc->document();
    m_contents = new Q3PtrList<NodeImpl>;
}

GenericRONamedNodeMapImpl::~GenericRONamedNodeMapImpl()
{
    while (!m_contents->isEmpty())
        m_contents->take(0)->deref();

    delete m_contents;
}

NodeImpl *GenericRONamedNodeMapImpl::getNamedItem ( NodeImpl::Id id, bool /*nsAware*/, DOMStringImpl* /*qName*/ ) const
{
    // ## do we need namespace support in this class?
    Q3PtrListIterator<NodeImpl> it(*m_contents);
    for (; it.current(); ++it)
        if (it.current()->id() == id)
            return it.current();
    return 0;
}

Node GenericRONamedNodeMapImpl::setNamedItem ( NodeImpl* /*arg*/, bool /*nsAware*/, DOMStringImpl* /*qName*/, int &exceptioncode )
{
    // can't modify this list through standard DOM functions
    // NO_MODIFICATION_ALLOWED_ERR: Raised if this map is readonly
    exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
    return 0;
}

Node GenericRONamedNodeMapImpl::removeNamedItem ( NodeImpl::Id /*id*/, bool /*nsAware*/, DOMStringImpl* /*qName*/, int &exceptioncode )
{
    // can't modify this list through standard DOM functions
    // NO_MODIFICATION_ALLOWED_ERR: Raised if this map is readonly
    exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
    return 0;
}

NodeImpl *GenericRONamedNodeMapImpl::item ( unsigned long index ) const
{
    if (index >= m_contents->count())
        return 0;

    return m_contents->at(index);
}

unsigned long GenericRONamedNodeMapImpl::length(  ) const
{
    return m_contents->count();
}

void GenericRONamedNodeMapImpl::addNode(NodeImpl *n)
{
    // The spec says that in the case of duplicates we only keep the first one
    if (getNamedItem(n->id(), false, 0))
        return;

    n->ref();
    m_contents->append(n);
}

NodeImpl::Id GenericRONamedNodeMapImpl::mapId(DOMStringImpl* namespaceURI,
                                              DOMStringImpl* localName, bool readonly)
{
    if (!m_doc)
	return 0;

    return m_doc->getId(NodeImpl::ElementId,
                        namespaceURI, 0, localName, readonly,
                        false /*don't lookupHTML*/);
}

// -----------------------------------------------------------------------------

void RegisteredListenerList::addEventListener(int id, EventListener *listener, const bool useCapture)
{
    RegisteredEventListener rl(static_cast<EventImpl::EventId>(id),listener,useCapture);
    if (!listeners)
        listeners = new Q3ValueList<RegisteredEventListener>;

    // if this id/listener/useCapture combination is already registered, do nothing.
    // the DOM2 spec says that "duplicate instances are discarded", and this keeps
    // the listener order intact.
    Q3ValueList<RegisteredEventListener>::iterator it;
    for (it = listeners->begin(); it != listeners->end(); ++it)
        if (*it == rl)
            return;

    listeners->append(rl);
}

void RegisteredListenerList::removeEventListener(int id, EventListener *listener, bool useCapture)
{
    if (!listeners) // nothing to remove
        return;

    RegisteredEventListener rl(static_cast<EventImpl::EventId>(id),listener,useCapture);

    Q3ValueList<RegisteredEventListener>::iterator it;
    for (it = listeners->begin(); it != listeners->end(); ++it)
        if (*it == rl) {
            listeners->remove(it);
            return;
        }
}

bool RegisteredListenerList::isHTMLEventListener(EventListener* listener)
{
    return (listener->eventListenerType() == "_khtml_HTMLEventListener");
}

void RegisteredListenerList::setHTMLEventListener(int id, EventListener *listener)
{
    if (!listeners)
        listeners = new Q3ValueList<RegisteredEventListener>;

    Q3ValueList<RegisteredEventListener>::iterator it;
    if (!listener) {
        for (it = listeners->begin(); it != listeners->end(); ++it) {
            if ((*it).id == id && isHTMLEventListener((*it).listener)) {
                listeners->remove(it);
                break;
            }
        }
        return;
    }

    // if this event already has a registered handler, insert the new one in
    // place of the old one, to preserve the order.
    RegisteredEventListener rl(static_cast<EventImpl::EventId>(id),listener,false);

    int i;
    for (i = 0, it = listeners->begin(); it != listeners->end(); ++it, ++i)
        if ((*it).id == id && isHTMLEventListener((*it).listener)) {
            listeners->insert(it, rl);
            listeners->remove(it);
            return;
        }

    listeners->append(rl);
}

EventListener *RegisteredListenerList::getHTMLEventListener(int id)
{
    if (!listeners)
        return 0;

    Q3ValueList<RegisteredEventListener>::iterator it;
    for (it = listeners->begin(); it != listeners->end(); ++it)
        if ((*it).id == id && isHTMLEventListener((*it).listener)) {
            return (*it).listener;
        }
    return 0;
}

bool RegisteredListenerList::hasEventListener(int id)
{
    if (!listeners)
        return false;

    Q3ValueList<RegisteredEventListener>::iterator it;
    for (it = listeners->begin(); it != listeners->end(); ++it)
        if ((*it).id == id)
            return true;

    return false;
}

void RegisteredListenerList::clear()
{
    delete listeners;
    listeners = 0;
}

bool RegisteredListenerList::stillContainsListener(const RegisteredEventListener& listener)
{
    if (!listeners)
        return false;
    return listeners->find(listener) != listeners->end();
}

RegisteredListenerList::~RegisteredListenerList() {
    delete listeners; listeners = 0;
}
