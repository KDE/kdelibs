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

#include <kjs/object.h>
#include <kjs/function.h>

#include "kjs_binding.h"

namespace KJS {

  class DOMNode : public NodeObject {
  public:
    DOMNode(DOM::Node n) : node(n) { }
    virtual KJSO *get(const UString &p);
    virtual DOM::Node toNode() const { return node; }
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    DOM::Node node;
  };

  class DOMNodeFunc : public InternalFunction {
    friend DOMNode;
  public:
    DOMNodeFunc(DOM::Node n, int i) : node(n), id(i) { }
    KJSO *execute(const List &);
    enum { InsertBefore, ReplaceChild, RemoveChild, AppendChild,
	   HasChildNodes, CloneNode };
  private:
    DOM::Node node;
    int id;
  };

  class DOMNodeList : public HostObject {
  public:
    DOMNodeList(DOM::NodeList l) : list(l) { }
    virtual KJSO *get(const UString &p);
  private:
    DOM::NodeList list;
  };

  class DOMDocument : public NodeObject {
  public:
    DOMDocument(DOM::Document d) : doc(d) { }
    virtual KJSO *get(const UString &p);
    virtual DOM::Node toNode() const { return doc; }
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    DOM::Document doc;
  };

  class DOMDocFunction : public InternalFunction {
  public:
    DOMDocFunction(DOM::Document d, int i);
    KJSO *execute(const List &);
    enum { CreateElement, CreateDocumentFragment, CreateTextNode,
	   CreateComment, CreateCDATASection, CreateProcessingInstruction,
	   CreateAttribute, CreateEntityReference, GetElementsByTagName };
  private:
    DOM::Document doc;
    int id;
  };

  class DOMAttr : public NodeObject {
  public:
    DOMAttr(DOM::Attr a) : attr(a) { }
    virtual KJSO *get(const UString &p);
    virtual void put(const UString &p, KJSO *v);
    virtual DOM::Node toNode() const { return attr; }
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    DOM::Attr attr;
  };

  class DOMElement : public NodeObject {
  public:
    DOMElement(DOM::Element e) : element(e) { }
    virtual KJSO *get(const UString &p);
    virtual DOM::Node toNode() const { return element; }
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    DOM::Element element;
  };

}; // namespace

#endif
