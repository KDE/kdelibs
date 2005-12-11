// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003 Apple Computer, Inc.
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _KJS_DOM_H_
#define _KJS_DOM_H_

#include "xml/dom_nodeimpl.h"
#include "xml/dom_docimpl.h"
#include "xml/dom_elementimpl.h"
#include "xml/dom_xmlimpl.h"

#include "ecma/kjs_binding.h"

namespace KJS {

  class DOMNode : public DOMObject {
  public:
    // Build a DOMNode
    DOMNode(ExecState *exec, DOM::NodeImpl* n);
    ~DOMNode();
    virtual bool toBoolean(ExecState *) const;
    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    ValueImp* getValueProperty(ExecState *exec, int token) const;

    virtual void put(ExecState *exec, const Identifier &propertyName, ValueImp* value, int attr = None);
    void putValueProperty(ExecState *exec, int token, ValueImp* value, int attr);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;

    virtual ValueImp* toPrimitive(ExecState *exec, Type preferred = UndefinedType) const;
    virtual UString toString(ExecState *exec) const;
    void setListener(ExecState *exec, int eventId, ValueImp* func) const;
    ValueImp* getListener(int eventId) const;
    virtual void pushEventHandlerScope(ExecState *exec, ScopeChain &scope) const;

    enum { NodeName, NodeValue, NodeType, ParentNode, ParentElement,
           ChildNodes, FirstChild, LastChild, PreviousSibling, NextSibling, Item,
           Attributes, NamespaceURI, Prefix, LocalName, OwnerDocument, InsertBefore,
           ReplaceChild, RemoveChild, AppendChild, HasAttributes, HasChildNodes,
           CloneNode, Normalize, IsSupported, AddEventListener, RemoveEventListener,
           DispatchEvent, Contains, InsertAdjacentHTML,
           OnAbort, OnBlur, OnChange, OnClick, OnDblClick, OnDragDrop, OnError,
           OnFocus, OnKeyDown, OnKeyPress, OnKeyUp, OnLoad, OnMouseDown,
           OnMouseMove, OnMouseOut, OnMouseOver, OnMouseUp, OnMove, OnReset,
           OnResize, OnSelect, OnSubmit, OnUnload,
           OffsetLeft, OffsetTop, OffsetWidth, OffsetHeight, OffsetParent,
           ClientWidth, ClientHeight, ScrollLeft, ScrollTop,
	   ScrollWidth, ScrollHeight, SourceIndex };

    //### toNode? virtual
    DOM::NodeImpl* impl() const { return m_impl.get(); }
  protected:
    SharedPtr<DOM::NodeImpl> m_impl;
  };

  class DOMNodeList : public DOMObject {
  public:
    DOMNodeList(ExecState *, DOM::NodeListImpl* l);
    ~DOMNodeList();

    ValueImp* indexGetter(ExecState *exec, unsigned index);
    
    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    virtual ValueImp* callAsFunction(ExecState *exec, ObjectImp* thisObj, const List&args);
    virtual bool implementsCall() const { return true; }
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    virtual bool toBoolean(ExecState *) const { return true; }
    static const ClassInfo info;
    
    enum { Length, Item, NamedItem };

    DOM::NodeListImpl* impl() const { return m_impl.get(); }

    DOM::NodeImpl* getByName(const Identifier& name);
  private:
    SharedPtr<DOM::NodeListImpl> m_impl;

    static ValueImp *nameGetter(ExecState *, const Identifier&, const PropertySlot&);
    static ValueImp *lengthGetter(ExecState *, const Identifier&, const PropertySlot&);
  };

  class DOMDocument : public DOMNode {
  public:
    // Build a DOMDocument
    DOMDocument(ExecState *exec, DOM::DocumentImpl* d);

    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    ValueImp* getValueProperty(ExecState *exec, int token) const;
    
    virtual void put(ExecState *exec, const Identifier &propertyName, ValueImp* value, int attr = None);
    void putValueProperty(ExecState *exec, int token, ValueImp* value, int attr);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { DocType, Implementation, DocumentElement,
           // Functions
           CreateElement, CreateDocumentFragment, CreateTextNode, CreateComment,
           CreateCDATASection, CreateProcessingInstruction, CreateAttribute,
           CreateEntityReference, GetElementsByTagName, ImportNode, CreateElementNS,
           CreateAttributeNS, GetElementsByTagNameNS, GetElementById,
           CreateRange, CreateNodeIterator, CreateTreeWalker, DefaultView,
           CreateEvent, StyleSheets, GetOverrideStyle, Abort, Load, LoadXML,
           PreferredStylesheetSet, SelectedStylesheetSet, ReadyState, Async };
    DOM::DocumentImpl* impl() { return static_cast<DOM::DocumentImpl*>(m_impl.get()); }
  };

  class DOMAttr : public DOMNode {
  public:
    DOMAttr(ExecState *exec, DOM::AttrImpl* a) : DOMNode(exec, a) { }
    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    ValueImp* getValueProperty(ExecState *exec, int token) const;
    
    virtual void put(ExecState *exec, const Identifier &propertyName, ValueImp* value, int attr = None);
    void putValueProperty(ExecState *exec, int token, ValueImp* value, int attr);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Name, Specified, ValueProperty, OwnerElement };
  };

  class DOMElement : public DOMNode {
  public:
    // Build a DOMElement
    DOMElement(ExecState *exec, DOM::ElementImpl* e);
    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    ValueImp* getValueProperty(ExecState *exec, int token) const;

    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { TagName, Style,
           GetAttribute, SetAttribute, RemoveAttribute, GetAttributeNode,
           SetAttributeNode, RemoveAttributeNode, GetElementsByTagName,
           GetAttributeNS, SetAttributeNS, RemoveAttributeNS, GetAttributeNodeNS,
           SetAttributeNodeNS, GetElementsByTagNameNS, HasAttribute, HasAttributeNS };
  private:
    static ValueImp *attributeGetter(ExecState *exec, const Identifier&, const PropertySlot& slot);
  };

  DOM::ElementImpl *toElement(ValueImp *); // returns 0 if passed-in value is not a DOMElement object

  class DOMDOMImplementation : public DOMObject {
  public:
    // Build a DOMDOMImplementation
    DOMDOMImplementation(ExecState *, DOM::DOMImplementationImpl* i);
    ~DOMDOMImplementation();
    // no put - all functions
    virtual const ClassInfo* classInfo() const { return &info; }
    virtual bool toBoolean(ExecState *) const { return true; }
    static const ClassInfo info;
    enum { HasFeature, CreateDocumentType, CreateDocument, CreateCSSStyleSheet, CreateHTMLDocument };
    
    DOM::DOMImplementationImpl* impl() const { return m_impl.get(); }
  private:
    SharedPtr<DOM::DOMImplementationImpl> m_impl;
  };

  class DOMDocumentType : public DOMNode {
  public:
    // Build a DOMDocumentType
    DOMDocumentType(ExecState *exec, DOM::DocumentTypeImpl* dt);

    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    ValueImp* getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Name, Entities, Notations, PublicId, SystemId, InternalSubset };
  };

  class DOMNamedNodeMap : public DOMObject {
  public:
    DOMNamedNodeMap(ExecState *, DOM::NamedNodeMapImpl* m);
    ~DOMNamedNodeMap();

    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    ValueImp* getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    virtual bool toBoolean(ExecState *) const { return true; }
    static const ClassInfo info;
    enum { GetNamedItem, SetNamedItem, RemoveNamedItem, Item, Length,
           GetNamedItemNS, SetNamedItemNS, RemoveNamedItemNS };
           
    DOM::NamedNodeMapImpl* impl() const { return m_impl.get(); }

    ValueImp* indexGetter(ExecState *exec, unsigned index);
  private:
    static ValueImp *lengthGetter(ExecState *, const Identifier&, const PropertySlot& slot);
    SharedPtr<DOM::NamedNodeMapImpl> m_impl;
  };

  class DOMProcessingInstruction : public DOMNode {
  public:
    DOMProcessingInstruction(ExecState *exec, DOM::ProcessingInstructionImpl* pi) : DOMNode(exec, pi) { }

    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    ValueImp* getValueProperty(ExecState *exec, int token) const;
    
    virtual void put(ExecState *exec, const Identifier &propertyName, ValueImp* value, int attr = None);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Target, Data, Sheet };
  };

  class DOMNotation : public DOMNode {
  public:
    DOMNotation(ExecState *exec, DOM::NotationImpl* n) : DOMNode(exec, n) { }

    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    ValueImp* getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { PublicId, SystemId };
  };

  class DOMEntity : public DOMNode {
  public:
    DOMEntity(ExecState *exec, DOM::EntityImpl* e) : DOMNode(exec, e) { }
    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    ValueImp* getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { PublicId, SystemId, NotationName };
  };

  // Constructor for Node - constructor stuff not implemented yet
  class NodeConstructor : public DOMObject {
  public:
    NodeConstructor(ExecState *);
    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    ValueImp* getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  // Constructor for DOMException - constructor stuff not implemented yet
  class DOMExceptionConstructor : public DOMObject {
  public:
    DOMExceptionConstructor(ExecState *);
    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    ValueImp* getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  bool checkNodeSecurity(ExecState *exec, const DOM::NodeImpl* n);
  KDE_EXPORT ValueImp* getDOMNode(ExecState *exec, DOM::NodeImpl* n);
  ValueImp* getDOMNamedNodeMap(ExecState *exec, DOM::NamedNodeMapImpl* m);
  ValueImp* getDOMNodeList(ExecState *exec, DOM::NodeListImpl* l);
  ValueImp* getDOMDOMImplementation(ExecState *exec, DOM::DOMImplementationImpl* i);
  ObjectImp *getNodeConstructor(ExecState *exec);
  ObjectImp *getDOMExceptionConstructor(ExecState *exec);

  // Internal class, used for the collection return by e.g. document.forms.myinput
  // when multiple nodes have the same name.
  class DOMNamedNodesCollection : public DOMObject {
  public:
    DOMNamedNodesCollection(ExecState *exec, const QList<SharedPtr<DOM::NodeImpl> >& nodes );
    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    const QList<SharedPtr<DOM::NodeImpl> > nodes() const { return m_nodes; }
    enum { Length };

    ValueImp* indexGetter(ExecState *exec, unsigned index);
  private:
    static ValueImp *lengthGetter(ExecState *, const Identifier&, const PropertySlot& slot);
    QList<SharedPtr<DOM::NodeImpl> > m_nodes;
  };

  class DOMCharacterData : public DOMNode {
  public:
    // Build a DOMCharacterData
    DOMCharacterData(ExecState *exec, DOM::CharacterDataImpl* d);
    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    ValueImp* getValueProperty(ExecState *, int token) const;
    virtual void put(ExecState *exec, const Identifier &propertyName, ValueImp* value, int attr = None);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    DOM::CharacterDataImpl* impl() const { return static_cast<DOM::CharacterDataImpl*>(m_impl.get()); }
    enum { Data, Length,
           SubstringData, AppendData, InsertData, DeleteData, ReplaceData };
  };

  class DOMText : public DOMCharacterData {
  public:
    DOMText(ExecState *exec, DOM::TextImpl* t);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    DOM::TextImpl* impl() const { return static_cast<DOM::TextImpl*>(m_impl.get()); }
    enum { SplitText };
  };

} // namespace

#endif
