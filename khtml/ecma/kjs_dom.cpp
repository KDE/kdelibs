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
/* Source for DOMNodeProtoTable. Use "make hashtables" to regenerate.
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
DEFINE_PROTOTYPE("DOMNode",DOMNodeProto)
IMPLEMENT_PROTOFUNC(DOMNodeProtoFunc)
IMPLEMENT_PROTOTYPE(DOMNodeProto,DOMNodeProtoFunc)

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

bool DOMNode::toBoolean(ExecState *) const
{
    return !node.isNull();
}

/* Source for DOMNodeTable. Use "make hashtables" to regenerate.
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
      kdWarning() << "Unhandled token in DOMNode::getValue : " << token << endl;
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
    node.setNodeValue(value.toString(exec).string());
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

  return String(toString(exec));
}

UString DOMNode::toString(ExecState *) const
{
  if (node.isNull())
    return "null";
  UString s;

  DOM::Element e = node;
  if ( !e.isNull() ) {
    s = e.nodeName().string();
  } else
    s = className(); // fallback

  return "[object " + s + "]";
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
      result = getDOMNode(exec,node.cloneNode(args[0].toBoolean(exec)));
      break;
    case DOMNode::AddEventListener: {
//        JSEventListener *listener = new JSEventListener(args[1]); // will get deleted when the node derefs it
        JSEventListener *listener = Window::retrieveActive(exec)->getJSEventListener(args[1]);
        node.addEventListener(args[0].toString(exec).string(),listener,args[2].toBoolean(exec));
        result = Undefined();
      }
      break;
    case DOMNode::RemoveEventListener: {
        JSEventListener *listener = Window::retrieveActive(exec)->getJSEventListener(args[1]);
        node.removeEventListener(args[0].toString(exec).string(),listener,args[2].toBoolean(exec));
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

// We have to implement hasProperty since we don't use a hashtable for 'length' and 'item'
// ## this breaks "for (..in..)" though.
bool DOMNodeList::hasProperty(ExecState *exec, const UString &p, bool recursive) const
{
  if (p == "length" || p == "item")
    return true;
  return ObjectImp::hasProperty(exec,p,recursive);
}

Value DOMNodeList::tryGet(ExecState *exec, const UString &p) const
{
  Value result;

  if (p == "length")
    result = Number(list.length());
  else if (p == "item") {
    // No need for a complete hashtable for a single func, but we still want
    // to use the caching feature of lookupOrCreateFunction.
    result = lookupOrCreateFunction<DOMNodeListFunc>(exec, p, this, DOMNodeListFunc::Item, 1, DontDelete|Function);
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
    result = getDOMNode(exec, list.item(args[0].toInt32(exec)));
  return result;
}

// -------------------------------------------------------------------------

const ClassInfo DOMAttr::info = { "Attr", &DOMNode::info, &DOMAttrTable, 0 };

/* Source for DOMAttrTable. Use "make hashtables" to regenerate.
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
    static_cast<DOM::Attr>(node).setValue(value.toString(exec).string());
    return;
  default:
    kdWarning() << "DOMAttr::putValue unhandled token " << token << endl;
  }
}

// -------------------------------------------------------------------------

/* Source for DOMDocumentProtoTable. Use "make hashtables" to regenerate.
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
  getOverrideStyle   DOMDocument::GetOverrideStyle             DontDelete|Function 2
@end
*/
DEFINE_PROTOTYPE("DOMDocument", DOMDocumentProto)
IMPLEMENT_PROTOFUNC(DOMDocumentProtoFunc)
IMPLEMENT_PROTOTYPE_WITH_PARENT(DOMDocumentProto, DOMDocumentProtoFunc, DOMNodeProto)

const ClassInfo DOMDocument::info = { "Document", &DOMNode::info, &DOMDocumentTable, 0 };

/* Source for DOMDocumentTable. Use "make hashtables" to regenerate.
@begin DOMDocumentTable 4
  doctype         DOMDocument::DocType                         DontDelete|ReadOnly
  implementation  DOMDocument::Implementation                  DontDelete|ReadOnly
  documentElement DOMDocument::DocumentElement                 DontDelete|ReadOnly
  styleSheets     DOMDocument::StyleSheets                     DontDelete|ReadOnly
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
  case StyleSheets:
    //kdDebug() << "DOMDocument::StyleSheets, returning " << doc.styleSheets().length() << " stylesheets" << endl;
    return getDOMStyleSheetList(exec, doc.styleSheets(), doc);
  default:
    kdWarning() << "DOMDocument::getValue unhandled token " << token << endl;
    return Value();
  }
}

Value DOMDocumentProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  DOM::Node node = static_cast<DOMNode *>( thisObj.imp() )->toNode();
  DOM::Document doc = static_cast<DOM::Document>(node);
  String str = args[0].toString(exec);
  DOM::DOMString s = str.value().string();

  switch(id) {
  case DOMDocument::CreateElement:
    return getDOMNode(exec,doc.createElement(s));
  case DOMDocument::CreateDocumentFragment:
    return getDOMNode(exec,doc.createDocumentFragment());
  case DOMDocument::CreateTextNode:
    return getDOMNode(exec,doc.createTextNode(s));
  case DOMDocument::CreateComment:
    return getDOMNode(exec,doc.createComment(s));
  case DOMDocument::CreateCDATASection:
    return getDOMNode(exec,doc.createCDATASection(s));  /* TODO: okay ? */
  case DOMDocument::CreateProcessingInstruction:
    return getDOMNode(exec,doc.createProcessingInstruction(args[0].toString(exec).string(),
                                                                 args[1].toString(exec).string()));
  case DOMDocument::CreateAttribute:
    return getDOMNode(exec,doc.createAttribute(s));
  case DOMDocument::CreateEntityReference:
    return getDOMNode(exec,doc.createEntityReference(args[0].toString(exec).string()));
  case DOMDocument::GetElementsByTagName:
    return getDOMNodeList(exec,doc.getElementsByTagName(s));
    /* TODO */
//  case DOMDocument::ImportNode: // new for DOM2 - not yet in khtml
  case DOMDocument::CreateElementNS: // new for DOM2
    return getDOMNode(exec,doc.createElementNS(args[0].toString(exec).string(), args[1].toString(exec).string()));
  case DOMDocument::CreateAttributeNS: // new for DOM2
    return getDOMNode(exec,doc.createAttributeNS(args[0].toString(exec).string(),args[1].toString(exec).string()));
/*  case DOMDocument::GetElementsByTagNameNS: // new for DOM2 - not yet in khtml
  case DOMDocument::GetElementById: // new for DOM2 - not yet in khtml*/
  case DOMDocument::CreateRange:
    return getDOMRange(exec,doc.createRange());
  case DOMDocument::CreateNodeIterator:
    if (args[2].isA(NullType)) {
        DOM::NodeFilter filter;
        return getDOMNodeIterator(exec,
                                  doc.createNodeIterator(toNode(args[0]),
                                                         (long unsigned int)(args[1].toNumber(exec)),
                                                         filter,args[3].toBoolean(exec)));
    }
    else {
      Object obj = Object::dynamicCast(args[2]);
      if (!obj.isNull())
      {
        DOM::CustomNodeFilter *customFilter = new JSNodeFilter(obj);
        DOM::NodeFilter filter = DOM::NodeFilter::createCustom(customFilter);
        return getDOMNodeIterator(exec,
          doc.createNodeIterator(
            toNode(args[0]),(long unsigned int)(args[1].toNumber(exec)),
            filter,args[3].toBoolean(exec)));
      }// else?
    }
  case DOMDocument::CreateTreeWalker:
    return getDOMTreeWalker(exec,doc.createTreeWalker(toNode(args[0]),(long unsigned int)(args[1].toNumber(exec)),
             toNodeFilter(args[2]),args[3].toBoolean(exec)));
  case DOMDocument::CreateEvent:
    return getDOMEvent(exec,doc.createEvent(s));
  case DOMDocument::GetOverrideStyle: {
      DOM::Node arg0 = toNode(args[0]);
      if (arg0.nodeType() != DOM::Node::ELEMENT_NODE)
        return Undefined(); // throw exception?
      else
        return getDOMCSSStyleDeclaration(exec,doc.getOverrideStyle(static_cast<DOM::Element>(arg0),args[1].toString(exec).string()));
    }
// case DOMDocument::DefaultView // TODO
  default:
    break;
  }

  return Undefined();
}

// -------------------------------------------------------------------------

/* Source for DOMElementProtoTable. Use "make hashtables" to regenerate.
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
DEFINE_PROTOTYPE("DOMElement",DOMElementProto)
IMPLEMENT_PROTOFUNC(DOMElementProtoFunc)
IMPLEMENT_PROTOTYPE_WITH_PARENT(DOMElementProto,DOMElementProtoFunc,DOMNodeProto)

const ClassInfo DOMElement::info = { "Element", &DOMNode::info, &DOMElementTable, 0 };
/* Source for DOMElementTable. Use "make hashtables" to regenerate.
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
      return getDOMCSSStyleDeclaration(exec,element.style());
    default:
      kdWarning() << "Unhandled token in DOMElement::tryGet : " << entry->value << endl;
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

  switch(id) {
    case DOMElement::GetAttribute:
      return String(element.getAttribute(args[0].toString(exec).string()));
    case DOMElement::SetAttribute:
      element.setAttribute(args[0].toString(exec).string(),args[1].toString(exec).string());
      return Undefined();
    case DOMElement::RemoveAttribute:
      element.removeAttribute(args[0].toString(exec).string());
      return Undefined();
    case DOMElement::GetAttributeNode:
      return getDOMNode(exec,element.getAttributeNode(args[0].toString(exec).string()));
    case DOMElement::SetAttributeNode:
      return getDOMNode(exec,element.setAttributeNode((new DOMNode(exec,KJS::toNode(args[0])))->toNode()));
    case DOMElement::RemoveAttributeNode:
      return getDOMNode(exec,element.removeAttributeNode((new DOMNode(exec,KJS::toNode(args[0])))->toNode()));
    case DOMElement::GetElementsByTagName:
      return getDOMNodeList(exec,element.getElementsByTagName(args[0].toString(exec).string()));
    case DOMElement::HasAttribute: // DOM2
      return Boolean(element.hasAttribute(args[0].toString(exec).string()));
    case DOMElement::Normalize: // this is moved to Node in DOM2
      element.normalize();
      return Undefined();
/*    case DOMElement::GetAttributeNS: // new for DOM2 - not yet in khtml
    case DOMElement::SetAttributeNS: // new for DOM2 - not yet in khtml
    case DOMElement::RemoveAttributeNS: // new for DOM2 - not yet in khtml
    case DOMElement::GetAttributeNodeNS: // new for DOM2 - not yet in khtml
    case DOMElement::SetAttributeNodeNS: // new for DOM2 - not yet in khtml
    case DOMElement::GetElementsByTagNameNS: // new for DOM2 - not yet in khtml
    case DOMElement::HasAttributeNS: // new for DOM2 - not yet in khtml*/
  default:
    return Undefined();
  }
}

// -------------------------------------------------------------------------

/* Source for DOMDOMImplementationProtoTable. Use "make hashtables" to regenerate.
@begin DOMDOMImplementationProtoTable 3
  hasFeature		DOMDOMImplementation::HasFeature		DontDelete|Function 2
  createCSSStyleSheet	DOMDOMImplementation::CreateCSSStyleSheet	DontDelete|Function 2
###new for DOM2 - not yet in khtml
#  createDocumentType	DOMDOMImplementation::CreateDocumentType	DontDelete|Function ?
#  createDocument	DOMDOMImplementation::CreateDocument		DontDelete|Function ?
@end
*/
DEFINE_PROTOTYPE("DOMImplementation",DOMDOMImplementationProto)
IMPLEMENT_PROTOFUNC(DOMDOMImplementationProtoFunc)
IMPLEMENT_PROTOTYPE(DOMDOMImplementationProto,DOMDOMImplementationProtoFunc)

const ClassInfo DOMDOMImplementation::info = { "DOMImplementation", 0, 0, 0 };

DOMDOMImplementation::DOMDOMImplementation(ExecState *exec, DOM::DOMImplementation i)
  : DOMObject(DOMDOMImplementationProto::self(exec)), implementation(i) { }

DOMDOMImplementation::~DOMDOMImplementation()
{
  domImplementations.remove(implementation.handle());
}

Value DOMDOMImplementationProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  DOM::DOMImplementation implementation = static_cast<DOMDOMImplementation *>( thisObj.imp() )->toImplementation();

  switch(id) {
    case DOMDOMImplementation::HasFeature:
      return Boolean(implementation.hasFeature(args[0].toString(exec).string(),args[1].toString(exec).string()));
/*    case DOMDOMImplementation::CreateDocumentType: // new for DOM2 - not yet in khtml
    case DOMDOMImplementation::CreateDocument: // new for DOM2 - not yet in khtml*/
    case DOMDOMImplementation::CreateCSSStyleSheet:
      return getDOMStyleSheet(exec,implementation.createCSSStyleSheet(args[0].toString(exec).string(),args[1].toString(exec).string()));
    default:
      break;
  }

  return Undefined();
}

// -------------------------------------------------------------------------

const ClassInfo DOMDocumentType::info = { "DocumentType", &DOMNode::info, &DOMDocumentTypeTable, 0 };

/* Source for DOMDocumentTypeTable. Use "make hashtables" to regenerate.
@begin DOMDocumentTypeTable 3
  name			DOMDocumentType::Name		DontDelete|ReadOnly
  entities		DOMDocumentType::Entities	DontDelete|ReadOnly
  notations		DOMDocumentType::Notations	DontDelete|ReadOnly
###new for DOM2 - not yet in khtml
#  publicId		DOMDocumentType::PublicId	DontDelete|ReadOnly
#  systemId		DOMDocumentType::SystemId	DontDelete|ReadOnly
#  internalSubset	DOMDocumentType::InternalSubset	DontDelete|ReadOnly
@end
*/
DOMDocumentType::DOMDocumentType(ExecState *exec, DOM::DocumentType dt)
  : DOMNode( /*### no proto yet*/exec, dt ) { }

Value DOMDocumentType::tryGet(ExecState *exec, const UString &propertyName) const
{
  return DOMObjectLookupGetValue<DOMDocumentType, DOMNode>(exec, propertyName, &DOMDocumentTypeTable, this);
}

Value DOMDocumentType::getValue(ExecState *exec, int token) const
{
  DOM::DocumentType type = static_cast<DOM::DocumentType>(node);
  switch (token) {
  case Name:
    return getString(type.name());
  case Entities:
    return getDOMNamedNodeMap(exec,type.entities());
  case Notations:
    return getDOMNamedNodeMap(exec,type.notations());
//  case PublicId: // new for DOM2 - not yet in khtml
//    return getString(type.publicId());
//  case SystemId: // new for DOM2 - not yet in khtml
//    return getString(type.systemId());
//  case InternalSubset: // new for DOM2 - not yet in khtml
//    return getString(type.internalSubset());
  default:
    kdWarning() << "DOMDocumentType::getValue unhandled token " << token << endl;
    return Value();
  }
}

// -------------------------------------------------------------------------

/* Source for DOMNamedNodeMapProtoTable. Use "make hashtables" to regenerate.
@begin DOMNamedNodeMapProtoTable 5
  getNamedItem		DOMNamedNodeMap::GetNamedItem		DontDelete|Function 1
  setNamedItem		DOMNamedNodeMap::SetNamedItem		DontDelete|Function 1
  removeNamedItem	DOMNamedNodeMap::RemoveNamedItem	DontDelete|Function 1
  item			DOMNamedNodeMap::Item			DontDelete|Function 1
### new for DOM2 - not yet in khtml
#  getNamedItemNS	DOMNamedNodeMap::GetNamedItemNS		DontDelete|Function 1
#  setNamedItemNS	DOMNamedNodeMap::SetNamedItemNS		DontDelete|Function 1
#  removeNamedItemNS	DOMNamedNodeMap::RemoveNamedItemNS	DontDelete|Function 1
@end
*/
DEFINE_PROTOTYPE("NamedNodeMap", DOMNamedNodeMapProto)
IMPLEMENT_PROTOFUNC(DOMNamedNodeMapProtoFunc)
IMPLEMENT_PROTOTYPE(DOMNamedNodeMapProto,DOMNamedNodeMapProtoFunc)

const ClassInfo DOMNamedNodeMap::info = { "NamedNodeMap", 0, 0, 0 };

DOMNamedNodeMap::DOMNamedNodeMap(ExecState *exec, DOM::NamedNodeMap m)
  : DOMObject(DOMNamedNodeMapProto::self(exec)), map(m) { }

DOMNamedNodeMap::~DOMNamedNodeMap()
{
  namedNodeMaps.remove(map.handle());
}

// We have to implement hasProperty since we don't use a hashtable for 'length'
// ## this breaks "for (..in..)" though.
bool DOMNamedNodeMap::hasProperty(ExecState *exec, const UString &p, bool recursive) const
{
  if (p == "length")
    return true;
  return DOMObject::hasProperty(exec,p,recursive);
}

Value DOMNamedNodeMap::tryGet(ExecState* exec, const UString &p) const
{
  if (p == "length")
    return Number(map.length());

  // array index ?
  bool ok;
  long unsigned int idx = p.toULong(&ok);
  if (ok)
    return getDOMNode(exec,map.item(idx));

  // Anything else (including functions, defined in the prototype)
  return DOMObject::tryGet(exec, p);
}

Value DOMNamedNodeMapProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  DOM::NamedNodeMap map = static_cast<DOMNamedNodeMap *>(thisObj.imp())->toMap();

  switch(id) {
    case DOMNamedNodeMap::GetNamedItem:
      return getDOMNode(exec,map.getNamedItem(args[0].toString(exec).string()));
    case DOMNamedNodeMap::SetNamedItem:
      return getDOMNode(exec,map.setNamedItem((new DOMNode(exec,KJS::toNode(args[0])))->toNode()));
    case DOMNamedNodeMap::RemoveNamedItem:
      return getDOMNode(exec,map.removeNamedItem(args[0].toString(exec).string()));
    case DOMNamedNodeMap::Item:
      return getDOMNode(exec, map.item(args[0].toInt32(exec)));
/*    case DOMNamedNodeMap::GetNamedItemNS: // new for DOM2 - not yet in khtml
    case DOMNamedNodeMap::SetNamedItemNS: // new for DOM2 - not yet in khtml
    case DOMNamedNodeMap::RemoveNamedItemNS: // new for DOM2 - not yet in khtml*/
    default:
      break;
  }

  return Undefined();
}

// -------------------------------------------------------------------------

const ClassInfo DOMProcessingInstruction::info = { "ProcessingInstruction", &DOMNode::info, &DOMProcessingInstructionTable, 0 };

/* Source for DOMProcessingInstructionTable. Use "make hashtables" to regenerate.
@begin DOMProcessingInstructionTable 3
  target	DOMProcessingInstruction::Target	DontDelete|ReadOnly
  data		DOMProcessingInstruction::Data		DontDelete
  sheet		DOMProcessingInstruction::Sheet		DontDelete|ReadOnly
@end
*/
Value DOMProcessingInstruction::tryGet(ExecState *exec, const UString &propertyName) const
{
  return DOMObjectLookupGetValue<DOMProcessingInstruction, DOMNode>(exec, propertyName, &DOMProcessingInstructionTable, this);
}

Value DOMProcessingInstruction::getValue(ExecState *exec, int token) const
{
  switch (token) {
  case Target:
    return getString(static_cast<DOM::ProcessingInstruction>(node).target());
  case Data:
    return getString(static_cast<DOM::ProcessingInstruction>(node).data());
  case Sheet:
    return getDOMStyleSheet(exec,static_cast<DOM::ProcessingInstruction>(node).sheet());
  default:
    kdWarning() << "DOMProcessingInstruction::getValue unhandled token " << token << endl;
    return Value();
  }
}

void DOMProcessingInstruction::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr)
{
  // Not worth using the hashtable for this one ;)
  if (propertyName == "data")
    static_cast<DOM::ProcessingInstruction>(node).setData(value.toString(exec).string());
  else
    DOMNode::tryPut(exec, propertyName,value,attr);
}

// -------------------------------------------------------------------------

const ClassInfo DOMNotation::info = { "Notation", &DOMNode::info, &DOMNotationTable, 0 };

/* Source for DOMNotationTable. Use "make hashtables" to regenerate.
@begin DOMNotationTable 2
  publicId		DOMNotation::PublicId	DontDelete|ReadOnly
  systemId		DOMNotation::SystemId	DontDelete|ReadOnly
@end
*/
Value DOMNotation::tryGet(ExecState *exec, const UString &propertyName) const
{
  return DOMObjectLookupGetValue<DOMNotation, DOMNode>(exec, propertyName, &DOMNotationTable, this);
}

Value DOMNotation::getValue(ExecState *, int token) const
{
  switch (token) {
  case PublicId:
    return getString(static_cast<DOM::Notation>(node).publicId());
  case SystemId:
    return getString(static_cast<DOM::Notation>(node).systemId());
  default:
    kdWarning() << "DOMNotation::getValue unhandled token " << token << endl;
    return Value();
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMEntity::info = { "Entity", &DOMNode::info, 0, 0 };

/* Source for DOMEntityTable. Use "make hashtables" to regenerate.
@begin DOMEntityTable 2
  publicId		DOMEntity::PublicId		DontDelete|ReadOnly
  systemId		DOMEntity::SystemId		DontDelete|ReadOnly
  notationName		DOMEntity::NotationName	DontDelete|ReadOnly
@end
*/
Value DOMEntity::tryGet(ExecState *exec, const UString &propertyName) const
{
  return DOMObjectLookupGetValue<DOMEntity, DOMNode>(exec, propertyName, &DOMEntityTable, this);
}

Value DOMEntity::getValue(ExecState *, int token) const
{
  switch (token) {
  case PublicId:
    return getString(static_cast<DOM::Entity>(node).publicId());
  case SystemId:
    return getString(static_cast<DOM::Entity>(node).systemId());
  case NotationName:
    return getString(static_cast<DOM::Entity>(node).notationName());
  default:
    kdWarning() << "DOMEntity::getValue unhandled token " << token << endl;
    return Value();
  }
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

const ClassInfo NodeConstructor::info = { "NodeConstructor", 0, &NodeConstructorTable, 0 };
/* Source for NodeConstructorTable. Use "make hashtables" to regenerate.
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
Value NodeConstructor::tryGet(ExecState *exec, const UString &propertyName) const
{
  return DOMObjectLookupGetValue<NodeConstructor, DOMObject>(exec, propertyName, &NodeConstructorTable, this);
}

Value NodeConstructor::getValue(ExecState *, int token) const
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
    kdWarning() << "NodeConstructor::getValue unhandled token " << token << endl;
    return Value();
  }
#endif
}

Object KJS::getNodeConstructor(ExecState *exec)
{
  return cacheGlobalObject<NodeConstructor>(exec, "[[node.constructor]]");
}

// -------------------------------------------------------------------------

const ClassInfo DOMExceptionConstructor::info = { "DOMExceptionConstructor", 0, 0, 0 };

/* Source for DOMExceptionConstructorTable. Use "make hashtables" to regenerate.
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

Value DOMExceptionConstructor::tryGet(ExecState *exec, const UString &propertyName) const
{
  return DOMObjectLookupGetValue<DOMExceptionConstructor, DOMObject>(exec, propertyName, &DOMExceptionConstructorTable, this);
}

Value DOMExceptionConstructor::getValue(ExecState *, int token) const
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
    kdWarning() << "DOMExceptionConstructor::getValue unhandled token " << token << endl;
    return Value();
  }
#endif
}

Object KJS::getDOMExceptionConstructor(ExecState *exec)
{
  return cacheGlobalObject<DOMExceptionConstructor>(exec, "[[DOMException.constructor]]");
}

// Such a collection is usually very short-lived, it only exists
// for constructs like document.forms.<name>[1],
// so it shouldn't be a problem that it's storing all the nodes (with the same name). (David)
DOMNamedNodesCollection::DOMNamedNodesCollection(ExecState *, QValueList<DOM::Node>& nodes )
  : DOMObject(), m_nodes(nodes)
{
  // Maybe we should ref (and deref in the dtor) the nodes, though ?
}

Value DOMNamedNodesCollection::tryGet(ExecState *exec, const UString &propertyName) const
{
  if (propertyName == "length")
    return Number(m_nodes.count());
  // index?
  bool ok;
  unsigned int u = propertyName.toULong(&ok);
  if (ok) {
    DOM::Node node = m_nodes[u];
    return getDOMNode(exec,node);
  }
  return DOMObject::tryGet(exec,propertyName);
}
