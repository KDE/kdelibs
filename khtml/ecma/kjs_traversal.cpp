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

#include "kjs_traversal.h"
#include "kjs_window.h"
#include "kjs_proxy.h"
#include <dom/dom_node.h>
#include <xml/dom_nodeimpl.h>
#include <xml/dom_docimpl.h>
#include <khtmlview.h>
#include <qptrdict.h>

using namespace KJS;

QPtrDict<DOMNodeIterator> nodeIterators;
QPtrDict<DOMNodeFilter> nodeFilters;
QPtrDict<DOMTreeWalker> treeWalkers;

// -------------------------------------------------------------------------

const ClassInfo DOMNodeIterator::info = { "NodeIterator", 0, 0, 0 };

DOMNodeIterator::~DOMNodeIterator()
{
  nodeIterators.remove(nodeIterator.handle());
}

Value DOMNodeIterator::tryGet(ExecState *exec, const UString &p) const
{
  DOM::NodeIterator ni(nodeIterator);
  if (p == "root")
    return getDOMNode(exec,ni.root());
  else if (p == "whatToShow")
    return Number(ni.whatToShow());
  else if (p == "filter")
    return getDOMNodeFilter(ni.filter());
  else if (p == "expandEntityReferences")
    return Boolean(ni.expandEntityReferences());
  else if (p == "nextNode")
    return new DOMNodeIteratorFunc(nodeIterator,DOMNodeIteratorFunc::NextNode);
  else if (p == "previousNode")
    return new DOMNodeIteratorFunc(nodeIterator,DOMNodeIteratorFunc::PreviousNode);
  else if (p == "detach")
    return new DOMNodeIteratorFunc(nodeIterator,DOMNodeIteratorFunc::Detach);
  else
    return DOMObject::tryGet(exec, p);
}

Value DOMNodeIteratorFunc::tryCall(ExecState *exec, Object &, const List &)
{
  Value result;

  switch (id) {
    case PreviousNode:
      result = getDOMNode(exec,nodeIterator.previousNode());
      break;
    case NextNode:
      result = getDOMNode(exec,nodeIterator.nextNode());
      break;
    case Detach:
      nodeIterator.detach();
      result = Undefined();
      break;
  };

  return result;
}

Value KJS::getDOMNodeIterator(DOM::NodeIterator ni)
{
  DOMNodeIterator *ret;
  if (ni.isNull())
    return Null();
  else if ((ret = nodeIterators[ni.handle()]))
    return ret;
  else {
    ret = new DOMNodeIterator(ni);
    nodeIterators.insert(ni.handle(),ret);
    return ret;
  }
}


// -------------------------------------------------------------------------

const ClassInfo NodeFilterPrototype::info = { "NodeFilterPrototype", 0, 0, 0 };
// ### make this protype of Range objects? (also for Node)

Value NodeFilterPrototype::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "FILTER_ACCEPT")
    return Number((long unsigned int)DOM::NodeFilter::FILTER_ACCEPT);
  if (p == "FILTER_REJECT")
    return Number((long unsigned int)DOM::NodeFilter::FILTER_REJECT);
  if (p == "FILTER_SKIP")
    return Number((long unsigned int)DOM::NodeFilter::FILTER_SKIP);
  if (p == "SHOW_ALL")
    return Number((long unsigned int)DOM::NodeFilter::SHOW_ALL);
  if (p == "SHOW_ELEMENT")
    return Number((long unsigned int)DOM::NodeFilter::SHOW_ELEMENT);
  if (p == "SHOW_ATTRIBUTE")
    return Number((long unsigned int)DOM::NodeFilter::SHOW_ATTRIBUTE);
  if (p == "SHOW_TEXT")
    return Number((long unsigned int)DOM::NodeFilter::SHOW_TEXT);
  if (p == "SHOW_CDATA_SECTION")
    return Number((long unsigned int)DOM::NodeFilter::SHOW_CDATA_SECTION);
  if (p == "SHOW_ENTITY_REFERENCE")
    return Number((long unsigned int)DOM::NodeFilter::SHOW_ENTITY_REFERENCE);
  if (p == "SHOW_ENTITY")
    return Number((long unsigned int)DOM::NodeFilter::SHOW_ENTITY);
  if (p == "SHOW_PROCESSING_INSTRUCTION")
    return Number((long unsigned int)DOM::NodeFilter::SHOW_PROCESSING_INSTRUCTION);
  if (p == "SHOW_COMMENT")
    return Number((long unsigned int)DOM::NodeFilter::SHOW_COMMENT);
  if (p == "SHOW_DOCUMENT")
    return Number((long unsigned int)DOM::NodeFilter::SHOW_DOCUMENT);
  if (p == "SHOW_DOCUMENT_TYPE")
    return Number((long unsigned int)DOM::NodeFilter::SHOW_DOCUMENT_TYPE);
  if (p == "SHOW_DOCUMENT_FRAGMENT")
    return Number((long unsigned int)DOM::NodeFilter::SHOW_DOCUMENT_FRAGMENT);
  if (p == "SHOW_NOTATION")
    return Number((long unsigned int)DOM::NodeFilter::SHOW_NOTATION);

  return DOMObject::tryGet(exec, p);
}

Value KJS::getNodeFilterPrototype(ExecState *exec)
{
    Value proto = exec->interpreter()->globalObject().get(exec, "[[nodeFilter.prototype]]");
    if (!proto.isNull())
        return proto;
    else
    {
        Object nodeFilterProto( new NodeFilterPrototype );
        exec->interpreter()->globalObject().put(exec, "[[nodeFilter.prototype]]", nodeFilterProto);
        return nodeFilterProto;
    }
}


// -------------------------------------------------------------------------

const ClassInfo DOMNodeFilter::info = { "NodeFilter", 0, 0, 0 };


DOMNodeFilter::~DOMNodeFilter()
{
  nodeFilters.remove(nodeFilter.handle());
}

Value DOMNodeFilter::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "acceptNode")
    return new DOMNodeFilterFunc(nodeFilter,DOMNodeFilterFunc::AcceptNode);
  else
    return DOMObject::tryGet(exec, p);
}

Value DOMNodeFilterFunc::tryCall(ExecState *, Object &, const List &args)
{
  Value result;

  switch (id) {
    case AcceptNode:
      result = Number(nodeFilter.acceptNode(toNode(args[0])));
      break;
  };

  return result;
}

Value KJS::getDOMNodeFilter(DOM::NodeFilter nf)
{
  DOMNodeFilter *ret;
  if (nf.isNull())
    return Null();
  else if ((ret = nodeFilters[nf.handle()]))
    return ret;
  else {
    ret = new DOMNodeFilter(nf);
    nodeFilters.insert(nf.handle(),ret);
    return ret;
  }
}



// -------------------------------------------------------------------------

const ClassInfo DOMTreeWalker::info = { "TreeWalker", 0, 0, 0 };


DOMTreeWalker::~DOMTreeWalker()
{
  treeWalkers.remove(treeWalker.handle());
}

Value DOMTreeWalker::tryGet(ExecState *exec, const UString &p) const
{
  DOM::TreeWalker tw(treeWalker);
  if (p == "root")
    return getDOMNode(exec,tw.root());
  if (p == "whatToShow")
    return Number(tw.whatToShow());
  if (p == "filter")
    return getDOMNodeFilter(tw.filter());
  if (p == "expandEntityReferences")
    return Boolean(tw.expandEntityReferences());
  if (p == "currentNode")
    return getDOMNode(exec,tw.currentNode());
  if (p == "parentNode")
    return new DOMTreewalkerFunc(treeWalker,DOMTreewalkerFunc::ParentNode);
  if (p == "firstChild")
    return new DOMTreewalkerFunc(treeWalker,DOMTreewalkerFunc::FirstChild);
  if (p == "lastChild")
    return new DOMTreewalkerFunc(treeWalker,DOMTreewalkerFunc::LastChild);
  if (p == "previousSibling")
    return new DOMTreewalkerFunc(treeWalker,DOMTreewalkerFunc::PreviousSibling);
  if (p == "nextSibling")
    return new DOMTreewalkerFunc(treeWalker,DOMTreewalkerFunc::NextSibling);
  if (p == "previousNode")
    return new DOMTreewalkerFunc(treeWalker,DOMTreewalkerFunc::PreviousNode);
  if (p == "nextNode")
    return new DOMTreewalkerFunc(treeWalker,DOMTreewalkerFunc::NextNode);
  else
    return DOMObject::tryGet(exec, p);
}

void DOMTreeWalker::tryPut(ExecState *exec, const UString &propertyName,
                           const Value& value, int attr)
{
  if (propertyName == "currentNode") {
    treeWalker.setCurrentNode(toNode(value));
  }
  else
    ObjectImp::put(exec, propertyName, value, attr);
}

Value DOMTreewalkerFunc::tryCall(ExecState *exec, Object &, const List &)
{
  Value result;

  switch (id) {
    case ParentNode:
      result = getDOMNode(exec,treeWalker.parentNode());
      break;
    case FirstChild:
      result = getDOMNode(exec,treeWalker.firstChild());
      break;
    case LastChild:
      result = getDOMNode(exec,treeWalker.lastChild());
      break;
    case PreviousSibling:
      result = getDOMNode(exec,treeWalker.previousSibling());
      break;
    case NextSibling:
      result = getDOMNode(exec,treeWalker.nextSibling());
      break;
    case PreviousNode:
      result = getDOMNode(exec,treeWalker.previousSibling());
      break;
    case NextNode:
      result = getDOMNode(exec,treeWalker.nextNode());
      break;
  };

  return result;
}

Value KJS::getDOMTreeWalker(DOM::TreeWalker tw)
{
  DOMTreeWalker *ret;
  if (tw.isNull())
    return Null();
  else if ((ret = treeWalkers[tw.handle()]))
    return ret;
  else {
    ret = new DOMTreeWalker(tw);
    treeWalkers.insert(tw.handle(),ret);
    return ret;
  }
}

DOM::NodeFilter KJS::toNodeFilter(const Value& val)
{
  Object obj = Object::dynamicCast(val);
  if (obj.isNull() || !obj.inherits(&DOMNodeFilter::info))
    return DOM::NodeFilter();

  const DOMNodeFilter *dobj = static_cast<const DOMNodeFilter*>(obj.imp());
  return dobj->toNodeFilter();
}

// -------------------------------------------------------------------------

JSNodeFilter::JSNodeFilter(Object & _filter) : DOM::CustomNodeFilter()
{
    filter = _filter;
}

JSNodeFilter::~JSNodeFilter()
{
}

short JSNodeFilter::acceptNode(const DOM::Node &n)
{
  KHTMLPart *part = static_cast<DOM::DocumentImpl *>( n.handle()->docPtr()->document() )->view()->part();
  KJSProxy *proxy = KJSProxy::proxy( part );
  if (proxy) {
    ExecState *exec = proxy->interpreter()->globalExec();
    Object acceptNodeFunc = Object::dynamicCast( filter.get(exec, "acceptNode") );
    if (acceptNodeFunc.implementsCall()) {
      List args;
      args.append(getDOMNode(exec,n));
      Value result = acceptNodeFunc.call(exec,filter,args);
      return result.toNumber(exec).intValue();
    }
  }

  return DOM::NodeFilter::FILTER_REJECT;
}
