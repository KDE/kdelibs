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

#ifndef _DOM_CORE_H_
#define _DOM_CORE_H_

#include <dom_node.h>
#include <dom_doc.h>
#include <dom_element.h>
#include <dom_xml.h>

#include <kjs/object.h>
#include <kjs/function.h>

#include "kjs_binding.h"

namespace KJS {

  class DOMNode : public NodeObject {
  public:
    DOMNode(DOM::Node n) : node(n) { }
    virtual KJSO tryGet(const UString &p) const;
    virtual void tryPut(const UString &p, const KJSO& v);
    virtual DOM::Node toNode() const { return node; }
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    DOM::Node node;
  };

  class DOMNodeFunc : public DOMFunction {
    friend DOMNode;
  public:
    DOMNodeFunc(DOM::Node n, int i) : node(n), id(i) { }
    Completion tryExecute(const List &);
    enum { InsertBefore, ReplaceChild, RemoveChild, AppendChild,
	   HasChildNodes, CloneNode };
  private:
    DOM::Node node;
    int id;
  };

  class DOMNodeList : public DOMObject {
  public:
    DOMNodeList(DOM::NodeList l) : list(l) { }
    virtual KJSO tryGet(const UString &p) const;
    // no put - all read-only
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    DOM::NodeList list;
  };

  class DOMNodeListFunc : public DOMFunction {
    friend DOMNodeList;
  public:
    DOMNodeListFunc(DOM::NodeList l, int i) : list(l), id(i) { }
    Completion tryExecute(const List &);
    enum { Item };
  private:
    DOM::NodeList list;
    int id;
  };

  class DOMDocument : public NodeObject {
  public:
    DOMDocument(DOM::Document d) : doc(d) { }
    virtual KJSO tryGet(const UString &p) const;
    virtual DOM::Node toNode() const { return doc; }
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    DOM::Document doc;
  };

  class DOMDocFunction : public DOMFunction {
  public:
    DOMDocFunction(DOM::Document d, int i);
    Completion tryExecute(const List &);
    enum { CreateElement, CreateDocumentFragment, CreateTextNode,
	   CreateComment, CreateCDATASection, CreateProcessingInstruction,
	   CreateAttribute, CreateEntityReference, GetElementsByTagName,
	   ImportNode, CreateElementNS, CreateAttributeNS, GetElementsByTagNameNS, GetElementById };
  private:
    DOM::Document doc;
    int id;
  };

  class DOMAttr : public NodeObject {
  public:
    DOMAttr(DOM::Attr a) : attr(a) { }
    virtual KJSO tryGet(const UString &p) const;
    virtual void tryPut(const UString &p, const KJSO& v);
    virtual DOM::Node toNode() const { return attr; }
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    DOM::Attr attr;
  };

  class DOMElement : public NodeObject {
  public:
    DOMElement(DOM::Element e) : element(e) { }
    virtual KJSO tryGet(const UString &p) const;
    // no put - all read-only
    virtual DOM::Node toNode() const { return element; }
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    DOM::Element element;
  };

  class DOMElementFunction : public DOMFunction {
  public:
    DOMElementFunction(DOM::Element e, int i);
    Completion tryExecute(const List &);
    enum { GetAttribute, SetAttribute, RemoveAttribute, GetAttributeNode,
           SetAttributeNode, RemoveAttributeNode, GetElementsByTagName,
           GetAttributeNS, SetAttributeNS, RemoveAttributeNS, GetAttributeNodeNS,
           SetAttributeNodeNS, GetElementsByTagNameNS, HasAttribute, HasAttributeNS };
  private:
    DOM::Element element;
    int id;
  };

  class DOMDOMImplementation : public DOMObject {
  public:
    DOMDOMImplementation(DOM::DOMImplementation i) : implementation(i) { }
    virtual KJSO tryGet(const UString &p) const;
    // no put - all functions
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    DOM::DOMImplementation implementation;
  };

  class DOMDOMImplementationFunction : public DOMFunction {
  public:
    DOMDOMImplementationFunction(DOM::DOMImplementation impl, int i);
    Completion tryExecute(const List &);
    enum { HasFeature, CreateDocumentType, CreateDocument };
  private:
    DOM::DOMImplementation implementation;
    int id;
  };

  class DOMDocumentType : public NodeObject {
  public:
    DOMDocumentType(DOM::DocumentType dt) : type(dt) { }
    virtual KJSO tryGet(const UString &p) const;
    // no put - all read-only
    virtual DOM::Node toNode() const { return type; }
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    DOM::DocumentType type;
  };

  class DOMNamedNodeMap : public DOMObject {
  public:
    DOMNamedNodeMap(DOM::NamedNodeMap m) : map(m) { }
    virtual KJSO tryGet(const UString &p) const;
    // no put - all read-only
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    DOM::NamedNodeMap map;
  };

  class DOMNamedNodeMapFunction : public DOMFunction {
  public:
    DOMNamedNodeMapFunction(DOM::NamedNodeMap m, int i);
    Completion tryExecute(const List &);
    enum { GetNamedItem, SetNamedItem, RemoveNamedItem, Item,
           GetNamedItemNS, SetNamedItemNS, RemoveNamedItemNS };
  private:
    DOM::NamedNodeMap map;
    int id;
  };

  class DOMProcessingInstruction : public NodeObject {
  public:
    DOMProcessingInstruction(DOM::ProcessingInstruction pi) : instruction(pi) { }
    virtual KJSO tryGet(const UString &p) const;
    virtual void tryPut(const UString &p, const KJSO& v);
    virtual DOM::Node toNode() const { return instruction; }
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    DOM::ProcessingInstruction instruction;
  };

  class DOMNotation : public NodeObject {
  public:
    DOMNotation(DOM::Notation n) : notation(n) { }
    virtual KJSO tryGet(const UString &p) const;
    // no put - all read-only
    virtual DOM::Node toNode() const { return notation; }
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    DOM::Notation notation;
  };

  class DOMEntity : public NodeObject {
  public:
    DOMEntity(DOM::Entity e) : entity(e) { }
    virtual KJSO tryGet(const UString &p) const;
    // no put - all read-only
    virtual DOM::Node toNode() const { return entity; }
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    DOM::Entity entity;
  };

}; // namespace

#endif
