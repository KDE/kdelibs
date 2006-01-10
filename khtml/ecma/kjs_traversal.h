// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
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

#ifndef _KJS_TRAVERSAL_H_
#define _KJS_TRAVERSAL_H_

#include "ecma/kjs_binding.h"
#include "ecma/kjs_dom.h"
#include "dom/dom2_traversal.h"

namespace KJS {

  class DOMNodeIterator : public DOMObject {
  public:
    DOMNodeIterator(ExecState *exec, DOM::NodeIteratorImpl* ni);
    ~DOMNodeIterator();
    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    ValueImp* getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Filter, Root, WhatToShow, ExpandEntityReferences,
           NextNode, PreviousNode, Detach };
    DOM::NodeIteratorImpl* impl() const { return m_impl.get(); }
  protected:
    SharedPtr<DOM::NodeIteratorImpl> m_impl;
  };

  // Constructor object NodeFilter
  class NodeFilterConstructor : public DOMObject {
  public:
    NodeFilterConstructor(ExecState *);
    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    ValueImp* getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  class DOMNodeFilter : public DOMObject {
  public:
    DOMNodeFilter(ExecState *exec, DOM::NodeFilterImpl* nf);
    ~DOMNodeFilter();
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    virtual DOM::NodeFilterImpl* impl() const { return m_impl.get(); }
    enum { AcceptNode };
  protected:
    SharedPtr<DOM::NodeFilterImpl> m_impl;
  };

  class DOMTreeWalker : public DOMObject {
  public:
    DOMTreeWalker(ExecState *exec, DOM::TreeWalkerImpl* tw);
    ~DOMTreeWalker();
    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    ValueImp* getValueProperty(ExecState *exec, int token) const;
    virtual void put(ExecState *exec, const Identifier &propertyName,
                        ValueImp* value, int attr = None);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Root, WhatToShow, Filter, ExpandEntityReferences, CurrentNode,
           ParentNode, FirstChild, LastChild, PreviousSibling, NextSibling,
           PreviousNode, NextNode };
    DOM::TreeWalkerImpl* impl() const { return m_impl.get(); }
  protected:
    SharedPtr<DOM::TreeWalkerImpl> m_impl;
  };

  ValueImp* getDOMNodeIterator(ExecState *exec, DOM::NodeIteratorImpl* ni);
  ValueImp* getNodeFilterConstructor(ExecState *exec);
  ValueImp* getDOMNodeFilter(ExecState *exec, DOM::NodeFilterImpl* nf);
  ValueImp* getDOMTreeWalker(ExecState *exec, DOM::TreeWalkerImpl* tw);

  /**
   * Convert an object to a NodeFilter. Returns a null Node if not possible.
   */
  DOM::NodeFilterImpl* toNodeFilter(ValueImp*);

  class JSNodeFilter : public DOM::CustomNodeFilter {
  public:
    JSNodeFilter(ObjectImp* _filter);
    virtual ~JSNodeFilter();
    virtual short acceptNode (const DOM::Node &n);
  protected:
    ObjectImp* filter;
  };

} // namespace

#endif
