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

#include <stdio.h>

#include <dom_string.h>

#include "kjs_dom.h"

using namespace KJS;

KJSO *DOMNode::get(const UString &p)
{
  KJSO *result;

  if (p == "nodeName")
    result = newString(node.nodeName());
  else if (p == "nodeValue")
    result = newString(node.nodeValue());
  else if (p == "nodeType")
    result = newNumber(node.nodeType());
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
  else
    result = newUndefined();

  return result;
}

KJSO *DOMNodeList::get(const UString &p)
{
  KJSO *result;

  if (p == "length")
    result = newNumber(list.length());
  else
    result = newUndefined();

  return result;
}

KJSO *DOMDocument::get(const UString &p)
{
  KJSO *result;

  // look in base class (Document)
  Ptr tmp = new DOMNode(doc);
  result = tmp->get(p);

  return result;
}

KJSO *DOMElement::get(const UString &p)
{
  KJSO *result;

  if (p == "tagName")
    result = newString(element.tagName());
  else {
    Ptr tmp = new DOMNode(element);
    return tmp->get(p);
  }

  return result;
}
