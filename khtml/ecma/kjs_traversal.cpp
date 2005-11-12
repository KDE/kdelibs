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

#include "kjs_traversal.h"
#include "kjs_traversal.lut.h"
#include "kjs_proxy.h"
#include <dom/dom_node.h>
#include <xml/dom_nodeimpl.h>
#include <xml/dom_docimpl.h>
#include <khtmlview.h>
#include <khtml_part.h>
#include <kdebug.h>

using namespace KJS;

// -------------------------------------------------------------------------

const ClassInfo DOMNodeIterator::info = { "NodeIterator", 0, &DOMNodeIteratorTable, 0 };
/*
@begin DOMNodeIteratorTable 5
  root				DOMNodeIterator::Root			DontDelete|ReadOnly
  whatToShow			DOMNodeIterator::WhatToShow		DontDelete|ReadOnly
  filter			DOMNodeIterator::Filter			DontDelete|ReadOnly
  expandEntityReferences	DOMNodeIterator::ExpandEntityReferences	DontDelete|ReadOnly
@end
@begin DOMNodeIteratorProtoTable 3
  nextNode	DOMNodeIterator::NextNode	DontDelete|Function 0
  previousNode	DOMNodeIterator::PreviousNode	DontDelete|Function 0
  detach	DOMNodeIterator::Detach		DontDelete|Function 0
@end
*/
DEFINE_PROTOTYPE("DOMNodeIterator",DOMNodeIteratorProto)
IMPLEMENT_PROTOFUNC_DOM(DOMNodeIteratorProtoFunc)
IMPLEMENT_PROTOTYPE(DOMNodeIteratorProto,DOMNodeIteratorProtoFunc)

DOMNodeIterator::DOMNodeIterator(ExecState *exec, DOM::NodeIterator ni)
  : DOMObject(DOMNodeIteratorProto::self(exec)), nodeIterator(ni) {}

DOMNodeIterator::~DOMNodeIterator()
{
  ScriptInterpreter::forgetDOMObject(nodeIterator.handle());
}

ValueImp *DOMNodeIterator::tryGet(ExecState *exec, const Identifier &p) const
{
  return DOMObjectLookupGetValue<DOMNodeIterator,DOMObject>(exec,p,&DOMNodeIteratorTable,this);
}

ValueImp *DOMNodeIterator::getValueProperty(ExecState *exec, int token) const
{
  DOM::NodeIterator ni(nodeIterator);
  switch (token) {
  case Root:
    return getDOMNode(exec,ni.root());
  case WhatToShow:
    return Number(ni.whatToShow());
  case Filter:
    return getDOMNodeFilter(exec,ni.filter());
  case ExpandEntityReferences:
    return Boolean(ni.expandEntityReferences());
 default:
   kdDebug(6070) << "WARNING: Unhandled token in DOMNodeIterator::getValueProperty : " << token << endl;
   return 0;
  }
}

ValueImp* DOMNodeIteratorProtoFunc::tryCall(ExecState *exec, ObjectImp* thisObj, const List &)
{
  KJS_CHECK_THIS( KJS::DOMNodeIterator, thisObj );
  DOM::NodeIterator nodeIterator = static_cast<DOMNodeIterator *>(thisObj)->toNodeIterator();
  switch (id) {
  case DOMNodeIterator::PreviousNode:
    return getDOMNode(exec,nodeIterator.previousNode());
  case DOMNodeIterator::NextNode:
    return getDOMNode(exec,nodeIterator.nextNode());
  case DOMNodeIterator::Detach:
    nodeIterator.detach();
    return Undefined();
  }
  return Undefined();
}

ValueImp *KJS::getDOMNodeIterator(ExecState *exec, DOM::NodeIterator ni)
{
  return cacheDOMObject<DOM::NodeIterator, DOMNodeIterator>(exec, ni);
}


// -------------------------------------------------------------------------

const ClassInfo NodeFilterConstructor::info = { "NodeFilterConstructor", 0, &NodeFilterConstructorTable, 0 };
/*
@begin NodeFilterConstructorTable 17
  FILTER_ACCEPT		DOM::NodeFilter::FILTER_ACCEPT	DontDelete|ReadOnly
  FILTER_REJECT		DOM::NodeFilter::FILTER_REJECT	DontDelete|ReadOnly
  FILTER_SKIP		DOM::NodeFilter::FILTER_SKIP	DontDelete|ReadOnly
  SHOW_ALL		DOM::NodeFilter::SHOW_ALL	DontDelete|ReadOnly
  SHOW_ELEMENT		DOM::NodeFilter::SHOW_ELEMENT	DontDelete|ReadOnly
  SHOW_ATTRIBUTE	DOM::NodeFilter::SHOW_ATTRIBUTE	DontDelete|ReadOnly
  SHOW_TEXT		DOM::NodeFilter::SHOW_TEXT	DontDelete|ReadOnly
  SHOW_CDATA_SECTION	DOM::NodeFilter::SHOW_CDATA_SECTION	DontDelete|ReadOnly
  SHOW_ENTITY_REFERENCE	DOM::NodeFilter::SHOW_ENTITY_REFERENCE	DontDelete|ReadOnly
  SHOW_ENTITY		DOM::NodeFilter::SHOW_ENTITY	DontDelete|ReadOnly
  SHOW_PROCESSING_INSTRUCTION	DOM::NodeFilter::SHOW_PROCESSING_INSTRUCTION	DontDelete|ReadOnly
  SHOW_COMMENT		DOM::NodeFilter::SHOW_COMMENT	DontDelete|ReadOnly
  SHOW_DOCUMENT		DOM::NodeFilter::SHOW_DOCUMENT	DontDelete|ReadOnly
  SHOW_DOCUMENT_TYPE	DOM::NodeFilter::SHOW_DOCUMENT_TYPE	DontDelete|ReadOnly
  SHOW_DOCUMENT_FRAGMENT	DOM::NodeFilter::SHOW_DOCUMENT_FRAGMENT	DontDelete|ReadOnly
  SHOW_NOTATION		DOM::NodeFilter::SHOW_NOTATION	DontDelete|ReadOnly
@end
*/

NodeFilterConstructor::NodeFilterConstructor(ExecState* exec)
  : DOMObject(exec->interpreter()->builtinObjectPrototype())
{
}

ValueImp *NodeFilterConstructor::tryGet(ExecState *exec, const Identifier &p) const
{
  return DOMObjectLookupGetValue<NodeFilterConstructor,DOMObject>(exec,p,&NodeFilterConstructorTable,this);
}

ValueImp *NodeFilterConstructor::getValueProperty(ExecState *, int token) const
{
  // We use the token as the value to return directly
  return Number(token);
}

ValueImp *KJS::getNodeFilterConstructor(ExecState *exec)
{
  return cacheGlobalObject<NodeFilterConstructor>(exec, "[[nodeFilter.constructor]]");
}

// -------------------------------------------------------------------------

const ClassInfo DOMNodeFilter::info = { "NodeFilter", 0, 0, 0 };
/*
@begin DOMNodeFilterProtoTable 1
  acceptNode	DOMNodeFilter::AcceptNode	DontDelete|Function 0
@end
*/
DEFINE_PROTOTYPE("DOMNodeFilter",DOMNodeFilterProto)
IMPLEMENT_PROTOFUNC_DOM(DOMNodeFilterProtoFunc)
IMPLEMENT_PROTOTYPE(DOMNodeFilterProto,DOMNodeFilterProtoFunc)

DOMNodeFilter::DOMNodeFilter(ExecState *exec, DOM::NodeFilter nf)
  : nodeFilter(nf) {
  setPrototype(DOMNodeFilterProto::self(exec));
}

DOMNodeFilter::~DOMNodeFilter()
{
  ScriptInterpreter::forgetDOMObject(nodeFilter.handle());
}

ValueImp *DOMNodeFilterProtoFunc::tryCall(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMNodeFilter, thisObj );
  DOM::NodeFilter nodeFilter = static_cast<DOMNodeFilter *>(thisObj)->toNodeFilter();
  switch (id) {
    case DOMNodeFilter::AcceptNode:
      return Number(nodeFilter.acceptNode(toNode(args[0])));
  }
  return Undefined();
}

ValueImp *KJS::getDOMNodeFilter(ExecState *exec, DOM::NodeFilter nf)
{
  return cacheDOMObject<DOM::NodeFilter, DOMNodeFilter>(exec, nf);
}

// -------------------------------------------------------------------------

const ClassInfo DOMTreeWalker::info = { "TreeWalker", 0, &DOMTreeWalkerTable, 0 };
/*
@begin DOMTreeWalkerTable 5
  root			DOMTreeWalker::Root		DontDelete|ReadOnly
  whatToShow		DOMTreeWalker::WhatToShow	DontDelete|ReadOnly
  filter		DOMTreeWalker::Filter		DontDelete|ReadOnly
  expandEntityReferences DOMTreeWalker::ExpandEntityReferences	DontDelete|ReadOnly
  currentNode		DOMTreeWalker::CurrentNode	DontDelete
@end
@begin DOMTreeWalkerProtoTable 7
  parentNode	DOMTreeWalker::ParentNode	DontDelete|Function 0
  firstChild	DOMTreeWalker::FirstChild	DontDelete|Function 0
  lastChild	DOMTreeWalker::LastChild	DontDelete|Function 0
  previousSibling DOMTreeWalker::PreviousSibling	DontDelete|Function 0
  nextSibling	DOMTreeWalker::NextSibling	DontDelete|Function 0
  previousNode	DOMTreeWalker::PreviousNode	DontDelete|Function 0
  nextNode	DOMTreeWalker::NextNode		DontDelete|Function 0
@end
*/
DEFINE_PROTOTYPE("DOMTreeWalker",DOMTreeWalkerProto)
IMPLEMENT_PROTOFUNC_DOM(DOMTreeWalkerProtoFunc)
IMPLEMENT_PROTOTYPE(DOMTreeWalkerProto,DOMTreeWalkerProtoFunc)

DOMTreeWalker::DOMTreeWalker(ExecState *exec, DOM::TreeWalker tw)
  : treeWalker(tw) {
  setPrototype(DOMTreeWalkerProto::self(exec));
}

DOMTreeWalker::~DOMTreeWalker()
{
  ScriptInterpreter::forgetDOMObject(treeWalker.handle());
}

ValueImp *DOMTreeWalker::tryGet(ExecState *exec, const Identifier &p) const
{
  return DOMObjectLookupGetValue<DOMTreeWalker,DOMObject>(exec,p,&DOMTreeWalkerTable,this);
}

ValueImp *DOMTreeWalker::getValueProperty(ExecState *exec, int token) const
{
  DOM::TreeWalker tw(treeWalker);
  switch (token) {
  case Root:
    return getDOMNode(exec,tw.root());
  case WhatToShow:
    return Number(tw.whatToShow());
  case Filter:
    return getDOMNodeFilter(exec,tw.filter());
  case ExpandEntityReferences:
    return Boolean(tw.expandEntityReferences());
  case CurrentNode:
    return getDOMNode(exec,tw.currentNode());
  default:
    kdDebug(6070) << "WARNING: Unhandled token in DOMTreeWalker::getValueProperty : " << token << endl;
    return 0;
  }
}

void DOMTreeWalker::tryPut(ExecState *exec, const Identifier &propertyName,
                           ValueImp *value, int attr)
{
  if (propertyName == "currentNode") {
    treeWalker.setCurrentNode(toNode(value));
  }
  else
    ObjectImp::put(exec, propertyName, value, attr);
}

ValueImp* DOMTreeWalkerProtoFunc::tryCall(ExecState *exec, ObjectImp *thisObj, const List &)
{
  KJS_CHECK_THIS( KJS::DOMTreeWalker, thisObj );
  DOM::TreeWalker treeWalker = static_cast<DOMTreeWalker *>(thisObj)->toTreeWalker();
  switch (id) {
    case DOMTreeWalker::ParentNode:
      return getDOMNode(exec,treeWalker.parentNode());
    case DOMTreeWalker::FirstChild:
      return getDOMNode(exec,treeWalker.firstChild());
    case DOMTreeWalker::LastChild:
      return getDOMNode(exec,treeWalker.lastChild());
    case DOMTreeWalker::PreviousSibling:
      return getDOMNode(exec,treeWalker.previousSibling());
    case DOMTreeWalker::NextSibling:
      return getDOMNode(exec,treeWalker.nextSibling());
    case DOMTreeWalker::PreviousNode:
      return getDOMNode(exec,treeWalker.previousSibling());
    case DOMTreeWalker::NextNode:
      return getDOMNode(exec,treeWalker.nextNode());
  }
  return Undefined();
}

ValueImp *KJS::getDOMTreeWalker(ExecState *exec, DOM::TreeWalker tw)
{
  return cacheDOMObject<DOM::TreeWalker, DOMTreeWalker>(exec, tw);
}

DOM::NodeFilter KJS::toNodeFilter(ValueImp *val)
{
  ObjectImp *obj = val->getObject();
  if (!obj || !obj->inherits(&DOMNodeFilter::info))
    return DOM::NodeFilter();

  const DOMNodeFilter *dobj = static_cast<const DOMNodeFilter*>(obj);
  return dobj->toNodeFilter();
}

// -------------------------------------------------------------------------

JSNodeFilter::JSNodeFilter(ObjectImp *_filter) : DOM::CustomNodeFilter(), filter( _filter )
{
}

JSNodeFilter::~JSNodeFilter()
{
}

short JSNodeFilter::acceptNode(const DOM::Node &n)
{
  KHTMLView *view = static_cast<DOM::DocumentImpl *>( n.handle()->docPtr()->document() )->view();
  if (!view)
      return DOM::NodeFilter::FILTER_REJECT;

  KHTMLPart *part = view->part();
  KJSProxy *proxy = part->jScript();
  if (proxy) {
    ExecState *exec = proxy->interpreter()->globalExec();
    ObjectImp *acceptNodeFunc = filter->get(exec, "acceptNode")->getObject();
    if (acceptNodeFunc && acceptNodeFunc->implementsCall()) {
      List args;
      args.append(getDOMNode(exec,n));
      ValueImp *result = acceptNodeFunc->call(exec,filter,args);
      if (exec->hadException())
	exec->clearException();
      return result->toInteger(exec);
    }
  }

  return DOM::NodeFilter::FILTER_REJECT;
}
