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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _KJS_TRAVERSAL_H_
#define _KJS_TRAVERSAL_H_

#include "ecma/kjs_dom.h"
#include "dom/dom2_traversal.h"

namespace KJS {

  class DOMNodeIterator : public DOMObject {
  public:
    DOMNodeIterator(ExecState *exec, DOM::NodeIterator ni);
    ~DOMNodeIterator();
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    Value getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Filter, Root, WhatToShow, ExpandEntityReferences,
           NextNode, PreviousNode, Detach };
    DOM::NodeIterator toNodeIterator() const { return nodeIterator; }
  protected:
    DOM::NodeIterator nodeIterator;
  };

  // Constructor object NodeFilter
  class NodeFilterConstructor : public DOMObject {
  public:
    NodeFilterConstructor(ExecState *);
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    Value getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  class DOMNodeFilter : public DOMObject {
  public:
    DOMNodeFilter(ExecState *exec, DOM::NodeFilter nf);
    ~DOMNodeFilter();
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    virtual DOM::NodeFilter toNodeFilter() const { return nodeFilter; }
    enum { AcceptNode };
  protected:
    DOM::NodeFilter nodeFilter;
  };

  class DOMTreeWalker : public DOMObject {
  public:
    DOMTreeWalker(ExecState *exec, DOM::TreeWalker tw);
    ~DOMTreeWalker();
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    Value getValueProperty(ExecState *exec, int token) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName,
                        const Value& value, int attr = None);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Root, WhatToShow, Filter, ExpandEntityReferences, CurrentNode,
           ParentNode, FirstChild, LastChild, PreviousSibling, NextSibling,
           PreviousNode, NextNode };
    DOM::TreeWalker toTreeWalker() const { return treeWalker; }
  protected:
    DOM::TreeWalker treeWalker;
  };

  Value getDOMNodeIterator(ExecState *exec, DOM::NodeIterator ni);
  Value getNodeFilterConstructor(ExecState *exec);
  Value getDOMNodeFilter(ExecState *exec, DOM::NodeFilter nf);
  Value getDOMTreeWalker(ExecState *exec, DOM::TreeWalker tw);

  /**
   * Convert an object to a NodeFilter. Returns a null Node if not possible.
   */
  DOM::NodeFilter toNodeFilter(const Value&);

  class JSNodeFilter : public DOM::CustomNodeFilter {
  public:
    JSNodeFilter(Object & _filter);
    virtual ~JSNodeFilter();
    virtual short acceptNode (const DOM::Node &n);
  protected:
    Object filter;
  };

} // namespace

#endif
