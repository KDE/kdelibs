// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003 Apple Computer, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <khtmlview.h>
#include "xml/dom2_eventsimpl.h"
#include "rendering/render_canvas.h"
#include "rendering/render_layer.h"
#include "xml/dom_nodeimpl.h"
#include "xml/dom_docimpl.h"
#include "misc/htmltags.h" // ID_*
#include "misc/htmlattrs.h" // ATTR_*
#include "html/html_baseimpl.h"
#include "html/html_documentimpl.h"
#include "html/html_miscimpl.h"
#include <kdebug.h>
#include <khtml_part.h>
#include <QList>

#include "kjs_dom.h"
#include "kjs_html.h"
#include "kjs_css.h"
#include "kjs_range.h"
#include "kjs_traversal.h"
#include "kjs_events.h"
#include "kjs_views.h"
#include "kjs_window.h"
#include "dom/dom_exception.h"
#include "kjs_dom.lut.h"
#include "khtmlpart_p.h"

using namespace KJS;

// -------------------------------------------------------------------------
/* Source for DOMNodeProtoTable.
@begin DOMNodeProtoTable 13
  insertBefore	DOMNode::InsertBefore	DontDelete|Function 2
  replaceChild	DOMNode::ReplaceChild	DontDelete|Function 2
  removeChild	DOMNode::RemoveChild	DontDelete|Function 1
  appendChild	DOMNode::AppendChild	DontDelete|Function 1
  hasAttributes	DOMNode::HasAttributes	DontDelete|Function 0
  hasChildNodes	DOMNode::HasChildNodes	DontDelete|Function 0
  cloneNode	DOMNode::CloneNode	DontDelete|Function 1
# DOM2
  normalize	DOMNode::Normalize	DontDelete|Function 0
  isSupported   DOMNode::IsSupported	DontDelete|Function 2
# from the EventTarget interface
  addEventListener	DOMNode::AddEventListener	DontDelete|Function 3
  removeEventListener	DOMNode::RemoveEventListener	DontDelete|Function 3
  dispatchEvent		DOMNode::DispatchEvent	DontDelete|Function 1
# IE extensions
  contains	DOMNode::Contains		DontDelete|Function 1
  insertAdjacentHTML	DOMNode::InsertAdjacentHTML	DontDelete|Function 2
# "DOM level 0" (from Gecko DOM reference; also in WinIE)
  item          DOMNode::Item           DontDelete|Function 1
@end
*/
DEFINE_PROTOTYPE("DOMNode",DOMNodeProto)
IMPLEMENT_PROTOFUNC(DOMNodeProtoFunc)
IMPLEMENT_PROTOTYPE(DOMNodeProto,DOMNodeProtoFunc)

const ClassInfo DOMNode::info = { "Node", 0, &DOMNodeTable, 0 };

DOMNode::DOMNode(ExecState *exec, DOM::NodeImpl* n)
  : DOMObject(DOMNodeProto::self(exec)), m_impl(n)
{
}

DOMNode::~DOMNode()
{
  ScriptInterpreter::forgetDOMObject(m_impl.get());
}

bool DOMNode::toBoolean(ExecState *) const
{
    return !m_impl.isNull();
}

/* Source for DOMNodeTable.
@begin DOMNodeTable 53
  nodeName	DOMNode::NodeName	DontDelete|ReadOnly
  nodeValue	DOMNode::NodeValue	DontDelete
  nodeType	DOMNode::NodeType	DontDelete|ReadOnly
  parentNode	DOMNode::ParentNode	DontDelete|ReadOnly
  parentElement	DOMNode::ParentElement	DontDelete|ReadOnly
  childNodes	DOMNode::ChildNodes	DontDelete|ReadOnly
  firstChild	DOMNode::FirstChild	DontDelete|ReadOnly
  lastChild	DOMNode::LastChild	DontDelete|ReadOnly
  previousSibling  DOMNode::PreviousSibling DontDelete|ReadOnly
  nextSibling	DOMNode::NextSibling	DontDelete|ReadOnly
  attributes	DOMNode::Attributes	DontDelete|ReadOnly
  namespaceURI	DOMNode::NamespaceURI	DontDelete|ReadOnly
# DOM2
  prefix	DOMNode::Prefix		DontDelete
  localName	DOMNode::LocalName	DontDelete|ReadOnly
  ownerDocument	DOMNode::OwnerDocument	DontDelete|ReadOnly
# Event handlers
# IE also has: onactivate, onbefore*, oncontextmenu, oncontrolselect, oncut,
# ondeactivate, ondrag*, ondrop, onfocusin, onfocusout, onhelp, onmousewheel,
# onmove*, onpaste, onpropertychange, onreadystatechange, onresizeend/start,
# onselectionchange, onstop
  onabort	DOMNode::OnAbort		DontDelete
  onblur	DOMNode::OnBlur			DontDelete
  onchange	DOMNode::OnChange		DontDelete
  onclick	DOMNode::OnClick		DontDelete
  ondblclick	DOMNode::OnDblClick		DontDelete
  ondragdrop	DOMNode::OnDragDrop		DontDelete
  onerror	DOMNode::OnError		DontDelete
  onfocus	DOMNode::OnFocus       		DontDelete
  onkeydown	DOMNode::OnKeyDown		DontDelete
  onkeypress	DOMNode::OnKeyPress		DontDelete
  onkeyup	DOMNode::OnKeyUp		DontDelete
  onload	DOMNode::OnLoad			DontDelete
  onmousedown	DOMNode::OnMouseDown		DontDelete
  onmousemove	DOMNode::OnMouseMove		DontDelete
  onmouseout	DOMNode::OnMouseOut		DontDelete
  onmouseover	DOMNode::OnMouseOver		DontDelete
  onmouseup	DOMNode::OnMouseUp		DontDelete
  onmove	DOMNode::OnMove			DontDelete
  onreset	DOMNode::OnReset		DontDelete
  onresize	DOMNode::OnResize		DontDelete
  onselect	DOMNode::OnSelect		DontDelete
  onsubmit	DOMNode::OnSubmit		DontDelete
  onunload	DOMNode::OnUnload		DontDelete
# IE extensions
  offsetLeft	DOMNode::OffsetLeft		DontDelete|ReadOnly
  offsetTop	DOMNode::OffsetTop		DontDelete|ReadOnly
  offsetWidth	DOMNode::OffsetWidth		DontDelete|ReadOnly
  offsetHeight	DOMNode::OffsetHeight		DontDelete|ReadOnly
  offsetParent	DOMNode::OffsetParent		DontDelete|ReadOnly
  clientWidth	DOMNode::ClientWidth		DontDelete|ReadOnly
  clientHeight	DOMNode::ClientHeight		DontDelete|ReadOnly
  scrollLeft	DOMNode::ScrollLeft		DontDelete
  scrollTop	DOMNode::ScrollTop		DontDelete
  scrollWidth   DOMNode::ScrollWidth            DontDelete|ReadOnly
  scrollHeight  DOMNode::ScrollHeight           DontDelete|ReadOnly
  sourceIndex	DOMNode::SourceIndex		DontDelete|ReadOnly
@end
*/

bool DOMNode::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMNode::getOwnPropertySlot " << propertyName.qstring() << endl;
#endif
  return getStaticValueSlot<DOMNode, DOMObject>(exec, &DOMNodeTable, this, propertyName, slot);
}

ValueImp* DOMNode::getValueProperty(ExecState *exec, int token) const
{
  NodeImpl& node = *impl();
  switch (token) {
  case NodeName:
    return String(node.nodeName());
  case NodeValue:
    return ::getStringOrNull(node.nodeValue()); // initially null, per domts/level1/core/hc_documentcreateelement.html
  case NodeType:
    return Number((unsigned int)node.nodeType());
  case ParentNode:
    return getDOMNode(exec,node.parentNode());
  case ParentElement: // IE only apparently
    return getDOMNode(exec,node.parentNode());
  case ChildNodes:
    return getDOMNodeList(exec,node.childNodes());
  case FirstChild:
    return getDOMNode(exec,node.firstChild());
  case LastChild:
    return getDOMNode(exec,node.lastChild());
  case PreviousSibling:
    return getDOMNode(exec,node.previousSibling());
  case NextSibling:
    return getDOMNode(exec,node.nextSibling());
  case Attributes: {
    DOM::NamedNodeMapImpl* attrs = 0;
    if (node.isElementNode()) {
      DOM::ElementImpl& el = static_cast<DOM::ElementImpl&>(node);
      attrs = el.attributes();
    }
    return getDOMNamedNodeMap(exec,attrs);
  }
  case NamespaceURI:
    return ::getStringOrNull(node.namespaceURI()); // Moz returns null if not set (dom/namespaces.html)
  case Prefix:
    return ::getStringOrNull(node.prefix());  // Moz returns null if not set (dom/namespaces.html)
  case LocalName:
    return ::getStringOrNull(node.localName());  // Moz returns null if not set (dom/namespaces.html)
  case OwnerDocument:
    return getDOMNode(exec,node.ownerDocument());
  case OnAbort:
    return getListener(DOM::EventImpl::ABORT_EVENT);
  case OnBlur:
    return getListener(DOM::EventImpl::BLUR_EVENT);
  case OnChange:
    return getListener(DOM::EventImpl::CHANGE_EVENT);
  case OnClick:
    return getListener(DOM::EventImpl::KHTML_ECMA_CLICK_EVENT);
  case OnDblClick:
    return getListener(DOM::EventImpl::KHTML_ECMA_DBLCLICK_EVENT);
  case OnDragDrop:
    return getListener(DOM::EventImpl::KHTML_DRAGDROP_EVENT);
  case OnError:
    return getListener(DOM::EventImpl::KHTML_ERROR_EVENT);
  case OnFocus:
    return getListener(DOM::EventImpl::FOCUS_EVENT);
  case OnKeyDown:
    return getListener(DOM::EventImpl::KEYDOWN_EVENT);
  case OnKeyPress:
    return getListener(DOM::EventImpl::KEYPRESS_EVENT);
  case OnKeyUp:
    return getListener(DOM::EventImpl::KEYUP_EVENT);
  case OnLoad:
    return getListener(DOM::EventImpl::LOAD_EVENT);
  case OnMouseDown:
    return getListener(DOM::EventImpl::MOUSEDOWN_EVENT);
  case OnMouseMove:
    return getListener(DOM::EventImpl::MOUSEMOVE_EVENT);
  case OnMouseOut:
    return getListener(DOM::EventImpl::MOUSEOUT_EVENT);
  case OnMouseOver:
    return getListener(DOM::EventImpl::MOUSEOVER_EVENT);
  case OnMouseUp:
    return getListener(DOM::EventImpl::MOUSEUP_EVENT);
  case OnMove:
    return getListener(DOM::EventImpl::KHTML_MOVE_EVENT);
  case OnReset:
    return getListener(DOM::EventImpl::RESET_EVENT);
  case OnResize:
    return getListener(DOM::EventImpl::RESIZE_EVENT);
  case OnSelect:
    return getListener(DOM::EventImpl::SELECT_EVENT);
  case OnSubmit:
    return getListener(DOM::EventImpl::SUBMIT_EVENT);
  case OnUnload:
    return getListener(DOM::EventImpl::UNLOAD_EVENT);
  case SourceIndex: {
    // Retrieves the ordinal position of the object, in source order, as the object
    // appears in the document's all collection
    // i.e. document.all[n.sourceIndex] == n
    DOM::DocumentImpl* doc = node.getDocument();
    if (doc->isHTMLDocument()) {
      HTMLCollectionImpl all(doc, HTMLCollectionImpl::DOC_ALL);
      unsigned long i = 0;
      DOM::NodeImpl* n = all.firstItem();
      for ( ; !n && n != impl(); n = all.nextItem() )
        ++i;
      Q_ASSERT( n ); // node not in document.all !?
      return Number(i);
    }
    return Undefined();
  }
  default:
    // no DOM standard, found in IE only

    // Make sure our layout is up to date before we allow a query on these attributes.
    DOM::DocumentImpl* docimpl = node.getDocument();
    if (docimpl) {
      docimpl->updateLayout();
    }

    khtml::RenderObject *rend = node.renderer();

    switch (token) {
    case OffsetLeft:
      return rend ? Number( rend->offsetLeft() ) : Undefined();
    case OffsetTop:
      return rend ? Number( rend->offsetTop() ) : Undefined();
    case OffsetWidth:
      return rend ? Number( rend->offsetWidth() ) : Undefined();
    case OffsetHeight:
      return rend ? Number( rend->offsetHeight() ) : Undefined();
    case OffsetParent:
    {
      khtml::RenderObject* par = rend ? rend->offsetParent() : 0;
      return getDOMNode( exec, par ? par->element() : 0 );
    }
    case ClientWidth:
      return rend ? Number( rend->clientWidth() ) : Undefined();
    case ClientHeight:
      return rend ? Number( rend->clientHeight() ) : Undefined();
    case ScrollWidth:
      return rend ? Number(rend->scrollWidth()) : Undefined();
    case ScrollHeight:
      return rend ? Number(rend->scrollHeight()) : Undefined();
    case ScrollLeft:
      if (rend && rend->layer()) {
          if (rend->isRoot() && !rend->style()->hidesOverflow())
              return Number( node.getDocument()->view() ? node.getDocument()->view()->contentsX() : 0);
          return Number( rend->layer()->scrollXOffset() );
      }
      return Number( 0 );
    case ScrollTop:
      if (rend && rend->layer()) {
          if (rend->isRoot() && !rend->style()->hidesOverflow())
              return Number( node.getDocument()->view() ? node.getDocument()->view()->contentsY() : 0);
          return Number( rend->layer()->scrollYOffset() );
      }
      return Number( 0 );
    default:
      kdDebug(6070) << "WARNING: Unhandled token in DOMNode::getValueProperty : " << token << endl;
      break;
    }
  }
  return Undefined();
}


void DOMNode::put(ExecState *exec, const Identifier& propertyName, ValueImp* value, int attr)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMNode::tryPut " << propertyName.qstring() << endl;
#endif
  lookupPut<DOMNode,DOMObject>(exec, propertyName, value, attr,
                                        &DOMNodeTable, this );
}

void DOMNode::putValueProperty(ExecState *exec, int token, ValueImp* value, int /*attr*/)
{
  DOMExceptionTranslator exception(exec);
  NodeImpl& node = *impl();
  
  switch (token) {
  case NodeValue:
    node.setNodeValue(value->toString(exec).domString(), exception);
    break;
  case Prefix:
    node.setPrefix(value->toString(exec).domString(), exception);
    break;
  case OnAbort:
    setListener(exec,DOM::EventImpl::ABORT_EVENT,value);
    break;
  case OnBlur:
    setListener(exec,DOM::EventImpl::BLUR_EVENT,value);
    break;
  case OnChange:
    setListener(exec,DOM::EventImpl::CHANGE_EVENT,value);
    break;
  case OnClick:
    setListener(exec,DOM::EventImpl::KHTML_ECMA_CLICK_EVENT,value);
    break;
  case OnDblClick:
    setListener(exec,DOM::EventImpl::KHTML_ECMA_DBLCLICK_EVENT,value);
    break;
  case OnDragDrop:
    setListener(exec,DOM::EventImpl::KHTML_DRAGDROP_EVENT,value);
    break;
  case OnError:
    setListener(exec,DOM::EventImpl::KHTML_ERROR_EVENT,value);
    break;
  case OnFocus:
    setListener(exec,DOM::EventImpl::FOCUS_EVENT,value);
    break;
  case OnKeyDown:
    setListener(exec,DOM::EventImpl::KEYDOWN_EVENT,value);
    break;
  case OnKeyPress:
    setListener(exec,DOM::EventImpl::KEYPRESS_EVENT,value);
    break;
  case OnKeyUp:
    setListener(exec,DOM::EventImpl::KEYUP_EVENT,value);
    break;
  case OnLoad:
    setListener(exec,DOM::EventImpl::LOAD_EVENT,value);
    break;
  case OnMouseDown:
    setListener(exec,DOM::EventImpl::MOUSEDOWN_EVENT,value);
    break;
  case OnMouseMove:
    setListener(exec,DOM::EventImpl::MOUSEMOVE_EVENT,value);
    break;
  case OnMouseOut:
    setListener(exec,DOM::EventImpl::MOUSEOUT_EVENT,value);
    break;
  case OnMouseOver:
    setListener(exec,DOM::EventImpl::MOUSEOVER_EVENT,value);
    break;
  case OnMouseUp:
    setListener(exec,DOM::EventImpl::MOUSEUP_EVENT,value);
    break;
  case OnMove:
    setListener(exec,DOM::EventImpl::KHTML_MOVE_EVENT,value);
    break;
  case OnReset:
    setListener(exec,DOM::EventImpl::RESET_EVENT,value);
    break;
  case OnResize:
    setListener(exec,DOM::EventImpl::RESIZE_EVENT,value);
    break;
  case OnSelect:
    setListener(exec,DOM::EventImpl::SELECT_EVENT,value);
    break;
  case OnSubmit:
    setListener(exec,DOM::EventImpl::SUBMIT_EVENT,value);
    break;
  case OnUnload:
    setListener(exec,DOM::EventImpl::UNLOAD_EVENT,value);
    break;
  case ScrollTop: {
    khtml::RenderObject *rend = node.renderer();
    if (rend && rend->layer()) {
        node.getDocument()->updateLayout();
        if (rend->style()->hidesOverflow())
            rend->layer()->scrollToYOffset(value->toInt32(exec));
        else if (rend->isRoot()) {
            Q3ScrollView* sview = node.getDocument()->view();
            if (sview)
                sview->setContentsPos(sview->contentsX(), value->toInt32(exec));
        }
    }
    break;
  }
  case ScrollLeft: {
    khtml::RenderObject *rend = node.renderer();
    if (rend && rend->layer()) {
        node.getDocument()->updateLayout();
        if (rend->style()->hidesOverflow())
            rend->layer()->scrollToXOffset(value->toInt32(exec));
        else if (rend->isRoot()) {
            Q3ScrollView* sview = node.getDocument()->view();
            if (sview)
                sview->setContentsPos(value->toInt32(exec), sview->contentsY());
        }
    }
    break;
  }
  default:
    kdDebug(6070) << "WARNING: DOMNode::putValueProperty unhandled token " << token << endl;
  }
}

ValueImp* DOMNode::toPrimitive(ExecState *exec, Type /*preferred*/) const
{
  if (m_impl.isNull())
    return Null();

  return String(toString(exec));
}

UString DOMNode::toString(ExecState *) const
{
  if (m_impl.isNull())
    return "null";
  UString s;

  
  if ( m_impl->isElementNode() ) {
    DOM::ElementImpl* e = static_cast<DOM::ElementImpl*>(impl());
    s = DOMString(e->nodeName().string());
  } else
    s = className(); // fallback

  return "[object " + s + "]";
}

void DOMNode::setListener(ExecState *exec, int eventId, ValueImp* func) const
{
  m_impl->setHTMLEventListener(eventId,Window::retrieveActive(exec)->getJSEventListener(func,true));
}

ValueImp* DOMNode::getListener(int eventId) const
{
  DOM::EventListener *listener = m_impl->getHTMLEventListener(eventId);
  JSEventListener *jsListener = static_cast<JSEventListener*>(listener);
  if ( jsListener && jsListener->listenerObj() )
    return jsListener->listenerObj();
  else
    return Null();
}

void DOMNode::pushEventHandlerScope(ExecState *, ScopeChain &) const
{
}

ValueImp* DOMNodeProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( DOMNode, thisObj );
  DOMExceptionTranslator exception(exec);
  DOM::NodeImpl& node = *static_cast<DOMNode *>( thisObj )->impl();
  switch (id) {
    case DOMNode::HasAttributes:
      return Boolean(node.hasAttributes());
    case DOMNode::HasChildNodes:
      return Boolean(node.hasChildNodes());
    case DOMNode::CloneNode:
      return getDOMNode(exec,node.cloneNode(args[0]->toBoolean(exec)));
    case DOMNode::Normalize:
      node.normalize();
      return Undefined();
    case DOMNode::IsSupported:
      return Boolean(node.isSupported(args[0]->toString(exec).domString(),args[1]->toString(exec).domString()));
    case DOMNode::AddEventListener: {
        JSEventListener *listener = Window::retrieveActive(exec)->getJSEventListener(args[1]);
        int id = EventImpl::typeToId(args[0]->toString(exec).domString());
        node.addEventListener(id,listener,args[2]->toBoolean(exec));
        return Undefined();
    }
    case DOMNode::RemoveEventListener: {
        JSEventListener *listener = Window::retrieveActive(exec)->getJSEventListener(args[1]);
        int id = EventImpl::typeToId(args[0]->toString(exec).domString());
        node.removeEventListener(id,listener,args[2]->toBoolean(exec));
        return Undefined();
    }
    case DOMNode::DispatchEvent: {
      SharedPtr<DOM::EventImpl> evt = toEvent(args[0]);
      node.dispatchEvent(evt.get(), exception);
      return Boolean(!evt->defaultPrevented());
    }
    case DOMNode::AppendChild:
      return getDOMNode(exec,node.appendChild(toNode(args[0]), exception));
    case DOMNode::RemoveChild:
      return getDOMNode(exec,node.removeChild(toNode(args[0]), exception));
    case DOMNode::InsertBefore:
      return getDOMNode(exec,node.insertBefore(toNode(args[0]), toNode(args[1]), exception));
    case DOMNode::ReplaceChild:
      return getDOMNode(exec,node.replaceChild(toNode(args[0]), toNode(args[1]), exception));
    case DOMNode::Contains:
    {
      DOM::NodeImpl* other = toNode(args[0]);
      if (other && node.isElementNode())
      {
          bool retval = other->isAncestor(&node);
          return Boolean(retval);
      }
      return Undefined();
    }
    case DOMNode::InsertAdjacentHTML:
    {
      // see http://www.faqts.com/knowledge_base/view.phtml/aid/5756
      // and http://msdn.microsoft.com/workshop/author/dhtml/reference/methods/insertAdjacentHTML.asp
      SharedPtr<DOM::RangeImpl> range = node.getDocument()->createRange();

      range->setStartBefore(&node, exception);
      if (exception.triggered()) return Undefined();

      SharedPtr<DOM::DocumentFragmentImpl> docFrag =
      static_cast<DOM::DocumentFragmentImpl*>(range->createContextualFragment(args[1]->toString(exec).domString(), exception).handle());
      if (exception.triggered()) return Undefined();

      DOMString where = args[0]->toString(exec).domString();

      if (where == "beforeBegin" || where == "BeforeBegin")
        node.parentNode()->insertBefore(docFrag.get(), &node, exception);
      else if (where == "afterBegin" || where == "AfterBegin")
        node.insertBefore(docFrag.get(), node.firstChild(), exception);
      else if (where == "beforeEnd" || where == "BeforeEnd")
        return getDOMNode(exec, node.appendChild(docFrag.get(), exception));
      else if (where == "afterEnd" || where == "AfterEnd")
        if (node.nextSibling())
	  node.parentNode()->insertBefore(docFrag.get(), node.nextSibling(),exception);
	else
	  node.parentNode()->appendChild(docFrag.get(),exception);

      return Undefined();
    }
    case DOMNode::Item: {
      SharedPtr<NodeListImpl> childNodes = node.childNodes();
      return getDOMNode(exec, childNodes->item(static_cast<unsigned long>(args[0]->toNumber(exec))));
    }
  }

  return Undefined();
}

// -------------------------------------------------------------------------

/*
@begin DOMNodeListProtoTable 2
  item		DOMNodeList::Item		DontDelete|Function 1
# IE extension (IE treats DOMNodeList like an HTMLCollection)
  namedItem	DOMNodeList::NamedItem		DontDelete|Function 1
@end
*/
DEFINE_PROTOTYPE("DOMNodeList", DOMNodeListProto)
IMPLEMENT_PROTOFUNC(DOMNodeListProtoFunc)
IMPLEMENT_PROTOTYPE(DOMNodeListProto,DOMNodeListProtoFunc)

const ClassInfo DOMNodeList::info = { "NodeList", 0, 0, 0 };

DOMNodeList::DOMNodeList(ExecState *exec, DOM::NodeListImpl* l)
 : DOMObject(DOMNodeListProto::self(exec)), m_impl(l) { }

DOMNodeList::~DOMNodeList()
{
  ScriptInterpreter::forgetDOMObject(m_impl.get());
}

ValueImp* DOMNodeList::indexGetter(ExecState *exec, unsigned index)
{
  return getDOMNode(exec, m_impl->item(index));
}

ValueImp *DOMNodeList::nameGetter(ExecState *exec, const Identifier& name, const PropertySlot& slot)
{
  DOMNodeList *thisObj = static_cast<DOMNodeList *>(slot.slotBase());
  return getDOMNode(exec, thisObj->getByName(name));
}

ValueImp *DOMNodeList::lengthGetter(ExecState *, const Identifier&, const PropertySlot& slot)
{
  DOMNodeList *thisObj = static_cast<DOMNodeList *>(slot.slotBase());
  return Number(thisObj->m_impl->length());
}

DOM::NodeImpl* DOMNodeList::getByName(const Identifier& name)
{
  //### M.O.: I bet IE checks name only for some tags.
  DOMString domName = name.domString();
  unsigned long l   = m_impl->length();
  for ( unsigned long i = 0; i < l; i++ ) {
    DOM::NodeImpl* n = m_impl->item( i );
    if (n->isElementNode()) {
      DOM::ElementImpl* e = static_cast<DOM::ElementImpl*>(n);
      if (e->getAttribute(ATTR_ID) == domName || e->getAttribute(ATTR_NAME) == domName)
        return n;
    }
  }
  return 0;
}

bool DOMNodeList::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  if (propertyName == lengthPropertyName) {
    slot.setCustom(this, lengthGetter);
    return true;
  }

  //### this could benefit from further nodelist/collection consolidation,
  //including moving nameditem down to nodelists.

  // Look in the prototype (for functions) before assuming it's an item's name
  ObjectImp *proto = prototype()->getObject();
  if (proto && proto->hasProperty(exec, propertyName))
    return false;

  //May be it's an index?
  if (getIndexSlot(this, *m_impl, propertyName, slot))
    return true;

  //May be it's a name -- check by ID
  //Check by id
  if (getByName(propertyName)) {
    slot.setCustom(this, nameGetter);
    return true;
  }

  return DOMObject::getOwnPropertySlot(exec, propertyName, slot);
}

ValueImp* DOMNodeList::callAsFunction(ExecState *exec, ObjectImp *, const List &args)
{
  // Do not use thisObj here. See HTMLCollection.
  UString s = args[0]->toString(exec);

  // index-based lookup?
  bool ok;
  unsigned int u = s.qstring().toULong(&ok);
  if (ok)
    return getDOMNode(exec,m_impl->item(u));

  // try lookup by name
  // ### NodeList::namedItem() would be cool to have
  // ### do we need to support the same two arg overload as in HTMLCollection?
  ValueImp* result = get(exec, Identifier(s));

  if (result)
    return result;

  return Undefined();
}

ValueImp* DOMNodeListProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMNodeList, thisObj );
  DOMNodeList* jsList     = static_cast<DOMNodeList *>(thisObj);
  DOM::NodeListImpl& list = *jsList->impl();
  switch (id) {
  case KJS::DOMNodeList::Item:
    return getDOMNode(exec, list.item(args[0]->toInt32(exec)));
  case KJS::DOMNodeList::NamedItem:
    return getDOMNode(exec, jsList->getByName(Identifier(args[0]->toString(exec))));
  default:
    return Undefined();
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMAttr::info = { "Attr", &DOMNode::info, &DOMAttrTable, 0 };

/* Source for DOMAttrTable.
@begin DOMAttrTable 5
  name		DOMAttr::Name		DontDelete|ReadOnly
  specified	DOMAttr::Specified	DontDelete|ReadOnly
  value		DOMAttr::ValueProperty	DontDelete
  ownerElement	DOMAttr::OwnerElement	DontDelete|ReadOnly
@end
*/
bool DOMAttr::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMAttr::tryGet " << propertyName.qstring() << endl;
#endif
  return getStaticValueSlot<DOMAttr, DOMNode>(exec, &DOMAttrTable, this, propertyName, slot);
}

ValueImp* DOMAttr::getValueProperty(ExecState *exec, int token) const
{
  AttrImpl *attr = static_cast<AttrImpl *>(impl());
  switch (token) {
  case Name:
    return String(attr->name());
  case Specified:
    return Boolean(attr->specified());
  case ValueProperty:
    return String(attr->nodeValue());
  case OwnerElement: // DOM2
    return getDOMNode(exec,attr->ownerElement());
  }
  return Null(); // not reached
}

void DOMAttr::put(ExecState *exec, const Identifier &propertyName, ValueImp* value, int attr)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMAttr::tryPut " << propertyName.qstring() << endl;
#endif
  lookupPut<DOMAttr,DOMNode>(exec, propertyName, value, attr,
                                      &DOMAttrTable, this );
}

void DOMAttr::putValueProperty(ExecState *exec, int token, ValueImp* value, int /*attr*/)
{
  DOMExceptionTranslator exception(exec);
  switch (token) {
  case ValueProperty:
    static_cast<AttrImpl *>(impl())->setValue(value->toString(exec).domString(), exception);
    return;
  default:
    kdWarning() << "DOMAttr::putValueProperty unhandled token " << token << endl;
  }
}

AttrImpl *toAttr(ValueImp *val)
{
    if (!val || !val->isObject(&DOMAttr::info))
        return 0;
    return static_cast<AttrImpl *>(static_cast<DOMNode *>(val)->impl());
}

// -------------------------------------------------------------------------

/* Source for DOMDocumentProtoTable.
@begin DOMDocumentProtoTable 23
  createElement   DOMDocument::CreateElement                   DontDelete|Function 1
  createDocumentFragment DOMDocument::CreateDocumentFragment   DontDelete|Function 1
  createTextNode  DOMDocument::CreateTextNode                  DontDelete|Function 1
  createComment   DOMDocument::CreateComment                   DontDelete|Function 1
  createCDATASection DOMDocument::CreateCDATASection           DontDelete|Function 1
  createProcessingInstruction DOMDocument::CreateProcessingInstruction DontDelete|Function 1
  createAttribute DOMDocument::CreateAttribute                 DontDelete|Function 1
  createEntityReference DOMDocument::CreateEntityReference     DontDelete|Function 1
  getElementsByTagName  DOMDocument::GetElementsByTagName      DontDelete|Function 1
  importNode           DOMDocument::ImportNode                 DontDelete|Function 2
  createElementNS      DOMDocument::CreateElementNS            DontDelete|Function 2
  createAttributeNS    DOMDocument::CreateAttributeNS          DontDelete|Function 2
  getElementsByTagNameNS  DOMDocument::GetElementsByTagNameNS  DontDelete|Function 2
  getElementById     DOMDocument::GetElementById               DontDelete|Function 1
  createRange        DOMDocument::CreateRange                  DontDelete|Function 0
  createNodeIterator DOMDocument::CreateNodeIterator           DontDelete|Function 3
  createTreeWalker   DOMDocument::CreateTreeWalker             DontDelete|Function 4
  createEvent        DOMDocument::CreateEvent                  DontDelete|Function 1
  getOverrideStyle   DOMDocument::GetOverrideStyle             DontDelete|Function 2
  abort              DOMDocument::Abort                        DontDelete|Function 0
  load               DOMDocument::Load                         DontDelete|Function 1
  loadXML            DOMDocument::LoadXML                      DontDelete|Function 2
@end
*/
DEFINE_PROTOTYPE("DOMDocument", DOMDocumentProto)
IMPLEMENT_PROTOFUNC(DOMDocumentProtoFunc)
IMPLEMENT_PROTOTYPE_WITH_PARENT(DOMDocumentProto, DOMDocumentProtoFunc, DOMNodeProto)

const ClassInfo DOMDocument::info = { "Document", &DOMNode::info, &DOMDocumentTable, 0 };

/* Source for DOMDocumentTable.
@begin DOMDocumentTable 4
  doctype         DOMDocument::DocType                         DontDelete|ReadOnly
  implementation  DOMDocument::Implementation                  DontDelete|ReadOnly
  documentElement DOMDocument::DocumentElement                 DontDelete|ReadOnly
  styleSheets     DOMDocument::StyleSheets                     DontDelete|ReadOnly
  preferredStylesheetSet  DOMDocument::PreferredStylesheetSet  DontDelete|ReadOnly
  selectedStylesheetSet  DOMDocument::SelectedStylesheetSet    DontDelete
  readyState      DOMDocument::ReadyState                      DontDelete|ReadOnly
  defaultView     DOMDocument::DefaultView                     DontDelete|ReadOnly
  async           DOMDocument::Async                           DontDelete
@end
*/

DOMDocument::DOMDocument(ExecState *exec, DOM::DocumentImpl* d)
  : DOMNode(exec, d) { }

bool DOMDocument::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMDocument::tryGet " << propertyName.qstring() << endl;
#endif
  return getStaticValueSlot<DOMDocument, DOMNode>(
    exec, &DOMDocumentTable, this, propertyName, slot);
}

ValueImp* DOMDocument::getValueProperty(ExecState *exec, int token) const
{
  DOM::DocumentImpl& doc = *static_cast<DOM::DocumentImpl*>(m_impl.get());

  switch(token) {
  case DocType:
    return getDOMNode(exec,doc.doctype());
  case Implementation:
    return getDOMDOMImplementation(exec,doc.implementation());
  case DocumentElement:
    return getDOMNode(exec,doc.documentElement());
  case StyleSheets:
    //kdDebug() << "DOMDocument::StyleSheets, returning " << doc.styleSheets().length() << " stylesheets" << endl;
    return getDOMStyleSheetList(exec, doc.styleSheets(), &doc);
  case DOMDocument::DefaultView: // DOM2
    return getDOMAbstractView(exec, doc.defaultView());
  case PreferredStylesheetSet:
    return String(doc.preferredStylesheetSet());
  case SelectedStylesheetSet:
    return String(doc.selectedStylesheetSet());
  case ReadyState:
    {
    if ( doc.view() )
    {
      KHTMLPart* part = doc.view()->part();
      if ( part ) {
        if (part->d->m_bComplete) return String("complete");
        if (doc.parsing()) return String("loading");
        return String("loaded");
        // What does the interactive value mean ?
        // Missing support for "uninitialized"
      }
    }
    return Undefined();
    }
  case Async:
    return Boolean(doc.async());
  default:
    kdDebug(6070) << "WARNING: DOMDocument::getValueProperty unhandled token " << token << endl;
    return Null();
  }
}

void DOMDocument::put(ExecState *exec, const Identifier& propertyName, ValueImp* value, int attr)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMDocument::tryPut " << propertyName.qstring() << endl;
#endif
  lookupPut<DOMDocument,DOMNode>(exec, propertyName, value, attr, &DOMDocumentTable, this );
}

void DOMDocument::putValueProperty(ExecState *exec, int token, ValueImp* value, int /*attr*/)
{
  DOM::DocumentImpl& doc = *static_cast<DOM::DocumentImpl*>(impl());
  switch (token) {
    case SelectedStylesheetSet: {
      doc.setSelectedStylesheetSet(value->toString(exec).domString());
      break;
    }
    case Async: {
      doc.setAsync(value->toBoolean(exec));
      break;
    }
  }
}

ValueImp* DOMDocumentProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMDocument, thisObj );
  DOMExceptionTranslator exception(exec);
  DOM::NodeImpl&     node = *static_cast<DOMNode *>( thisObj )->impl();
  DOM::DocumentImpl& doc  = static_cast<DOM::DocumentImpl&>(node);
  
  KJS::UString str = args[0]->toString(exec);
  DOM::DOMString s = str.domString();

  switch(id) {
  case DOMDocument::CreateElement:
    return getDOMNode(exec,doc.createElement(s, exception));
  case DOMDocument::CreateDocumentFragment:
    return getDOMNode(exec,doc.createDocumentFragment());
  case DOMDocument::CreateTextNode:
    return getDOMNode(exec,doc.createTextNode(s.implementation()));
  case DOMDocument::CreateComment:
    return getDOMNode(exec,doc.createComment(s.implementation()));
  case DOMDocument::CreateCDATASection:
    return getDOMNode(exec,doc.createCDATASection(s.implementation()));  /* TODO: okay ? */
  case DOMDocument::CreateProcessingInstruction:
    return getDOMNode(exec,doc.createProcessingInstruction(args[0]->toString(exec).domString(),
                              args[1]->toString(exec).domString().implementation()));
  case DOMDocument::CreateAttribute:
    return getDOMNode(exec,doc.createAttribute(s, exception));
  case DOMDocument::CreateEntityReference:
    return getDOMNode(exec,doc.createEntityReference(args[0]->toString(exec).domString()));
  case DOMDocument::GetElementsByTagName:
    return getDOMNodeList(exec,doc.getElementsByTagName(s));
  case DOMDocument::ImportNode: // DOM2
    return getDOMNode(exec,doc.importNode(toNode(args[0]), args[1]->toBoolean(exec), exception));
  case DOMDocument::CreateElementNS: // DOM2
    return getDOMNode(exec,doc.createElementNS(args[0]->toString(exec).domString(), args[1]->toString(exec).domString(), exception));
  case DOMDocument::CreateAttributeNS: // DOM2
    return getDOMNode(exec,doc.createAttributeNS(args[0]->toString(exec).domString(),args[1]->toString(exec).domString(), exception));
  case DOMDocument::GetElementsByTagNameNS: // DOM2
    return getDOMNodeList(exec,doc.getElementsByTagNameNS(args[0]->toString(exec).domString(),
                                                          args[1]->toString(exec).domString()));
  case DOMDocument::GetElementById:
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMDocument::GetElementById looking for " << args[0]->toString(exec).qstring() << endl;
#endif
    return getDOMNode(exec,doc.getElementById(args[0]->toString(exec).domString()));
  case DOMDocument::CreateRange:
    return getDOMRange(exec,doc.createRange());
  case DOMDocument::CreateNodeIterator:
    if (args[2]->type() == NullType) {
        return getDOMNodeIterator(exec,
                                  doc.createNodeIterator(toNode(args[0]),
                                                         (long unsigned int)(args[1]->toNumber(exec)),
                                                         0,args[3]->toBoolean(exec), exception));
    }
    else {
      ObjectImp *obj = args[2]->getObject();
      if (obj)
      {
        DOM::CustomNodeFilter *customFilter = new JSNodeFilter(obj);
        DOM::NodeFilter filter = DOM::NodeFilter::createCustom(customFilter);
        return getDOMNodeIterator(exec,
          doc.createNodeIterator(
            toNode(args[0]),(long unsigned int)(args[1]->toNumber(exec)),
            filter.handle(),args[3]->toBoolean(exec), exception));
      }// else?
    }
  case DOMDocument::CreateTreeWalker:
    return getDOMTreeWalker(exec,doc.createTreeWalker(toNode(args[0]),(long unsigned int)(args[1]->toNumber(exec)),
             toNodeFilter(args[2]),args[3]->toBoolean(exec), exception));
  case DOMDocument::CreateEvent:
    return getDOMEvent(exec,doc.createEvent(s, exception));
  case DOMDocument::GetOverrideStyle: {
    DOM::NodeImpl* arg0 = toNode(args[0]);
    if (!arg0->isElementNode())
      return Undefined(); // throw exception?
    else
      return getDOMCSSStyleDeclaration(exec,doc.getOverrideStyle(static_cast<DOM::ElementImpl*>(arg0),args[1]->toString(exec).domString().implementation()));
  }
  case DOMDocument::Abort:
    doc.abort();
    break;
  case DOMDocument::Load: {
    Window* active = Window::retrieveActive(exec);
    // Complete the URL using the "active part" (running interpreter). We do this for the security
    // check and to make sure we load exactly the same url as we have verified to be safe
    KHTMLPart *khtmlpart = qobject_cast<KHTMLPart*>(active->part());
    if (khtmlpart) {
      // Security: only allow documents to be loaded from the same host
      QString dstUrl = khtmlpart->htmlDocument().completeURL(s).string();
      KParts::ReadOnlyPart *part = static_cast<KJS::ScriptInterpreter*>(exec->interpreter())->part();
      if (part->url().host() == KURL(dstUrl).host()) {
	kdDebug(6070) << "JavaScript: access granted for document.load() of " << dstUrl << endl;
	doc.load(dstUrl);
      }
      else {
	kdDebug(6070) << "JavaScript: access denied for document.load() of " << dstUrl << endl;
      }
    }
    break;
  }
  case DOMDocument::LoadXML:
    doc.loadXML(s);
    break;
  default:
    break;
  }

  return Undefined();
}

// -------------------------------------------------------------------------

/* Source for DOMElementProtoTable.
@begin DOMElementProtoTable 17
  getAttribute		DOMElement::GetAttribute	DontDelete|Function 1
  setAttribute		DOMElement::SetAttribute	DontDelete|Function 2
  removeAttribute	DOMElement::RemoveAttribute	DontDelete|Function 1
  getAttributeNode	DOMElement::GetAttributeNode	DontDelete|Function 1
  setAttributeNode	DOMElement::SetAttributeNode	DontDelete|Function 2
  removeAttributeNode	DOMElement::RemoveAttributeNode	DontDelete|Function 1
  getElementsByTagName	DOMElement::GetElementsByTagName	DontDelete|Function 1
  hasAttribute		DOMElement::HasAttribute	DontDelete|Function 1
  getAttributeNS	DOMElement::GetAttributeNS	DontDelete|Function 2
  setAttributeNS	DOMElement::SetAttributeNS	DontDelete|Function 3
  removeAttributeNS	DOMElement::RemoveAttributeNS	DontDelete|Function 2
  getAttributeNodeNS	DOMElement::GetAttributeNodeNS	DontDelete|Function 2
  setAttributeNodeNS	DOMElement::SetAttributeNodeNS	DontDelete|Function 1
  getElementsByTagNameNS DOMElement::GetElementsByTagNameNS	DontDelete|Function 2
  hasAttributeNS	DOMElement::HasAttributeNS	DontDelete|Function 2
@end
*/
DEFINE_PROTOTYPE("DOMElement",DOMElementProto)
IMPLEMENT_PROTOFUNC(DOMElementProtoFunc)
IMPLEMENT_PROTOTYPE_WITH_PARENT(DOMElementProto,DOMElementProtoFunc,DOMNodeProto)

const ClassInfo DOMElement::info = { "Element", &DOMNode::info, &DOMElementTable, 0 };
/* Source for DOMElementTable.
@begin DOMElementTable 3
  tagName	DOMElement::TagName                         DontDelete|ReadOnly
  style		DOMElement::Style                           DontDelete|ReadOnly
@end
*/
DOMElement::DOMElement(ExecState *exec, DOM::ElementImpl* e)
  : DOMNode(exec, e) { }

ValueImp* DOMElement::getValueProperty(ExecState *exec, int token) const
{
  DOM::ElementImpl& element = static_cast<DOM::ElementImpl&>(*m_impl);
  switch( token ) {
    case TagName:
      return String(element.tagName());
    case Style:
      return getDOMCSSStyleDeclaration(exec,element.styleRules());
    default:
      kdDebug(6070) << "WARNING: Unhandled token in DOMElement::getValueProperty : " << token << endl;
      return Undefined();
  }
}

bool DOMElement::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMElement::getOwnPropertySlot " << propertyName.qstring() << endl;
#endif

  //DOM::Element element = static_cast<DOM::Element>(node);
  if (getStaticOwnValueSlot(&DOMElementTable, this, propertyName, slot))
    return true;
  
  // We have to check in DOMNode before giving access to attributes, otherwise
  // onload="..." would make onload return the string (attribute value) instead of
  // the listener object (function).
  if (DOMNode::getOwnPropertySlot(exec, propertyName, slot))
    return true;

  // Give access to attributes
  ElementImpl &element = *static_cast<ElementImpl *>(impl());

  if (element.hasAttribute(propertyName.domString())) {
    slot.setCustom(this, attributeGetter);
    return true;
  }

  return false;
}

ValueImp *DOMElement::attributeGetter(ExecState *exec, const Identifier& propertyName, const PropertySlot& slot)
{
  DOMElement *thisObj = static_cast<DOMElement *>(slot.slotBase());

  ElementImpl *element = static_cast<ElementImpl *>(thisObj->impl());
  DOM::DOMString attr = element->getAttribute(propertyName.domString());
  return getStringOrNull(attr);
}

ValueImp* DOMElementProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMNode, thisObj ); // node should be enough here, given the cast
  DOMExceptionTranslator exception(exec);
  DOM::NodeImpl&    node    = *static_cast<DOMNode *>( thisObj )->impl();
  DOM::ElementImpl& element = static_cast<DOM::ElementImpl&>(node);

  switch(id) {
    case DOMElement::GetAttribute:
      return String(UString(element.getAttribute(args[0]->toString(exec).domString())));
    case DOMElement::SetAttribute:
      element.setAttribute(args[0]->toString(exec).domString(),args[1]->toString(exec).domString(), exception);
      return Undefined();
    case DOMElement::RemoveAttribute:
      element.removeAttribute(args[0]->toString(exec).domString(), exception);
      return Undefined();
    case DOMElement::GetAttributeNode:
      return getDOMNode(exec,element.getAttributeNode(args[0]->toString(exec).domString()));
    case DOMElement::SetAttributeNode: {
      Attr old = element.setAttributeNode(toAttr(args[0]), exception);
      return getDOMNode(exec, static_cast<AttrImpl*>(old.handle()));
    }
    case DOMElement::RemoveAttributeNode: {
      Attr old = element.removeAttributeNode(toAttr(args[0]), exception);
      return getDOMNode(exec,static_cast<AttrImpl*>(old.handle()));
    }
    case DOMElement::GetElementsByTagName:
      return getDOMNodeList(exec,element.getElementsByTagName(args[0]->toString(exec).domString()));
    case DOMElement::HasAttribute: // DOM2
      return Boolean(element.hasAttribute(args[0]->toString(exec).domString()));
    case DOMElement::GetAttributeNS: // DOM2
      return String(element.getAttributeNS(args[0]->toString(exec).domString(),args[1]->toString(exec).domString(), exception));
    case DOMElement::SetAttributeNS: // DOM2
      element.setAttributeNS(args[0]->toString(exec).domString(),args[1]->toString(exec).domString(),args[2]->toString(exec).domString(),exception);
      return Undefined();
    case DOMElement::RemoveAttributeNS: // DOM2
      element.removeAttributeNS(args[0]->toString(exec).domString(),args[1]->toString(exec).domString(), exception);
      return Undefined();
    case DOMElement::GetAttributeNodeNS: // DOM2 
      return getDOMNode(exec,element.getAttributeNodeNS(args[0]->toString(exec).domString(),args[1]->toString(exec).domString(),exception));
    case DOMElement::SetAttributeNodeNS: { // DOM2
      Attr old = element.setAttributeNodeNS(toAttr(args[0]),exception);
      return getDOMNode(exec,static_cast<AttrImpl*>(old.handle()));
    }
    case DOMElement::GetElementsByTagNameNS: // DOM2
      return getDOMNodeList(exec,element.getElementsByTagNameNS(args[0]->toString(exec).domString(),args[1]->toString(exec).domString()));
    case DOMElement::HasAttributeNS: // DOM2
      return Boolean(element.hasAttributeNS(args[0]->toString(exec).domString(),args[1]->toString(exec).domString()));
  default:
    return Undefined();
  }
}

DOM::ElementImpl *KJS::toElement(ValueImp *v)
{
  DOM::NodeImpl* node = toNode(v);
  if (node && node->isElementNode())
    return static_cast<DOM::ElementImpl*>(node);
  return 0;
}

// -------------------------------------------------------------------------

/* Source for DOMDOMImplementationProtoTable.
@begin DOMDOMImplementationProtoTable 5
  hasFeature		DOMDOMImplementation::HasFeature		DontDelete|Function 2
  createCSSStyleSheet	DOMDOMImplementation::CreateCSSStyleSheet	DontDelete|Function 2
# DOM2
  createDocumentType	DOMDOMImplementation::CreateDocumentType	DontDelete|Function 3
  createDocument	DOMDOMImplementation::CreateDocument		DontDelete|Function 3
  createHTMLDocument    DOMDOMImplementation::CreateHTMLDocument        DontDelete|Function 1
@end
*/
DEFINE_PROTOTYPE("DOMImplementation",DOMDOMImplementationProto)
IMPLEMENT_PROTOFUNC(DOMDOMImplementationProtoFunc)
IMPLEMENT_PROTOTYPE(DOMDOMImplementationProto,DOMDOMImplementationProtoFunc)

const ClassInfo DOMDOMImplementation::info = { "DOMImplementation", 0, 0, 0 };

DOMDOMImplementation::DOMDOMImplementation(ExecState *exec, DOM::DOMImplementationImpl* i)
  : DOMObject(DOMDOMImplementationProto::self(exec)), m_impl(i) { }

DOMDOMImplementation::~DOMDOMImplementation()
{
  ScriptInterpreter::forgetDOMObject(m_impl.get());
}

ValueImp* DOMDOMImplementationProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMDOMImplementation, thisObj );
  DOM::DOMImplementationImpl& implementation = *static_cast<DOMDOMImplementation *>( thisObj )->impl();
  DOMExceptionTranslator exception(exec);

  switch(id) {
  case DOMDOMImplementation::HasFeature:
    return Boolean(implementation.hasFeature(args[0]->toString(exec).domString(),args[1]->toString(exec).domString()));
  case DOMDOMImplementation::CreateDocumentType: // DOM2
    return getDOMNode(exec,implementation.createDocumentType(args[0]->toString(exec).domString(),args[1]->toString(exec).domString(),args[2]->toString(exec).domString(),exception));
  case DOMDOMImplementation::CreateDocument: { // DOM2
    // Initially set the URL to document of the creator... this is so that it resides in the same
    // host/domain for security checks. The URL will be updated if Document.load() is called.
    KHTMLPart *part = qobject_cast<KHTMLPart*>(static_cast<KJS::ScriptInterpreter*>(exec->interpreter())->part());
    if (part) {
      //### this should probably be pushed to the impl
      DOM::NodeImpl* supposedDocType = toNode(args[2]);
      if (supposedDocType->nodeType() != DOM::Node::DOCUMENT_TYPE_NODE) {
        setDOMException(exec, DOMException::NOT_FOUND_ERR);
        return Null();
      }
      DOM::DocumentTypeImpl* docType = static_cast<DOM::DocumentTypeImpl*>(supposedDocType);

      DOM::DocumentImpl* doc = implementation.createDocument(args[0]->toString(exec).domString(),args[1]->toString(exec).domString(),docType,exception);
      if (!doc)
        return Null();
      KURL url = static_cast<DocumentImpl*>(part->document().handle())->URL();
      doc->setURL(url.url());
      return getDOMNode(exec,doc);
    }
    break;
  }
  case DOMDOMImplementation::CreateCSSStyleSheet: // DOM2
    return getDOMStyleSheet(exec,implementation.createCSSStyleSheet(
        args[0]->toString(exec).domString().implementation(),
        args[1]->toString(exec).domString().implementation(),exception));
  case DOMDOMImplementation::CreateHTMLDocument: // DOM2-HTML
    return getDOMNode(exec, implementation.createHTMLDocument(args[0]->toString(exec).domString()));
  default:
    break;
  }
  return Undefined();
}

// -------------------------------------------------------------------------

const ClassInfo DOMDocumentType::info = { "DocumentType", &DOMNode::info, &DOMDocumentTypeTable, 0 };

/* Source for DOMDocumentTypeTable.
@begin DOMDocumentTypeTable 6
  name			DOMDocumentType::Name		DontDelete|ReadOnly
  entities		DOMDocumentType::Entities	DontDelete|ReadOnly
  notations		DOMDocumentType::Notations	DontDelete|ReadOnly
# DOM2
  publicId		DOMDocumentType::PublicId	DontDelete|ReadOnly
  systemId		DOMDocumentType::SystemId	DontDelete|ReadOnly
  internalSubset	DOMDocumentType::InternalSubset	DontDelete|ReadOnly
@end
*/
DOMDocumentType::DOMDocumentType(ExecState *exec, DOM::DocumentTypeImpl* dt)
  : DOMNode( /*### no proto yet*/exec, dt ) { }

bool DOMDocumentType::getOwnPropertySlot(ExecState *exec, const Identifier &propertyName, KJS::PropertySlot& slot)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMDocumentType::getOwnPropertySlot " << propertyName.qstring() << endl;
#endif
  return getStaticValueSlot<DOMDocumentType, DOMNode>(exec, &DOMDocumentTypeTable, this, propertyName, slot);
}

ValueImp* DOMDocumentType::getValueProperty(ExecState *exec, int token) const
{
  DOM::DocumentTypeImpl& type = static_cast<DOM::DocumentTypeImpl&>(*m_impl);
  switch (token) {
  case Name:
    return String(type.name());
  case Entities:
    return getDOMNamedNodeMap(exec,type.entities());
  case Notations:
    return getDOMNamedNodeMap(exec,type.notations());
  case PublicId: // DOM2
    return String(type.publicId());
  case SystemId: // DOM2
    return String(type.systemId());
  case InternalSubset: // DOM2
    return ::getStringOrNull(type.internalSubset()); // can be null, see domts/level2/core/internalSubset01.html
  default:
    kdDebug(6070) << "WARNING: DOMDocumentType::getValueProperty unhandled token " << token << endl;
    return Null();
  }
}

// -------------------------------------------------------------------------

/* Source for DOMNamedNodeMapProtoTable.
@begin DOMNamedNodeMapProtoTable 7
  getNamedItem		DOMNamedNodeMap::GetNamedItem		DontDelete|Function 1
  setNamedItem		DOMNamedNodeMap::SetNamedItem		DontDelete|Function 1
  removeNamedItem	DOMNamedNodeMap::RemoveNamedItem	DontDelete|Function 1
  item			DOMNamedNodeMap::Item			DontDelete|Function 1
# DOM2
  getNamedItemNS	DOMNamedNodeMap::GetNamedItemNS		DontDelete|Function 2
  setNamedItemNS	DOMNamedNodeMap::SetNamedItemNS		DontDelete|Function 1
  removeNamedItemNS	DOMNamedNodeMap::RemoveNamedItemNS	DontDelete|Function 2
@end
@begin DOMNamedNodeMapTable 7
  length		DOMNamedNodeMap::Length			DontDelete|Function 1
@end
*/
DEFINE_PROTOTYPE("NamedNodeMap", DOMNamedNodeMapProto)
IMPLEMENT_PROTOFUNC(DOMNamedNodeMapProtoFunc)
IMPLEMENT_PROTOTYPE(DOMNamedNodeMapProto,DOMNamedNodeMapProtoFunc)

const ClassInfo DOMNamedNodeMap::info = { "NamedNodeMap", 0, &DOMNamedNodeMapTable, 0 };

DOMNamedNodeMap::DOMNamedNodeMap(ExecState *exec, DOM::NamedNodeMapImpl* m)
  : DOMObject(DOMNamedNodeMapProto::self(exec)), m_impl(m) { }

DOMNamedNodeMap::~DOMNamedNodeMap()
{
  ScriptInterpreter::forgetDOMObject(m_impl.get());
}

ValueImp* DOMNamedNodeMap::indexGetter(ExecState *exec, unsigned index)
{
  return getDOMNode(exec, m_impl->item(index));
}

ValueImp *DOMNamedNodeMap::lengthGetter(ExecState *, const Identifier&, const PropertySlot& slot)
{
  DOMNamedNodeMap *thisObj = static_cast<DOMNamedNodeMap *>(slot.slotBase());
  return Number(thisObj->m_impl->length());
}

bool DOMNamedNodeMap::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot) {
  if (propertyName == lengthPropertyName) {
    slot.setCustom(this, lengthGetter);
    return true;
  }

  //May be it's an index?
  if (getIndexSlot(this, *m_impl, propertyName, slot))
    return true;

  return DOMObject::getOwnPropertySlot(exec, propertyName, slot);
}

ValueImp* DOMNamedNodeMapProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMNamedNodeMap, thisObj );
  DOMExceptionTranslator exception(exec);
  DOM::NamedNodeMapImpl& map = *static_cast<DOMNamedNodeMap *>(thisObj)->impl();

  switch(id) {
    case DOMNamedNodeMap::GetNamedItem:
      return getDOMNode(exec, map.getNamedItem(args[0]->toString(exec).domString()));
    case DOMNamedNodeMap::SetNamedItem: {
      DOM::Node old = map.setNamedItem(toNode(args[0]),exception);
      return getDOMNode(exec, old.handle());
    }
    case DOMNamedNodeMap::RemoveNamedItem: {
      DOM::Node old = map.removeNamedItem(args[0]->toString(exec).domString(),exception);
      return getDOMNode(exec, old.handle());
    }
    case DOMNamedNodeMap::Item:
      return getDOMNode(exec, map.item(args[0]->toInt32(exec)));
    case DOMNamedNodeMap::GetNamedItemNS: {// DOM2
      DOM::Node old = map.getNamedItemNS(args[0]->toString(exec).domString(),args[1]->toString(exec).domString());
      return getDOMNode(exec, old.handle());
    }
    case DOMNamedNodeMap::SetNamedItemNS: {// DOM2
      DOM::Node old = map.setNamedItemNS(toNode(args[0]),exception);
      return getDOMNode(exec, old.handle());
    }
    case DOMNamedNodeMap::RemoveNamedItemNS: { // DOM2
      DOM::Node old = map.removeNamedItemNS(args[0]->toString(exec).domString(),args[1]->toString(exec).domString(),exception);
      return getDOMNode(exec, old.handle());
    }
    default:
      break;
  }

  return Undefined();
}

// -------------------------------------------------------------------------

const ClassInfo DOMProcessingInstruction::info = { "ProcessingInstruction", &DOMNode::info, &DOMProcessingInstructionTable, 0 };

/* Source for DOMProcessingInstructionTable.
@begin DOMProcessingInstructionTable 3
  target	DOMProcessingInstruction::Target	DontDelete|ReadOnly
  data		DOMProcessingInstruction::Data		DontDelete
  sheet		DOMProcessingInstruction::Sheet		DontDelete|ReadOnly
@end
*/
bool DOMProcessingInstruction::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<DOMProcessingInstruction, DOMNode>(exec, &DOMProcessingInstructionTable, this, propertyName, slot);
}

ValueImp* DOMProcessingInstruction::getValueProperty(ExecState *exec, int token) const
{
  DOM::ProcessingInstructionImpl& pi = *static_cast<DOM::ProcessingInstructionImpl*>(m_impl.get());
  switch (token) {
  case Target:
    return String(pi.target());
  case Data:
    return String(pi.data());
  case Sheet:
    return getDOMStyleSheet(exec,pi.sheet());
  default:
    kdDebug(6070) << "WARNING: DOMProcessingInstruction::getValueProperty unhandled token " << token << endl;
    return Null();
  }
}

void DOMProcessingInstruction::put(ExecState *exec, const Identifier &propertyName, ValueImp* value, int attr)
{
  DOM::ProcessingInstructionImpl& pi = *static_cast<DOM::ProcessingInstructionImpl*>(m_impl.get());
  // Not worth using the hashtable for this one ;)
  if (propertyName == "data") {
    DOMExceptionTranslator exception(exec);
    pi.setData(value->toString(exec).domString(),exception);
  } else
    DOMNode::put(exec, propertyName,value,attr);
}

// -------------------------------------------------------------------------

const ClassInfo DOMNotation::info = { "Notation", &DOMNode::info, &DOMNotationTable, 0 };

/* Source for DOMNotationTable.
@begin DOMNotationTable 2
  publicId		DOMNotation::PublicId	DontDelete|ReadOnly
  systemId		DOMNotation::SystemId	DontDelete|ReadOnly
@end
*/
bool DOMNotation::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<DOMNotation, DOMNode>(exec, &DOMNotationTable, this, propertyName, slot);
}

ValueImp* DOMNotation::getValueProperty(ExecState *, int token) const
{
  DOM::NotationImpl& nota = *static_cast<DOM::NotationImpl*>(m_impl.get());
  switch (token) {
  case PublicId:
    return String(nota.publicId());
  case SystemId:
    return String(nota.systemId());
  default:
    kdDebug(6070) << "WARNING: DOMNotation::getValueProperty unhandled token " << token << endl;
    return Null();
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMEntity::info = { "Entity", &DOMNode::info, 0, 0 };

/* Source for DOMEntityTable.
@begin DOMEntityTable 2
  publicId		DOMEntity::PublicId		DontDelete|ReadOnly
  systemId		DOMEntity::SystemId		DontDelete|ReadOnly
  notationName		DOMEntity::NotationName	DontDelete|ReadOnly
@end
*/
bool DOMEntity::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<DOMEntity, DOMNode>(exec, &DOMEntityTable, this, propertyName, slot);
}

ValueImp* DOMEntity::getValueProperty(ExecState *, int token) const
{
  DOM::EntityImpl& entity = *static_cast<DOM::EntityImpl*>(m_impl.get());
  switch (token) {
  case PublicId:
    return String(entity.publicId());
  case SystemId:
    return String(entity.systemId());
  case NotationName:
    return String(entity.notationName());
  default:
    kdDebug(6070) << "WARNING: DOMEntity::getValueProperty unhandled token " << token << endl;
    return Null();
  }
}

// -------------------------------------------------------------------------

bool KJS::checkNodeSecurity(ExecState *exec, const DOM::NodeImpl* n)
{
  // Check to see if the currently executing interpreter is allowed to access the specified node
  if (!n)
    return true;
  KHTMLView *view = n->getDocument()->view();
  Window* win = view && view->part() ? Window::retrieveWindow(view->part()) : 0L;
  if ( !win || !win->isSafeScript(exec) )
    return false;
  return true;
}

ValueImp* KJS::getDOMNode(ExecState *exec, DOM::NodeImpl* n)
{
  DOMObject *ret = 0;
  if (!n)
    return Null();
  ScriptInterpreter* interp = static_cast<ScriptInterpreter *>(exec->interpreter());
  if ((ret = interp->getDOMObject(n)))
    return ret;

  switch (n->nodeType()) {
    case DOM::Node::ELEMENT_NODE:
      if (static_cast<DOM::ElementImpl*>(n)->isHTMLElement())
        ret = new HTMLElement(exec, static_cast<DOM::HTMLElementImpl*>(n));
      else
        ret = new DOMElement(exec, static_cast<DOM::ElementImpl*>(n));
      break;
    case DOM::Node::ATTRIBUTE_NODE:
      ret = new DOMAttr(exec, static_cast<DOM::AttrImpl*>(n));
      break;
    case DOM::Node::TEXT_NODE:
    case DOM::Node::CDATA_SECTION_NODE:
      ret = new DOMText(exec, static_cast<DOM::TextImpl*>(n));
      break;
    case DOM::Node::ENTITY_REFERENCE_NODE:
      ret = new DOMNode(exec, n);
      break;
    case DOM::Node::ENTITY_NODE:
      ret = new DOMEntity(exec, static_cast<DOM::EntityImpl*>(n));
      break;
    case DOM::Node::PROCESSING_INSTRUCTION_NODE:
      ret = new DOMProcessingInstruction(exec, static_cast<DOM::ProcessingInstructionImpl*>(n));
      break;
    case DOM::Node::COMMENT_NODE:
      ret = new DOMCharacterData(exec, static_cast<DOM::CharacterDataImpl*>(n));
      break;
    case DOM::Node::DOCUMENT_NODE:
      if (static_cast<DOM::DocumentImpl*>(n)->isHTMLDocument())
        ret = new HTMLDocument(exec, static_cast<DOM::HTMLDocumentImpl*>(n));
      else
        ret = new DOMDocument(exec, static_cast<DOM::DocumentImpl*>(n));
      break;
    case DOM::Node::DOCUMENT_TYPE_NODE:
      ret = new DOMDocumentType(exec, static_cast<DOM::DocumentTypeImpl*>(n));
      break;
    case DOM::Node::DOCUMENT_FRAGMENT_NODE:
      ret = new DOMNode(exec, n);
      break;
    case DOM::Node::NOTATION_NODE:
      ret = new DOMNotation(exec, static_cast<DOM::NotationImpl*>(n));
      break;
    default:
      ret = new DOMNode(exec, n);
  }
  interp->putDOMObject(n,ret);

  return ret;
}

ValueImp* KJS::getDOMNamedNodeMap(ExecState *exec, DOM::NamedNodeMapImpl* m)
{
  return cacheDOMObject<DOM::NamedNodeMapImpl, KJS::DOMNamedNodeMap>(exec, m);
}

ValueImp* KJS::getDOMNodeList(ExecState *exec, DOM::NodeListImpl* l)
{
  return cacheDOMObject<DOM::NodeListImpl, KJS::DOMNodeList>(exec, l);
}

ValueImp* KJS::getDOMDOMImplementation(ExecState *exec, DOM::DOMImplementationImpl* i)
{
  return cacheDOMObject<DOM::DOMImplementationImpl, KJS::DOMDOMImplementation>(exec, i);
}

// -------------------------------------------------------------------------

const ClassInfo NodeConstructor::info = { "NodeConstructor", 0, &NodeConstructorTable, 0 };
/* Source for NodeConstructorTable.
@begin NodeConstructorTable 11
  ELEMENT_NODE		DOM::Node::ELEMENT_NODE		DontDelete|ReadOnly
  ATTRIBUTE_NODE	DOM::Node::ATTRIBUTE_NODE		DontDelete|ReadOnly
  TEXT_NODE		DOM::Node::TEXT_NODE		DontDelete|ReadOnly
  CDATA_SECTION_NODE	DOM::Node::CDATA_SECTION_NODE	DontDelete|ReadOnly
  ENTITY_REFERENCE_NODE	DOM::Node::ENTITY_REFERENCE_NODE	DontDelete|ReadOnly
  ENTITY_NODE		DOM::Node::ENTITY_NODE		DontDelete|ReadOnly
  PROCESSING_INSTRUCTION_NODE DOM::Node::PROCESSING_INSTRUCTION_NODE DontDelete|ReadOnly
  COMMENT_NODE		DOM::Node::COMMENT_NODE		DontDelete|ReadOnly
  DOCUMENT_NODE		DOM::Node::DOCUMENT_NODE		DontDelete|ReadOnly
  DOCUMENT_TYPE_NODE	DOM::Node::DOCUMENT_TYPE_NODE	DontDelete|ReadOnly
  DOCUMENT_FRAGMENT_NODE DOM::Node::DOCUMENT_FRAGMENT_NODE	DontDelete|ReadOnly
  NOTATION_NODE		DOM::Node::NOTATION_NODE		DontDelete|ReadOnly
@end
*/

NodeConstructor::NodeConstructor(ExecState *exec)
  : DOMObject(exec->lexicalInterpreter()->builtinObjectPrototype())
{
}

bool NodeConstructor::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<NodeConstructor, DOMObject>(exec, &NodeConstructorTable, this, propertyName, slot);
}

ValueImp* NodeConstructor::getValueProperty(ExecState *, int token) const
{
  // We use the token as the value to return directly
  return Number((unsigned int)token);
#if 0
  switch (token) {
  case ELEMENT_NODE:
    return Number((unsigned int)DOM::Node::ELEMENT_NODE);
  case ATTRIBUTE_NODE:
    return Number((unsigned int)DOM::Node::ATTRIBUTE_NODE);
  case TEXT_NODE:
    return Number((unsigned int)DOM::Node::TEXT_NODE);
  case CDATA_SECTION_NODE:
    return Number((unsigned int)DOM::Node::CDATA_SECTION_NODE);
  case ENTITY_REFERENCE_NODE:
    return Number((unsigned int)DOM::Node::ENTITY_REFERENCE_NODE);
  case ENTITY_NODE:
    return Number((unsigned int)DOM::Node::ENTITY_NODE);
  case PROCESSING_INSTRUCTION_NODE:
    return Number((unsigned int)DOM::Node::PROCESSING_INSTRUCTION_NODE);
  case COMMENT_NODE:
    return Number((unsigned int)DOM::Node::COMMENT_NODE);
  case DOCUMENT_NODE:
    return Number((unsigned int)DOM::Node::DOCUMENT_NODE);
  case DOCUMENT_TYPE_NODE:
    return Number((unsigned int)DOM::Node::DOCUMENT_TYPE_NODE);
  case DOCUMENT_FRAGMENT_NODE:
    return Number((unsigned int)DOM::Node::DOCUMENT_FRAGMENT_NODE);
  case NOTATION_NODE:
    return Number((unsigned int)DOM::Node::NOTATION_NODE);
  default:
    kdDebug(6070) << "WARNING: NodeConstructor::getValueProperty unhandled token " << token << endl;
    return Null();
  }
#endif
}

ObjectImp *KJS::getNodeConstructor(ExecState *exec)
{
  return cacheGlobalObject<NodeConstructor>(exec, "[[node.constructor]]");
}

// -------------------------------------------------------------------------

const ClassInfo DOMExceptionConstructor::info = { "DOMExceptionConstructor", 0, 0, 0 };

/* Source for DOMExceptionConstructorTable.
@begin DOMExceptionConstructorTable 15
  INDEX_SIZE_ERR		DOM::DOMException::INDEX_SIZE_ERR		DontDelete|ReadOnly
  DOMSTRING_SIZE_ERR		DOM::DOMException::DOMSTRING_SIZE_ERR	DontDelete|ReadOnly
  HIERARCHY_REQUEST_ERR		DOM::DOMException::HIERARCHY_REQUEST_ERR	DontDelete|ReadOnly
  WRONG_DOCUMENT_ERR		DOM::DOMException::WRONG_DOCUMENT_ERR	DontDelete|ReadOnly
  INVALID_CHARACTER_ERR		DOM::DOMException::INVALID_CHARACTER_ERR	DontDelete|ReadOnly
  NO_DATA_ALLOWED_ERR		DOM::DOMException::NO_DATA_ALLOWED_ERR	DontDelete|ReadOnly
  NO_MODIFICATION_ALLOWED_ERR	DOM::DOMException::NO_MODIFICATION_ALLOWED_ERR	DontDelete|ReadOnly
  NOT_FOUND_ERR			DOM::DOMException::NOT_FOUND_ERR		DontDelete|ReadOnly
  NOT_SUPPORTED_ERR		DOM::DOMException::NOT_SUPPORTED_ERR	DontDelete|ReadOnly
  INUSE_ATTRIBUTE_ERR		DOM::DOMException::INUSE_ATTRIBUTE_ERR	DontDelete|ReadOnly
  INVALID_STATE_ERR		DOM::DOMException::INVALID_STATE_ERR	DontDelete|ReadOnly
  SYNTAX_ERR			DOM::DOMException::SYNTAX_ERR		DontDelete|ReadOnly
  INVALID_MODIFICATION_ERR	DOM::DOMException::INVALID_MODIFICATION_ERR	DontDelete|ReadOnly
  NAMESPACE_ERR			DOM::DOMException::NAMESPACE_ERR		DontDelete|ReadOnly
  INVALID_ACCESS_ERR		DOM::DOMException::INVALID_ACCESS_ERR	DontDelete|ReadOnly
@end
*/

DOMExceptionConstructor::DOMExceptionConstructor(ExecState* exec)
  : DOMObject(exec->lexicalInterpreter()->builtinObjectPrototype())
{
}

bool DOMExceptionConstructor::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<DOMExceptionConstructor, DOMObject>(exec, &DOMExceptionConstructorTable, this, propertyName, slot);
}

ValueImp* DOMExceptionConstructor::getValueProperty(ExecState *, int token) const
{
  // We use the token as the value to return directly
  return Number((unsigned int)token);
#if 0
  switch (token) {
  case INDEX_SIZE_ERR:
    return Number((unsigned int)DOM::DOMException::INDEX_SIZE_ERR);
  case DOMSTRING_SIZE_ERR:
    return Number((unsigned int)DOM::DOMException::DOMSTRING_SIZE_ERR);
  case HIERARCHY_REQUEST_ERR:
    return Number((unsigned int)DOM::DOMException::HIERARCHY_REQUEST_ERR);
  case WRONG_DOCUMENT_ERR:
    return Number((unsigned int)DOM::DOMException::WRONG_DOCUMENT_ERR);
  case INVALID_CHARACTER_ERR:
    return Number((unsigned int)DOM::DOMException::INVALID_CHARACTER_ERR);
  case NO_DATA_ALLOWED_ERR:
    return Number((unsigned int)DOM::DOMException::NO_DATA_ALLOWED_ERR);
  case NO_MODIFICATION_ALLOWED_ERR:
    return Number((unsigned int)DOM::DOMException::NO_MODIFICATION_ALLOWED_ERR);
  case NOT_FOUND_ERR:
    return Number((unsigned int)DOM::DOMException::NOT_FOUND_ERR);
  case NOT_SUPPORTED_ERR:
    return Number((unsigned int)DOM::DOMException::NOT_SUPPORTED_ERR);
  case INUSE_ATTRIBUTE_ERR:
    return Number((unsigned int)DOM::DOMException::INUSE_ATTRIBUTE_ERR);
  case INVALID_STATE_ERR:
    return Number((unsigned int)DOM::DOMException::INVALID_STATE_ERR);
  case SYNTAX_ERR:
    return Number((unsigned int)DOM::DOMException::SYNTAX_ERR);
  case INVALID_MODIFICATION_ERR:
    return Number((unsigned int)DOM::DOMException::INVALID_MODIFICATION_ERR);
  case NAMESPACE_ERR:
    return Number((unsigned int)DOM::DOMException::NAMESPACE_ERR);
  case INVALID_ACCESS_ERR:
    return Number((unsigned int)DOM::DOMException::INVALID_ACCESS_ERR);
  default:
    kdDebug(6070) << "WARNING: DOMExceptionConstructor::getValueProperty unhandled token " << token << endl;
    return Null();
  }
#endif
}

ObjectImp *KJS::getDOMExceptionConstructor(ExecState *exec)
{
  return cacheGlobalObject<DOMExceptionConstructor>(exec, "[[DOMException.constructor]]");
}

// -------------------------------------------------------------------------

/* Source for DOMNamedNodesCollection.
@begin DOMNamedNodesCollectionTable 1
  length		KJS::DOMNamedNodesCollection::Length		DontDelete|ReadOnly
@end
*/
const ClassInfo KJS::DOMNamedNodesCollection::info = { "DOMNamedNodesCollection", 0, &DOMNamedNodesCollectionTable, 0 };

// Such a collection is usually very short-lived, it only exists
// for constructs like document.forms.<name>[1],
// so it shouldn't be a problem that it's storing all the nodes (with the same name). (David)
DOMNamedNodesCollection::DOMNamedNodesCollection(ExecState *exec, const QList<SharedPtr<DOM::NodeImpl> >& nodes)
  : DOMObject(exec->lexicalInterpreter()->builtinObjectPrototype()),
  m_nodes(nodes)
{
  // Maybe we should ref (and deref in the dtor) the nodes, though ?
}

ValueImp* DOMNamedNodesCollection::indexGetter(ExecState *exec, unsigned index)
{
  return getDOMNode(exec, m_nodes[index].get());
}

ValueImp *DOMNamedNodesCollection::lengthGetter(ExecState *, const Identifier&, const PropertySlot& slot)
{
  DOMNamedNodesCollection *thisObj = static_cast<DOMNamedNodesCollection *>(slot.slotBase());
  return Number(thisObj->m_nodes.size());
}


bool DOMNamedNodesCollection::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  kdDebug(6070) << k_funcinfo << propertyName.ascii() << endl;

  if (propertyName == lengthPropertyName) {
    slot.setCustom(this, lengthGetter);
    return true;
  }

  //May be it's an index?
  if (getIndexSlot(this, m_nodes.size(), propertyName, slot))
    return true;
  
  return DOMObject::getOwnPropertySlot(exec,propertyName,slot);
}

// -------------------------------------------------------------------------

const ClassInfo DOMCharacterData::info = { "CharacterImp",
					  &DOMNode::info, &DOMCharacterDataTable, 0 };
/*
@begin DOMCharacterDataTable 2
  data		DOMCharacterData::Data		DontDelete
  length	DOMCharacterData::Length	DontDelete|ReadOnly
@end
@begin DOMCharacterDataProtoTable 7
  substringData	DOMCharacterData::SubstringData	DontDelete|Function 2
  appendData	DOMCharacterData::AppendData	DontDelete|Function 1
  insertData	DOMCharacterData::InsertData	DontDelete|Function 2
  deleteData	DOMCharacterData::DeleteData	DontDelete|Function 2
  replaceData	DOMCharacterData::ReplaceData	DontDelete|Function 2
@end
*/
DEFINE_PROTOTYPE("DOMCharacterData",DOMCharacterDataProto)
IMPLEMENT_PROTOFUNC(DOMCharacterDataProtoFunc)
IMPLEMENT_PROTOTYPE_WITH_PARENT(DOMCharacterDataProto,DOMCharacterDataProtoFunc, DOMNodeProto)

DOMCharacterData::DOMCharacterData(ExecState *exec, DOM::CharacterDataImpl* d)
 : DOMNode(exec, d) {}

bool DOMCharacterData::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
#ifdef KJS_VERBOSE
  kdDebug(6070)<<"DOMCharacterData::tryGet "<<p.string().string()<<endl;
#endif
  return getStaticValueSlot<DOMCharacterData, DOMNode>(exec, &DOMCharacterDataTable, this, propertyName, slot);
}

ValueImp* DOMCharacterData::getValueProperty(ExecState *, int token) const
{
  DOM::CharacterDataImpl& data = *impl();
  switch (token) {
  case Data:
    return String(data.data());
  case Length:
    return Number(data.length());
 default:
   kdDebug(6070) << "WARNING: Unhandled token in DOMCharacterData::getValueProperty : " << token << endl;
   return Null();
  }
}

void DOMCharacterData::put(ExecState *exec, const Identifier &propertyName, ValueImp* value, int attr)
{
  if (propertyName == "data") {
    DOMExceptionTranslator exception(exec);
    impl()->setData(value->toString(exec).domString(), exception);
  } else
    DOMNode::put(exec, propertyName,value,attr);
}

ValueImp* DOMCharacterDataProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMCharacterData, thisObj );
  DOM::CharacterDataImpl& data = *static_cast<DOMCharacterData *>(thisObj)->impl();
  DOMExceptionTranslator exception(exec);
  switch(id) {
    case DOMCharacterData::SubstringData:
      return String(data.substringData(args[0]->toInteger(exec),args[1]->toInteger(exec),exception));
    case DOMCharacterData::AppendData:
      data.appendData(args[0]->toString(exec).domString(),exception);
      return Undefined();
      break;
    case DOMCharacterData::InsertData:
      data.insertData(args[0]->toInteger(exec),args[1]->toString(exec).domString(), exception);
      return  Undefined();
      break;
    case DOMCharacterData::DeleteData:
      data.deleteData(args[0]->toInteger(exec),args[1]->toInteger(exec),exception);
      return  Undefined();
      break;
    case DOMCharacterData::ReplaceData:
      data.replaceData(args[0]->toInteger(exec),args[1]->toInteger(exec),args[2]->toString(exec).domString(),exception);
      return Undefined();
      break;
    default:
      return Undefined();
  }
  return Undefined();
}

// -------------------------------------------------------------------------

const ClassInfo DOMText::info = { "Text",
				 &DOMCharacterData::info, 0, 0 };
/*
@begin DOMTextProtoTable 1
  splitText	DOMText::SplitText	DontDelete|Function 1
@end
*/
DEFINE_PROTOTYPE("DOMText",DOMTextProto)
IMPLEMENT_PROTOFUNC(DOMTextProtoFunc)
IMPLEMENT_PROTOTYPE_WITH_PARENT(DOMTextProto,DOMTextProtoFunc,DOMCharacterDataProto)

DOMText::DOMText(ExecState *exec, DOM::TextImpl* t)
  : DOMCharacterData(exec, t) { }



ValueImp* DOMTextProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMText, thisObj );
  DOM::TextImpl& text = *static_cast<DOMText *>(thisObj)->impl();
  DOMExceptionTranslator exception(exec);
  switch(id) {
    case DOMText::SplitText:
      return getDOMNode(exec,text.splitText(args[0]->toInteger(exec), exception));
      break;
    default:
      return Undefined();
  }
  return Undefined();
}
