// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
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
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#include "nodes.h"

//#include <iostream>
#include <math.h>
#include <assert.h>
#ifdef KJS_DEBUG_MEM
#include <stdio.h>
#include <typeinfo>
#endif

#include "collector.h"
#include "debugger.h"
#include "function_object.h"
#include "internal.h"
#include "value.h"
#include "object.h"
#include "types.h"
#include "interpreter.h"
#include "lexer.h"
#include "operations.h"
#include "ustring.h"

using namespace KJS;

#define KJS_BREAKPOINT \
  if (!hitStatement(exec)) \
    return Completion(Normal);

#define KJS_ABORTPOINT \
  if (exec->interpreter()->imp()->debugger() && \
      exec->interpreter()->imp()->debugger()->imp()->aborted()) \
    return Completion(Normal);

#define KJS_CHECKEXCEPTION \
  if (exec->hadException()) \
    return Completion(Throw, exec->exception()); \
  if (Collector::outOfMemory()) \
    return Completion(Throw, Error::create(exec,GeneralError,"Out of memory"));

#define KJS_CHECKEXCEPTIONVALUE \
  if (exec->hadException()) \
    return exec->exception(); \
  if (Collector::outOfMemory()) \
    return Undefined(); // will be picked up by KJS_CHECKEXCEPTION

#define KJS_CHECKEXCEPTIONLIST \
  if (exec->hadException()) \
    return List(); \
  if (Collector::outOfMemory()) \
    return List(); // will be picked up by KJS_CHECKEXCEPTION

#ifdef KJS_DEBUG_MEM
std::list<Node *> * Node::s_nodes = 0L;
#endif
// ------------------------------ Node -----------------------------------------

Node::Node()
{
  line = Lexer::curr()->lineNo();
  refcount = 0;
#ifdef KJS_DEBUG_MEM
  if (!s_nodes)
    s_nodes = new std::list<Node *>;
  s_nodes->push_back(this);
#endif
}

Node::~Node()
{
#ifdef KJS_DEBUG_MEM
  s_nodes->remove( this );
#endif
}

#ifdef KJS_DEBUG_MEM
void Node::finalCheck()
{
  fprintf( stderr, "Node::finalCheck(): list count       : %d\n", s_nodes->size() );
  std::list<Node *>::iterator it = s_nodes->begin();
  for ( uint i = 0; it != s_nodes->end() ; ++it, ++i )
    fprintf( stderr, "[%d] Still having node %p (%s) (refcount %d)\n", i, (void*)*it, typeid( **it ).name(), (*it)->refcount );
  delete s_nodes;
  s_nodes = 0L;
}
#endif

Value Node::throwError(ExecState *exec, ErrorType e, const char *msg)
{
  Object err = Error::create(exec, e, msg, lineNo(), sourceId());
  exec->setException(err);
  return err;
}

// ------------------------------ StatementNode --------------------------------
StatementNode::StatementNode() : l0(-1), l1(-1), sid(-1), breakPoint(false)
{
}

StatementNode::~StatementNode()
{
}

void StatementNode::setLoc(int line0, int line1, int sourceId)
{
    l0 = line0;
    l1 = line1;
    sid = sourceId;
}

// return true if the debugger wants us to stop at this point
bool StatementNode::hitStatement(ExecState *exec)
{
  Debugger *dbg = exec->interpreter()->imp()->debugger();
  if (dbg)
    return dbg->atStatement(exec,sid,l0,l1);
  else
    return true; // continue
}

// return true if the debugger wants us to stop at this point
bool StatementNode::abortStatement(ExecState *exec)
{
  Debugger *dbg = exec->interpreter()->imp()->debugger();
  if (dbg)
    return dbg->imp()->aborted();
  else
    return false;
}

// ------------------------------ NullNode -------------------------------------

Value NullNode::evaluate(ExecState */*exec*/)
{
  return Null();
}

// ------------------------------ BooleanNode ----------------------------------

Value BooleanNode::evaluate(ExecState */*exec*/)
{
  return Boolean(value);
}

// ------------------------------ NumberNode -----------------------------------

Value NumberNode::evaluate(ExecState */*exec*/)
{
  return Number(value);
}

// ------------------------------ StringNode -----------------------------------

Value StringNode::evaluate(ExecState */*exec*/)
{
  return String(value);
}

// ------------------------------ RegExpNode -----------------------------------

Value RegExpNode::evaluate(ExecState *exec)
{
  List list;
  String p(pattern);
  String f(flags);
  list.append(p);
  list.append(f);

  Object reg = exec->interpreter()->imp()->builtinRegExp();
  return reg.construct(exec,list);
}

// ------------------------------ ThisNode -------------------------------------

// ECMA 11.1.1
Value ThisNode::evaluate(ExecState *exec)
{
  return exec->context().thisValue();
}

// ------------------------------ ResolveNode ----------------------------------

// ECMA 11.1.2 & 10.1.4
Value ResolveNode::evaluate(ExecState *exec)
{
  const List chain = exec->context().scopeChain();
  ListIterator scope = chain.begin();

  while (scope != chain.end()) {
    ObjectImp *o = static_cast<ObjectImp*>((*scope).imp());

    //cout << "Resolve: looking at '" << ident.ascii() << "'"
    //     << " in " << (void*)o << " " << o->classInfo()->className << endl;
    if (o->hasProperty(exec,ident)) {
      //cout << "Resolve: FOUND '" << ident.ascii() << "'"
      //     << " in " << (void*)o << " " << o->classInfo()->className << endl;
      return Reference(Object(o), ident);
    }
    scope++;
  }

  // identifier not found
  //cout << "Resolve: didn't find '" << ident.ascii() << "'" << endl;
  return Reference(Null(), ident);
}

// ------------------------------ GroupNode ------------------------------------

GroupNode::~GroupNode()
{
}

void GroupNode::ref()
{
  Node::ref();
  if ( group )
    group->ref();
}

bool GroupNode::deref()
{
  if ( group && group->deref() )
    delete group;
  return Node::deref();
}

// ECMA 11.1.6
Value GroupNode::evaluate(ExecState *exec)
{
  return group->evaluate(exec);
}

// ------------------------------ ElisionNode ----------------------------------

ElisionNode::~ElisionNode()
{
}

void ElisionNode::ref()
{
  Node::ref();
  if ( elision )
    elision->ref();
}

bool ElisionNode::deref()
{
  if ( elision && elision->deref() )
    delete elision;
  return Node::deref();
}

// ECMA 11.1.4
Value ElisionNode::evaluate(ExecState *exec)
{
  if (elision)
    return Number(elision->evaluate(exec).toNumber(exec) + 1);
  else
    return Number(1);
}

// ------------------------------ ElementNode ----------------------------------

ElementNode::~ElementNode()
{
}

void ElementNode::ref()
{
  Node::ref();
  if ( list )
    list->ref();
  if ( elision )
    elision->ref();
  if ( node )
    node->ref();
}

bool ElementNode::deref()
{
  if ( list && list->deref() )
    delete list;
  if ( elision && elision->deref() )
    delete elision;
  if ( node && node->deref() )
    delete node;
  return Node::deref();
}

// ECMA 11.1.4
Value ElementNode::evaluate(ExecState *exec)
{
  Object array;
  Value val;
  int length = 0;
  int elisionLen = elision ? elision->evaluate(exec).toInt32(exec) : 0;
  KJS_CHECKEXCEPTIONVALUE

  if (list) {
    array = Object(static_cast<ObjectImp*>(list->evaluate(exec).imp()));
    KJS_CHECKEXCEPTIONVALUE
    val = node->evaluate(exec).getValue(exec);
    length = array.get(exec,"length").toInt32(exec);
  } else {
    Value newArr = exec->interpreter()->builtinArray().construct(exec,List::empty());
    array = Object(static_cast<ObjectImp*>(newArr.imp()));
    val = node->evaluate(exec).getValue(exec);
    KJS_CHECKEXCEPTIONVALUE
  }

  array.put(exec, UString::from(elisionLen + length), val);

  return array;
}

// ------------------------------ ArrayNode ------------------------------------

ArrayNode::~ArrayNode()
{
}

void ArrayNode::ref()
{
  Node::ref();
  if ( element )
    element->ref();
  if ( elision )
    elision->ref();
}

bool ArrayNode::deref()
{
  if ( element && element->deref() )
    delete element;
  if ( elision && elision->deref() )
    delete elision;
  return Node::deref();
}

// ECMA 11.1.4
Value ArrayNode::evaluate(ExecState *exec)
{
  Object array;
  int length;
  int elisionLen = elision ? elision->evaluate(exec).toInt32(exec) : 0;
  KJS_CHECKEXCEPTIONVALUE

  if (element) {
    array = Object(static_cast<ObjectImp*>(element->evaluate(exec).imp()));
    KJS_CHECKEXCEPTIONVALUE
    length = opt ? array.get(exec,"length").toInt32(exec) : 0;
  } else {
    Value newArr = exec->interpreter()->builtinArray().construct(exec,List::empty());
    array = Object(static_cast<ObjectImp*>(newArr.imp()));
    length = 0;
  }

  if (opt)
    array.put(exec,"length", Number(elisionLen + length), DontEnum | DontDelete);

  return array;
}

// ------------------------------ ObjectLiteralNode ----------------------------

ObjectLiteralNode::~ObjectLiteralNode()
{
}

void ObjectLiteralNode::ref()
{
  Node::ref();
  if ( list )
    list->ref();
}

bool ObjectLiteralNode::deref()
{
  if ( list && list->deref() )
    delete list;
  return Node::deref();
}

// ECMA 11.1.5
Value ObjectLiteralNode::evaluate(ExecState *exec)
{
  if (list)
    return list->evaluate(exec);

  return exec->interpreter()->builtinObject().construct(exec,List::empty());
}

// ------------------------------ PropertyValueNode ----------------------------

PropertyValueNode::~PropertyValueNode()
{
}

void PropertyValueNode::ref()
{
  Node::ref();
  if ( name )
    name->ref();
  if ( assign )
    assign->ref();
  if ( list )
    list->ref();
}

bool PropertyValueNode::deref()
{
  if ( name && name->deref() )
    delete name;
  if ( assign && assign->deref() )
    delete assign;
  if ( list && list->deref() )
    delete list;
  return Node::deref();
}

// ECMA 11.1.5
Value PropertyValueNode::evaluate(ExecState *exec)
{
  Object obj;
  if (list) {
    obj = Object(static_cast<ObjectImp*>(list->evaluate(exec).imp()));
    KJS_CHECKEXCEPTIONVALUE
  }
  else {
    Value newObj = exec->interpreter()->builtinObject().construct(exec,List::empty());
    obj = Object(static_cast<ObjectImp*>(newObj.imp()));
  }
  Value n = name->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value a = assign->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v = a.getValue(exec);

  obj.put(exec,n.toString(exec), v);

  return obj;
}

// ------------------------------ PropertyNode ---------------------------------

// ECMA 11.1.5
Value PropertyNode::evaluate(ExecState */*exec*/)
{
  Value s;

  if (str.isNull()) {
    s = String(UString::from(numeric));
  } else
    s = String(str);

  return s;
}

// ------------------------------ AccessorNode1 --------------------------------

AccessorNode1::~AccessorNode1()
{
}

void AccessorNode1::ref()
{
  Node::ref();
  if ( expr1 )
    expr1->ref();
  if ( expr2 )
    expr2->ref();
}

bool AccessorNode1::deref()
{
  if ( expr1 && expr1->deref() )
    delete expr1;
  if ( expr2 && expr2->deref() )
    delete expr2;
  return Node::deref();
}

// ECMA 11.2.1a
Value AccessorNode1::evaluate(ExecState *exec)
{
  Value e1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v1 = e1.getValue(exec);
  Value e2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v2 = e2.getValue(exec);
  Object o = v1.toObject(exec);
  String s = v2.toString(exec);
  return Reference(o, s.value());
}

// ------------------------------ AccessorNode2 --------------------------------

AccessorNode2::~AccessorNode2()
{
}

void AccessorNode2::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool AccessorNode2::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.2.1b
Value AccessorNode2::evaluate(ExecState *exec)
{
  Value e = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v = e.getValue(exec);
  Object o = v.toObject(exec);
  return Reference(o, ident);
}

// ------------------------------ ArgumentListNode -----------------------------

ArgumentListNode::ArgumentListNode(Node *e) : list(0L), expr(e)
{
}

ArgumentListNode::ArgumentListNode(ArgumentListNode *l, Node *e)
  : list(l), expr(e)
{
}

ArgumentListNode::~ArgumentListNode()
{
}

void ArgumentListNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
  if ( list )
    list->ref();
}

bool ArgumentListNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  if ( list && list->deref() )
    delete list;
  return Node::deref();
}

Value ArgumentListNode::evaluate(ExecState */*exec*/)
{
  assert(0);
  return Value(); // dummy, see evaluateList()
}

// ECMA 11.2.4
List ArgumentListNode::evaluateList(ExecState *exec)
{
  List l;
  if (list) {
    l = list->evaluateList(exec);
    KJS_CHECKEXCEPTIONLIST
  }

  Value e = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONLIST
  Value v = e.getValue(exec);

  l.append(v);

  return l;
}

// ------------------------------ ArgumentsNode --------------------------------

ArgumentsNode::ArgumentsNode(ArgumentListNode *l) : list(l)
{
}

ArgumentsNode::~ArgumentsNode()
{
}

void ArgumentsNode::ref()
{
  Node::ref();
  if ( list )
    list->ref();
}

bool ArgumentsNode::deref()
{
  if ( list && list->deref() )
    delete list;
  return Node::deref();
}

Value ArgumentsNode::evaluate(ExecState */*exec*/)
{
  assert(0);
  return Value(); // dummy, see evaluateList()
}

// ECMA 11.2.4
List ArgumentsNode::evaluateList(ExecState *exec)
{
  if (!list)
    return List();

  return list->evaluateList(exec);
}

// ------------------------------ NewExprNode ----------------------------------

// ECMA 11.2.2

NewExprNode::~NewExprNode()
{
}

void NewExprNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
  if ( args )
    args->ref();
}

bool NewExprNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  if ( args && args->deref() )
    delete args;
  return Node::deref();
}

Value NewExprNode::evaluate(ExecState *exec)
{
  Value e = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v = e.getValue(exec);

  List argList;
  if (args) {
    argList = args->evaluateList(exec);
    KJS_CHECKEXCEPTIONVALUE
  }

  if (v.type() != ObjectType) {
    return throwError(exec, TypeError, "Expression is no object. Cannot be new'ed");
  }

  Object constr = Object(static_cast<ObjectImp*>(v.imp()));
  if (!constr.implementsConstruct()) {
    return throwError(exec, TypeError, "Expression is no constructor.");
  }

  Value res = constr.construct(exec,argList);

  return res;
}

// ------------------------------ FunctionCallNode -----------------------------

FunctionCallNode::~FunctionCallNode()
{
}

void FunctionCallNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
  if ( args )
    args->ref();
}

bool FunctionCallNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  if ( args && args->deref() )
    delete args;
  return Node::deref();
}

// ECMA 11.2.3
Value FunctionCallNode::evaluate(ExecState *exec)
{
  Value e = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  List argList = args->evaluateList(exec);

  KJS_CHECKEXCEPTIONVALUE

  Value v = e.getValue(exec);

  if (v.type() != ObjectType) {
#ifndef NDEBUG
    printInfo(exec, "WARNING: Failed function call attempt on", e, line);
#endif
    return throwError(exec, TypeError, "Expression is no object. Cannot be called.");
  }

  Object func = Object(static_cast<ObjectImp*>(v.imp()));

  if (!func.implementsCall()) {
#ifndef NDEBUG
    printInfo(exec, "Failed function call attempt on", e, line);
#endif
    return throwError(exec, TypeError, "Expression does not allow calls.");
  }

  Value thisVal;
  if (e.type() == ReferenceType)
    thisVal = e.getBase(exec);
  else
    thisVal = Null();

  if (thisVal.type() == ObjectType &&
      Object::dynamicCast(thisVal).inherits(&ActivationImp::info))
    thisVal = Null();

  if (thisVal.type() != ObjectType) {
    // ECMA 11.2.3 says that in this situation the this value should be null.
    // However, section 10.2.3 says that in the case where the value provided
    // by the caller is null, the global object should be used. It also says
    // that the section does not apply to interal functions, but for simplicity
    // of implementation we use the global object anyway here. This guarantees
    // that in host objects you always get a valid object for this.
    // thisVal = Null();
    thisVal = exec->interpreter()->globalObject();
  }

  Object thisObj = Object::dynamicCast(thisVal);
  Value result = func.call(exec,thisObj, argList);

  return result;
}

// ------------------------------ PostfixNode ----------------------------------

PostfixNode::~PostfixNode()
{
}

void PostfixNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool PostfixNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.3
Value PostfixNode::evaluate(ExecState *exec)
{
  Value e = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v = e.getValue(exec);
  Number n = v.toNumber(exec);

  double newValue = (oper == OpPlusPlus) ? n.value() + 1 : n.value() - 1;
  Value n2 = Number(newValue);

  e.putValue(exec,n2);

  return n;
}

// ------------------------------ DeleteNode -----------------------------------

DeleteNode::~DeleteNode()
{
}

void DeleteNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool DeleteNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.4.1
Value DeleteNode::evaluate(ExecState *exec)
{
  Value e = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  if (e.type() != ReferenceType)
    return Boolean(true);
  Value b = e.getBase(exec);
  UString n = e.getPropertyName(exec);

  // The spec doesn't mention what to do if the base is null... just return true
  if (b.type() != ObjectType) {
    assert(b.type() == NullType);
    return Boolean(true);
  }

  Object o = Object(static_cast<ObjectImp*>(b.imp()));

  bool ret = o.deleteProperty(exec,n);

  return Boolean(ret);
}

// ------------------------------ VoidNode -------------------------------------

VoidNode::~VoidNode()
{
}

void VoidNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool VoidNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.4.2
Value VoidNode::evaluate(ExecState *exec)
{
  Value dummy1 = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value dummy2 = dummy1.getValue(exec);

  return Undefined();
}

// ------------------------------ TypeOfNode -----------------------------------

TypeOfNode::~TypeOfNode()
{
}

void TypeOfNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool TypeOfNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.4.3
Value TypeOfNode::evaluate(ExecState *exec)
{
  const char *s = 0L;
  Value e = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  if (e.type() == ReferenceType) {
    Value b = e.getBase(exec);
    if (b.type() == NullType)
      return String("undefined");
  }
  Value v = e.getValue(exec);
  switch (v.type())
    {
    case UndefinedType:
      s = "undefined";
      break;
    case NullType:
      s = "object";
      break;
    case BooleanType:
      s = "boolean";
      break;
    case NumberType:
      s = "number";
      break;
    case StringType:
      s = "string";
      break;
    default:
      if (v.type() == ObjectType && static_cast<ObjectImp*>(v.imp())->implementsCall())
	s = "function";
      else
	s = "object";
      break;
    }

  return String(s);
}

// ------------------------------ PrefixNode -----------------------------------

PrefixNode::~PrefixNode()
{
}

void PrefixNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool PrefixNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.4.4 and 11.4.5
Value PrefixNode::evaluate(ExecState *exec)
{
  Value e = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v = e.getValue(exec);
  Number n = v.toNumber(exec);

  double newValue = (oper == OpPlusPlus) ? n.value() + 1 : n.value() - 1;
  Value n2 = Number(newValue);

  e.putValue(exec,n2);

  return n2;
}

// ------------------------------ UnaryPlusNode --------------------------------

UnaryPlusNode::~UnaryPlusNode()
{
}

void UnaryPlusNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool UnaryPlusNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.4.6
Value UnaryPlusNode::evaluate(ExecState *exec)
{
  Value e = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v = e.getValue(exec);

  return Number(v.toNumber(exec)); /* TODO: optimize */
}

// ------------------------------ NegateNode -----------------------------------

NegateNode::~NegateNode()
{
}

void NegateNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool NegateNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.4.7
Value NegateNode::evaluate(ExecState *exec)
{
  Value e = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v = e.getValue(exec);
  Number n = v.toNumber(exec);

  double d = -n.value();

  return Number(d);
}

// ------------------------------ BitwiseNotNode -------------------------------

BitwiseNotNode::~BitwiseNotNode()
{
}

void BitwiseNotNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool BitwiseNotNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.4.8
Value BitwiseNotNode::evaluate(ExecState *exec)
{
  Value e = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v = e.getValue(exec);
  int i32 = v.toInt32(exec);

  return Number(~i32);
}

// ------------------------------ LogicalNotNode -------------------------------

LogicalNotNode::~LogicalNotNode()
{
}

void LogicalNotNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool LogicalNotNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.4.9
Value LogicalNotNode::evaluate(ExecState *exec)
{
  Value e = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v = e.getValue(exec);
  bool b = v.toBoolean(exec);

  return Boolean(!b);
}

// ------------------------------ MultNode -------------------------------------

MultNode::~MultNode()
{
}

void MultNode::ref()
{
  Node::ref();
  if ( term1 )
    term1->ref();
  if ( term2 )
    term2->ref();
}

bool MultNode::deref()
{
  if ( term1 && term1->deref() )
    delete term1;
  if ( term2 && term2->deref() )
    delete term2;
  return Node::deref();
}

// ECMA 11.5
Value MultNode::evaluate(ExecState *exec)
{
  Value t1 = term1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v1 = t1.getValue(exec);

  Value t2 = term2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v2 = t2.getValue(exec);

  return mult(exec,v1, v2, oper);
}

// ------------------------------ AddNode --------------------------------------

AddNode::~AddNode()
{
}

void AddNode::ref()
{
  Node::ref();
  if ( term1 )
    term1->ref();
  if ( term2 )
    term2->ref();
}

bool AddNode::deref()
{
  if ( term1 && term1->deref() )
    delete term1;
  if ( term2 && term2->deref() )
    delete term2;
  return Node::deref();
}

// ECMA 11.6
Value AddNode::evaluate(ExecState *exec)
{
  Value t1 = term1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v1 = t1.getValue(exec);

  Value t2 = term2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v2 = t2.getValue(exec);

  return add(exec,v1, v2, oper);
}

// ------------------------------ ShiftNode ------------------------------------

ShiftNode::~ShiftNode()
{
}

void ShiftNode::ref()
{
  Node::ref();
  if ( term1 )
    term1->ref();
  if ( term2 )
    term2->ref();
}

bool ShiftNode::deref()
{
  if ( term1 && term1->deref() )
    delete term1;
  if ( term2 && term2->deref() )
    delete term2;
  return Node::deref();
}

// ECMA 11.7
Value ShiftNode::evaluate(ExecState *exec)
{
  Value t1 = term1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v1 = t1.getValue(exec);
  Value t2 = term2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v2 = t2.getValue(exec);
  unsigned int i2 = v2.toUInt32(exec);
  i2 &= 0x1f;

  long result;
  switch (oper) {
  case OpLShift:
    result = v1.toInt32(exec) << i2;
    break;
  case OpRShift:
    result = v1.toInt32(exec) >> i2;
    break;
  case OpURShift:
    result = v1.toUInt32(exec) >> i2;
    break;
  default:
    assert(!"ShiftNode: unhandled switch case");
    result = 0L;
  }

  return Number(static_cast<double>(result));
}

// ------------------------------ RelationalNode -------------------------------

RelationalNode::~RelationalNode()
{
}

void RelationalNode::ref()
{
  Node::ref();
  if ( expr1 )
    expr1->ref();
  if ( expr2 )
    expr2->ref();
}

bool RelationalNode::deref()
{
  if ( expr1 && expr1->deref() )
    delete expr1;
  if ( expr2 && expr2->deref() )
    delete expr2;
  return Node::deref();
}

// ECMA 11.8
Value RelationalNode::evaluate(ExecState *exec)
{
  Value e1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v1 = e1.getValue(exec);
  Value e2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v2 = e2.getValue(exec);

  bool b;
  if (oper == OpLess || oper == OpGreaterEq) {
    int r = relation(exec, v1, v2);
    if (r < 0)
      b = false;
    else
      b = (oper == OpLess) ? (r == 1) : (r == 0);
  } else if (oper == OpGreater || oper == OpLessEq) {
    int r = relation(exec, v2, v1);
    if (r < 0)
      b = false;
    else
      b = (oper == OpGreater) ? (r == 1) : (r == 0);
  } else if (oper == OpIn) {
      // Is all of this OK for host objects?
      if (v2.type() != ObjectType)
          return throwError(exec,  TypeError,
                             "Shift expression not an object into IN expression." );
      Object o2(static_cast<ObjectImp*>(v2.imp()));
      b = o2.hasProperty(exec,v1.toString(exec));
  } else {
    if (v2.type() != ObjectType)
        return throwError(exec,  TypeError,
                           "Called instanceof operator on non-object." );

    Object o2(static_cast<ObjectImp*>(v2.imp()));
    if (!o2.implementsHasInstance()) {
      // According to the spec, only some types of objects "imlement" the [[HasInstance]] property.
      // But we are supposed to throw an exception where the object does not "have" the [[HasInstance]]
      // property. It seems that all object have the property, but not all implement it, so in this
      // case we return false (consistent with mozilla)
      return Boolean(false);
      //      return throwError(exec, TypeError,
      //			"Object does not implement the [[HasInstance]] method." );
    }
    return o2.hasInstance(exec, v1);
  }

  return Boolean(b);
}

// ------------------------------ EqualNode ------------------------------------

EqualNode::~EqualNode()
{
}

void EqualNode::ref()
{
  Node::ref();
  if ( expr1 )
    expr1->ref();
  if ( expr2 )
    expr2->ref();
}

bool EqualNode::deref()
{
  if ( expr1 && expr1->deref() )
    delete expr1;
  if ( expr2 && expr2->deref() )
    delete expr2;
  return Node::deref();
}

// ECMA 11.9
Value EqualNode::evaluate(ExecState *exec)
{
  Value e1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value e2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v1 = e1.getValue(exec);
  Value v2 = e2.getValue(exec);

  bool result;
  if (oper == OpEqEq || oper == OpNotEq) {
    // == and !=
    bool eq = equal(exec,v1, v2);
    result = oper == OpEqEq ? eq : !eq;
  } else {
    // === and !==
    bool eq = strictEqual(exec,v1, v2);
    result = oper == OpStrEq ? eq : !eq;
  }
  return Boolean(result);
}

// ------------------------------ BitOperNode ----------------------------------

BitOperNode::~BitOperNode()
{
}

void BitOperNode::ref()
{
  Node::ref();
  if ( expr1 )
    expr1->ref();
  if ( expr2 )
    expr2->ref();
}

bool BitOperNode::deref()
{
  if ( expr1 && expr1->deref() )
    delete expr1;
  if ( expr2 && expr2->deref() )
    delete expr2;
  return Node::deref();
}

// ECMA 11.10
Value BitOperNode::evaluate(ExecState *exec)
{
  Value e1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v1 = e1.getValue(exec);
  Value e2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v2 = e2.getValue(exec);
  int i1 = v1.toInt32(exec);
  int i2 = v2.toInt32(exec);
  int result;
  if (oper == OpBitAnd)
    result = i1 & i2;
  else if (oper == OpBitXOr)
    result = i1 ^ i2;
  else
    result = i1 | i2;

  return Number(result);
}

// ------------------------------ BinaryLogicalNode ----------------------------

BinaryLogicalNode::~BinaryLogicalNode()
{
}

void BinaryLogicalNode::ref()
{
  Node::ref();
  if ( expr1 )
    expr1->ref();
  if ( expr2 )
    expr2->ref();
}

bool BinaryLogicalNode::deref()
{
  if ( expr1 && expr1->deref() )
    delete expr1;
  if ( expr2 && expr2->deref() )
    delete expr2;
  return Node::deref();
}

// ECMA 11.11
Value BinaryLogicalNode::evaluate(ExecState *exec)
{
  Value e1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v1 = e1.getValue(exec);
  bool b1 = v1.toBoolean(exec);
  if ((!b1 && oper == OpAnd) || (b1 && oper == OpOr))
    return v1;

  Value e2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v2 = e2.getValue(exec);

  return v2;
}

// ------------------------------ ConditionalNode ------------------------------

ConditionalNode::~ConditionalNode()
{
}

void ConditionalNode::ref()
{
  Node::ref();
  if ( expr1 )
    expr1->ref();
  if ( expr2 )
    expr2->ref();
  if ( logical )
    logical->ref();
}

bool ConditionalNode::deref()
{
  if ( expr1 && expr1->deref() )
    delete expr1;
  if ( expr2 && expr2->deref() )
    delete expr2;
  if ( logical && logical->deref() )
    delete logical;
  return Node::deref();
}

// ECMA 11.12
Value ConditionalNode::evaluate(ExecState *exec)
{
  Value e = logical->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v = e.getValue(exec);
  bool b = v.toBoolean(exec);

  if (b)
    e = expr1->evaluate(exec);
  else
    e = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return e.getValue(exec);
}

// ------------------------------ AssignNode -----------------------------------

AssignNode::~AssignNode()
{
}

void AssignNode::ref()
{
  Node::ref();
  if ( left )
    left->ref();
  if ( expr )
    expr->ref();
}

bool AssignNode::deref()
{
  if ( left && left->deref() )
    delete left;
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.13
Value AssignNode::evaluate(ExecState *exec)
{

  Value l, e, v;
  if (oper == OpEqual) {
    l = left->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE
    e = expr->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE
    v = e.getValue(exec);
  } else {
    l = left->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE
    Value v1 = l.getValue(exec);
    e = expr->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE
    Value v2 = e.getValue(exec);
    int i1 = v1.toInt32(exec);
    int i2 = v2.toInt32(exec);
    unsigned int ui;
    switch (oper) {
    case OpMultEq:
      v = mult(exec, v1, v2, '*');
      break;
    case OpDivEq:
      v = mult(exec, v1, v2, '/');
      break;
    case OpPlusEq:
      v = add(exec, v1, v2, '+');
      break;
    case OpMinusEq:
      v = add(exec, v1, v2, '-');
      break;
    case OpLShift:
      v = Number(i1 <<= i2);
      break;
    case OpRShift:
      v = Number(i1 >>= i2);
      break;
    case OpURShift:
      ui = v1.toUInt32(exec);
      v = Number(ui >>= i2);
      break;
    case OpAndEq:
      v = Number(i1 &= i2);
      break;
    case OpXOrEq:
      v = Number(i1 ^= i2);
      break;
    case OpOrEq:
      v = Number(i1 |= i2);
      break;
    case OpModEq: {
      double d1 = v1.toNumber(exec);
      double d2 = v2.toNumber(exec);
      v = Number(fmod(d1,d2));
    }
      break;
    default:
      v = Undefined();
    }
  };
  l.putValue(exec,v);

  KJS_CHECKEXCEPTIONVALUE

  return v;
}

// ------------------------------ CommaNode ------------------------------------

CommaNode::~CommaNode()
{
}

void CommaNode::ref()
{
  Node::ref();
  if ( expr1 )
    expr1->ref();
  if ( expr2 )
    expr2->ref();
}

bool CommaNode::deref()
{
  if ( expr1 && expr1->deref() )
    delete expr1;
  if ( expr2 && expr2->deref() )
    delete expr2;
  return Node::deref();
}

// ECMA 11.14
Value CommaNode::evaluate(ExecState *exec)
{
  Value e = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value dummy = e.getValue(exec); // ignore return value
  e = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return e.getValue(exec);
}

// ------------------------------ StatListNode ---------------------------------

StatListNode::~StatListNode()
{
}

void StatListNode::ref()
{
  Node::ref();
  if ( statement )
    statement->ref();
  if ( list )
    list->ref();
}

bool StatListNode::deref()
{
  if ( statement && statement->deref() )
    delete statement;
  if ( list && list->deref() )
    delete list;
  return Node::deref();
}

// ECMA 12.1
Completion StatListNode::execute(ExecState *exec)
{
  if (!list) {
    Completion c = statement->execute(exec);
    KJS_ABORTPOINT
    if (exec->hadException()) {
      Value ex = exec->exception();
      exec->clearException();
      return Completion(Throw, ex);
    }
    else
      return c;
  }

  Completion l = list->execute(exec);
  KJS_ABORTPOINT
  if (l.complType() != Normal)
    return l;
  Completion e = statement->execute(exec);
  KJS_ABORTPOINT;

  if (exec->hadException()) {
    Value ex = exec->exception();
    exec->clearException();
    return Completion(Throw, ex);
  }

  Value v = e.isValueCompletion() ? e.value() : l.value();

  return Completion(e.complType(), v, e.target() );
}

void StatListNode::processVarDecls(ExecState *exec)
{
  statement->processVarDecls(exec);

  if (list)
    list->processVarDecls(exec);
}

// ------------------------------ AssignExprNode -------------------------------

AssignExprNode::~AssignExprNode()
{
}

void AssignExprNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool AssignExprNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 12.2
Value AssignExprNode::evaluate(ExecState *exec)
{
  return expr->evaluate(exec);
}

// ------------------------------ VarDeclNode ----------------------------------

VarDeclNode::VarDeclNode(const UString *id, AssignExprNode *in)
    : ident(*id), init(in)
{
}

VarDeclNode::~VarDeclNode()
{
}

void VarDeclNode::ref()
{
  Node::ref();
  if ( init )
    init->ref();
}

bool VarDeclNode::deref()
{
  if ( init && init->deref() )
    delete init;
  return Node::deref();
}

// ECMA 12.2
Value VarDeclNode::evaluate(ExecState *exec)
{
  Object variable = Object::dynamicCast(exec->context().variableObject());

  Value val, tmp;
  if (init) {
      tmp = init->evaluate(exec);
      KJS_CHECKEXCEPTIONVALUE
      val = tmp.getValue(exec);
  } else {
      if ( variable.hasProperty(exec, ident ) ) // already declared ?
          return Value();
      val = Undefined();
  }

#ifdef KJS_VERBOSE
  printInfo(exec,(UString("new variable ")+ident).cstring().c_str(),val);
#endif
  // We use Internal to bypass all checks in derived objects, e.g. so that
  // "var location" creates a dynamic property instead of activating window.location.
  variable.put(exec, ident, val, DontDelete | Internal);

  return String(ident);
}

void VarDeclNode::processVarDecls(ExecState *exec)
{
  Object variable = exec->context().variableObject();
  variable.put(exec,ident, Undefined(), DontDelete);
}

// ------------------------------ VarDeclListNode ------------------------------

VarDeclListNode::~VarDeclListNode()
{
}

void VarDeclListNode::ref()
{
  Node::ref();
  if ( list )
    list->ref();
  if ( var )
    var->ref();
}

bool VarDeclListNode::deref()
{
  if ( list && list->deref() )
    delete list;
  if ( var && var->deref() )
    delete var;
  return Node::deref();
}


// ECMA 12.2
Value VarDeclListNode::evaluate(ExecState *exec)
{
  if (list)
    (void) list->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  (void) var->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return Undefined();
}

void VarDeclListNode::processVarDecls(ExecState *exec)
{
  if (list)
    list->processVarDecls(exec);

  var->processVarDecls(exec);
}

// ------------------------------ VarStatementNode -----------------------------

VarStatementNode::~VarStatementNode()
{
}

void VarStatementNode::ref()
{
  Node::ref();
  if ( list )
    list->ref();
}

bool VarStatementNode::deref()
{
  if ( list && list->deref() )
    delete list;
  return Node::deref();
}

// ECMA 12.2
Completion VarStatementNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  (void) list->evaluate(exec); // returns 0L
  KJS_CHECKEXCEPTION

  return Completion(Normal);
}

void VarStatementNode::processVarDecls(ExecState *exec)
{
  list->processVarDecls(exec);
}

// ------------------------------ BlockNode ------------------------------------

BlockNode::~BlockNode()
{
}

void BlockNode::ref()
{
  Node::ref();
  if ( source )
    source->ref();
}

bool BlockNode::deref()
{
  if ( source && source->deref() )
    delete source;
  return Node::deref();
}

// ECMA 12.1
Completion BlockNode::execute(ExecState *exec)
{
  if (!source)
    return Completion(Normal);

  source->processFuncDecl(exec);

  return source->execute(exec);
}

void BlockNode::processVarDecls(ExecState *exec)
{
  if (source)
    source->processVarDecls(exec);
}

// ------------------------------ EmptyStatementNode ---------------------------

// ECMA 12.3
Completion EmptyStatementNode::execute(ExecState */*exec*/)
{
  return Completion(Normal);
}

// ------------------------------ ExprStatementNode ----------------------------

ExprStatementNode::~ExprStatementNode()
{
}

void ExprStatementNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool ExprStatementNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 12.4
Completion ExprStatementNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Value e = expr->evaluate(exec);
  KJS_CHECKEXCEPTION
  Value v = e.getValue(exec);

  return Completion(Normal, v);
}

// ------------------------------ IfNode ---------------------------------------

IfNode::~IfNode()
{
}

void IfNode::ref()
{
  Node::ref();
  if ( statement1 )
    statement1->ref();
  if ( statement2 )
    statement2->ref();
  if ( expr )
    expr->ref();
}

bool IfNode::deref()
{
  if ( statement1 && statement1->deref() )
    delete statement1;
  if ( statement2 && statement2->deref() )
    delete statement2;
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 12.5
Completion IfNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Value e = expr->evaluate(exec);
  KJS_CHECKEXCEPTION
  Value v = e.getValue(exec);
  bool b = v.toBoolean(exec);

  // if ... then
  if (b)
    return statement1->execute(exec);

  // no else
  if (!statement2)
    return Completion(Normal);

  // else
  return statement2->execute(exec);
}

void IfNode::processVarDecls(ExecState *exec)
{
  statement1->processVarDecls(exec);

  if (statement2)
    statement2->processVarDecls(exec);
}

// ------------------------------ DoWhileNode ----------------------------------

DoWhileNode::~DoWhileNode()
{
}

void DoWhileNode::ref()
{
  Node::ref();
  if ( statement )
    statement->ref();
  if ( expr )
    expr->ref();
}

bool DoWhileNode::deref()
{
  if ( statement && statement->deref() )
    delete statement;
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 12.6.1
Completion DoWhileNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Value be, bv;
  Completion c;
  Value value;

  do {
    // bail out on error
    KJS_CHECKEXCEPTION

    c = statement->execute(exec);
    if (!((c.complType() == Continue) && ls.contains(c.target()))) {
      if ((c.complType() == Break) && ls.contains(c.target()))
        return Completion(Normal, value);
      if (c.complType() != Normal)
        return c;
    }
    be = expr->evaluate(exec);
    KJS_CHECKEXCEPTION
    bv = be.getValue(exec);
  } while (bv.toBoolean(exec));

  return Completion(Normal, value);
}

void DoWhileNode::processVarDecls(ExecState *exec)
{
  statement->processVarDecls(exec);
}

// ------------------------------ WhileNode ------------------------------------

WhileNode::~WhileNode()
{
}

void WhileNode::ref()
{
  Node::ref();
  if ( statement )
    statement->ref();
  if ( expr )
    expr->ref();
}

bool WhileNode::deref()
{
  if ( statement && statement->deref() )
    delete statement;
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 12.6.2
Completion WhileNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Value be, bv;
  Completion c;
  bool b(false);
  Value value;

  while (1) {
    be = expr->evaluate(exec);
    KJS_CHECKEXCEPTION
    bv = be.getValue(exec);
    b = bv.toBoolean(exec);

    // bail out on error
    KJS_CHECKEXCEPTION

    if (!b)
      return Completion(Normal, value);

    c = statement->execute(exec);
    if (c.isValueCompletion())
      value = c.value();

    if ((c.complType() == Continue) && ls.contains(c.target()))
      continue;
    if ((c.complType() == Break) && ls.contains(c.target()))
      return Completion(Normal, value);
    if (c.complType() != Normal)
      return c;
  }
}

void WhileNode::processVarDecls(ExecState *exec)
{
  statement->processVarDecls(exec);
}

// ------------------------------ ForNode --------------------------------------

ForNode::~ForNode()
{
}

void ForNode::ref()
{
  Node::ref();
  if ( statement )
    statement->ref();
  if ( expr1 )
    expr1->ref();
  if ( expr2 )
    expr2->ref();
  if ( expr3 )
    expr3->ref();
}

bool ForNode::deref()
{
  if ( statement && statement->deref() )
    delete statement;
  if ( expr1 && expr1->deref() )
    delete expr1;
  if ( expr2 && expr2->deref() )
    delete expr2;
  if ( expr3 && expr3->deref() )
    delete expr3;
  return Node::deref();
}

// ECMA 12.6.3
Completion ForNode::execute(ExecState *exec)
{
  Value e, v, cval;
  bool b;

  if (expr1) {
    e = expr1->evaluate(exec);
    KJS_CHECKEXCEPTION
    v = e.getValue(exec);
  }
  while (1) {
    if (expr2) {
      e = expr2->evaluate(exec);
      KJS_CHECKEXCEPTION
      v = e.getValue(exec);
      b = v.toBoolean(exec);
      if (b == false)
	return Completion(Normal, cval);
    }
    // bail out on error
    KJS_CHECKEXCEPTION

    Completion c = statement->execute(exec);
    if (c.isValueCompletion())
      cval = c.value();
    if (!((c.complType() == Continue) && ls.contains(c.target()))) {
      if ((c.complType() == Break) && ls.contains(c.target()))
        return Completion(Normal, cval);
      if (c.complType() != Normal)
      return c;
    }
    if (expr3) {
      e = expr3->evaluate(exec);
      KJS_CHECKEXCEPTION
      v = e.getValue(exec);
    }
  }
}

void ForNode::processVarDecls(ExecState *exec)
{
  if (expr1)
    expr1->processVarDecls(exec);

  statement->processVarDecls(exec);
}

// ------------------------------ ForInNode ------------------------------------

ForInNode::ForInNode(Node *l, Node *e, StatementNode *s)
  : init(0L), lexpr(l), expr(e), varDecl(0L), statement(s)
{
}

ForInNode::ForInNode(const UString *i, AssignExprNode *in, Node *e, StatementNode *s)
  : ident(*i), init(in), expr(e), statement(s)
{
  // for( var foo = bar in baz )
  varDecl = new VarDeclNode(&ident, init);
  lexpr = new ResolveNode(&ident);
}

ForInNode::~ForInNode()
{
}

void ForInNode::ref()
{
  Node::ref();
  if ( statement )
    statement->ref();
  if ( expr )
    expr->ref();
  if ( lexpr )
    lexpr->ref();
  if ( init )
    init->ref();
  if ( varDecl )
    varDecl->ref();
}

bool ForInNode::deref()
{
  if ( statement && statement->deref() )
    delete statement;
  if ( expr && expr->deref() )
    delete expr;
  if ( lexpr && lexpr->deref() )
    delete lexpr;
  if ( init && init->deref() )
    delete init;
  if ( varDecl && varDecl->deref() )
    delete varDecl;
  return Node::deref();
}

// ECMA 12.6.4
Completion ForInNode::execute(ExecState *exec)
{
  Value e, retval;
  Object v;
  Completion c;
  List propList;

  if ( varDecl ) {
    varDecl->evaluate(exec);
    KJS_CHECKEXCEPTION
  }

  e = expr->evaluate(exec);
  KJS_CHECKEXCEPTION
  v = e.getValue(exec).toObject(exec);
  propList = v.propList(exec);

  ListIterator propIt = propList.begin();

  while (propIt != propList.end()) {
    UString name = propIt->getPropertyName(exec);
    if (!v.hasProperty(exec,name)) {
      propIt++;
      continue;
    }

    e = lexpr->evaluate(exec);
    KJS_CHECKEXCEPTION
    e.putValue(exec,String(name));

    c = statement->execute(exec);
    if (c.isValueCompletion())
      retval = c.value();

    if (!((c.complType() == Continue) && ls.contains(c.target()))) {
      if ((c.complType() == Break) && ls.contains(c.target()))
        break;
      if (c.complType() != Normal) {
        return c;
      }
    }

    propIt++;
  }

  // bail out on error
  KJS_CHECKEXCEPTION

  return Completion(Normal, retval);
}

void ForInNode::processVarDecls(ExecState *exec)
{
  statement->processVarDecls(exec);
}

// ------------------------------ ContinueNode ---------------------------------

// ECMA 12.7
Completion ContinueNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Value dummy;
  return exec->context().imp()->seenLabels()->contains(ident) ?
    Completion(Continue, dummy, ident) :
    Completion(Throw,
	       throwError(exec, SyntaxError, "Label not found in containing block"));
}

// ------------------------------ BreakNode ------------------------------------

// ECMA 12.8
Completion BreakNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Value dummy;
  return exec->context().imp()->seenLabels()->contains(ident) ?
    Completion(Break, dummy, ident) :
    Completion(Throw,
	       throwError(exec, SyntaxError, "Label not found in containing block"));
}

// ------------------------------ ReturnNode -----------------------------------

ReturnNode::~ReturnNode()
{
}

void ReturnNode::ref()
{
  Node::ref();
  if ( value )
    value->ref();
}

bool ReturnNode::deref()
{
  if ( value && value->deref() )
    delete value;
  return Node::deref();
}

// ECMA 12.9
Completion ReturnNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  if (!value)
    return Completion(ReturnValue, Undefined());

  Value e = value->evaluate(exec);
  KJS_CHECKEXCEPTION
  Value v = e.getValue(exec);

  return Completion(ReturnValue, v);
}

// ------------------------------ WithNode -------------------------------------

WithNode::~WithNode()
{
}

void WithNode::ref()
{
  Node::ref();
  if ( statement )
    statement->ref();
  if ( expr )
    expr->ref();
}

bool WithNode::deref()
{
  if ( statement && statement->deref() )
    delete statement;
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 12.10
Completion WithNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Value e = expr->evaluate(exec);
  KJS_CHECKEXCEPTION
  Value v = e.getValue(exec);
  Object o = v.toObject(exec);
  KJS_CHECKEXCEPTION
  exec->context().imp()->pushScope(o);
  Completion res = statement->execute(exec);
  exec->context().imp()->popScope();

  return res;
}

void WithNode::processVarDecls(ExecState *exec)
{
  statement->processVarDecls(exec);
}

// ------------------------------ CaseClauseNode -------------------------------

CaseClauseNode::~CaseClauseNode()
{
}

void CaseClauseNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
  if ( list )
    list->ref();
}

bool CaseClauseNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  if ( list && list->deref() )
    delete list;
  return Node::deref();
}

// ECMA 12.11
Value CaseClauseNode::evaluate(ExecState *exec)
{
  Value e = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v = e.getValue(exec);

  return v;
}

// ECMA 12.11
Completion CaseClauseNode::evalStatements(ExecState *exec)
{
  if (list)
    return list->execute(exec);
  else
    return Completion(Normal, Undefined());
}

void CaseClauseNode::processVarDecls(ExecState *exec)
{
  if (list)
    list->processVarDecls(exec);
}

// ------------------------------ ClauseListNode -------------------------------

ClauseListNode::~ClauseListNode()
{
}

void ClauseListNode::ref()
{
  Node::ref();
  if ( cl )
    cl->ref();
  if ( nx )
    nx->ref();
}

bool ClauseListNode::deref()
{
  if ( cl && cl->deref() )
    delete cl;
  if ( nx && nx->deref() )
    delete nx;
  return Node::deref();
}

Value ClauseListNode::evaluate(ExecState */*exec*/)
{
  /* should never be called */
  assert(false);
  return Value();
}

// ECMA 12.11
ClauseListNode* ClauseListNode::append(CaseClauseNode *c)
{
  ClauseListNode *l = this;
  while (l->nx)
    l = l->nx;
  l->nx = new ClauseListNode(c);

  return this;
}

void ClauseListNode::processVarDecls(ExecState *exec)
{
  if (cl)
    cl->processVarDecls(exec);
  if (nx)
    nx->processVarDecls(exec);
}

// ------------------------------ CaseBlockNode --------------------------------

CaseBlockNode::~CaseBlockNode()
{
}

void CaseBlockNode::ref()
{
  Node::ref();
  if ( def )
    def->ref();
  if ( list1 )
    list1->ref();
  if ( list2 )
    list2->ref();
}

bool CaseBlockNode::deref()
{
  if ( def && def->deref() )
    delete def;
  if ( list1 && list1->deref() )
    delete list1;
  if ( list2 && list2->deref() )
    delete list2;
  return Node::deref();
}

Value CaseBlockNode::evaluate(ExecState */*exec*/)
{
  /* should never be called */
  assert(false);
  return Value();
}

// ECMA 12.11
Completion CaseBlockNode::evalBlock(ExecState *exec, const Value& input)
{
  Value v;
  Completion res;
  ClauseListNode *a = list1, *b = list2;
  CaseClauseNode *clause;

    while (a) {
      clause = a->clause();
      a = a->next();
      v = clause->evaluate(exec);
      KJS_CHECKEXCEPTION
      if (strictEqual(exec, input, v)) {
	res = clause->evalStatements(exec);
	if (res.complType() != Normal)
	  return res;
	while (a) {
	  res = a->clause()->evalStatements(exec);
	  if (res.complType() != Normal)
	    return res;
	  a = a->next();
	}
	break;
      }
    }

  while (b) {
    clause = b->clause();
    b = b->next();
    v = clause->evaluate(exec);
    KJS_CHECKEXCEPTION
    if (strictEqual(exec, input, v)) {
      res = clause->evalStatements(exec);
      if (res.complType() != Normal)
	return res;
      goto step18;
    }
  }

  // default clause
  if (def) {
    res = def->evalStatements(exec);
    if (res.complType() != Normal)
      return res;
  }
  b = list2;
 step18:
  while (b) {
    clause = b->clause();
    res = clause->evalStatements(exec);
    if (res.complType() != Normal)
      return res;
    b = b->next();
  }

  // bail out on error
  KJS_CHECKEXCEPTION

  return Completion(Normal);
}

void CaseBlockNode::processVarDecls(ExecState *exec)
{
  if (list1)
    list1->processVarDecls(exec);
  if (def)
    def->processVarDecls(exec);
  if (list2)
    list2->processVarDecls(exec);
}

// ------------------------------ SwitchNode -----------------------------------

SwitchNode::~SwitchNode()
{
}

void SwitchNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
  if ( block )
    block->ref();
}

bool SwitchNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  if ( block && block->deref() )
    delete block;
  return Node::deref();
}

// ECMA 12.11
Completion SwitchNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Value e = expr->evaluate(exec);
  KJS_CHECKEXCEPTION
  Value v = e.getValue(exec);
  Completion res = block->evalBlock(exec,v);

  if ((res.complType() == Break) && ls.contains(res.target()))
    return Completion(Normal, res.value());
  else
    return res;
}

void SwitchNode::processVarDecls(ExecState *exec)
{
  block->processVarDecls(exec);
}

// ------------------------------ LabelNode ------------------------------------

LabelNode::~LabelNode()
{
}

void LabelNode::ref()
{
  Node::ref();
  if ( statement )
    statement->ref();
}

bool LabelNode::deref()
{
  if ( statement && statement->deref() )
    delete statement;
  return Node::deref();
}

// ECMA 12.12
Completion LabelNode::execute(ExecState *exec)
{
  Completion e;

  if (!exec->context().imp()->seenLabels()->push(label)) {
    return Completion( Throw,
		       throwError(exec, SyntaxError, "Duplicated label found" ));
  };
  e = statement->execute(exec);
  exec->context().imp()->seenLabels()->pop();

  if ((e.complType() == Break) && (e.target() == label))
    return Completion(Normal, e.value());
  else
    return e;
}

void LabelNode::processVarDecls(ExecState *exec)
{
  statement->processVarDecls(exec);
}

// ------------------------------ ThrowNode ------------------------------------

ThrowNode::~ThrowNode()
{
}

void ThrowNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool ThrowNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 12.13
Completion ThrowNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Value e = expr->evaluate(exec);
  KJS_CHECKEXCEPTION
  Value v = e.getValue(exec);

  // bail out on error
  KJS_CHECKEXCEPTION

  return Completion(Throw, v);
}

// ------------------------------ CatchNode ------------------------------------

CatchNode::~CatchNode()
{
}

void CatchNode::ref()
{
  Node::ref();
  if ( block )
    block->ref();
}

bool CatchNode::deref()
{
  if ( block && block->deref() )
    delete block;
  return Node::deref();
}

Completion CatchNode::execute(ExecState */*exec*/)
{
  // should never be reached. execute(exec, arg) is used instead
  assert(0L);
  return Completion();
}

// ECMA 12.14
Completion CatchNode::execute(ExecState *exec, const Value &arg)
{
  /* TODO: correct ? Not part of the spec */

  exec->clearException();

  Object obj(new ObjectImp());
  obj.put(exec, ident, arg, DontDelete);
  exec->context().imp()->pushScope(obj);
  Completion c = block->execute(exec);
  exec->context().imp()->popScope();

  return c;
}

void CatchNode::processVarDecls(ExecState *exec)
{
  block->processVarDecls(exec);
}

// ------------------------------ FinallyNode ----------------------------------

FinallyNode::~FinallyNode()
{
}

void FinallyNode::ref()
{
  Node::ref();
  if ( block )
    block->ref();
}

bool FinallyNode::deref()
{
  if ( block && block->deref() )
    delete block;
  return Node::deref();
}

// ECMA 12.14
Completion FinallyNode::execute(ExecState *exec)
{
  return block->execute(exec);
}

void FinallyNode::processVarDecls(ExecState *exec)
{
  block->processVarDecls(exec);
}

// ------------------------------ TryNode --------------------------------------

TryNode::~TryNode()
{
}

void TryNode::ref()
{
  Node::ref();
  if ( block )
    block->ref();
  if ( _final )
    _final->ref();
  if ( _catch )
    _catch->ref();
}

bool TryNode::deref()
{
  if ( block && block->deref() )
    delete block;
  if ( _final && _final->deref() )
    delete _final;
  if ( _catch && _catch->deref() )
    delete _catch;
  return Node::deref();
}

// ECMA 12.14
Completion TryNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Completion c, c2;

  c = block->execute(exec);

  if (!_final) {
    if (c.complType() != Throw)
      return c;
    return _catch->execute(exec,c.value());
  }

  if (!_catch) {
    c2 = _final->execute(exec);
    return (c2.complType() == Normal) ? c : c2;
  }

  if (c.complType() == Throw)
    c = _catch->execute(exec,c.value());

  c2 = _final->execute(exec);
  return (c2.complType() == Normal) ? c : c2;
}

void TryNode::processVarDecls(ExecState *exec)
{
  block->processVarDecls(exec);
  if (_final)
    _final->processVarDecls(exec);
  if (_catch)
    _catch->processVarDecls(exec);
}

// ------------------------------ ParameterNode --------------------------------

ParameterNode::~ParameterNode()
{
}

void ParameterNode::ref()
{
  Node::ref();
  if ( next )
    next->ref();
}

bool ParameterNode::deref()
{
  if ( next && next->deref() )
    delete next;
  return Node::deref();
}

ParameterNode* ParameterNode::append(const UString *i)
{
  ParameterNode *p = this;
  while (p->next)
    p = p->next;

  p->next = new ParameterNode(i);

  return this;
}

// ECMA 13
Value ParameterNode::evaluate(ExecState */*exec*/)
{
  return Undefined();
}

// ------------------------------ FunctionBodyNode -----------------------------


FunctionBodyNode::FunctionBodyNode(SourceElementsNode *s)
  : source(s)
{
  setLoc(-1, -1, -1);
  //fprintf(stderr,"FunctionBodyNode::FunctionBodyNode %p\n",this);
}

FunctionBodyNode::~FunctionBodyNode()
{
  //fprintf(stderr,"FunctionBodyNode::~FunctionBodyNode %p\n",this);
}

void FunctionBodyNode::ref()
{
  Node::ref();
  if ( source )
    source->ref();
  //fprintf( stderr, "FunctionBodyNode::ref() %p. Refcount now %d\n", (void*)this, refcount);
}

bool FunctionBodyNode::deref()
{
  if ( source && source->deref() )
    delete source;
  //fprintf( stderr, "FunctionBodyNode::deref() %p. Refcount now %d\n", (void*)this, refcount-1);
  return Node::deref();
}

// ECMA 13 + 14 for ProgramNode
Completion FunctionBodyNode::execute(ExecState *exec)
{
  /* TODO: workaround for empty body which I don't see covered by the spec */
  if (!source)
    return Completion(Normal);

  source->processFuncDecl(exec);

  return source->execute(exec);
}

void FunctionBodyNode::processFuncDecl(ExecState *exec)
{
  if (source)
    source->processFuncDecl(exec);
}

void FunctionBodyNode::processVarDecls(ExecState *exec)
{
  if (source)
    source->processVarDecls(exec);
}

// ------------------------------ FuncDeclNode ---------------------------------

FuncDeclNode::~FuncDeclNode()
{
}

void FuncDeclNode::ref()
{
  Node::ref();
  if ( param )
    param->ref();
  if ( body )
    body->ref();
}

bool FuncDeclNode::deref()
{
  if ( param && param->deref() )
    delete param;
  if ( body && body->deref() )
    delete body;
  return Node::deref();
}

// ECMA 13
void FuncDeclNode::processFuncDecl(ExecState *exec)
{
  const List sc = exec->context().imp()->scopeChain();

  // TODO: let this be an object with [[Class]] property "Function"
  FunctionImp *fimp = new DeclaredFunctionImp(exec, ident, body, sc);
  Object func(fimp); // protect from GC

  //  Value proto = exec->interpreter()->builtinObject().construct(exec,List::empty());
  List empty;
  Value proto = exec->interpreter()->builtinObject().construct(exec,empty);
  func.put(exec, "prototype", proto, Internal|DontDelete);

  int plen = 0;
  for(ParameterNode *p = param; p != 0L; p = p->nextParam(), plen++)
    fimp->addParameter(p->ident());

  func.put(exec, "length", Number(plen), ReadOnly|DontDelete|DontEnum);

  exec->context().imp()->variableObject().put(exec,ident,func);

  if (body) {
    // hack the scope so that the function gets put as a property of func, and it's scope
    // contains the func as well as our current scope
    Object oldVar = exec->context().imp()->variableObject();
    exec->context().imp()->setVariableObject(func);
    exec->context().imp()->pushScope(func);
    body->processFuncDecl(exec);
    exec->context().imp()->popScope();
    exec->context().imp()->setVariableObject(oldVar);
  }
}

// ------------------------------ FuncExprNode ---------------------------------

FuncExprNode::~FuncExprNode()
{
}

void FuncExprNode::ref()
{
  Node::ref();
  if ( param )
    param->ref();
  if ( body )
    body->ref();
}

bool FuncExprNode::deref()
{
  if ( param && param->deref() )
    delete param;
  if ( body && body->deref() )
    delete body;
  return Node::deref();
}


// ECMA 13
Value FuncExprNode::evaluate(ExecState *exec)
{
  const List sc = exec->context().scopeChain();
  FunctionImp *fimp = new DeclaredFunctionImp(exec, UString::null, body, sc);
  Value ret(fimp);
  List empty;
  Value proto = exec->interpreter()->builtinObject().construct(exec,empty);
  fimp->put(exec, "prototype", proto, Internal|DontDelete);

  int plen = 0;
  for(ParameterNode *p = param; p != 0L; p = p->nextParam(), plen++)
    fimp->addParameter(p->ident());
  fimp->put(exec,"length", Number(plen), ReadOnly|DontDelete|DontEnum);

  return ret;
}

// ------------------------------ SourceElementNode ----------------------------

SourceElementNode::~SourceElementNode()
{
}

void SourceElementNode::ref()
{
  Node::ref();
  if ( statement )
    statement->ref();
  if ( function )
    function->ref();
}

bool SourceElementNode::deref()
{
  if ( statement && statement->deref() )
    delete statement;
  if ( function && function->deref() )
    delete function;
  return Node::deref();
}

// ECMA 14
Completion SourceElementNode::execute(ExecState *exec)
{
  if (statement)
    return statement->execute(exec);

  return Completion(Normal);
}

// ECMA 14
void SourceElementNode::processFuncDecl(ExecState *exec)
{
  if (function)
    function->processFuncDecl(exec);
}

void SourceElementNode::processVarDecls(ExecState *exec)
{
  if (statement)
    statement->processVarDecls(exec);
}

// ------------------------------ SourceElementsNode ---------------------------

SourceElementsNode::~SourceElementsNode()
{
}

void SourceElementsNode::ref()
{
  Node::ref();
  if ( element )
    element->ref();
  if ( elements )
    elements->ref();
}

bool SourceElementsNode::deref()
{
  if ( element && element->deref() )
    delete element;
  if ( elements && elements->deref() )
    delete elements;
  return Node::deref();
}

// ECMA 14
Completion SourceElementsNode::execute(ExecState *exec)
{
  KJS_CHECKEXCEPTION

  if (!elements)
    return element->execute(exec);

  Completion c1 = elements->execute(exec);
  KJS_CHECKEXCEPTION
  if (c1.complType() != Normal)
    return c1;

  Completion c2 = element->execute(exec);
  KJS_CHECKEXCEPTION

  // The spec says to return c2 here, but it seems that mozilla returns c1 if
  // c2 doesn't have a value
  if (c2.complType() == Normal && c2.value().isNull())
    return c1;
  else
    return c2;
}

// ECMA 14
void SourceElementsNode::processFuncDecl(ExecState *exec)
{
  if (elements)
    elements->processFuncDecl(exec);

  element->processFuncDecl(exec);
}

void SourceElementsNode::processVarDecls(ExecState *exec)
{
  if (elements)
    elements->processVarDecls(exec);

  element->processVarDecls(exec);
}

ProgramNode::ProgramNode(SourceElementsNode *s): FunctionBodyNode(s) {
    //fprintf(stderr,"ProgramNode::ProgramNode %p\n",this);
}

ProgramNode::~ProgramNode() {
    //fprintf(stderr,"ProgramNode::~ProgramNode %p\n",this);
}
