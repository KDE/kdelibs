// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
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

#ifndef _KJS_TRAVERSAL_H_
#define _KJS_TRAVERSAL_H_

#include "kjs_dom.h"
#include <dom2_traversal.h>

namespace KJS {

  class DOMNodeIterator : public DOMObject {
  public:
    DOMNodeIterator(DOM::NodeIterator ni) : nodeIterator(ni) {}
    ~DOMNodeIterator();
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  protected:
    DOM::NodeIterator nodeIterator;
  };

  class DOMNodeIteratorFunc : public DOMFunction {
    friend class DOMNode;
  public:
    DOMNodeIteratorFunc(DOM::NodeIterator ni, int i)
        : DOMFunction(), nodeIterator(ni), id(i) { }
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args);
    enum { NextNode, PreviousNode, Detach };
  private:
    DOM::NodeIterator nodeIterator;
    int id;
  };

  // Prototype object NodeFilter
  class NodeFilterPrototype : public DOMObject {
  public:
    NodeFilterPrototype() { }
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  class DOMNodeFilter : public DOMObject {
  public:
    DOMNodeFilter(DOM::NodeFilter nf) : nodeFilter(nf) {}
    ~DOMNodeFilter();
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    virtual DOM::NodeFilter toNodeFilter() const { return nodeFilter; }
  protected:
    DOM::NodeFilter nodeFilter;
  };

  class DOMNodeFilterFunc : public DOMFunction {
    friend class DOMNode;
  public:
    DOMNodeFilterFunc(DOM::NodeFilter nf, int i)
        : DOMFunction(), nodeFilter(nf), id(i) {}

    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args);
    enum { AcceptNode };
  private:
    DOM::NodeFilter nodeFilter;
    int id;
  };


  class DOMTreeWalker : public DOMObject {
  public:
    DOMTreeWalker(DOM::TreeWalker tw) : treeWalker(tw) {}
    ~DOMTreeWalker();
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName,
                        const Value& value, int attr = None);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  protected:
    DOM::TreeWalker treeWalker;
  };

  class DOMTreewalkerFunc : public DOMFunction {
    friend class DOMNode;
  public:
    DOMTreewalkerFunc(DOM::TreeWalker tw, int i)
        : DOMFunction(), treeWalker(tw), id(i) { }
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args);
    enum { ParentNode, FirstChild, LastChild, PreviousSibling, NextSibling,
           PreviousNode, NextNode };
  private:
    DOM::TreeWalker treeWalker;
    int id;
  };

  Value getDOMNodeIterator(DOM::NodeIterator ni);
  Value getNodeFilterPrototype(ExecState *exec);
  Value getDOMNodeFilter(DOM::NodeFilter nf);
  Value getDOMTreeWalker(DOM::TreeWalker tw);

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

}; // namespace

#endif
