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

#include <kjs/operations.h>
#include <dom_string.h>
#include <dom_xml.h>

#include "kjs_text.h"
#include "kjs_dom.h"

using namespace KJS;

const TypeInfo DOMNode::info = { "Node", HostType, 0, 0, 0 };

KJSO DOMNode::get(const UString &p) const
{
  KJSO result;

  if (p == "nodeName")
    result = String(node.nodeName());
  else if (p == "nodeValue")
    result = String(node.nodeValue());
  else if (p == "nodeType")
    result = Number(node.nodeType());
  else if (p == "parentNode")
    result = new DOMNode(node.parentNode());
  else if (p == "childNodes")
    result = new DOMNodeList(node.childNodes());
  else if (p == "firstChild")
    result = new DOMNode(node.firstChild());
  else if (p == "lastChild")
    result = new DOMNode(node.lastChild());
  else if (p == "previousSibling")
    result = new DOMNode(node.previousSibling());
  else if (p == "nextSibling")
    result = new DOMNode(node.nextSibling());
  //  else if (p == "attributes")
  //    result = new DOMNamedNodeMap(node.attributes());
  else if (p == "ownerDocument")
    result = new DOMDocument(node.ownerDocument());
  else if (p == "insertBefore")
    result = new DOMNodeFunc(node, DOMNodeFunc::InsertBefore);
  else if (p == "replaceChild")
    result = new DOMNodeFunc(node, DOMNodeFunc::ReplaceChild);
  else if (p == "removeChild")
    result = new DOMNodeFunc(node, DOMNodeFunc::RemoveChild);
  else if (p == "appendChild")
    result = new DOMNodeFunc(node, DOMNodeFunc::AppendChild);
  else if (p == "hasChildNodes")
    result = new DOMNodeFunc(node, DOMNodeFunc::HasChildNodes);
  else if (p == "cloneNode")
    result = new DOMNodeFunc(node, DOMNodeFunc::CloneNode);
  else
    result = Undefined();

  return result;
}

Completion DOMNodeFunc::execute(const List &args)
{
  KJSO result;

  if (id == HasChildNodes)
    result = Boolean(node.hasChildNodes());
  else if (id == CloneNode) {
    Boolean b = args[0].toBoolean();
    result = new DOMNode(node.cloneNode(b.value()));
  } else {
    DOM::Node n1 = toNode(args[0]);
    if (id == AppendChild) {
      result = new DOMNode(node.appendChild(n1));
    } else if (id == RemoveChild) {
      result = new DOMNode(node.removeChild(n1));
    } else {
      DOM::Node n2 = toNode(args[1]);
      if (id == InsertBefore)
	result = new DOMNode(node.insertBefore(n1, n2));
      else
	result = new DOMNode(node.replaceChild(n1, n2));
    }
  }
  return Completion(Normal, result);
}

KJSO DOMNodeList::get(const UString &p) const
{
  KJSO result;

  if (p == "length")
    result = Number(list.length());
  else
    result = Undefined();

  return result;
}

const TypeInfo DOMAttr::info = { "Attr", HostType, &DOMNode::info, 0, 0 };

KJSO DOMAttr::get(const UString &p) const
{
  KJSO result;
  if (p == "name") {
    result = String(attr.name()); }
  else if (p == "specified")
    result = Boolean(attr.specified());
  else if (p == "value")
    result = String(attr.value());
  else {
    KJSO tmp(new DOMNode(attr));
    result = tmp.get(p);
  }
  return result;
}

void DOMAttr::put(const UString &p, const KJSO& v)
{
  if (p == "value") {
    String s = v.toString();
    attr.setValue(s.value().string());
  } else {
    KJSO tmp(new DOMNode(attr));
    tmp.put(p, v);
  }
}

const TypeInfo DOMDocument::info = { "Document", HostType,
				     &DOMNode::info, 0, 0 };

KJSO DOMDocument::get(const UString &p) const
{
  KJSO result;

  if (p == "doctype")
    return Undefined(); /* TODO */
  else if (p == "implementation")
    return Undefined(); /* TODO */
  else if (p == "documentElement")
    return new DOMElement(doc.documentElement());
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

  // look in base class (Document)
  KJSO tmp(new DOMNode(doc));
  result = tmp.get(p);

  return result;
}

DOMDocFunction::DOMDocFunction(DOM::Document d, int i)
  : doc(d), id(i)
{
}

#include <html_document.h>

Completion DOMDocFunction::execute(const List &args)
{
  KJSO result;
  String str = args[0].toString();
  DOM::DOMString s = str.value().string();

  DOM::Element e;

  DOM::HTMLDocument d = DOM::HTMLDocument();
  DOM::Element e2 = d.createElement(DOM::DOMString("br"));

  switch(id) {
  case CreateElement:
    e = doc.createElement(s);
    result = new DOMElement(doc.createElement(s));
    break;
  case CreateTextNode:
    result = new DOMText(doc.createTextNode(s));
    break;
  case CreateComment:
    result = new DOMText(doc.createComment(s)); /* TODO: okay ? */
    break;
  case CreateCDATASection:
    result = new DOMText(doc.createCDATASection(s));  /* TODO: okay ? */
    break;
  case CreateAttribute:
    result = new DOMAttr(doc.createAttribute(s));
    break;
  case GetElementsByTagName:
    result = new DOMNodeList(doc.getElementsByTagName(s));
    break;
    /* TODO */
  default:
    result = Undefined();
  }

  return Completion(Normal, result);
}

const TypeInfo DOMElement::info = { "Element", HostType,
				    &DOMNode::info, 0, 0 };

KJSO DOMElement::get(const UString &p) const
{
  KJSO result;

  if (p == "tagName") {
    DOM::DOMString str = element.tagName();
    result = String(element.tagName());
  } else {
    KJSO tmp(new DOMNode(element));
    return tmp.get(p);
  }

  return result;
}
