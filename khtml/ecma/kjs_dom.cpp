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

using namespace KJS;

QPtrDict<DOMNode> nodes(1021);
QPtrDict<DOMNamedNodeMap> namedNodeMaps;
QPtrDict<DOMNodeList> nodeLists;
QPtrDict<DOMDOMImplementation> domImplementations;

// -------------------------------------------------------------------------

const ClassInfo DOMNode::info = { "Node", 0, 0, 0 };

DOMNode::~DOMNode()
{
  nodes.remove(node.handle());
}

Boolean DOMNode::toBoolean(ExecState *) const
{
    return Boolean(!node.isNull());
}

bool DOMNode::hasProperty(ExecState *exec, const UString &p, bool recursive) const
{
  if (p == "nodeName" || p == "nodeValue" || p == "nodeType" ||
      p == "parentNode" || p == "childNodes" || p == "firstChild" ||
      p == "lastChild" || p == "previousSibling" || p == "nextSibling" ||
      p == "attributes" ||
      /* new for DOM2 - not yet in khtml
      p == "namespaceURI" || p == "prefix" || p == "localName" || */
      p == "ownerDocument" || p == "insertBefore" || p == "replaceChild" ||
      p == "removeChild" || p == "appendChild" || p == "hasChildNodes" ||
      p == "cloneNode" || p == "hasAttributes" ||
      /* moved here from Element in DOM2
      p == "normalize"  || p == "supports" */
      // no DOM standard, found in IE only
      p == "offsetLeft" || p == "offsetTop" || p == "offsetWidth" || p == "offsetHeight" ||
      p == "offsetParent" || p == "parentElement" ||
      p == "scrollLeft" || p == "scrollTop" || p == "addEventListener" ||
      p == "removeEventListener" || p == "dispatchEvent")
    return true;

  return recursive && ObjectImp::hasProperty(exec, p, true);
}

Value DOMNode::tryGet(ExecState *exec, const UString &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMNode::tryGet " << p.qstring().latin1() << endl;
#endif
  Value result;
  khtml::RenderObject *rend = node.handle() ? node.handle()->renderer() : 0L;

  if (p == "nodeName")
    result = getString(node.nodeName());
  else if (p == "nodeValue")
    result = getString(node.nodeValue());
  else if (p == "nodeType")
    result = Number((unsigned int)node.nodeType());
  else if (p == "parentNode")
    result = getDOMNode(node.parentNode());
  else if (p == "parentElement") // IE only apparently
    result = getDOMNode(node.parentNode());
  else if (p == "childNodes")
    result = getDOMNodeList(node.childNodes());
  else if (p == "firstChild")
    result = getDOMNode(node.firstChild());
  else if (p == "lastChild")
    result = getDOMNode(node.lastChild());
  else if (p == "previousSibling")
    result = getDOMNode(node.previousSibling());
  else if (p == "nextSibling")
    result = getDOMNode(node.nextSibling());
  else if (p == "attributes")
    result = getDOMNamedNodeMap(node.attributes());
//  else if (p == "namespaceURI") // new for DOM2 - not yet in khtml
//    result = getString(node.namespaceURI());
//  else if (p == "prefix") // new for DOM2 - not yet in khtml
//    result = getString(node.prefix());
//  else if (p == "localName") // new for DOM2 - not yet in khtml
//    result = getString(node.localName());
  else if (p == "ownerDocument")
    result = getDOMNode(node.ownerDocument());
  // methods
  else if (p == "insertBefore")
    result = new DOMNodeFunc(node, DOMNodeFunc::InsertBefore);
  else if (p == "replaceChild")
    result = new DOMNodeFunc(node, DOMNodeFunc::ReplaceChild);
  else if (p == "removeChild")
    result = new DOMNodeFunc(node, DOMNodeFunc::RemoveChild);
  else if (p == "appendChild")
    result = new DOMNodeFunc(node, DOMNodeFunc::AppendChild);
  else if (p == "hasAttributes") // DOM2
    result = new DOMNodeFunc(node, DOMNodeFunc::HasAttributes);
  else if (p == "hasChildNodes")
    result = new DOMNodeFunc(node, DOMNodeFunc::HasChildNodes);
  else if (p == "cloneNode")
    result = new DOMNodeFunc(node, DOMNodeFunc::CloneNode);
//  else if (p == "normalize") // moved here from Element in DOM2
//    result = new DOMNodeFunc(node, DOMNodeFunc::Normalize);
//  else if (p == "supports") // new for DOM2 - not yet in khtml
//    result = new DOMNodeFunc(node, DOMNodeFunc::Supports);
  else if (p == "addEventListener") // from the EventTarget interface
    result = new DOMNodeFunc(node, DOMNodeFunc::AddEventListener);
  else if (p == "removeEventListener") // from the EventTarget interface
    result = new DOMNodeFunc(node, DOMNodeFunc::RemoveEventListener);
  else if (p == "dispatchEvent") // from the EventTarget interface
    result = new DOMNodeFunc(node, DOMNodeFunc::DispatchEvent);
  else if (p == "contains")
    result = new DOMNodeFunc(node, DOMNodeFunc::Contains);
  else if (p == "onabort")
    result = getListener(DOM::EventImpl::ABORT_EVENT);
  else if (p == "onblur")
    result = getListener(DOM::EventImpl::BLUR_EVENT);
  else if (p == "onchange")
    result = getListener(DOM::EventImpl::CHANGE_EVENT);
  else if (p == "onclick")
    result = getListener(DOM::EventImpl::KHTML_CLICK_EVENT);
  else if (p == "ondblclick")
    result = getListener(DOM::EventImpl::KHTML_DBLCLICK_EVENT);
  else if (p == "ondragdrop")
    result = getListener(DOM::EventImpl::KHTML_DRAGDROP_EVENT);
  else if (p == "onerror")
    result = getListener(DOM::EventImpl::KHTML_ERROR_EVENT);
  else if (p == "onfocus")
    result = getListener(DOM::EventImpl::FOCUS_EVENT);
  else if (p == "onkeydown")
    result = getListener(DOM::EventImpl::KHTML_KEYDOWN_EVENT);
  else if (p == "onkeypress")
    result = getListener(DOM::EventImpl::KHTML_KEYPRESS_EVENT);
  else if (p == "onkeyup")
    result = getListener(DOM::EventImpl::KHTML_KEYUP_EVENT);
  else if (p == "onload")
    result = getListener(DOM::EventImpl::LOAD_EVENT);
  else if (p == "onmousedown")
    result = getListener(DOM::EventImpl::MOUSEDOWN_EVENT);
  else if (p == "onmousemove")
    result = getListener(DOM::EventImpl::MOUSEMOVE_EVENT);
  else if (p == "onmouseout")
    result = getListener(DOM::EventImpl::MOUSEOUT_EVENT);
  else if (p == "onmouseover")
    result = getListener(DOM::EventImpl::MOUSEOVER_EVENT);
  else if (p == "onmouseup")
    result = getListener(DOM::EventImpl::MOUSEUP_EVENT);
  else if (p == "onmove")
    result = getListener(DOM::EventImpl::KHTML_MOVE_EVENT);
  else if (p == "onreset")
    result = getListener(DOM::EventImpl::RESET_EVENT);
  else if (p == "onresize")
    result = getListener(DOM::EventImpl::RESIZE_EVENT);
  else if (p == "onselect")
    result = getListener(DOM::EventImpl::SELECT_EVENT);
  else if (p == "onsubmit")
    result = getListener(DOM::EventImpl::SUBMIT_EVENT);
  else if (p == "onunload")
    result = getListener(DOM::EventImpl::UNLOAD_EVENT);
  else {
    // no DOM standard, found in IE only

    // make sure our rendering is up to date before
    // we allow a query on these attributes.
    // ### how often does it fall into the final else case ?
    if ( node.handle() && node.handle()->ownerDocument() )
      node.handle()->ownerDocument()->updateRendering();

    if (p == "offsetLeft")
      result = rend ? static_cast<Value>(Number(rend->xPos())) : Value(Undefined());
    else if (p == "offsetTop")
      result = rend ? static_cast<Value>(Number(rend->yPos())) : Value(Undefined());
    else if (p == "offsetWidth")
      result = rend ? static_cast<Value>(Number(rend->width()) ) : Value(Undefined());
    else if (p == "offsetHeight")
      result = rend ? static_cast<Value>(Number(rend->height() ) ) : Value(Undefined());
    else if (p == "offsetParent")
      result = getDOMNode(node.parentNode()); // not necessarily correct
    else if (p == "clientWidth")
      result = rend ? static_cast<Value>(Number(rend->contentWidth())) : Value(Undefined());
    else if (p == "clientHeight")
      result = rend ? static_cast<Value>(Number(rend->contentHeight())) : Value(Undefined());
    else if (p == "scrollLeft")
      result = rend ? static_cast<Value>(Number(-rend->xPos() + node.ownerDocument().view()->contentsX())) : Value(Undefined());
    else if (p == "scrollTop")
      result = rend ? static_cast<Value>(Number(-rend->yPos() + node.ownerDocument().view()->contentsY())) : Value(Undefined());
    else
      result = ObjectImp::get(exec, p);
  }

  return result;
}

void DOMNode::tryPut(ExecState *exec, const UString &p, const Value& value, int attr)
{
  if (p == "nodeValue") {
    node.setNodeValue(value.toString(exec).value().string());
  }
//  else if (p == "prefix") { // new for DOM2 - not yet in khtml
//    node.setPrefix(v.toString().value().string());
//  }
  else if (p == "onabort")
    setListener(exec,DOM::EventImpl::ABORT_EVENT,value);
  else if (p == "onblur")
    setListener(exec,DOM::EventImpl::BLUR_EVENT,value);
  else if (p == "onchange")
    setListener(exec,DOM::EventImpl::CHANGE_EVENT,value);
  else if (p == "onclick")
    setListener(exec,DOM::EventImpl::KHTML_CLICK_EVENT,value);
  else if (p == "ondblclick")
    setListener(exec,DOM::EventImpl::KHTML_DBLCLICK_EVENT,value);
  else if (p == "ondragdrop")
    setListener(exec,DOM::EventImpl::KHTML_DRAGDROP_EVENT,value);
  else if (p == "onerror")
    setListener(exec,DOM::EventImpl::KHTML_ERROR_EVENT,value);
  else if (p == "onfocus")
    setListener(exec,DOM::EventImpl::FOCUS_EVENT,value);
  else if (p == "onkeydown")
    setListener(exec,DOM::EventImpl::KHTML_KEYDOWN_EVENT,value);
  else if (p == "onkeypress")
    setListener(exec,DOM::EventImpl::KHTML_KEYPRESS_EVENT,value);
  else if (p == "onkeyup")
    setListener(exec,DOM::EventImpl::KHTML_KEYUP_EVENT,value);
  else if (p == "onload")
    setListener(exec,DOM::EventImpl::LOAD_EVENT,value);
  else if (p == "onmousedown")
    setListener(exec,DOM::EventImpl::MOUSEDOWN_EVENT,value);
  else if (p == "onmousemove")
    setListener(exec,DOM::EventImpl::MOUSEMOVE_EVENT,value);
  else if (p == "onmouseout")
    setListener(exec,DOM::EventImpl::MOUSEOUT_EVENT,value);
  else if (p == "onmouseover")
    setListener(exec,DOM::EventImpl::MOUSEOVER_EVENT,value);
  else if (p == "onmouseup")
    setListener(exec,DOM::EventImpl::MOUSEUP_EVENT,value);
  else if (p == "onmove")
    setListener(exec,DOM::EventImpl::KHTML_MOVE_EVENT,value);
  else if (p == "onreset")
    setListener(exec,DOM::EventImpl::RESET_EVENT,value);
  else if (p == "onresize")
    setListener(exec,DOM::EventImpl::RESIZE_EVENT,value);
  else if (p == "onselect")
    setListener(exec,DOM::EventImpl::SELECT_EVENT,value);
  else if (p == "onsubmit")
    setListener(exec,DOM::EventImpl::SUBMIT_EVENT,value);
  else if (p == "onunload")
    setListener(exec,DOM::EventImpl::UNLOAD_EVENT,value);
  else
    ObjectImp::put(exec, p, value, attr);
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

List *DOMNode::eventHandlerScope() const
{
  return 0;
}

Value DOMNodeFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;
  switch (id) {
    case HasAttributes:
      result = Boolean(node.hasAttributes());
      break;
    case HasChildNodes:
      result = Boolean(node.hasChildNodes());
      break;
    case CloneNode:
      result = getDOMNode(node.cloneNode(args[0].toBoolean(exec).value()));
      break;
    case AddEventListener: {
//        JSEventListener *listener = new JSEventListener(args[1]); // will get deleted when the node derefs it
        JSEventListener *listener = Window::retrieveActive(exec)->getJSEventListener(args[1]);
        node.addEventListener(args[0].toString(exec).value().string(),listener,args[2].toBoolean(exec).value());
        result = Undefined();
      }
      break;
    case RemoveEventListener: {
        JSEventListener *listener = Window::retrieveActive(exec)->getJSEventListener(args[1]);
        node.removeEventListener(args[0].toString(exec).value().string(),listener,args[2].toBoolean(exec).value());
        result = Undefined();
      }
      break;
    case DispatchEvent:
      result = Boolean(node.dispatchEvent(toEvent(args[0])));
      break;
    case AppendChild:
      result = getDOMNode(node.appendChild(toNode(args[0])));
      break;
    case RemoveChild:
      result = getDOMNode(node.removeChild(toNode(args[0])));
      break;
    case InsertBefore:
      result = getDOMNode(node.insertBefore(toNode(args[0]), toNode(args[1])));
      break;
    case ReplaceChild:
      result = getDOMNode(node.replaceChild(toNode(args[0]), toNode(args[1])));
      break;
    case Contains:
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
  else if (p == "item")
    result = new DOMNodeListFunc(list, DOMNodeListFunc::Item);
  else {
    // array index ?
    bool ok;
    long unsigned int idx = p.toULong(&ok);
    if (ok)
      result = getDOMNode(list.item(idx));
    else {
      DOM::HTMLElement e;
      unsigned long l = list.length();
      bool found = false;

      for ( unsigned long i = 0; i < l; i++ )
        if ( ( e = list.item( i ) ).id() == p.string() ) {
          result = getDOMNode( list.item( i ) );
          found = true;
          break;
        }

      if ( !found )
        result = ObjectImp::get(exec, p);
    }
  }

  return result;
}

Value DOMNodeListFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;

  if (id == Item)
    result = getDOMNode(list.item(args[0].toNumber(exec).intValue()));
  return result;
}

// -------------------------------------------------------------------------

const ClassInfo DOMAttr::info = { "Attr", &DOMNode::info, 0, 0 };

Value DOMAttr::tryGet(ExecState *exec, const UString &p) const
{
  Value result;
  if (p == "name") {
    result = getString(static_cast<DOM::Attr>(node).name()); }
  else if (p == "specified")
    result = Boolean(static_cast<DOM::Attr>(node).specified());
  else if (p == "value")
    result = getString(static_cast<DOM::Attr>(node).value());
//  else if (p == "ownerElement") // new for DOM2 - not yet in khtml
//    rseult = getDOMNode(static_cast<DOM::Attr>(node).ownerElement());
  else
    result = DOMNode::tryGet(exec, p);

  return result;
}

void DOMAttr::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr)
{
  if (propertyName == "value")
    static_cast<DOM::Attr>(node).setValue(value.toString(exec).value().string());
  else
    DOMNode::tryPut(exec,propertyName,value,attr);
}

// -------------------------------------------------------------------------

const ClassInfo DOMDocument::info = { "Document", &DOMNode::info, 0, 0 };

bool DOMDocument::hasProperty(ExecState *exec, const UString &p, bool recursive) const
{
  if (p == "doctype" || p == "implementation" || p == "documentElement" ||
      p == "createElement" || p == "createDocumentFragment" ||
      p == "createTextNode" || p == "createComment" ||
      p == "createCDATASection" || p == "createProcessingInstruction" ||
      p == "createAttribute" || p == "createEntityReference" ||
      p == "getElementsByTagName" ||
      /* new for DOM2 - not yet in khtml
      p == "importNode" || p == "createElementNS" ||
      p == "createAttributeNS" || p == "getElementsByTagNameNS" ||
      p == "getElementById" ) || */
      p == "createRange" || p == "createNodeIterator" ||
      p == "createTreeWalker" || p == "defaultView" || p == "createEvent" ||
      p == "styleSheets" || p == "getOverrideStyle")
    return true;

  return recursive && DOMNode::hasProperty(exec, p, true);
}

Value DOMDocument::tryGet(ExecState *exec, const UString &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMDocument::tryGet " << p.qstring().latin1() << endl;
#endif
  DOM::Document doc = static_cast<DOM::Document>(node);

  if (p == "doctype")
    return getDOMNode(doc.doctype());
  if (p == "implementation")
    return getDOMDOMImplementation(doc.implementation());
  else if (p == "documentElement")
    return getDOMNode(doc.documentElement());
  // methods
  else if (p == "createElement")
    return new DOMDocFunction(doc, DOMDocFunction::CreateElement);
  else if (p == "createDocumentFragment")
    return new DOMDocFunction(doc, DOMDocFunction::CreateDocumentFragment);
  else if (p == "createTextNode")
    return new DOMDocFunction(doc, DOMDocFunction::CreateTextNode);
  else if (p == "createComment")
    return new DOMDocFunction(doc, DOMDocFunction::CreateComment);
  else if (p == "createCDATASection")
    return new DOMDocFunction(doc, DOMDocFunction::CreateCDATASection);
  else if (p == "createProcessingInstruction")
    return new DOMDocFunction(doc, DOMDocFunction::CreateProcessingInstruction);
  else if (p == "createAttribute")
    return new DOMDocFunction(doc, DOMDocFunction::CreateAttribute);
  else if (p == "createEntityReference")
    return new DOMDocFunction(doc, DOMDocFunction::CreateEntityReference);
  else if (p == "getElementsByTagName")
    return new DOMDocFunction(doc, DOMDocFunction::GetElementsByTagName);
//  else if (p == "importNode") // new for DOM2 - not yet in khtml
//    return new DOMDocFunction(doc, DOMDocFunction::ImportNode);
  else if (p == "createElementNS") // new for DOM2 - not yet in khtml
    return new DOMDocFunction(doc, DOMDocFunction::CreateElementNS);
  else if (p == "createAttributeNS") // new for DOM2 - not yet in khtml
    return new DOMDocFunction(doc, DOMDocFunction::CreateAttributeNS);
/*  else if (p == "getElementsByTagNameNS") // new for DOM2 - not yet in khtml
    return new DOMDocFunction(doc, DOMDocFunction::GetElementsByTagNameNS);
  else if (p == "getElementById") // new for DOM2 - not yet in khtml
    return new DOMDocFunction(doc, DOMDocFunction::GetElementById);*/
  else if (p == "createRange")
    return new DOMDocFunction(doc, DOMDocFunction::CreateRange);
  else if (p == "createNodeIterator")
    return new DOMDocFunction(doc, DOMDocFunction::CreateNodeIterator);
  else if (p == "createTreeWalker")
    return new DOMDocFunction(doc, DOMDocFunction::CreateTreeWalker);
  else if (p == "defaultView")
    return getDOMAbstractView(doc.defaultView());
  else if (p == "createEvent")
    return new DOMDocFunction(doc, DOMDocFunction::CreateEvent);
  else if (p == "styleSheets")
    return getDOMStyleSheetList(doc.styleSheets());
  else if (p == "getOverrideStyle")
    return new DOMDocFunction(doc, DOMDocFunction::GetOverrideStyle);

  return DOMNode::tryGet(exec, p);
}


Value DOMDocFunction::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;
  String str = args[0].toString(exec);
  DOM::DOMString s = str.value().string();

  switch(id) {
  case CreateElement:
    result = getDOMNode(doc.createElement(s));
    break;
  case CreateDocumentFragment:
    result = getDOMNode(doc.createDocumentFragment());
    break;
  case CreateTextNode:
    result = getDOMNode(doc.createTextNode(s));
    break;
  case CreateComment:
    result = getDOMNode(doc.createComment(s));
    break;
  case CreateCDATASection:
    result = getDOMNode(doc.createCDATASection(s));  /* TODO: okay ? */
    break;
  case CreateProcessingInstruction:
    result = getDOMNode(doc.createProcessingInstruction(args[0].toString(exec).value().string(),
                                                                 args[1].toString(exec).value().string()));
    break;
  case CreateAttribute:
    result = getDOMNode(doc.createAttribute(s));
    break;
  case CreateEntityReference:
    result = getDOMNode(doc.createEntityReference(args[0].toString(exec).value().string()));
    break;
  case GetElementsByTagName:
    result = getDOMNodeList(doc.getElementsByTagName(s));
    break;
    /* TODO */
//  case ImportNode: // new for DOM2 - not yet in khtml
  case CreateElementNS: // new for DOM2
    result = getDOMNode(doc.createElementNS(args[0].toString(exec).value().string(),args[1].toString(exec).value().string()));
    break;
  case CreateAttributeNS: // new for DOM2
    result = getDOMNode(doc.createAttributeNS(args[0].toString(exec).value().string(),args[1].toString(exec).value().string()));
    break;
/*  case GetElementsByTagNameNS: // new for DOM2 - not yet in khtml
  case GetElementById: // new for DOM2 - not yet in khtml*/
  case CreateRange:
    result = getDOMRange(doc.createRange());
    break;
  case CreateNodeIterator:
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
  case CreateTreeWalker:
    result = getDOMTreeWalker(doc.createTreeWalker(toNode(args[0]),(long unsigned int)(args[1].toNumber(exec).value()),
             toNodeFilter(args[2]),args[3].toBoolean(exec).value()));
    break;
  case CreateEvent:
    result = getDOMEvent(doc.createEvent(s));
    break;
  case GetOverrideStyle: {
      DOM::Node arg0 = toNode(args[0]);
      if (arg0.nodeType() != DOM::Node::ELEMENT_NODE)
        result = Undefined(); // throw exception?
      else
        result = getDOMCSSStyleDeclaration(doc.getOverrideStyle(static_cast<DOM::Element>(arg0),args[1].toString(exec).value().string()));
    }
    break;
  default:
    result = Undefined();
    break;
  }

  return result;
}

// -------------------------------------------------------------------------

const ClassInfo DOMElement::info = { "Element", &DOMNode::info, 0, 0 };

// No hasProperty for DOMElement ??

Value DOMElement::tryGet(ExecState *exec, const UString &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMElement::tryGet " << p.qstring() << endl;
#endif
  DOM::Element element = static_cast<DOM::Element>(node);

  if (p == "tagName")
    return getString(element.tagName());
  else if (p == "getAttribute")
    return new DOMElementFunction(element, DOMElementFunction::GetAttribute);
  else if (p == "setAttribute")
    return new DOMElementFunction(element, DOMElementFunction::SetAttribute);
  else if (p == "removeAttribute")
    return new DOMElementFunction(element, DOMElementFunction::RemoveAttribute);
  else if (p == "getAttributeNode")
    return new DOMElementFunction(element, DOMElementFunction::GetAttributeNode);
  else if (p == "setAttributeNode")
    return new DOMElementFunction(element, DOMElementFunction::SetAttributeNode);
  else if (p == "removeAttributeNode")
    return new DOMElementFunction(element, DOMElementFunction::RemoveAttributeNode);
  else if (p == "getElementsByTagName")
    return new DOMElementFunction(element, DOMElementFunction::GetElementsByTagName);
  else if (p == "normalize") // this is moved to Node in DOM2
    return new DOMElementFunction(element, DOMElementFunction::Normalize);
  else if (p == "style")
    return getDOMCSSStyleDeclaration(element.style());
//    return new DOMCSSStyleDeclaration(element.style()));
/*  else if (p == "getAttributeNS") // new for DOM2 - not yet in khtml
    return new DOMElementFunction(element, DOMElementFunction::GetAttributeNS);
  else if (p == "setAttributeNS") // new for DOM2 - not yet in khtml
    return new DOMElementFunction(element, DOMElementFunction::SetAttributeNS);
  else if (p == "removeAttributeNS") // new for DOM2 - not yet in khtml
    return new DOMElementFunction(element, DOMElementFunction::RemoveAttributeNS);
  else if (p == "getAttributeNodeNS") // new for DOM2 - not yet in khtml
    return new DOMElementFunction(element, DOMElementFunction::GetAttributeNodeNS);
  else if (p == "setAttributeNodeNS") // new for DOM2 - not yet in khtml
    return new DOMElementFunction(element, DOMElementFunction::SetAttributeNodeNS);
  else if (p == "getElementsByTagNameNS") // new for DOM2 - not yet in khtml
    return new DOMElementFunction(element, DOMElementFunction::GetElementsByTagNameNS);
  else if (p == "hasAttributeNS") // new for DOM2 - not yet in khtml
    return new DOMElementFunction(element, DOMElementFunction::HasAttributeNS);*/
  else if (p == "hasAttribute") // DOM2
    return new DOMElementFunction(element, DOMElementFunction::HasAttribute);
  else
  {
    DOM::DOMString attr = element.getAttribute( p.string() );
    // Give access to attributes
    if ( !attr.isNull() )
      return getString( attr );
    else
      return DOMNode::tryGet(exec, p);
  }
}

Value DOMElementFunction::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;

  switch(id) {
    case GetAttribute:
      result = String(element.getAttribute(args[0].toString(exec).value().string()));
      break;
    case SetAttribute:
      element.setAttribute(args[0].toString(exec).value().string(),args[1].toString(exec).value().string());
      result = Undefined();
      break;
    case RemoveAttribute:
      element.removeAttribute(args[0].toString(exec).value().string());
      result = Undefined();
      break;
    case GetAttributeNode:
      result = getDOMNode(element.getAttributeNode(args[0].toString(exec).value().string()));
      break;
    case SetAttributeNode:
      result = getDOMNode(element.setAttributeNode((new DOMNode(KJS::toNode(args[0])))->toNode()));
      break;
    case RemoveAttributeNode:
      result = getDOMNode(element.removeAttributeNode((new DOMNode(KJS::toNode(args[0])))->toNode()));
      break;
    case GetElementsByTagName:
      result = getDOMNodeList(element.getElementsByTagName(args[0].toString(exec).value().string()));
      break;
    case Normalize: {  // this is moved to Node in DOM2
        element.normalize();
        result = Undefined();
      }
      break;
/*    case GetAttributeNS: // new for DOM2 - not yet in khtml
    case SetAttributeNS: // new for DOM2 - not yet in khtml
    case RemoveAttributeNS: // new for DOM2 - not yet in khtml
    case GetAttributeNodeNS: // new for DOM2 - not yet in khtml
    case SetAttributeNodeNS: // new for DOM2 - not yet in khtml
    case GetElementsByTagNameNS: // new for DOM2 - not yet in khtml
    case HasAttributeNS: // new for DOM2 - not yet in khtml*/
    case HasAttribute: // DOM2
      result = Boolean(element.hasAttribute(args[0].toString(exec).value().string()));
      break;
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

Value DOMDOMImplementationFunction::tryCall(ExecState *exec, Object &thisObj, const List &args)
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
    return getDOMNamedNodeMap(type.entities());
  else if (p == "notations")
    return getDOMNamedNodeMap(type.notations());
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

Value DOMNamedNodeMap::tryGet(ExecState *exec, const UString &p) const
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
    result = getDOMNode(map.item(idx));

  return result;
}

Value DOMNamedNodeMapFunction::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;

  switch(id) {
    case GetNamedItem:
      result = getDOMNode(map.getNamedItem(args[0].toString(exec).value().string()));
      break;
    case SetNamedItem:
      result = getDOMNode(map.setNamedItem((new DOMNode(KJS::toNode(args[0])))->toNode()));
      break;
    case RemoveNamedItem:
      result = getDOMNode(map.removeNamedItem(args[0].toString(exec).value().string()));
      break;
    case Item:
      result = getDOMNode(map.item(args[0].toNumber(exec).intValue()));
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

Value KJS::getDOMNode(DOM::Node n)
{
  DOMNode *ret = 0;
  if (n.isNull())
    return Null();
  else if ((ret = nodes[n.handle()]))
    return ret;

  switch (n.nodeType()) {
    case DOM::Node::ELEMENT_NODE:
      if (static_cast<DOM::Element>(n).isHTMLElement())
        ret = new HTMLElement(static_cast<DOM::HTMLElement>(n));
      else
        ret = new DOMElement(static_cast<DOM::Element>(n));
      break;
    case DOM::Node::ATTRIBUTE_NODE:
      ret = new DOMAttr(static_cast<DOM::Attr>(n));
      break;
    case DOM::Node::TEXT_NODE:
    case DOM::Node::CDATA_SECTION_NODE:
      ret = new DOMText(static_cast<DOM::Text>(n));
      break;
    case DOM::Node::ENTITY_REFERENCE_NODE:
      ret = new DOMNode(n);
      break;
    case DOM::Node::ENTITY_NODE:
      ret = new DOMEntity(static_cast<DOM::Entity>(n));
      break;
    case DOM::Node::PROCESSING_INSTRUCTION_NODE:
      ret = new DOMProcessingInstruction(static_cast<DOM::ProcessingInstruction>(n));
      break;
    case DOM::Node::COMMENT_NODE:
      ret = new DOMCharacterData(static_cast<DOM::CharacterData>(n));
      break;
    case DOM::Node::DOCUMENT_NODE:
      if (static_cast<DOM::Document>(n).isHTMLDocument())
        ret = new HTMLDocument(static_cast<DOM::HTMLDocument>(n));
      else
        ret = new DOMDocument(static_cast<DOM::Document>(n));
      break;
    case DOM::Node::DOCUMENT_TYPE_NODE:
      ret = new DOMDocumentType(static_cast<DOM::DocumentType>(n));
      break;
    case DOM::Node::DOCUMENT_FRAGMENT_NODE:
      ret = new DOMNode(n);
      break;
    case DOM::Node::NOTATION_NODE:
      ret = new DOMNotation(static_cast<DOM::Notation>(n));
      break;
    default:
      ret = new DOMNode(n);
  }
  nodes.insert(n.handle(),ret);

  return ret;
}

Value KJS::getDOMNamedNodeMap(DOM::NamedNodeMap m)
{
  DOMNamedNodeMap *ret;
  if (m.isNull())
    return Null();
  else if ((ret = namedNodeMaps[m.handle()]))
    return ret;
  else {
    ret = new DOMNamedNodeMap(m);
    namedNodeMaps.insert(m.handle(),ret);
    return ret;
  }
}

Value KJS::getDOMNodeList(DOM::NodeList l)
{
  DOMNodeList *ret;
  if (l.isNull())
    return Null();
  else if ((ret = nodeLists[l.handle()]))
    return ret;
  else {
    ret = new DOMNodeList(l);
    nodeLists.insert(l.handle(),ret);
    return ret;
  }
}

Value KJS::getDOMDOMImplementation(DOM::DOMImplementation i)
{
  DOMDOMImplementation *ret;
  if (i.isNull())
    return Null();
  else if ((ret = domImplementations[i.handle()]))
    return ret;
  else {
    ret = new DOMDOMImplementation(i);
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
