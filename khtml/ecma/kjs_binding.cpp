/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
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

#include <kjs/kjs.h>
#include <kjs/object.h>
#include <kjs/function.h>
#include <kjs/operations.h>

#include <qptrdict.h>

#include <khtml_part.h>
#include <html_element.h>
#include <html_head.h>
#include <html_inline.h>
#include <html_image.h>
#include <dom_string.h>
#include <dom_exception.h>
#include <html_misc.h>

#include "kjs_binding.h"
#include "kjs_dom.h"
#include "kjs_html.h"
#include "kjs_text.h"
#include "kjs_window.h"
#include "kjs_navigator.h"

using namespace KJS;

// ### make sure these get cleared when the corresponding nodes/objects get deleted
QPtrDict<NodeObject> nodes;
QPtrDict<DOMNamedNodeMap> namedNodeMaps;
QPtrDict<DOMNodeList> nodeLists;
QPtrDict<KJS::HTMLCollection> htmlCollections;
QPtrDict<DOMDOMImplementation> domImplementations;

KJSO DOMObject::get(const UString &p) const
{
  KJSO result;
  try {
    result = tryGet(p);
  }
  catch (DOM::DOMException e) {
    result = Undefined();
  }
  return result;
}

void DOMObject::put(const UString &p, const KJSO& v)
{
  try {
    tryPut(p,v);
  }
  catch (DOM::DOMException e) {
  }
}

KJSO DOMFunction::get(const UString &p) const
{
  KJSO result;
  try {
    result = tryGet(p);
  }
  catch (DOM::DOMException e) {
    result = Undefined();
  }
  return result;
}

Completion DOMFunction::execute(const List &args)
{
  Completion completion;
  try {
    completion = tryExecute(args);
  }
  catch (DOM::DOMException e) {
    completion = Completion(Normal,Undefined());
  }
  return completion;
}

UString::UString(const QString &d)
{
  unsigned int len = d.length();
  UChar *dat = new UChar[len];
  memcpy(dat, d.unicode(), len * sizeof(UChar));
  rep = UString::Rep::create(dat, len);
}

UString::UString(const DOM::DOMString &d)
{
  unsigned int len = d.length();
  UChar *dat = new UChar[len];
  memcpy(dat, d.unicode(), len * sizeof(UChar));
  rep = UString::Rep::create(dat, len);
}

DOM::DOMString UString::string() const
{
  return DOM::DOMString((QChar*) data(), size());
}

QString UString::qstring() const
{
  return QString((QChar*) data(), size());
}

QConstString UString::qconststring() const
{
  return QConstString((QChar*) data(), size());
}

KJSO NodeObject::toPrimitive(Type preferred) const
{
  if (preferred == HostType) {
    // return a unique handle for comparisons
    DOM::Node n = toNode();
    if (n.isNull())
      return Null();
    else
      return Number((int)n.handle());
  }

  return String("");
}

DOM::Node KJS::toNode(const KJSO& obj)
{
  if (!obj.derivedFrom("Node")) {
    //    printf("Can't convert %s to Node.\n", obj.imp()->typeInfo()->name);
    return DOM::Node();
  }

  //  printf("Converting %s to Node.\n", obj.imp()->typeInfo()->name);
  const NodeObject *dobj = static_cast<const NodeObject*>(obj.imp());
  DOM::Node n = dobj->toNode();

  return n;
}

KJSO KJS::getDOMNode(DOM::Node n)
{
  NodeObject *ret = 0;
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

bool NodeObject::equals(const KJSO& other) const
{
  if (other.derivedFrom("Node")) {
    NodeObject *otherNode = static_cast<NodeObject*>(other.imp());
    return toNode() == otherNode->toNode();
  }
  return false;
}

KJSO KJS::getDOMNamedNodeMap(DOM::NamedNodeMap m)
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

KJSO KJS::getDOMNodeList(DOM::NodeList l)
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

KJSO KJS::getHTMLCollection(DOM::HTMLCollection c)
{
  HTMLCollection *ret;
  if (c.isNull())
    return Null();
  else if ((ret = htmlCollections[c.handle()]))
    return ret;
  else {
    ret = new HTMLCollection(c);
    htmlCollections.insert(c.handle(),ret);
    return ret;
  }
}

KJSO KJS::getDOMDOMImplementation(DOM::DOMImplementation i)
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


KJSO KJS::getString(DOM::DOMString s)
{
  if (s.isNull())
    return Null();
  else
    return String(s);
}




