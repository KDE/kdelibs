// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <qptrdict.h>

#include <khtmlview.h>
#include <xml/dom2_eventsimpl.h>
#include <rendering/render_object.h>
#include <kdebug.h>

#include "kjs_text.h"
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

using namespace KJS;

// ### This cache should be in the Window object (i.e. per part)
// and all nodes should be marked by the window, so that the GC can't
// delete bindings that have been created (thus losing any custom-set properties)
QPtrDict<DOMNode> nodes(1021);
QPtrDict<DOMNamedNodeMap> namedNodeMaps;
QPtrDict<DOMNodeList> nodeLists;
QPtrDict<DOMDOMImplementation> domImplementations;

// -------------------------------------------------------------------------
/*
@begin DOMNodeProtoTable 11
  insertBefore	DOMNode::InsertBefore	DontDelete|Function 2
  replaceChild	DOMNode::ReplaceChild	DontDelete|Function 2
  removeChild	DOMNode::RemoveChild	DontDelete|Function 1
  appendChild	DOMNode::AppendChild	DontDelete|Function 1
  hasAttributes	DOMNode::HasAttributes	DontDelete|Function 0
  hasChildNodes	DOMNode::HasChildNodes	DontDelete|Function 0
  cloneNode	DOMNode::CloneNode	DontDelete|Function 1
#normalize // moved here from Element in DOM2 DOMNode::Normalize
#supports // new for DOM2 - not yet in khtml  DOMNode::Supports
# from the EventTarget interface
  addEventListener	DOMNode::AddEventListener	DontDelete|Function 3
  removeEventListener	DOMNode::RemoveEventListener	DontDelete|Function 3
  dispatchEvent		DOMNode::DispatchEvent	DontDelete|Function 1
  contains	DOMNode::Contains		DontDelete|Function 1
@end
*/
IMPLEMENT_PROTOFUNC(DOMNodeProtoFunc)
IMPLEMENT_PROTOTYPE("DOMNode",DOMNodeProto,DOMNodeProtoFunc,DOMObjectProto)

const ClassInfo DOMNode::info = { "Node", 0, &DOMNodeTable, 0 };

DOMNode::DOMNode(ExecState *exec, DOM::Node n)
  : DOMObject(DOMNodeProto::self(exec)), node(n)
{
}

DOMNode::DOMNode(Object proto, DOM::Node n)
  : DOMObject(proto), node(n)
{
}

DOMNode::~DOMNode()
{
  nodes.remove(node.handle());
}

Boolean DOMNode::toBoolean(ExecState *) const
{
    return Boolean(!node.isNull());
}

/*
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
#// new for DOM2 - not yet in khtml
#namespaceURI	DOMNode::NamespaceURI	DontDelete|ReadOnly
#prefix		DOMNode::Prefix		DontDelete
#localName	DOMNode::LocalName	DontDelete|ReadOnly
  ownerDocument	DOMNode::OwnerDocument	DontDelete|ReadOnly
#
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
  onmousedwn	DOMNode::OnMouseDown		DontDelete
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
#
  offsetLeft	DOMNode::OffsetLeft		DontDelete|ReadOnly
  offsetTop	DOMNode::OffsetTop		DontDelete|ReadOnly
  offsetWidth	DOMNode::OffsetWidth		DontDelete|ReadOnly
  offsetHeight	DOMNode::OffsetHeight		DontDelete|ReadOnly
  offsetParent	DOMNode::OffsetParent		DontDelete|ReadOnly
  clientWidth	DOMNode::ClientWidth		DontDelete|ReadOnly
  clientHeight	DOMNode::ClientHeight		DontDelete|ReadOnly
  scrollLeft	DOMNode::ScrollLeft		DontDelete|ReadOnly
  scrollTop	DOMNode::ScrollTop		DontDelete|ReadOnly
@end
*/
Value DOMNode::tryGet(ExecState *exec, const UString &propertyName) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMNode::tryGet " << propertyName.qstring() << endl;
#endif
  return DOMObjectLookupGetValue<DOMNode, DOMObject>(exec, propertyName, &DOMNodeTable, this);
}

Value DOMNode::getValue(ExecState *exec, int token) const
{
  khtml::RenderObject *rend = node.handle() ? node.handle()->renderer() : 0L;

  switch (token) {
  case NodeName:
    return getString(node.nodeName());
  case NodeValue:
    return getString(node.nodeValue());
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
  case Attributes:
    return getDOMNamedNodeMap(exec,node.attributes());
// new for DOM2 - not yet in khtml
//  case NamespaceURI:
//    return getString(node.namespaceURI());
//  case Prefix:
//    return getString(node.prefix());
//  case LocalName:
//    return getString(node.localName());
  case OwnerDocument:
    return getDOMNode(exec,node.ownerDocument());
  case OnAbort:
    return getListener(DOM::EventImpl::ABORT_EVENT);
  case OnBlur:
    return getListener(DOM::EventImpl::BLUR_EVENT);
  case OnChange:
    return getListener(DOM::EventImpl::CHANGE_EVENT);
  case OnClick:
    return getListener(DOM::EventImpl::KHTML_CLICK_EVENT);
  case OnDblClick:
    return getListener(DOM::EventImpl::KHTML_DBLCLICK_EVENT);
  case OnDragDrop:
    return getListener(DOM::EventImpl::KHTML_DRAGDROP_EVENT);
  case OnError:
    return getListener(DOM::EventImpl::KHTML_ERROR_EVENT);
  case OnFocus:
    return getListener(DOM::EventImpl::FOCUS_EVENT);
  case OnKeyDown:
    return getListener(DOM::EventImpl::KHTML_KEYDOWN_EVENT);
  case OnKeyPress:
    return getListener(DOM::EventImpl::KHTML_KEYPRESS_EVENT);
  case OnKeyUp:
    return getListener(DOM::EventImpl::KHTML_KEYUP_EVENT);
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
  default:
    // no DOM standard, found in IE only

    // make sure our rendering is up to date before
    // we allow a query on these attributes.
    // ### how often does it fall into the final else case ?
    if ( node.handle() && node.handle()->ownerDocument() )
      node.handle()->ownerDocument()->updateRendering();

    switch (token) {
    case OffsetLeft:
      return rend ? static_cast<Value>(Number(rend->xPos())) : Value(Undefined());
    case OffsetTop:
      return rend ? static_cast<Value>(Number(rend->yPos())) : Value(Undefined());
    case OffsetWidth:
      return rend ? static_cast<Value>(Number(rend->width()) ) : Value(Undefined());
    case OffsetHeight:
      return rend ? static_cast<Value>(Number(rend->height() ) ) : Value(Undefined());
    case OffsetParent:
      return getDOMNode(exec,node.parentNode()); // not necessarily correct
    case ClientWidth:
      return rend ? static_cast<Value>(Number(rend->contentWidth())) : Value(Undefined());
    case ClientHeight:
      return rend ? static_cast<Value>(Number(rend->contentHeight())) : Value(Undefined());
    case ScrollLeft:
      return rend ? static_cast<Value>(Number(-rend->xPos() + node.ownerDocument().view()->contentsX())) : Value(Undefined());
    case ScrollTop:
      return rend ? static_cast<Value>(Number(-rend->yPos() + node.ownerDocument().view()->contentsY())) : Value(Undefined());
    default:
      fprintf(stderr, "Unhandled token in DOMNode::getValue : %d\n", token);
      break;
    }
  }

  return Value();
}

void DOMNode::tryPut(ExecState *exec, const UString& propertyName, const Value& value, int attr)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMNode::tryPut " << propertyName.qstring() << endl;
#endif
  DOMObjectLookupPut<DOMNode,DOMObject>(exec, propertyName, value, attr,
                                        &DOMNodeTable, this );
}

void DOMNode::putValue(ExecState *exec, int token, const Value& value, int /*attr*/)
{
  switch (token) {
  case NodeValue:
    node.setNodeValue(value.toString(exec).value().string());
    break;
  //case Prefix:
   // new for DOM2 - not yet in khtml
   //    node.setPrefix(v.toString().value().string());
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
    setListener(exec,DOM::EventImpl::KHTML_CLICK_EVENT,value);
    break;
  case OnDblClick:
    setListener(exec,DOM::EventImpl::KHTML_DBLCLICK_EVENT,value);
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
    setListener(exec,DOM::EventImpl::KHTML_KEYDOWN_EVENT,value);
    break;
  case OnKeyPress:
    setListener(exec,DOM::EventImpl::KHTML_KEYPRESS_EVENT,value);
    break;
  case OnKeyUp:
    setListener(exec,DOM::EventImpl::KHTML_KEYUP_EVENT,value);
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
  default:
    kdWarning() << "DOMNode::putValue unhandled token " << token << endl;
  }
}

Value DOMNode::toPrimitive(ExecState *exec, Type /*preferred*/) const
{
  if (node.isNull())
    return Null();

  return toString(exec);
}

String DOMNode::toString(ExecState *) const
{
  if (node.isNull())
    return String("null");
  DOM::DOMString s = "DOMNode"; // fallback

  DOM::Element e = node;
  if ( !e.isNull() ) {
    s = e.nodeName();
  }

  return String("[object " + UString(s) + "]");
}

void DOMNode::setListener(ExecState *exec, int eventId, Value func) const
{
  node.handle()->setHTMLEventListener(eventId,Window::retrieveActive(exec)->getJSEventListener(func,true));
}

Value DOMNode::getListener(int eventId) const
{
    DOM::EventListener *listener = node.handle()->getHTMLEventListener(eventId);
    if (listener)
	return static_cast<JSEventListener*>(listener)->listenerObj();
    else
	return Null();
}

List DOMNode::eventHandlerScope(ExecState *) const
{
  return List::empty();
}

Value DOMNodeProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  DOM::Node node = static_cast<DOMNode *>( thisObj.imp() )->toNode();
  Value result;
  switch (id) {
    case DOMNode::HasAttributes:
      result = Boolean(node.hasAttributes());
      break;
    case DOMNode::HasChildNodes:
      result = Boolean(node.hasChildNodes());
      break;
    case DOMNode::CloneNode:
      result = getDOMNode(exec,node.cloneNode(args[0].toBoolean(exec).value()));
      break;
    case DOMNode::AddEventListener: {
//        JSEventListener *listener = new JSEventListener(args[1]); // will get deleted when the node derefs it
        JSEventListener *listener = Window::retrieveActive(exec)->getJSEventListener(args[1]);
        node.addEventListener(args[0].toString(exec).value().string(),listener,args[2].toBoolean(exec).value());
        result = Undefined();
      }
      break;
    case DOMNode::RemoveEventListener: {
        JSEventListener *listener = Window::retrieveActive(exec)->getJSEventListener(args[1]);
        node.removeEventListener(args[0].toString(exec).value().string(),listener,args[2].toBoolean(exec).value());
        result = Undefined();
      }
      break;
    case DOMNode::DispatchEvent:
      result = Boolean(node.dispatchEvent(toEvent(args[0])));
      break;
    case DOMNode::AppendChild:
      result = getDOMNode(exec,node.appendChild(toNode(args[0])));
      break;
    case DOMNode::RemoveChild:
      result = getDOMNode(exec,node.removeChild(toNode(args[0])));
      break;
    case DOMNode::InsertBefore:
      result = getDOMNode(exec,node.insertBefore(toNode(args[0]), toNode(args[1])));
      break;
    case DOMNode::ReplaceChild:
      result = getDOMNode(exec,node.replaceChild(toNode(args[0]), toNode(args[1])));
      break;
    case DOMNode::Contains:
    {
        int exceptioncode=0;
	DOM::Node other = toNode(args[0]);
	if (!other.isNull() && node.nodeType()==DOM::Node::ELEMENT_NODE)
	{
	    DOM::NodeBaseImpl *impl = static_cast<DOM::NodeBaseImpl *>(node.handle());
	    bool retval = !impl->checkNoOwner(other.handle(),exceptioncode);
	    result = Boolean(retval && exceptioncode == 0);
	}
    }
  }

  return result;
}

// -------------------------------------------------------------------------

const ClassInfo DOMNodeList::info = { "NodeList", 0, 0, 0 };

DOMNodeList::~DOMNodeList()
{
  nodeLists.remove(list.handle());
}

Value DOMNodeList::tryGet(ExecState *exec, const UString &p) const
{
  Value result;

  if (p == "length")
    result = Number(list.length());
  else if (p == "item") {
    // No need for a complete hashtable for a single func, but we still want
    // to use the caching feature of lookupOrCreateFunction.
    HashEntry entry;
    entry.value = DOMNodeListFunc::Item;
    entry.params = 1;
    entry.attr = DontDelete|Function;
    result = lookupOrCreateFunction<DOMNodeListFunc, DOMNodeList>(exec, p, this, &entry);
    //result = new DOMNodeListFunc(exec, DOMNodeListFunc::Item, 1);
  }
  else {
    // array index ?
    bool ok;
    long unsigned int idx = p.toULong(&ok);
    if (ok)
      result = getDOMNode(exec,list.item(idx));
    else {
      DOM::HTMLElement e;
      unsigned long l = list.length();
      bool found = false;

      for ( unsigned long i = 0; i < l; i++ )
        if ( ( e = list.item( i ) ).id() == p.string() ) {
          result = getDOMNode(exec, list.item( i ) );
          found = true;
          break;
        }

      if ( !found )
        result = ObjectImp::get(exec, p);
    }
  }

  return result;
}

DOMNodeListFunc::DOMNodeListFunc(ExecState *exec, int i, int len)
  : DOMFunction(), id(i)
{
  Value protect(this);
  put(exec,"length",Number(len),DontDelete|ReadOnly|DontEnum);
}

// Not a prototype class currently, but should probably be converted to one
Value DOMNodeListFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  DOM::NodeList list = static_cast<DOMNodeList *>(thisObj.imp())->nodeList();
  Value result;

  if (id == Item)
    result = getDOMNode(exec,list.item(args[0].toNumber(exec).intValue()));
  return result;
}

// -------------------------------------------------------------------------

const ClassInfo DOMAttr::info = { "Attr", &DOMNode::info, &DOMAttrTable, 0 };

/*
@begin DOMAttrTable 5
  name		DOMAttr::Name		DontDelete|ReadOnly
  specified	DOMAttr::Specified	DontDelete|ReadOnly
  value		DOMAttr::ValueProperty	DontDelete|ReadOnly
  ownerElement	DOMAttr::OwnerElement	DontDelete|ReadOnly
@end
*/
Value DOMAttr::tryGet(ExecState *exec, const UString &propertyName) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMAttr::tryPut " << propertyName.qstring() << endl;
#endif
  return DOMObjectLookupGetValue<DOMAttr,DOMNode>(exec, propertyName,
                                                  &DOMAttrTable, this );
}

Value DOMAttr::getValue(ExecState *exec, int token) const
{
  switch (token) {
  case Name:
    return getString(static_cast<DOM::Attr>(node).name());
  case Specified:
    return Boolean(static_cast<DOM::Attr>(node).specified());
  case ValueProperty:
    return getString(static_cast<DOM::Attr>(node).value());
  case OwnerElement: // DOM2
    return getDOMNode(exec,static_cast<DOM::Attr>(node).ownerElement());
  }
  return Value(); // not reached
}

void DOMAttr::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMAttr::tryPut " << propertyName.qstring() << endl;
#endif
  DOMObjectLookupPut<DOMAttr,DOMNode>(exec, propertyName, value, attr,
                                      &DOMAttrTable, this );
}

void DOMAttr::putValue(ExecState *exec, int token, const Value& value, int /*attr*/)
{
  switch (token) {
  case ValueProperty:
    static_cast<DOM::Attr>(node).setValue(value.toString(exec).value().string());
    return;
  default:
    kdWarning() << "DOMAttr::putValue unhandled token " << token << endl;
  }
}

// -------------------------------------------------------------------------

/*
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
#  importNode           DOMDocument::ImportNode                 DontDelete|Function ?
  createElementNS      DOMDocument::CreateElementNS            DontDelete|Function 2
  createAttributeNS    DOMDocument::CreateAttributeNS          DontDelete|Function 2
  getElementsByTagNameNS  DOMDocument::GetElementsByTagNameNS  DontDelete|Function 2
  getElementById     DOMDocument::GetElementById               DontDelete|Function 1
  createRange        DOMDocument::CreateRange                  DontDelete|Function 0
  createNodeIterator DOMDocument::CreateNodeIterator           DontDelete|Function 3
  createTreeWalker   DOMDocument::CreateTreeWalker             DontDelete|Function 4
#  defaultView        DOMDocument::DefaultView                  DontDelete|Function ?
  createEvent        DOMDocument::CreateEvent                  DontDelete|Function 1
  styleSheets        DOMDocument::StyleSheets                  DontDelete|Function 0
  getOverrideStyle   DOMDocument::GetOverrideStyle             DontDelete|Function 2
@end
*/
IMPLEMENT_PROTOFUNC(DOMDocumentProtoFunc)
IMPLEMENT_PROTOTYPE("DOMDocument", DOMDocumentProto, DOMDocumentProtoFunc, DOMNodeProto)

const ClassInfo DOMDocument::info = { "Document", &DOMNode::info, &DOMDocumentTable, 0 };

/*
@begin DOMDocumentTable 3
  doctype         DOMDocument::DocType                         DontDelete|ReadOnly
  implementation  DOMDocument::Implementation                  DontDelete|ReadOnly
  documentElement DOMDocument::DocumentElement                 DontDelete|ReadOnly
@end
*/

DOMDocument::DOMDocument(ExecState *exec, DOM::Document d)
  : DOMNode(DOMDocumentProto::self(exec), d) { }

DOMDocument::DOMDocument(Object proto, DOM::Document d)
  : DOMNode(proto, d) { }

Value DOMDocument::tryGet(ExecState *exec, const UString &propertyName) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMDocument::tryGet " << propertyName.qstring() << endl;
#endif
  return DOMObjectLookupGetValue<DOMDocument, DOMNode>(
    exec, propertyName, &DOMDocumentTable, this);
}

Value DOMDocument::getValue(ExecState *exec, int token) const
{
  DOM::Document doc = static_cast<DOM::Document>(node);

  switch(token) {
  case DocType:
    return getDOMNode(exec,doc.doctype());
  case Implementation:
    return getDOMDOMImplementation(exec,doc.implementation());
  case DocumentElement:
    return getDOMNode(exec,doc.documentElement());
  default:
    kdWarning() << "DOMDocument::getValue unhandled token " << token << endl;
    return Value();
  }
}

Value DOMDocumentProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  DOM::Node node = static_cast<DOMNode *>( thisObj.imp() )->toNode();
  DOM::Document doc = static_cast<DOM::Document>(node);
  Value result;
  String str = args[0].toString(exec);
  DOM::DOMString s = str.value().string();

  switch(id) {
  case DOMDocument::CreateElement:
    result = getDOMNode(exec,doc.createElement(s));
    break;
  case DOMDocument::CreateDocumentFragment:
    result = getDOMNode(exec,doc.createDocumentFragment());
    break;
  case DOMDocument::CreateTextNode:
    result = getDOMNode(exec,doc.createTextNode(s));
    break;
  case DOMDocument::CreateComment:
    result = getDOMNode(exec,doc.createComment(s));
    break;
  case DOMDocument::CreateCDATASection:
    result = getDOMNode(exec,doc.createCDATASection(s));  /* TODO: okay ? */
    break;
  case DOMDocument::CreateProcessingInstruction:
    result = getDOMNode(exec,doc.createProcessingInstruction(args[0].toString(exec).value().string(),
                                                                 args[1].toString(exec).value().string()));
    break;
  case DOMDocument::CreateAttribute:
    result = getDOMNode(exec,doc.createAttribute(s));
    break;
  case DOMDocument::CreateEntityReference:
    result = getDOMNode(exec,doc.createEntityReference(args[0].toString(exec).value().string()));
    break;
  case DOMDocument::GetElementsByTagName:
    result = getDOMNodeList(exec,doc.getElementsByTagName(s));
    break;
    /* TODO */
//  case DOMDocument::ImportNode: // new for DOM2 - not yet in khtml
  case DOMDocument::CreateElementNS: // new for DOM2
    result = getDOMNode(exec,doc.createElementNS(args[0].toString(exec).value().string(),args[1].toString(exec).value().string()));
    break;
  case DOMDocument::CreateAttributeNS: // new for DOM2
    result = getDOMNode(exec,doc.createAttributeNS(args[0].toString(exec).value().string(),args[1].toString(exec).value().string()));
    break;
/*  case DOMDocument::GetElementsByTagNameNS: // new for DOM2 - not yet in khtml
  case DOMDocument::GetElementById: // new for DOM2 - not yet in khtml*/
  case DOMDocument::CreateRange:
    result = getDOMRange(doc.createRange());
    break;
  case DOMDocument::CreateNodeIterator:
    if (args[2].isA(NullType)) {
        DOM::NodeFilter filter;
	result = getDOMNodeIterator(doc.createNodeIterator(toNode(args[0]),(long unsigned int)(args[1].toNumber(exec).value()),
				    filter,args[3].toBoolean(exec).value()));
    }
    else {
      Object obj = Object::dynamicCast(args[2]);
      if (!obj.isNull())
      {
	DOM::CustomNodeFilter *customFilter = new JSNodeFilter(obj);
	DOM::NodeFilter filter = DOM::NodeFilter::createCustom(customFilter);
	result = getDOMNodeIterator(
          doc.createNodeIterator(
            toNode(args[0]),(long unsigned int)(args[1].toNumber(exec).value()),
            filter,args[3].toBoolean(exec).value()));
      }// else?
    }
    break;
  case DOMDocument::CreateTreeWalker:
    result = getDOMTreeWalker(doc.createTreeWalker(toNode(args[0]),(long unsigned int)(args[1].toNumber(exec).value()),
             toNodeFilter(args[2]),args[3].toBoolean(exec).value()));
    break;
  case DOMDocument::CreateEvent:
    result = getDOMEvent(doc.createEvent(s));
    break;
  case DOMDocument::GetOverrideStyle: {
      DOM::Node arg0 = toNode(args[0]);
      if (arg0.nodeType() != DOM::Node::ELEMENT_NODE)
        result = Undefined(); // throw exception?
      else
        result = getDOMCSSStyleDeclaration(doc.getOverrideStyle(static_cast<DOM::Element>(arg0),args[1].toString(exec).value().string()));
    }
    break;
// case DOMDocument::DefaultView // TODO
// case DOMDocument::StyleSheets // TODO
  default:
    result = Undefined();
    break;
  }

  return result;
}

// -------------------------------------------------------------------------

/*
@begin DOMElementProtoTable 11
  getAttribute		DOMElement::GetAttribute	DontDelete|Function 1
  setAttribute		DOMElement::SetAttribute	DontDelete|Function 2
  removeAttribute	DOMElement::RemoveAttribute	DontDelete|Function 1
  getAttributeNode	DOMElement::GetAttributeNode	DontDelete|Function 1
  setAttributeNode	DOMElement::SetAttributeNode	DontDelete|Function 2
  removeAttributeNode	DOMElement::RemoveAttributeNode	DontDelete|Function 1
  getElementsByTagName	DOMElement::GetElementsByTagName	DontDelete|Function 1
  hasAttribute		DOMElement::HasAttribute	DontDelete|Function 1
# this is moved to Node in DOM2
  normalize		DOMElement::Normalize		DontDelete|Function 0
#### new for DOM2 - not yet in khtml
#  getAttributeNS	DOMElement::GetAttributeNS	DontDelete|Function 2?
#  setAttributeNS	DOMElement::SetAttributeNS	DontDelete|Function 3?
#  removeAttributeNS	DOMElement::RemoveAttributeNS	DontDelete|Function 2?
#  getAttributeNodeNS	DOMElement::GetAttributeNodeNS	DontDelete|Function 2?
#  setAttributeNodeNS	DOMElement::SetAttributeNodeNS	DontDelete|Function 3?
#  removeAttributeNodeNS DOMElement::RemoveAttributeNodeNS	DontDelete|Function 2?
#  getElementsByTagNameNS DOMElement::GetElementsByTagNameNS	DontDelete|Function 2?
#  hasAttributeNS	DOMElement::HasAttributeNS	DontDelete|Function 2?
@end
*/
IMPLEMENT_PROTOFUNC(DOMElementProtoFunc)
IMPLEMENT_PROTOTYPE("DOMElement",DOMElementProto,DOMElementProtoFunc, DOMNodeProto)

const ClassInfo DOMElement::info = { "Element", &DOMNode::info, &DOMElementTable, 0 };
/*
@begin DOMElementTable 3
  tagName	DOMElement::TagName                         DontDelete|ReadOnly
  style		DOMElement::Style                           DontDelete|ReadOnly
@end
*/
DOMElement::DOMElement(ExecState *exec, DOM::Element e)
  : DOMNode(DOMElementProto::self(exec), e) { }

DOMElement::DOMElement(Object proto, DOM::Element e)
  : DOMNode(proto, e) { }

Value DOMElement::tryGet(ExecState *exec, const UString &propertyName) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMElement::tryGet " << propertyName.qstring() << endl;
#endif
  DOM::Element element = static_cast<DOM::Element>(node);

  const HashEntry* entry = Lookup::findEntry(&DOMElementTable, propertyName);
  if (entry)
  {
    switch( entry->value ) {
    case TagName:
      return getString(element.tagName());
    case Style:
      return getDOMCSSStyleDeclaration(element.style());
    default:
      fprintf(stderr, "Unhandled token in DOMElement::tryGet : %d\n", entry->value);
      break;
    }
  }
  DOM::DOMString attr = element.getAttribute( propertyName.string() );
  // Give access to attributes
  if ( !attr.isNull() )
    return getString( attr );
  else
    return DOMNode::tryGet(exec, propertyName);
}

Value DOMElementProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  DOM::Node node = static_cast<DOMNode *>( thisObj.imp() )->toNode();
  DOM::Element element = static_cast<DOM::Element>(node);
  Value result;

  switch(id) {
    case DOMElement::GetAttribute:
      result = String(element.getAttribute(args[0].toString(exec).value().string()));
      break;
    case DOMElement::SetAttribute:
      element.setAttribute(args[0].toString(exec).value().string(),args[1].toString(exec).value().string());
      result = Undefined();
      break;
    case DOMElement::RemoveAttribute:
      element.removeAttribute(args[0].toString(exec).value().string());
      result = Undefined();
      break;
    case DOMElement::GetAttributeNode:
      result = getDOMNode(exec,element.getAttributeNode(args[0].toString(exec).value().string()));
      break;
    case DOMElement::SetAttributeNode:
      result = getDOMNode(exec,element.setAttributeNode((new DOMNode(exec,KJS::toNode(args[0])))->toNode()));
      break;
    case DOMElement::RemoveAttributeNode:
      result = getDOMNode(exec,element.removeAttributeNode((new DOMNode(exec,KJS::toNode(args[0])))->toNode()));
      break;
    case DOMElement::GetElementsByTagName:
      result = getDOMNodeList(exec,element.getElementsByTagName(args[0].toString(exec).value().string()));
      break;
    case DOMElement::HasAttribute: // DOM2
      result = Boolean(element.hasAttribute(args[0].toString(exec).value().string()));
      break;
    case DOMElement::Normalize: {  // this is moved to Node in DOM2
        element.normalize();
        result = Undefined();
      }
      break;
/*    case DOMElement::GetAttributeNS: // new for DOM2 - not yet in khtml
    case DOMElement::SetAttributeNS: // new for DOM2 - not yet in khtml
    case DOMElement::RemoveAttributeNS: // new for DOM2 - not yet in khtml
    case DOMElement::GetAttributeNodeNS: // new for DOM2 - not yet in khtml
    case DOMElement::SetAttributeNodeNS: // new for DOM2 - not yet in khtml
    case DOMElement::GetElementsByTagNameNS: // new for DOM2 - not yet in khtml
    case DOMElement::HasAttributeNS: // new for DOM2 - not yet in khtml*/
  default:
    result = Undefined();
  }

  return result;
}

// -------------------------------------------------------------------------

const ClassInfo DOMDOMImplementation::info = { "DOMImplementation", 0, 0, 0 };

DOMDOMImplementation::~DOMDOMImplementation()
{
  domImplementations.remove(implementation.handle());
}

Value DOMDOMImplementation::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "hasFeature")
    return new DOMDOMImplementationFunction(implementation, DOMDOMImplementationFunction::HasFeature);
//  else if (p == "createDocumentType") // new for DOM2 - not yet in khtml
//    return new DOMDOMImplementationFunction(implementation, DOMDOMImplementationFunction::CreateDocumentType);
//  else if (p == "createDocument") // new for DOM2 - not yet in khtml
//    return new DOMDOMImplementationFunction(implementation, DOMDOMImplementationFunction::CreateDocument);
  else if (p == "createCSSStyleSheet")
    return new DOMDOMImplementationFunction(implementation, DOMDOMImplementationFunction::CreateCSSStyleSheet);
  else
    return ObjectImp::get(exec, p);
}

Value DOMDOMImplementationFunction::tryCall(ExecState *exec, Object &/*thisObj*/, const List &args)
{
  Value result;

  switch(id) {
    case HasFeature:
      result = Boolean(implementation.hasFeature(args[0].toString(exec).value().string(),args[1].toString(exec).value().string()));
      break;
/*    case CreateDocumentType: // new for DOM2 - not yet in khtml
    case CreateDocument: // new for DOM2 - not yet in khtml*/
    case CreateCSSStyleSheet:
      result = getDOMStyleSheet(implementation.createCSSStyleSheet(args[0].toString(exec).value().string(),args[1].toString(exec).value().string()));
      break;
    default:
      result = Undefined();
  }

  return result;
}

// -------------------------------------------------------------------------

const ClassInfo DOMDocumentType::info = { "DocumentType", &DOMNode::info, 0, 0 };

Value DOMDocumentType::tryGet(ExecState *exec, const UString &p) const
{
  DOM::DocumentType type = static_cast<DOM::DocumentType>(node);

  if (p == "name")
    return getString(type.name());
  else if (p == "entities")
    return getDOMNamedNodeMap(exec,type.entities());
  else if (p == "notations")
    return getDOMNamedNodeMap(exec,type.notations());
//  else if (p == "publicId") // new for DOM2 - not yet in khtml
//    return getString(type.publicId());
//  else if (p == "systemId") // new for DOM2 - not yet in khtml
//    return getString(type.systemId());
//  else if (p == "internalSubset") // new for DOM2 - not yet in khtml
//    return getString(type.internalSubset());
  else
    return DOMNode::tryGet(exec, p);
}

// -------------------------------------------------------------------------

const ClassInfo DOMNamedNodeMap::info = { "NamedNodeMap", 0, 0, 0 };

DOMNamedNodeMap::~DOMNamedNodeMap()
{
  namedNodeMaps.remove(map.handle());
}

Value DOMNamedNodeMap::tryGet(ExecState* exec, const UString &p) const
{
  Value result;

  if (p == "length")
    return Number(map.length());
  else if (p == "getNamedItem")
    result = new DOMNamedNodeMapFunction(map, DOMNamedNodeMapFunction::GetNamedItem);
  else if (p == "setNamedItem")
    result = new DOMNamedNodeMapFunction(map, DOMNamedNodeMapFunction::SetNamedItem);
  else if (p == "removeNamedItem")
    result = new DOMNamedNodeMapFunction(map, DOMNamedNodeMapFunction::RemoveNamedItem);
  else if (p == "item")
    result = new DOMNamedNodeMapFunction(map, DOMNamedNodeMapFunction::Item);
//  else if (p == "getNamedItemNS") // new for DOM2 - not yet in khtml
//    result = new DOMNamedNodeMapFunction(map, DOMNamedNodeMapFunction::GetNamedItemNS);
//  else if (p == "setNamedItemNS") // new for DOM2 - not yet in khtml
//    result = new DOMNamedNodeMapFunction(map, DOMNamedNodeMapFunction::SetNamedItemNS);
//  else if (p == "removeNamedItemNS") // new for DOM2 - not yet in khtml
//    result = new DOMNamedNodeMapFunction(map, DOMNamedNodeMapFunction::RemoveNamedItemNS);
  else
    result = Undefined();

  // array index ?
  bool ok;
  long unsigned int idx = p.toULong(&ok);
  if (ok)
    result = getDOMNode(exec,map.item(idx));

  return result;
}

Value DOMNamedNodeMapFunction::tryCall(ExecState *exec, Object &/*thisObj*/, const List &args)
{
  Value result;

  switch(id) {
    case GetNamedItem:
      result = getDOMNode(exec,map.getNamedItem(args[0].toString(exec).value().string()));
      break;
    case SetNamedItem:
      result = getDOMNode(exec,map.setNamedItem((new DOMNode(exec,KJS::toNode(args[0])))->toNode()));
      break;
    case RemoveNamedItem:
      result = getDOMNode(exec,map.removeNamedItem(args[0].toString(exec).value().string()));
      break;
    case Item:
      result = getDOMNode(exec,map.item(args[0].toNumber(exec).intValue()));
      break;
/*    case GetNamedItemNS: // new for DOM2 - not yet in khtml
    case SetNamedItemNS: // new for DOM2 - not yet in khtml
    case RemoveNamedItemNS: // new for DOM2 - not yet in khtml*/
    default:
      result = Undefined();
  }

  return result;
}

// -------------------------------------------------------------------------

const ClassInfo DOMProcessingInstruction::info = { "ProcessingInstruction", &DOMNode::info, 0, 0 };

Value DOMProcessingInstruction::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "target")
    return getString(static_cast<DOM::ProcessingInstruction>(node).target());
  else if (p == "data")
    return getString(static_cast<DOM::ProcessingInstruction>(node).data());
  else if (p == "sheet")
    return getDOMStyleSheet(static_cast<DOM::ProcessingInstruction>(node).sheet());
  else
    return DOMNode::tryGet(exec, p);
}

void DOMProcessingInstruction::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr)
{
  if (propertyName == "data")
    static_cast<DOM::ProcessingInstruction>(node).setData(value.toString(exec).value().string());
  else
    DOMNode::tryPut(exec, propertyName,value,attr);
}

// -------------------------------------------------------------------------

const ClassInfo DOMNotation::info = { "Notation", &DOMNode::info, 0, 0 };

Value DOMNotation::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "publicId")
    return getString(static_cast<DOM::Notation>(node).publicId());
  else if (p == "systemId")
    return getString(static_cast<DOM::Notation>(node).systemId());
  else
    return DOMNode::tryGet(exec, p);
}

// -------------------------------------------------------------------------

const ClassInfo DOMEntity::info = { "Entity", &DOMNode::info, 0, 0 };

Value DOMEntity::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "publicId")
    return getString(static_cast<DOM::Entity>(node).publicId());
  else if (p == "systemId")
    return getString(static_cast<DOM::Entity>(node).systemId());
  else if (p == "notationName")
    return getString(static_cast<DOM::Entity>(node).notationName());
  else
    return DOMNode::tryGet(exec, p);
}

// -------------------------------------------------------------------------

Value KJS::getDOMNode(ExecState *exec, DOM::Node n)
{
  DOMNode *ret = 0;
  if (n.isNull())
    return Null();
  if ((ret = nodes[n.handle()]))
    return ret;

  switch (n.nodeType()) {
    case DOM::Node::ELEMENT_NODE:
      if (static_cast<DOM::Element>(n).isHTMLElement())
        ret = new HTMLElement(exec, static_cast<DOM::HTMLElement>(n));
      else
        ret = new DOMElement(exec, static_cast<DOM::Element>(n));
      break;
    case DOM::Node::ATTRIBUTE_NODE:
      ret = new DOMAttr(exec, static_cast<DOM::Attr>(n));
      break;
    case DOM::Node::TEXT_NODE:
    case DOM::Node::CDATA_SECTION_NODE:
      ret = new DOMText(exec, static_cast<DOM::Text>(n));
      break;
    case DOM::Node::ENTITY_REFERENCE_NODE:
      ret = new DOMNode(exec, n);
      break;
    case DOM::Node::ENTITY_NODE:
      ret = new DOMEntity(exec, static_cast<DOM::Entity>(n));
      break;
    case DOM::Node::PROCESSING_INSTRUCTION_NODE:
      ret = new DOMProcessingInstruction(exec, static_cast<DOM::ProcessingInstruction>(n));
      break;
    case DOM::Node::COMMENT_NODE:
      ret = new DOMCharacterData(exec, static_cast<DOM::CharacterData>(n));
      break;
    case DOM::Node::DOCUMENT_NODE:
      if (static_cast<DOM::Document>(n).isHTMLDocument())
        ret = new HTMLDocument(exec, static_cast<DOM::HTMLDocument>(n));
      else
        ret = new DOMDocument(exec, static_cast<DOM::Document>(n));
      fprintf(stderr, "created new html/dom document\n");
      break;
    case DOM::Node::DOCUMENT_TYPE_NODE:
      ret = new DOMDocumentType(exec, static_cast<DOM::DocumentType>(n));
      break;
    case DOM::Node::DOCUMENT_FRAGMENT_NODE:
      ret = new DOMNode(exec, n);
      break;
    case DOM::Node::NOTATION_NODE:
      ret = new DOMNotation(exec, static_cast<DOM::Notation>(n));
      break;
    default:
      ret = new DOMNode(exec, n);
  }
  nodes.insert(n.handle(),ret);

  return ret;
}

Value KJS::getDOMNamedNodeMap(ExecState *exec, DOM::NamedNodeMap m)
{
  DOMNamedNodeMap *ret;
  if (m.isNull())
    return Null();
  else if ((ret = namedNodeMaps[m.handle()]))
    return ret;
  else {
    ret = new DOMNamedNodeMap(exec, m);
    namedNodeMaps.insert(m.handle(),ret);
    return ret;
  }
}

Value KJS::getDOMNodeList(ExecState *exec, DOM::NodeList l)
{
  DOMNodeList *ret;
  if (l.isNull())
    return Null();
  else if ((ret = nodeLists[l.handle()]))
    return ret;
  else {
    ret = new DOMNodeList(exec, l);
    nodeLists.insert(l.handle(),ret);
    return ret;
  }
}

Value KJS::getDOMDOMImplementation(ExecState *exec, DOM::DOMImplementation i)
{
  DOMDOMImplementation *ret;
  if (i.isNull())
    return Null();
  else if ((ret = domImplementations[i.handle()]))
    return ret;
  else {
    ret = new DOMDOMImplementation(exec, i);
    domImplementations.insert(i.handle(),ret);
    return ret;
  }
}

// -------------------------------------------------------------------------

const ClassInfo NodePrototype::info = { "NodePrototype", 0, 0, 0 };

Value NodePrototype::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "ELEMENT_NODE")
    return Number((unsigned int)DOM::Node::ELEMENT_NODE);
  if (p == "ATTRIBUTE_NODE")
    return Number((unsigned int)DOM::Node::ATTRIBUTE_NODE);
  if (p == "TEXT_NODE")
    return Number((unsigned int)DOM::Node::TEXT_NODE);
  if (p == "CDATA_SECTION_NODE")
    return Number((unsigned int)DOM::Node::CDATA_SECTION_NODE);
  if (p == "ENTITY_REFERENCE_NODE")
    return Number((unsigned int)DOM::Node::ENTITY_REFERENCE_NODE);
  if (p == "ENTITY_NODE")
    return Number((unsigned int)DOM::Node::ENTITY_NODE);
  if (p == "PROCESSING_INSTRUCTION_NODE")
    return Number((unsigned int)DOM::Node::PROCESSING_INSTRUCTION_NODE);
  if (p == "COMMENT_NODE")
    return Number((unsigned int)DOM::Node::COMMENT_NODE);
  if (p == "DOCUMENT_NODE")
    return Number((unsigned int)DOM::Node::DOCUMENT_NODE);
  if (p == "DOCUMENT_TYPE_NODE")
    return Number((unsigned int)DOM::Node::DOCUMENT_TYPE_NODE);
  if (p == "DOCUMENT_FRAGMENT_NODE")
    return Number((unsigned int)DOM::Node::DOCUMENT_FRAGMENT_NODE);
  if (p == "NOTATION_NODE")
    return Number((unsigned int)DOM::Node::NOTATION_NODE);

  return DOMObject::tryGet(exec, p);
}

Value KJS::getNodePrototype(ExecState *exec)
{
  Value proto = exec->interpreter()->globalObject().get(exec, "[[node.prototype]]");
  if (!proto.isNull())
    return proto;
  else
  {
    Value nodeProto( new NodePrototype );
    exec->interpreter()->globalObject().put(exec, "[[node.prototype]]", nodeProto);
    return nodeProto;
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMExceptionPrototype::info = { "DOMExceptionPrototype", 0, 0, 0 };

Value DOMExceptionPrototype::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "INDEX_SIZE_ERR")
    return Number((unsigned int)DOM::DOMException::INDEX_SIZE_ERR);
  if (p == "DOMSTRING_SIZE_ERR")
    return Number((unsigned int)DOM::DOMException::DOMSTRING_SIZE_ERR);
  if (p == "HIERARCHY_REQUEST_ERR")
    return Number((unsigned int)DOM::DOMException::HIERARCHY_REQUEST_ERR);
  if (p == "WRONG_DOCUMENT_ERR")
    return Number((unsigned int)DOM::DOMException::WRONG_DOCUMENT_ERR);
  if (p == "INVALID_CHARACTER_ERR")
    return Number((unsigned int)DOM::DOMException::INVALID_CHARACTER_ERR);
  if (p == "NO_DATA_ALLOWED_ERR")
    return Number((unsigned int)DOM::DOMException::NO_DATA_ALLOWED_ERR);
  if (p == "NO_MODIFICATION_ALLOWED_ERR")
    return Number((unsigned int)DOM::DOMException::NO_MODIFICATION_ALLOWED_ERR);
  if (p == "NOT_FOUND_ERR")
    return Number((unsigned int)DOM::DOMException::NOT_FOUND_ERR);
  if (p == "NOT_SUPPORTED_ERR")
    return Number((unsigned int)DOM::DOMException::NOT_SUPPORTED_ERR);
  if (p == "INUSE_ATTRIBUTE_ERR")
    return Number((unsigned int)DOM::DOMException::INUSE_ATTRIBUTE_ERR);
  if (p == "INVALID_STATE_ERR")
    return Number((unsigned int)DOM::DOMException::INVALID_STATE_ERR);
  if (p == "SYNTAX_ERR")
    return Number((unsigned int)DOM::DOMException::SYNTAX_ERR);
  if (p == "INVALID_MODIFICATION_ERR")
    return Number((unsigned int)DOM::DOMException::INVALID_MODIFICATION_ERR);
  if (p == "NAMESPACE_ERR")
    return Number((unsigned int)DOM::DOMException::NAMESPACE_ERR);
  if (p == "INVALID_ACCESS_ERR")
    return Number((unsigned int)DOM::DOMException::INVALID_ACCESS_ERR);

  return DOMObject::tryGet(exec, p);
}

Value KJS::getDOMExceptionPrototype(ExecState *exec)
{
  Value proto = exec->interpreter()->globalObject().get(exec, "[[DOMException.prototype]]");
  if (!proto.isNull())
    return proto;
  else
  {
    Object domExceptionProto( new DOMExceptionPrototype );
    exec->interpreter()->globalObject().put(exec, "[[DOMException.prototype]]", domExceptionProto /*, flags? */);
    return domExceptionProto;
  }

}
