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

#ifndef _KJS_DOM_H_
#define _KJS_DOM_H_

#include <dom_node.h>
#include <dom_doc.h>
#include <dom_element.h>
#include <dom_xml.h>

#include "kjs_binding.h"

namespace KJS {

  class DOMNode : public DOMObject {
  public:
    DOMNode(DOM::Node n) : node(n) { }
    ~DOMNode();
    virtual Boolean toBoolean(ExecState *) const;
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    Value getValue(ExecState *exec, int token) const;

    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    void putValue(ExecState *exec, int token, const Value& value, int attr);
    virtual DOM::Node toNode() const { return node; }
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;

    virtual Value toPrimitive(ExecState *exec, Type preferred = UndefinedType) const;
    virtual String toString(ExecState *exec) const;
    void setListener(ExecState *exec, int eventId, Value func) const;
    Value getListener(int eventId) const;
    virtual List eventHandlerScope() const;

    enum { NodeName, NodeValue, NodeType, ParentNode, ParentElement,
           ChildNodes, FirstChild, LastChild, PreviousSibling, NextSibling,
           Attributes, NamespaceURI, Prefix, LocalName, OwnerDocument, InsertBefore,
           ReplaceChild, RemoveChild, AppendChild, HasAttributes, HasChildNodes,
           CloneNode, Normalize, Supports, AddEventListener, RemoveEventListener,
           DispatchEvent, Contains,
           OnAbort, OnBlur, OnChange, OnClick, OnDblClick, OnDragDrop, OnError,
           OnFocus, OnKeyDown, OnKeyPress, OnKeyUp, OnLoad, OnMouseDown,
           OnMouseMove, OnMouseOut, OnMouseOver, OnMouseUp, OnMove, OnReset,
           OnResize, OnSelect, OnSubmit, OnUnload,
           OffsetLeft, OffsetTop, OffsetWidth, OffsetHeight, OffsetParent,
           ClientWidth, ClientHeight, ScrollLeft, ScrollTop };

  protected:
    DOM::Node node;
  };

  class DOMNodeFunc : public DOMFunction {
    friend class DOMNode;
  public:
    DOMNodeFunc(ExecState *exec, DOM::Node n, int i, int l);
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List &);
  private:
    DOM::Node node;
    int id;
  };

  class DOMNodeList : public DOMObject {
  public:
    DOMNodeList(DOM::NodeList l) : list(l) { }
    ~DOMNodeList();
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    virtual Boolean toBoolean(ExecState *) const { return Boolean(true); }
    static const ClassInfo info;
  private:
    DOM::NodeList list;
  };

  class DOMNodeListFunc : public DOMFunction {
    friend class DOMNodeList;
  public:
    DOMNodeListFunc(DOM::NodeList l, int i)
        : DOMFunction(), list(l), id(i) { }
      virtual Value tryCall(ExecState *exec, Object &thisObj, const List &);
      enum { Item };
  private:
    DOM::NodeList list;
    int id;
  };

  class DOMDocument : public DOMNode {
  public:
    DOMDocument(DOM::Document d) : DOMNode(d) { }
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    virtual bool hasProperty(ExecState *exec, const UString &propertyName, bool recursive = true) const;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  class DOMDocFunction : public DOMFunction {
  public:
    DOMDocFunction(DOM::Document d, int i)
      : DOMFunction(), doc(d), id(i) {}
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List &);
    enum { CreateElement, CreateDocumentFragment, CreateTextNode,
	   CreateComment, CreateCDATASection, CreateProcessingInstruction,
	   CreateAttribute, CreateEntityReference, GetElementsByTagName,
	   ImportNode, CreateElementNS, CreateAttributeNS, GetElementsByTagNameNS, GetElementById,
	   CreateRange, CreateNodeIterator, CreateTreeWalker, CreateEvent, GetOverrideStyle };
  private:
    DOM::Document doc;
    int id;
  };

  class DOMAttr : public DOMNode {
  public:
    DOMAttr(DOM::Attr a) : DOMNode(a) { }
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  class DOMElement : public DOMNode {
  public:
    DOMElement(DOM::Element e) : DOMNode(e) { }
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  class DOMElementFunction : public DOMFunction {
  public:
    DOMElementFunction(DOM::Element e, int i)
      : DOMFunction(), element(e), id(i) {}
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List &);
    enum { GetAttribute, SetAttribute, RemoveAttribute, GetAttributeNode,
           SetAttributeNode, RemoveAttributeNode, GetElementsByTagName,
           GetAttributeNS, SetAttributeNS, RemoveAttributeNS, GetAttributeNodeNS,
           SetAttributeNodeNS, GetElementsByTagNameNS, HasAttribute, HasAttributeNS,
           Normalize };
  private:
    DOM::Element element;
    int id;
  };

  class DOMDOMImplementation : public DOMObject {
  public:
    DOMDOMImplementation(DOM::DOMImplementation i) : implementation(i) { }
    ~DOMDOMImplementation();
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    // no put - all functions
    virtual const ClassInfo* classInfo() const { return &info; }
    virtual Boolean toBoolean(ExecState *) const { return Boolean(true); }
    static const ClassInfo info;
  private:
    DOM::DOMImplementation implementation;
  };

  class DOMDOMImplementationFunction : public DOMFunction {
  public:
    DOMDOMImplementationFunction(DOM::DOMImplementation impl, int i)
      : DOMFunction(), implementation(impl), id(i) {}
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List &);
    enum { HasFeature, CreateDocumentType, CreateDocument, CreateCSSStyleSheet };
  private:
    DOM::DOMImplementation implementation;
    int id;
  };

  class DOMDocumentType : public DOMNode {
  public:
    DOMDocumentType(DOM::DocumentType dt) : DOMNode(dt) { }
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  class DOMNamedNodeMap : public DOMObject {
  public:
    DOMNamedNodeMap(DOM::NamedNodeMap m) : map(m) { }
    ~DOMNamedNodeMap();
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    virtual Boolean toBoolean(ExecState *) const { return Boolean(true); }
    static const ClassInfo info;
  private:
    DOM::NamedNodeMap map;
  };

  class DOMNamedNodeMapFunction : public DOMFunction {
  public:
    DOMNamedNodeMapFunction(DOM::NamedNodeMap m, int i)
      : DOMFunction(), map(m), id(i) {}
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List &);
    enum { GetNamedItem, SetNamedItem, RemoveNamedItem, Item,
           GetNamedItemNS, SetNamedItemNS, RemoveNamedItemNS };
  private:
    DOM::NamedNodeMap map;
    int id;
  };

  class DOMProcessingInstruction : public DOMNode {
  public:
    DOMProcessingInstruction(DOM::ProcessingInstruction pi) : DOMNode(pi) { }
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  class DOMNotation : public DOMNode {
  public:
    DOMNotation(DOM::Notation n) : DOMNode(n) { }
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  class DOMEntity : public DOMNode {
  public:
    DOMEntity(DOM::Entity e) : DOMNode(e) { }
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  // Prototype object Node
  class NodePrototype : public DOMObject {
  public:
    NodePrototype() { }
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  // Prototype object DOMException
  class DOMExceptionPrototype : public DOMObject {
  public:
    DOMExceptionPrototype() { }
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  Value getDOMNode(DOM::Node n);
  Value getDOMNamedNodeMap(DOM::NamedNodeMap m);
  Value getDOMNodeList(DOM::NodeList l);
  Value getDOMDOMImplementation(DOM::DOMImplementation i);
  Value getNodePrototype(ExecState *exec);
  Value getDOMExceptionPrototype(ExecState *exec);

}; // namespace

#endif
