// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _KJS_DOM_H_
#define _KJS_DOM_H_

#include "dom/dom_node.h"
#include "dom/dom_doc.h"
#include "dom/dom_element.h"
#include "dom/dom_xml.h"

#include "ecma/kjs_binding.h"

namespace KJS {

  class DOMNode : public DOMObject {
  public:
    // Build a DOMNode
    DOMNode(ExecState *exec, const DOM::Node& n);
    // Constructor for inherited classes
    DOMNode(const Object& proto, const DOM::Node& n);
    ~DOMNode();
    virtual bool toBoolean(ExecState *) const;
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;

    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    void putValueProperty(ExecState *exec, int token, const Value& value, int attr);
    virtual DOM::Node toNode() const { return node; }
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;

    virtual Value toPrimitive(ExecState *exec, Type preferred = UndefinedType) const;
    virtual UString toString(ExecState *exec) const;
    void setListener(ExecState *exec, int eventId, const Value& func) const;
    Value getListener(int eventId) const;
    virtual List eventHandlerScope(ExecState *exec) const;

    enum { NodeName, NodeValue, NodeType, ParentNode, ParentElement,
           ChildNodes, FirstChild, LastChild, PreviousSibling, NextSibling,
           Attributes, NamespaceURI, Prefix, LocalName, OwnerDocument, InsertBefore,
           ReplaceChild, RemoveChild, AppendChild, HasAttributes, HasChildNodes,
           CloneNode, Normalize, IsSupported, AddEventListener, RemoveEventListener,
           DispatchEvent, Contains,
           OnAbort, OnBlur, OnChange, OnClick, OnDblClick, OnDragDrop, OnError,
           OnFocus, OnKeyDown, OnKeyPress, OnKeyUp, OnLoad, OnMouseDown,
           OnMouseMove, OnMouseOut, OnMouseOver, OnMouseUp, OnMove, OnReset,
           OnResize, OnSelect, OnSubmit, OnUnload,
           OffsetLeft, OffsetTop, OffsetWidth, OffsetHeight, OffsetParent,
           ClientWidth, ClientHeight, ScrollLeft, ScrollTop, SourceIndex };

  protected:
    DOM::Node node;
  };

  class DOMNodeList : public DOMObject {
  public:
    DOMNodeList(ExecState *, const DOM::NodeList& l);
    ~DOMNodeList();
    virtual bool hasProperty(ExecState *exec, const UString &p) const;
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    virtual Value call(ExecState *exec, Object &thisObj, const List&args);
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args);
    virtual bool implementsCall() const { return true; }
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    virtual bool toBoolean(ExecState *) const { return true; }
    static const ClassInfo info;
    DOM::NodeList nodeList() const { return list; }
  private:
    DOM::NodeList list;
  };

  class DOMNodeListFunc : public DOMFunction {
    friend class DOMNodeList;
  public:
    DOMNodeListFunc(ExecState *exec, int id, int len);
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List &);
    enum { Item };
  private:
    int id;
  };

  class DOMDocument : public DOMNode {
  public:
    // Build a DOMDocument
    DOMDocument(ExecState *exec, const DOM::Document& d);
    // Constructor for inherited classes
    DOMDocument(const Object& proto, const DOM::Document& d);
    virtual ~DOMDocument();
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { DocType, Implementation, DocumentElement,
           // Functions
           CreateElement, CreateDocumentFragment, CreateTextNode, CreateComment,
           CreateCDATASection, CreateProcessingInstruction, CreateAttribute,
           CreateEntityReference, GetElementsByTagName, ImportNode, CreateElementNS,
           CreateAttributeNS, GetElementsByTagNameNS, GetElementById,
           CreateRange, CreateNodeIterator, CreateTreeWalker, DefaultView,
           CreateEvent, StyleSheets, GetOverrideStyle, ReadyState };
  };

  class DOMAttr : public DOMNode {
  public:
    DOMAttr(ExecState *exec, const DOM::Attr& a) : DOMNode(exec, a) { }
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    Value getValueProperty(ExecState *exec, int token) const;
    void putValueProperty(ExecState *exec, int token, const Value& value, int attr);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Name, Specified, ValueProperty, OwnerElement };
  };

  class DOMElement : public DOMNode {
  public:
    // Build a DOMElement
    DOMElement(ExecState *exec, const DOM::Element& e);
    // Constructor for inherited classes
    DOMElement(const Object& proto, const DOM::Element& e);
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { TagName, Style,
           GetAttribute, SetAttribute, RemoveAttribute, GetAttributeNode,
           SetAttributeNode, RemoveAttributeNode, GetElementsByTagName,
           GetAttributeNS, SetAttributeNS, RemoveAttributeNS, GetAttributeNodeNS,
           SetAttributeNodeNS, GetElementsByTagNameNS, HasAttribute, HasAttributeNS };
  };

  class DOMDOMImplementation : public DOMObject {
  public:
    // Build a DOMDOMImplementation
    DOMDOMImplementation(ExecState *, const DOM::DOMImplementation& i);
    ~DOMDOMImplementation();
    // no put - all functions
    virtual const ClassInfo* classInfo() const { return &info; }
    virtual bool toBoolean(ExecState *) const { return true; }
    static const ClassInfo info;
    enum { HasFeature, CreateDocumentType, CreateDocument, CreateCSSStyleSheet, CreateHTMLDocument };
    DOM::DOMImplementation toImplementation() const { return implementation; }
  private:
    DOM::DOMImplementation implementation;
  };

  class DOMDocumentType : public DOMNode {
  public:
    // Build a DOMDocumentType
    DOMDocumentType(ExecState *exec, const DOM::DocumentType& dt);
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Name, Entities, Notations, PublicId, SystemId, InternalSubset };
  };

  class DOMNamedNodeMap : public DOMObject {
  public:
    DOMNamedNodeMap(ExecState *, const DOM::NamedNodeMap& m);
    ~DOMNamedNodeMap();
    virtual bool hasProperty(ExecState *exec, const UString &p) const;
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    virtual bool toBoolean(ExecState *) const { return true; }
    static const ClassInfo info;
    enum { GetNamedItem, SetNamedItem, RemoveNamedItem, Item,
           GetNamedItemNS, SetNamedItemNS, RemoveNamedItemNS };
    DOM::NamedNodeMap toMap() const { return map; }
  private:
    DOM::NamedNodeMap map;
  };

  class DOMProcessingInstruction : public DOMNode {
  public:
    DOMProcessingInstruction(ExecState *exec, const DOM::ProcessingInstruction& pi) : DOMNode(exec, pi) { }
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Target, Data, Sheet };
  };

  class DOMNotation : public DOMNode {
  public:
    DOMNotation(ExecState *exec, const DOM::Notation& n) : DOMNode(exec, n) { }
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { PublicId, SystemId };
  };

  class DOMEntity : public DOMNode {
  public:
    DOMEntity(ExecState *exec, const DOM::Entity& e) : DOMNode(exec, e) { }
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { PublicId, SystemId, NotationName };
  };

  // Constructor for Node - constructor stuff not implemented yet
  class NodeConstructor : public DOMObject {
  public:
    NodeConstructor(ExecState *);
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  // Constructor for DOMException - constructor stuff not implemented yet
  class DOMExceptionConstructor : public DOMObject {
  public:
    DOMExceptionConstructor(ExecState *);
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  Value getDOMNode(ExecState *exec, const DOM::Node& n);
  Value getDOMNamedNodeMap(ExecState *exec, const DOM::NamedNodeMap& m);
  Value getDOMNodeList(ExecState *exec, const DOM::NodeList& l);
  Value getDOMDOMImplementation(ExecState *exec, const DOM::DOMImplementation& i);
  Object getNodeConstructor(ExecState *exec);
  Object getDOMExceptionConstructor(ExecState *exec);

  // Internal class, used for the collection return by e.g. document.forms.myinput
  // when multiple nodes have the same name.
  class DOMNamedNodesCollection : public DOMObject {
  public:
    DOMNamedNodesCollection(ExecState *exec, const QValueList<DOM::Node>& nodes );
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    const QValueList<DOM::Node>& nodes() const { return m_nodes; }
  private:
    QValueList<DOM::Node> m_nodes;
  };

  class DOMCharacterData : public DOMNode {
  public:
    // Build a DOMCharacterData
    DOMCharacterData(ExecState *exec, const DOM::CharacterData& d);
    // Constructor for inherited classes
    DOMCharacterData(const Object& proto, const DOM::CharacterData& d);
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    Value getValueProperty(ExecState *, int token) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    DOM::CharacterData toData() const { return static_cast<DOM::CharacterData>(node); }
    enum { Data, Length,
           SubstringData, AppendData, InsertData, DeleteData, ReplaceData };
  };

  class DOMText : public DOMCharacterData {
  public:
    DOMText(ExecState *exec, const DOM::Text& t);
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    Value getValueProperty(ExecState *, int token) const;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    DOM::Text toText() const { return static_cast<DOM::Text>(node); }
    enum { SplitText };
  };

}; // namespace

#endif
