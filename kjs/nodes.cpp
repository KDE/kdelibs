// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2002, 2003 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
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
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "nodes.h"

#include <math.h>
#include <assert.h>
#ifdef KJS_DEBUG_MEM
#include <stdio.h>
#include <typeinfo>
#endif
#ifdef KJS_VERBOSE
#include <iostream>
using namespace std;
#endif

#include "collector.h"
#include "context.h"
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
  if (exec->dynamicInterpreter()->imp()->debugger() && \
      exec->dynamicInterpreter()->imp()->debugger()->imp()->aborted()) \
    return Completion(Normal);

#define KJS_CHECKEXCEPTION \
  if (exec->hadException()) { \
    setExceptionDetailsIfNeeded(exec); \
    return Completion(Throw, exec->exception()); \
  } \
  if (Collector::outOfMemory()) \
    return Completion(Throw, Error::create(exec,GeneralError,"Out of memory"));

#define KJS_CHECKEXCEPTIONVALUE \
  if (exec->hadException()) { \
    setExceptionDetailsIfNeeded(exec); \
    return exec->exception(); \
  } \
  if (Collector::outOfMemory()) \
    return Undefined(); // will be picked up by KJS_CHECKEXCEPTION

#define KJS_CHECKEXCEPTIONREFERENCE \
  if (exec->hadException()) { \
    setExceptionDetailsIfNeeded(exec); \
    return Reference::makeValueReference(Undefined()); \
  } \
  if (Collector::outOfMemory()) \
    return Reference::makeValueReference(Undefined()); // will be picked up by KJS_CHECKEXCEPTION

#define KJS_CHECKEXCEPTIONLIST \
  if (exec->hadException()) { \
    setExceptionDetailsIfNeeded(exec); \
    return List(); \
  } \
  if (Collector::outOfMemory()) \
    return List(); // will be picked up by KJS_CHECKEXCEPTION

#ifdef KJS_DEBUG_MEM
std::list<Node *> * Node::s_nodes = 0L;
#endif

// ----------------------------- Node -----------------------------------------

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

Reference Node::evaluateReference(ExecState *exec) const
{
  Value v = evaluate(exec);
  KJS_CHECKEXCEPTIONREFERENCE
  return Reference::makeValueReference(v);
}

// fallback for those nodes without a evaluate() reimplementation
// TODO: reimplemint in each sub class, make Node::evaluate() pure virtual
Value Node::evaluate(ExecState *exec) const
{
  //  fprintf(stderr, "%s::evaluate()\n", typeid(*this).name());
  return evaluateReference(exec).getValue(exec);
}

bool Node::toBoolean(ExecState *exec) const
{
//   fprintf(stderr, "Node(%s)::toBoolean()\n", typeid(*this).name());
  return evaluate(exec).toBoolean(exec);
}

double Node::toNumber(ExecState *exec) const
{
//   fprintf(stderr, "Node(%s)::toNumber()\n", typeid(*this).name());
  return evaluate(exec).toNumber(exec);
}

UString Node::toString(ExecState *exec) const
{
  return evaluate(exec).toString(exec);
}

#ifdef KJS_DEBUG_MEM
void Node::finalCheck()
{
  if (!s_nodes) {
	  fprintf(stderr, "Node::finalCheck(): list 0\n");
	  return;
  }
  fprintf( stderr, "Node::finalCheck(): list count       : %d\n", (int)s_nodes->size() );
  std::list<Node *>::iterator it = s_nodes->begin();
  for ( uint i = 0; it != s_nodes->end() ; ++it, ++i )
    fprintf( stderr, "[%d] Still having node %p (%s) (refcount %d)\n", i, (void*)*it, typeid( **it ).name(), (*it)->refcount );
  delete s_nodes;
  s_nodes = 0L;
}
#endif

Value Node::throwError(ExecState *exec, ErrorType e, const char *msg) const
{
  Object err = Error::create(exec, e, msg, lineNo(), sourceId());
  exec->setException(err);
  return err;
}

Value Node::throwError(ExecState *exec, ErrorType e, const char *msg,
                       const Value &v, const Node *expr) const
{
  char *vStr = strdup(v.toString(exec).ascii());
  char *exprStr = strdup(expr->toCode().ascii());

  int length =  strlen(msg) - 4 /* two %s */ + strlen(vStr) + strlen(exprStr) + 1 /* null terminator */;
  char *str = new char[length];
  sprintf(str, msg, vStr, exprStr);
  free(vStr);
  free(exprStr);

  Value result = throwError(exec, e, str);
  delete [] str;

  return result;
}

Value Node::throwError(ExecState *exec, ErrorType e, const char *msg, Identifier label) const
{
  const char *l = label.ascii();
  int length = strlen(msg) - 2 /* %s */ + strlen(l) + 1 /* null terminator */;
  char *message = new char[length];
  sprintf(message, msg, l);

  Value result = throwError(exec, e, message);
  delete [] message;

  return result;
}


void Node::setExceptionDetailsIfNeeded(ExecState *exec) const
{
    if (exec->hadException()) {
        Object exception = exec->exception().toObject(exec);
        if (!exception.hasProperty(exec, "line") /* &&
            !exception.hasProperty(exec, "sourceURL")*/ ) {
            exception.put(exec, "line", Number(line));
//             exception.put(exec, "sourceURL", String(sourceURL));
        }
    }
}

// ----------------------------- StatementNode --------------------------------
StatementNode::StatementNode() : l0(-1), l1(-1), sourceCode(0), breakPoint(false)
{
}

StatementNode::~StatementNode()
{
  if (sourceCode)
    sourceCode->deref();
}

void StatementNode::setLoc(int line0, int line1, SourceCode *src)
{
  // ### require these to be passed to the constructor
  l0 = line0;
  l1 = line1;
  if (sourceCode != src) {
    if (sourceCode)
      sourceCode->deref();
    sourceCode = src;
    sourceCode->ref();
  }
}

// return true if the debugger wants us to stop at this point
bool StatementNode::hitStatement(ExecState *exec)
{
  assert(sourceCode);
  assert(exec->context().imp()->sourceId == sourceCode->sid);
  exec->context().imp()->setLines(l0,l1);
  Debugger *dbg = exec->dynamicInterpreter()->imp()->debugger();
  if (dbg)
    return dbg->atStatement(exec);
  else
    return true; // continue
}

// return true if the debugger wants us to stop at this point
bool StatementNode::abortStatement(ExecState *exec)
{
  Debugger *dbg = exec->dynamicInterpreter()->imp()->debugger();
  if (dbg)
    return dbg->imp()->aborted();
  else
    return false;
}

void StatementNode::processFuncDecl(ExecState *)
{
}

// ----------------------------- NullNode -------------------------------------

Value NullNode::evaluate(ExecState *) const
{
  return Null();
}

bool NullNode::toBoolean(ExecState *) const
{
  return false;
}

double NullNode::toNumber(ExecState *) const
{
  return 0.0;
}

UString NullNode::toString(ExecState *) const
{
  return "null";
}

// ----------------------------- BooleanNode ----------------------------------

Value BooleanNode::evaluate(ExecState *) const
{
  return Boolean(val);
}

bool BooleanNode::toBoolean(ExecState *) const
{
  return val;
}

double BooleanNode::toNumber(ExecState *) const
{
  return val ? 1.0 : 0.0;
}

UString BooleanNode::toString(ExecState *) const
{
  return val ? "true" : "false";
}

// ----------------------------- NumberNode -----------------------------------

Value NumberNode::evaluate(ExecState *) const
{
  return Number(val);
}

bool NumberNode::toBoolean(ExecState *) const
{
  return !((val == 0) /* || (iVal() == N0) */ || isNaN(val));
}

double NumberNode::toNumber(ExecState *) const
{
  return val;
}

UString NumberNode::toString(ExecState *) const
{
  return UString::from(val);
}

// ----------------------------- StringNode -----------------------------------

Value StringNode::evaluate(ExecState *) const
{
  return String(val);
}

bool StringNode::toBoolean(ExecState *) const
{
  return !val.isEmpty();
}

double StringNode::toNumber(ExecState *) const
{
  return val.toDouble();
}

UString StringNode::toString(ExecState *) const
{
  return val;
}

// ----------------------------- RegExpNode -----------------------------------

Value RegExpNode::evaluate(ExecState *exec) const
{
  List list;
  String p(pattern);
  String f(flags);
  list.append(p);
  list.append(f);

  Object reg = exec->lexicalInterpreter()->imp()->builtinRegExp();
  return reg.construct(exec,list);
}

bool RegExpNode::toBoolean(ExecState *) const
{
  return true;
}

// ----------------------------- ThisNode -------------------------------------

// ECMA 11.1.1
Value ThisNode::evaluate(ExecState *exec) const
{
  return exec->context().imp()->thisValue();
}

// ----------------------------- ResolveNode ----------------------------------

// ECMA 11.1.2 & 10.1.4
Value ResolveNode::evaluate(ExecState *exec) const
{
  return evaluateReference(exec).getValue(exec);
}

Reference ResolveNode::evaluateReference(ExecState *exec) const
{
  ScopeChain chain = exec->context().imp()->scopeChain();

  while (!chain.isEmpty()) {
    ObjectImp *o = chain.top();

    //cerr << "Resolve: looking at '" << ident.ascii() << "'"
    //     << " in " << (void*)o << " " << o->classInfo()->className << endl;
    if (o->hasProperty(exec,ident)) {
      //cerr << "Resolve: FOUND '" << ident.ascii() << "'"
      //     << " in " << (void*)o << " " << o->classInfo()->className << endl;
      return Reference(o, ident);
    }

    chain.pop();
  }

  // identifier not found
#ifdef KJS_VERBOSE
  cerr << "Resolve::evaluateReference: didn't find '" << ident.ustring().ascii() << "'" << endl;
#endif
  return Reference(Null(), ident);
}

// ----------------------------- GroupNode ------------------------------------

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
Value GroupNode::evaluate(ExecState *exec) const
{
  return group->evaluate(exec);
}

Reference GroupNode::evaluateReference(ExecState *exec) const
{
  return group->evaluateReference(exec);
}

// ----------------------------- ElementNode ----------------------------------

void ElementNode::ref()
{
  for (ElementNode *n = this; n; n = n->list) {
    n->Node::ref();
    if (n->node)
      n->node->ref();
  }
}

bool ElementNode::deref()
{
  ElementNode *next;
  for (ElementNode *n = this; n; n = next) {
    next = n->list;
    if (n->node && n->node->deref())
      delete n->node;
    if (n != this && n->Node::deref())
      delete n;
  }
  return Node::deref();
}

// ECMA 11.1.4
Value ElementNode::evaluate(ExecState *exec) const
{
  Object array = exec->lexicalInterpreter()->builtinArray().construct(exec, List::empty());
  int length = 0;
  for (const ElementNode *n = this; n; n = n->list) {
    Value val = n->node->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE
    length += n->elision;
    array.put(exec, length++, val);
  }
  return array;
}

// ----------------------------- ArrayNode ------------------------------------

void ArrayNode::ref()
{
  Node::ref();
  if ( element )
    element->ref();
}

bool ArrayNode::deref()
{
  if ( element && element->deref() )
    delete element;
  return Node::deref();
}

// ECMA 11.1.4
Value ArrayNode::evaluate(ExecState *exec) const
{
  Object array;
  int length;

  if (element) {
    array = Object(static_cast<ObjectImp*>(element->evaluate(exec).imp()));
    KJS_CHECKEXCEPTIONVALUE
    length = opt ? array.get(exec,lengthPropertyName).toInt32(exec) : 0;
  } else {
    Value newArr = exec->lexicalInterpreter()->builtinArray().construct(exec,List::empty());
    array = Object(static_cast<ObjectImp*>(newArr.imp()));
    length = 0;
  }

  if (opt)
    array.put(exec,lengthPropertyName, Number(elision + length), DontEnum | DontDelete);

  return array;
}

// ----------------------------- ObjectLiteralNode ----------------------------

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
Value ObjectLiteralNode::evaluate(ExecState *exec) const
{
  if (list)
    return list->evaluate(exec);

  return exec->lexicalInterpreter()->builtinObject().construct(exec,List::empty());
}

// ----------------------------- PropertyValueNode ----------------------------

void PropertyValueNode::ref()
{
  for (PropertyValueNode *n = this; n; n = n->list) {
    n->Node::ref();
    if (n->name)
      n->name->ref();
    if (n->assign)
      n->assign->ref();
  }
}

bool PropertyValueNode::deref()
{
  PropertyValueNode *next;
  for (PropertyValueNode *n = this; n; n = next) {
    next = n->list;
    if ( n->name && n->name->deref() )
      delete n->name;
    if ( n->assign && n->assign->deref() )
      delete n->assign;
    if (n != this && n->Node::deref() )
      delete n;
  }
  return Node::deref();
}

// ECMA 11.1.5
Value PropertyValueNode::evaluate(ExecState *exec) const
{
  Object obj = exec->lexicalInterpreter()->builtinObject().construct(exec, List::empty());

  for (const PropertyValueNode *p = this; p; p = p->list) {
    Value n = p->name->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE
    Value v = p->assign->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE

    obj.put(exec, Identifier(n.toString(exec)), v);
  }

  return obj;
}

// ----------------------------- PropertyNode ---------------------------------

// ECMA 11.1.5
Value PropertyNode::evaluate(ExecState * /*exec*/) const
{
  Value s;

  if (str.isNull()) {
    s = String(UString::from(numeric));
  } else {
    s = String(str.ustring());
  }

  return s;
}

// ----------------------------- AccessorNode1 --------------------------------

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
Reference AccessorNode1::evaluateReference(ExecState *exec) const
{
  Value v1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONREFERENCE
  Value v2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONREFERENCE
#ifndef NDEBUG
  // catch errors before being caught in toObject(). better error message.
  if (v1.isA(UndefinedType) || v1.isA(NullType)) {
    UString s = "Attempted to access property on %s object "
                "(result of expression %s)";
        (void)throwError(exec, TypeError, s.cstring().c_str(), v1, this);
    return Reference::makeValueReference(Undefined());
  }
#endif
  Object o = v1.toObject(exec);
  unsigned i;
  if (v2.toUInt32(i))
    return Reference(o, i);
  UString s = v2.toString(exec);
  return Reference(o, Identifier(s));
}

// ----------------------------- AccessorNode2 --------------------------------

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
Reference AccessorNode2::evaluateReference(ExecState *exec) const
{
  Value v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONREFERENCE
  assert(v.isValid());
#ifndef NDEBUG
  // catch errors before being caught in toObject(). better error message.
  if (v.isA(UndefinedType) || v.isA(NullType)) {
    UString s = "Attempted to access '" + ident.ustring() +
                "' property on %s object (result of expression %s)";
        (void)throwError(exec, TypeError, s.cstring().c_str(), v, this);
    return Reference::makeValueReference(Undefined());
  }
#endif
  Object o = v.toObject(exec);
  return Reference(o, ident);
}

// ----------------------------- ArgumentListNode -----------------------------

void ArgumentListNode::ref()
{
  for (ArgumentListNode *n = this; n; n = n->list) {
    n->Node::ref();
    if (n->expr)
      n->expr->ref();
  }
}

bool ArgumentListNode::deref()
{
  ArgumentListNode *next;
  for (ArgumentListNode *n = this; n; n = next) {
    next = n->list;
    if (n->expr && n->expr->deref())
      delete n->expr;
    if (n != this && n->Node::deref())
      delete n;
  }
  return Node::deref();
}

Value ArgumentListNode::evaluate(ExecState * /*exec*/) const
{
  assert(0);
  return Value(); // dummy, see evaluateList()
}

// ECMA 11.2.4
List ArgumentListNode::evaluateList(ExecState *exec) const
{
  List l;

  for (const ArgumentListNode *n = this; n; n = n->list) {
    Value v = n->expr->evaluate(exec);
    KJS_CHECKEXCEPTIONLIST
    l.append(v);
  }

  return l;
}

// ----------------------------- ArgumentsNode --------------------------------

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

Value ArgumentsNode::evaluate(ExecState * /*exec*/) const
{
  assert(0);
  return Value(); // dummy, see evaluateList()
}

// ECMA 11.2.4
List ArgumentsNode::evaluateList(ExecState *exec) const
{
  if (!list)
    return List();

  return list->evaluateList(exec);
}

// ----------------------------- NewExprNode ----------------------------------

// ECMA 11.2.2

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

Value NewExprNode::evaluate(ExecState *exec) const
{
  Value v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  List argList;
  if (args) {
    argList = args->evaluateList(exec);
    KJS_CHECKEXCEPTIONVALUE
  }

  if (v.type() != ObjectType) {
    return throwError(exec, TypeError, "Value %s (result of expression %s) is not an object. Cannot be used with new.", v, expr);
  }

  Object constr = Object(static_cast<ObjectImp*>(v.imp()));
  if (!constr.implementsConstruct()) {
    return throwError(exec, TypeError, "Value %s (result of expression %s) is not a constructor. Cannot be used with new.", v, expr);
  }

  Value res = constr.construct(exec,argList);

  return res;
}

// ----------------------------- FunctionCallNode -----------------------------

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
Value FunctionCallNode::evaluate(ExecState *exec) const
{
  Reference ref = expr->evaluateReference(exec);
  KJS_CHECKEXCEPTIONVALUE

  List argList = args->evaluateList(exec);
  KJS_CHECKEXCEPTIONVALUE

  Value v = ref.getValue(exec);
  KJS_CHECKEXCEPTIONVALUE

  if (v.type() != ObjectType) {
    return throwError(exec, TypeError, "Value %s (result of expression %s) is not an object. Cannot be called.", v, expr);
  }

  Object func = Object(static_cast<ObjectImp*>(v.imp()));

  if (!func.implementsCall()) {
    return throwError(exec, TypeError, "Object %s (result of expression %s) does not allow calls.", v, expr);
  }

  Value thisVal;
  if (ref.isMutable())
    thisVal = ref.getBase(exec);
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
    thisVal = exec->dynamicInterpreter()->globalObject();
  }

  Object thisObj = Object::dynamicCast(thisVal);
  Value result = func.call(exec,thisObj, argList);

  return result;
}

// ----------------------------- PostfixNode ----------------------------------

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
Value PostfixNode::evaluate(ExecState *exec) const
{
  Reference ref = expr->evaluateReference(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v = ref.getValue(exec);
  double n = v.toNumber(exec);

  double newValue = (oper == OpPlusPlus) ? n + 1 : n - 1;

  ref.putValue(exec, Number(newValue));

  return Number(n);
}

// ----------------------------- DeleteNode -----------------------------------

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
Value DeleteNode::evaluate(ExecState *exec) const
{
  Reference ref = expr->evaluateReference(exec);
  KJS_CHECKEXCEPTIONVALUE
  return Boolean(ref.deleteValue(exec));
}

// ----------------------------- VoidNode -------------------------------------

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
Value VoidNode::evaluate(ExecState *exec) const
{
  Value dummy1 = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return Undefined();
}

// ----------------------------- TypeOfNode -----------------------------------

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
Value TypeOfNode::evaluate(ExecState *exec) const
{
  const char *s = 0L;
  Reference ref = expr->evaluateReference(exec);
  KJS_CHECKEXCEPTIONVALUE
  if (ref.isMutable()) {
    Value b = ref.getBase(exec);
    if (b.type() == NullType)
      return String("undefined");
  }
  Value v = ref.getValue(exec);
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

// ----------------------------- PrefixNode -----------------------------------

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
Value PrefixNode::evaluate(ExecState *exec) const
{
  Reference ref = expr->evaluateReference(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v = ref.getValue(exec);
  double n = v.toNumber(exec);

  double newValue = (oper == OpPlusPlus) ? n + 1 : n - 1;
  Value n2 = Number(newValue);

  ref.putValue(exec,n2);

  return n2;
}

// ----------------------------- UnaryPlusNode --------------------------------

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
double UnaryPlusNode::toNumber(ExecState *exec) const
{
  return expr->toNumber(exec);
}

// could go
Value UnaryPlusNode::evaluate(ExecState *exec) const
{
  Value v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return Number(v.toNumber(exec)); /* TODO: optimize */
}

// ----------------------------- NegateNode -----------------------------------

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
double NegateNode::toNumber(ExecState *exec) const
{
  return -expr->toNumber(exec);
}

Value NegateNode::evaluate(ExecState *exec) const
{
  Value v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  double d = -v.toNumber(exec);

  return Number(d);
}

// ----------------------------- BitwiseNotNode -------------------------------

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
Value BitwiseNotNode::evaluate(ExecState *exec) const
{
  Value v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  int i32 = v.toInt32(exec);

  return Number(~i32);
}

// ----------------------------- LogicalNotNode -------------------------------

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
bool LogicalNotNode::toBoolean(ExecState *exec) const
{
  return !expr->toBoolean(exec);
}

// could remove this
Value LogicalNotNode::evaluate(ExecState *exec) const
{
  bool b = expr->toBoolean(exec);
  KJS_CHECKEXCEPTIONVALUE

  return Boolean(!b);
}

// ----------------------------- MultNode -------------------------------------

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
Value MultNode::evaluate(ExecState *exec) const
{
  Value v1 = term1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  Value v2 = term2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return mult(exec,v1, v2, oper);
}

// ----------------------------- AddNode --------------------------------------

// factory for an appropriate addition or substraction node
Node* AddNode::create(Node *t1, Node *t2, char op)
{
  // ### many more combinations to check for
  // fold constants
  if ((t1->type() == NumberType || t1->type() == BooleanType) &&
      (t2->type() == NumberType || t2->type() == BooleanType)) {
    double d = t2->toNumber(0);
    Node* n = new NumberNode(t1->toNumber(0) + (op == '+' ? d : -d));
    delete t1;
    delete t2;
    return n;
  }

  if (op == '+' && t2->type() == StringType)
    return new AppendStringNode(t1, t2->toString(0));

  // fall back to generic node
  return new AddNode(t1, t2, op);
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
Value AddNode::evaluate(ExecState *exec) const
{
  Value v1 = term1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  Value v2 = term2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return add(exec,v1, v2, oper);
}

// ------------------------ AddNumberNode ------------------------------------

void AppendStringNode::ref()
{
  Node::ref();
  term->ref();
}

bool AppendStringNode::deref()
{
  if (term->deref())
    delete term;
  return Node::deref();
}

// ECMA 11.6 (special case of string appending)
Value AppendStringNode::evaluate(ExecState *exec) const
{
  UString s = term->toString(exec);
  KJS_CHECKEXCEPTIONVALUE

  return String(s + str);
}

// ----------------------------- ShiftNode ------------------------------------

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
Value ShiftNode::evaluate(ExecState *exec) const
{
  Value v1 = term1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v2 = term2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  unsigned int i2 = v2.toUInt32(exec);
  i2 &= 0x1f;

  switch (oper) {
  case OpLShift:
    return Number(v1.toInt32(exec) << i2);
  case OpRShift:
    return Number(v1.toInt32(exec) >> i2);
  case OpURShift:
    return Number(v1.toUInt32(exec) >> i2);
  default:
    assert(!"ShiftNode: unhandled switch case");
    return Undefined();
  }
}

// ----------------------------- RelationalNode -------------------------------

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
Value RelationalNode::evaluate(ExecState *exec) const
{
  Value v1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

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
                            "Value %s (result of expression %s) is not an object. Cannot be used with IN expression.", v2, expr2);
      Object o2(static_cast<ObjectImp*>(v2.imp()));
      b = o2.hasProperty(exec,Identifier(v1.toString(exec)));
  } else {
    if (v2.type() != ObjectType)
        return throwError(exec,  TypeError,
                          "Value %s (result of expression %s) is not an object. Cannot be used with instanceof operator.", v2, expr2);

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

// ----------------------------- EqualNode ------------------------------------

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
Value EqualNode::evaluate(ExecState *exec) const
{
  Value v1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

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

// ----------------------------- BitOperNode ----------------------------------

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
Value BitOperNode::evaluate(ExecState *exec) const
{
  Value v1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
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

// ----------------------------- BinaryLogicalNode ----------------------------

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
Value BinaryLogicalNode::evaluate(ExecState *exec) const
{
  Value v1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  bool b1 = v1.toBoolean(exec);
  if ((!b1 && oper == OpAnd) || (b1 && oper == OpOr))
    return v1;

  Value v2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return v2;
}

// ----------------------------- ConditionalNode ------------------------------

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
Value ConditionalNode::evaluate(ExecState *exec) const
{
  bool b = logical->toBoolean(exec);
  KJS_CHECKEXCEPTIONVALUE

  Value v = b ? expr1->evaluate(exec) : expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return v;
}

// ----------------------------- AssignNode -----------------------------------

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
Value AssignNode::evaluate(ExecState *exec) const
{
  Reference l = left->evaluateReference(exec);
  KJS_CHECKEXCEPTIONVALUE
  Value v;
  if (oper == OpEqual) {
    v = expr->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE
  } else {
    Value v1 = l.getValue(exec);
    Value v2 = expr->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE
    int i1;
    int i2;
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
      i1 = v1.toInt32(exec);
      i2 = v2.toInt32(exec);
      v = Number(i1 << i2);
      break;
    case OpRShift:
      i1 = v1.toInt32(exec);
      i2 = v2.toInt32(exec);
      v = Number(i1 >> i2);
      break;
    case OpURShift:
      ui = v1.toUInt32(exec);
      i2 = v2.toInt32(exec);
      v = Number(ui >> i2);
      break;
    case OpAndEq:
      i1 = v1.toInt32(exec);
      i2 = v2.toInt32(exec);
      v = Number(i1 & i2);
      break;
    case OpXOrEq:
      i1 = v1.toInt32(exec);
      i2 = v2.toInt32(exec);
      v = Number(i1 ^ i2);
      break;
    case OpOrEq:
      i1 = v1.toInt32(exec);
      i2 = v2.toInt32(exec);
      v = Number(i1 | i2);
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

// ----------------------------- CommaNode ------------------------------------

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
Value CommaNode::evaluate(ExecState *exec) const
{
  (void) expr1->evaluate(exec);  // ignore return value
  KJS_CHECKEXCEPTIONVALUE
  Value v = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return v;
}

// ----------------------------- StatListNode ---------------------------------

StatListNode::StatListNode(StatementNode *s)
  : statement(s), list(this)
{
  setLoc(s->firstLine(), s->lastLine(), s->code());
}

StatListNode::StatListNode(StatListNode *l, StatementNode *s)
  : statement(s), list(l->list)
{
  l->list = this;
  setLoc(l->firstLine(),s->lastLine(),l->code());
}

void StatListNode::ref()
{
  for (StatListNode *n = this; n; n = n->list) {
    n->Node::ref();
    if (n->statement)
      n->statement->ref();
  }
}

bool StatListNode::deref()
{
  StatListNode *next;
  for (StatListNode *n = this; n; n = next) {
    next = n->list;
    if (n->statement && n->statement->deref())
      delete n->statement;
    if (n != this && n->Node::deref())
      delete n;
  }
  return StatementNode::deref();
}

// ECMA 12.1
Completion StatListNode::execute(ExecState *exec)
{
  Completion c = statement->execute(exec);
  KJS_ABORTPOINT
  if (exec->hadException()) {
    Value ex = exec->exception();
    exec->clearException();
    return Completion(Throw, ex);
  }

  if (c.complType() != Normal)
    return c;

  Value v = c.value();

  for (StatListNode *n = list; n; n = n->list) {
    Completion c2 = n->statement->execute(exec);
    KJS_ABORTPOINT
    if (c2.complType() != Normal)
      return c2;

    if (exec->hadException()) {
      Value ex = exec->exception();
      exec->clearException();
      return Completion(Throw, ex);
    }

    if (c2.isValueCompletion())
      v = c2.value();
    c = c2;
  }

  return Completion(c.complType(), v, c.target());
}

void StatListNode::processVarDecls(ExecState *exec)
{
  for (StatListNode *n = this; n; n = n->list)
    n->statement->processVarDecls(exec);
}

// ----------------------------- AssignExprNode -------------------------------

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
Value AssignExprNode::evaluate(ExecState *exec) const
{
  return expr->evaluate(exec);
}

// ----------------------------- VarDeclNode ----------------------------------

VarDeclNode::VarDeclNode(const Identifier &id, AssignExprNode *in, Type t)
    : varType(t), ident(id), init(in)
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
Value VarDeclNode::evaluate(ExecState *exec) const
{
  Object variable = Object::dynamicCast(exec->context().imp()->variableObject());

  Value val;
  if (init) {
      val = init->evaluate(exec);
      KJS_CHECKEXCEPTIONVALUE
  } else {
    // ### check attributes? reuse check done in processVarDecls()?
      if (variable.imp()->getDirect(ident)) // already declared ?
          return Value();
      val = Undefined();
  }

#ifdef KJS_VERBOSE
  printInfo(exec,(UString("new variable ")+ident.ustring()).cstring().c_str(),val);
#endif
  // We use Internal to bypass all checks in derived objects, e.g. so that
  // "var location" creates a dynamic property instead of activating window.location.
  int flags = Internal;
  if (exec->context().imp()->codeType() != EvalCode)
    flags |= DontDelete;
  if (varType == VarDeclNode::Constant)
    flags |= ReadOnly;
  variable.put(exec, ident, val, flags);

  // the spec wants us to return the name of the identifier here
  // but we'll save the construction and copying as the return
  // value isn't used by the caller
  return Value();
}

void VarDeclNode::processVarDecls(ExecState *exec)
{
  Object variable = exec->context().variableObject();
  // ### use getDirect()? Check attributes?
  // ### avoid duplication with actions performed in evaluate()?
  if ( !variable.hasProperty( exec, ident ) ) { // already declared ?
    int flags = None;
    if (exec->_context->codeType() != EvalCode)
      flags |= DontDelete;
    if (varType == VarDeclNode::Constant)
      flags |= ReadOnly;
    // TODO: check for forbidden redeclaration of consts
    variable.put(exec, ident, Undefined(), flags);
  }
}

// ----------------------------- VarDeclListNode ------------------------------

void VarDeclListNode::ref()
{
  for (VarDeclListNode *n = this; n; n = n->list) {
    n->Node::ref();
    if (n->var)
      n->var->ref();
  }
}

bool VarDeclListNode::deref()
{
  VarDeclListNode *next;
  for (VarDeclListNode *n = this; n; n = next) {
    next = n->list;
    if (n->var && n->var->deref())
      delete n->var;
    if (n != this && n->Node::deref())
      delete n;
  }
  return Node::deref();
}


// ECMA 12.2
Value VarDeclListNode::evaluate(ExecState *exec) const
{
  for (const VarDeclListNode *n = this; n; n = n->list) {
    (void)n->var->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE
  }
  return Undefined();
}

void VarDeclListNode::processVarDecls(ExecState *exec)
{
  for (VarDeclListNode *n = this; n; n = n->list)
    n->var->processVarDecls(exec);
}

// ----------------------------- VarStatementNode -----------------------------

void VarStatementNode::ref()
{
  StatementNode::ref();
  if ( list )
    list->ref();
}

bool VarStatementNode::deref()
{
  if ( list && list->deref() )
    delete list;
  return StatementNode::deref();
}

// ECMA 12.2
Completion VarStatementNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  (void) list->evaluate(exec);
  KJS_CHECKEXCEPTION

  return Completion(Normal);
}

void VarStatementNode::processVarDecls(ExecState *exec)
{
  list->processVarDecls(exec);
}

// ----------------------------- BlockNode ------------------------------------

BlockNode::BlockNode(SourceElementsNode *s)
{
  if (s) {
    source = s->elements;
    s->elements = 0;
    setLoc(s->firstLine(), s->lastLine(), s->code());
  } else {
    source = 0;
  }
}

void BlockNode::ref()
{
  StatementNode::ref();
  if ( source )
    source->ref();
}

bool BlockNode::deref()
{
  if ( source && source->deref() )
    delete source;
  return StatementNode::deref();
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

// ----------------------------- EmptyStatementNode ---------------------------

// ECMA 12.3
Completion EmptyStatementNode::execute(ExecState * /*exec*/)
{
  return Completion(Normal);
}

// ----------------------------- ExprStatementNode ----------------------------

void ExprStatementNode::ref()
{
  StatementNode::ref();
  if ( expr )
    expr->ref();
}

bool ExprStatementNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return StatementNode::deref();
}

// ECMA 12.4
Completion ExprStatementNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Value v = expr->evaluate(exec);
  KJS_CHECKEXCEPTION

  return Completion(Normal, v);
}

// ----------------------------- IfNode ---------------------------------------

void IfNode::ref()
{
  StatementNode::ref();
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
  return StatementNode::deref();
}

// ECMA 12.5
Completion IfNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  assert(expr);
  bool b = expr->toBoolean(exec);
  KJS_CHECKEXCEPTION

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

// ----------------------------- DoWhileNode ----------------------------------

void DoWhileNode::ref()
{
  StatementNode::ref();
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
  return StatementNode::deref();
}

// ECMA 12.6.1
Completion DoWhileNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Completion c;
  Value value;
  bool b;

  do {
    // bail out on error
    KJS_CHECKEXCEPTION

    exec->context().imp()->seenLabels()->pushIteration();
    c = statement->execute(exec);
    exec->context().imp()->seenLabels()->popIteration();
    if (!((c.complType() == Continue) && ls.contains(c.target()))) {
      if ((c.complType() == Break) && ls.contains(c.target()))
        return Completion(Normal, value);
      if (c.complType() != Normal)
        return c;
    }
    b = expr->toBoolean(exec);
    KJS_CHECKEXCEPTION
  } while (b);

  return Completion(Normal, value);
}

void DoWhileNode::processVarDecls(ExecState *exec)
{
  statement->processVarDecls(exec);
}

// ----------------------------- WhileNode ------------------------------------

void WhileNode::ref()
{
  StatementNode::ref();
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
  return StatementNode::deref();
}

// ECMA 12.6.2
Completion WhileNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Completion c;
  Value value;

  while (1) {
    bool b = expr->toBoolean(exec);
    KJS_CHECKEXCEPTION

    // bail out on error
    KJS_CHECKEXCEPTION

    if (!b)
      return Completion(Normal, value);

    exec->context().imp()->seenLabels()->pushIteration();
    c = statement->execute(exec);
    exec->context().imp()->seenLabels()->popIteration();
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

// ----------------------------- ForNode --------------------------------------

void ForNode::ref()
{
  StatementNode::ref();
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
  return StatementNode::deref();
}

// ECMA 12.6.3
Completion ForNode::execute(ExecState *exec)
{
  Value v, cval;

  if (expr1) {
    v = expr1->evaluate(exec);
    KJS_CHECKEXCEPTION
  }
  for (;;) {
    if (expr2) {
      bool b = expr2->toBoolean(exec);
      KJS_CHECKEXCEPTION
      if (!b)
	return Completion(Normal, cval);
    }
    // bail out on error
    KJS_CHECKEXCEPTION

    exec->context().imp()->seenLabels()->pushIteration();
    Completion c = statement->execute(exec);
    exec->context().imp()->seenLabels()->popIteration();
    if (c.isValueCompletion())
      cval = c.value();
    if (!((c.complType() == Continue) && ls.contains(c.target()))) {
      if ((c.complType() == Break) && ls.contains(c.target()))
        return Completion(Normal, cval);
      if (c.complType() != Normal)
        return c;
    }
    if (expr3) {
      v = expr3->evaluate(exec);
      KJS_CHECKEXCEPTION
    }
  }
}

void ForNode::processVarDecls(ExecState *exec)
{
  if (expr1)
    expr1->processVarDecls(exec);

  statement->processVarDecls(exec);
}

// ----------------------------- ForInNode ------------------------------------

ForInNode::ForInNode(Node *l, Node *e, StatementNode *s)
  : init(0L), lexpr(l), expr(e), varDecl(0L), statement(s)
{
}

ForInNode::ForInNode(const Identifier &i, AssignExprNode *in, Node *e, StatementNode *s)
  : ident(i), init(in), expr(e), statement(s)
{
  // for( var foo = bar in baz )
  varDecl = new VarDeclNode(ident, init, VarDeclNode::Variable);
  lexpr = new ResolveNode(ident);
}

void ForInNode::ref()
{
  StatementNode::ref();
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
  return StatementNode::deref();
}

// ECMA 12.6.4
Completion ForInNode::execute(ExecState *exec)
{
  Value retval;
  Completion c;

  if ( varDecl ) {
    (void)varDecl->evaluate(exec);
    KJS_CHECKEXCEPTION
  }

  Value v = expr->evaluate(exec);
  // for Null and Undefined, we want to make sure not to go through
  // the loop at all, because their object wrappers will have a
  // property list but will throw an exception if you attempt to
  // access any property.
  if (v.isA(NullType) || v.isA(UndefinedType))
    return Completion(Normal, retval);

  Object o = v.toObject(exec);
  KJS_CHECKEXCEPTION
  ReferenceList propList = o.propList(exec);

  ReferenceListIterator propIt = propList.begin();

  while (propIt != propList.end()) {
    Identifier name = propIt->getPropertyName(exec);
    if (!o.hasProperty(exec,name)) {
      propIt++;
      continue;
    }

    Reference ref = lexpr->evaluateReference(exec);
    KJS_CHECKEXCEPTION
    ref.putValue(exec, String(name.ustring()));

    exec->context().imp()->seenLabels()->pushIteration();
    c = statement->execute(exec);
    exec->context().imp()->seenLabels()->popIteration();
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

// ----------------------------- ContinueNode ---------------------------------

// ECMA 12.7
Completion ContinueNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Value dummy;

  if (ident.isEmpty() && !exec->context().imp()->seenLabels()->inIteration())
    return Completion(Throw,
		      throwError(exec, SyntaxError, "continue used outside of iteration statement"));
  else if (!ident.isEmpty() && !exec->context().imp()->seenLabels()->contains(ident))
    return Completion(Throw,
                      throwError(exec, SyntaxError, "Label %s not found in containing block. Can't continue.", ident));
  else
    return Completion(Continue, dummy, ident);
}

// ----------------------------- BreakNode ------------------------------------

// ECMA 12.8
Completion BreakNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Value dummy;

  if (ident.isEmpty() && !exec->context().imp()->seenLabels()->inIteration() &&
      !exec->context().imp()->seenLabels()->inSwitch())
    return Completion(Throw,
		      throwError(exec, SyntaxError, "break used outside of iteration or switch statement"));
  else if (!ident.isEmpty() && !exec->context().imp()->seenLabels()->contains(ident))
    return Completion(Throw,
                      throwError(exec, SyntaxError, "Label %s not found in containing block. Can't break.", ident));
  else
    return Completion(Break, dummy, ident);
}

// ----------------------------- ReturnNode -----------------------------------

void ReturnNode::ref()
{
  StatementNode::ref();
  if ( value )
    value->ref();
}

bool ReturnNode::deref()
{
  if ( value && value->deref() )
    delete value;
  return StatementNode::deref();
}

// ECMA 12.9
Completion ReturnNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  CodeType codeType = exec->context().imp()->codeType();
  if (codeType != FunctionCode) {
    return Completion(Throw, throwError(exec, SyntaxError, "Invalid return statement."));    
  }

  if (!value)
    return Completion(ReturnValue, Undefined());

  Value v = value->evaluate(exec);
  KJS_CHECKEXCEPTION

  return Completion(ReturnValue, v);
}

// ----------------------------- WithNode -------------------------------------

void WithNode::ref()
{
  StatementNode::ref();
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
  return StatementNode::deref();
}

// ECMA 12.10
Completion WithNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Value v = expr->evaluate(exec);
  KJS_CHECKEXCEPTION
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

// ----------------------------- CaseClauseNode -------------------------------

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
Value CaseClauseNode::evaluate(ExecState *exec) const
{
  Value v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return v;
}

// ECMA 12.11
Completion CaseClauseNode::evalStatements(ExecState *exec) const
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

// ----------------------------- ClauseListNode -------------------------------

void ClauseListNode::ref()
{
  for (ClauseListNode *n = this; n; n = n->nx) {
    n->Node::ref();
    if (n->cl)
      n->cl->ref();
  }
}

bool ClauseListNode::deref()
{
  ClauseListNode *next;
  for (ClauseListNode *n = this; n; n = next) {
    next = n->nx;
    if (n->cl && n->cl->deref())
      delete n->cl;
    if (n != this && n->Node::deref())
      delete n;
  }
  return Node::deref();
}

Value ClauseListNode::evaluate(ExecState * /*exec*/) const
{
  /* should never be called */
  assert(false);
  return Value();
}

// ECMA 12.11
void ClauseListNode::processVarDecls(ExecState *exec)
{
  for (ClauseListNode *n = this; n; n = n->nx)
    if (n->cl)
      n->cl->processVarDecls(exec);
}

// ----------------------------- CaseBlockNode --------------------------------

CaseBlockNode::CaseBlockNode(ClauseListNode *l1, CaseClauseNode *d,
                             ClauseListNode *l2)
{
  def = d;
  if (l1) {
    list1 = l1->nx;
    l1->nx = 0;
  } else {
    list1 = 0;
  }
  if (l2) {
    list2 = l2->nx;
    l2->nx = 0;
  } else {
    list2 = 0;
  }
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

Value CaseBlockNode::evaluate(ExecState * /*exec*/) const
{
  /* should never be called */
  assert(false);
  return Value();
}

// ECMA 12.11
Completion CaseBlockNode::evalBlock(ExecState *exec, const Value& input) const
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

// ----------------------------- SwitchNode -----------------------------------

void SwitchNode::ref()
{
  StatementNode::ref();
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
  return StatementNode::deref();
}

// ECMA 12.11
Completion SwitchNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Value v = expr->evaluate(exec);
  KJS_CHECKEXCEPTION
  exec->context().imp()->seenLabels()->pushSwitch();
  Completion res = block->evalBlock(exec,v);
  exec->context().imp()->seenLabels()->popSwitch();

  if ((res.complType() == Break) && ls.contains(res.target()))
    return Completion(Normal, res.value());
  else
    return res;
}

void SwitchNode::processVarDecls(ExecState *exec)
{
  block->processVarDecls(exec);
}

// ----------------------------- LabelNode ------------------------------------

void LabelNode::ref()
{
  StatementNode::ref();
  if ( statement )
    statement->ref();
}

bool LabelNode::deref()
{
  if ( statement && statement->deref() )
    delete statement;
  return StatementNode::deref();
}

// ECMA 12.12
Completion LabelNode::execute(ExecState *exec)
{
  Completion e;

  if (!exec->context().imp()->seenLabels()->push(label)) {
    return Completion( Throw,
                       throwError(exec, SyntaxError, "Duplicated label %s found.", label));
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

// ----------------------------- ThrowNode ------------------------------------

void ThrowNode::ref()
{
  StatementNode::ref();
  if ( expr )
    expr->ref();
}

bool ThrowNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return StatementNode::deref();
}

// ECMA 12.13
Completion ThrowNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Value v = expr->evaluate(exec);
  KJS_CHECKEXCEPTION

  // bail out on error
  KJS_CHECKEXCEPTION

  Debugger *dbg = exec->interpreter()->imp()->debugger();
  if (dbg)
    dbg->exception(exec,v,exec->context().imp()->inTryCatch());

  return Completion(Throw, v);
}

// ----------------------------- CatchNode ------------------------------------

void CatchNode::ref()
{
  StatementNode::ref();
  if ( block )
    block->ref();
}

bool CatchNode::deref()
{
  if ( block && block->deref() )
    delete block;
  return StatementNode::deref();
}

Completion CatchNode::execute(ExecState * /*exec*/)
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

// ----------------------------- FinallyNode ----------------------------------

void FinallyNode::ref()
{
  StatementNode::ref();
  if ( block )
    block->ref();
}

bool FinallyNode::deref()
{
  if ( block && block->deref() )
    delete block;
  return StatementNode::deref();
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

// ----------------------------- TryNode --------------------------------------

void TryNode::ref()
{
  StatementNode::ref();
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
  return StatementNode::deref();
}

// ECMA 12.14
Completion TryNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Completion c, c2;

  if (_catch)
    exec->context().imp()->pushTryCatch();
  c = block->execute(exec);
  if (_catch)
    exec->context().imp()->popTryCatch();

  if (!_final) {
    if (c.complType() != Throw)
      return c;
    return _catch->execute(exec,c.value());
  }

  if (!_catch) {
    Value exception = exec->_exception;
    exec->_exception = Value();

    c2 = _final->execute(exec);

    if (!exec->hadException() && c2.complType() != Throw)
      exec->_exception = exception;

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

// ----------------------------- ParameterNode --------------------------------

void ParameterNode::ref()
{
  for (ParameterNode *n = this; n; n = n->next)
    n->Node::ref();
}

bool ParameterNode::deref()
{
  ParameterNode *next;
  for (ParameterNode *n = this; n; n = next) {
    next = n->next;
    if (n != this && n->Node::deref())
      delete n;
  }
  return Node::deref();
}

// ECMA 13
Value ParameterNode::evaluate(ExecState * /*exec*/) const
{
  return Undefined();
}

// ----------------------------- FunctionBodyNode -----------------------------


FunctionBodyNode::FunctionBodyNode(SourceElementsNode *s)
  : BlockNode(s)
{
  //fprintf(stderr,"FunctionBodyNode::FunctionBodyNode %p\n",this);
}

void FunctionBodyNode::processFuncDecl(ExecState *exec)
{
  if (source)
    source->processFuncDecl(exec);
}

// ----------------------------- FuncDeclNode ---------------------------------

void FuncDeclNode::ref()
{
  StatementNode::ref();
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
  return StatementNode::deref();
}

// ECMA 13
void FuncDeclNode::processFuncDecl(ExecState *exec)
{
  ContextImp *ctx = exec->context().imp();
  // TODO: let this be an object with [[Class]] property "Function"
  FunctionImp *fimp = new DeclaredFunctionImp(exec, ident, body, exec->context().imp()->scopeChain());
  Object func(fimp); // protect from GC

  //  Value proto = exec->lexicalInterpreter()->builtinObject().construct(exec,List::empty());
  List empty;
  Object proto = exec->lexicalInterpreter()->builtinObject().construct(exec,empty);
  proto.put(exec, constructorPropertyName, func, ReadOnly|DontDelete|DontEnum);
  func.put(exec, prototypePropertyName, proto, Internal|DontDelete);

  int plen = 0;
  for(const ParameterNode *p = param; p != 0L; p = p->nextParam(), plen++)
    fimp->addParameter(p->ident());

  func.put(exec, lengthPropertyName, Number(plen), ReadOnly|DontDelete|DontEnum);

#ifdef KJS_VERBOSE
  fprintf(stderr,"KJS: new function %s in %p\n", ident.ustring().cstring().c_str(), ctx->variableObject().imp());
#endif
  if (exec->_context->codeType() == EvalCode) {
      // ECMA 10.2.2
      ctx->variableObject().put(exec, ident, func, Internal);
  } else {
      ctx->variableObject().put(exec, ident, func, DontDelete | Internal);
  }

  if (body) {
    // hack the scope so that the function gets put as a property of func, and it's scope
    // contains the func as well as our current scope
    Object oldVar = ctx->variableObject();
    ctx->setVariableObject(func);
    ctx->pushScope(func);
    body->processFuncDecl(exec);
    ctx->popScope();
    ctx->setVariableObject(oldVar);
  }
}

// ----------------------------- FuncExprNode ---------------------------------

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
Value FuncExprNode::evaluate(ExecState *exec) const
{
  FunctionImp *fimp = new DeclaredFunctionImp(exec, Identifier::null(), body, exec->context().imp()->scopeChain());
  Value ret(fimp);
  List empty;
  Value proto = exec->lexicalInterpreter()->builtinObject().construct(exec,empty);
  fimp->put(exec, prototypePropertyName, proto, Internal|DontDelete);

  for(const ParameterNode *p = param; p != 0L; p = p->nextParam())
    fimp->addParameter(p->ident());

  return ret;
}

// ----------------------------- SourceElementsNode ---------------------------

SourceElementsNode::SourceElementsNode(StatementNode *s1)
{
  element = s1;
  elements = this;
  setLoc(s1->firstLine(), s1->lastLine(), s1->code());
}

SourceElementsNode::SourceElementsNode(SourceElementsNode *s1, StatementNode *s2)
{
  elements = s1->elements;
  s1->elements = this;
  element = s2;
  setLoc(s1->firstLine(), s2->lastLine(), s1->code());
}

void SourceElementsNode::ref()
{
  for (SourceElementsNode *n = this; n; n = n->elements) {
    n->Node::ref();
    if (n->element)
      n->element->ref();
  }
}

bool SourceElementsNode::deref()
{
  SourceElementsNode *next;
  for (SourceElementsNode *n = this; n; n = next) {
    next = n->elements;
    if (n->element && n->element->deref())
      delete n->element;
    if (n != this && n->Node::deref())
      delete n;
  }
  return StatementNode::deref();
}

// ECMA 14
Completion SourceElementsNode::execute(ExecState *exec)
{
  KJS_CHECKEXCEPTION

  Completion c1 = element->execute(exec);
  KJS_CHECKEXCEPTION;
  if (c1.complType() != Normal)
    return c1;

  for (SourceElementsNode *n = elements; n; n = n->elements) {
    Completion c2 = n->element->execute(exec);
    if (c2.complType() != Normal)
      return c2;
    // The spec says to return c2 here, but it seems that mozilla returns c1 if
    // c2 doesn't have a value
    if (c2.value().isValid())
      c1 = c2;
  }

  return c1;
}

// ECMA 14
void SourceElementsNode::processFuncDecl(ExecState *exec)
{
  for (SourceElementsNode *n = this; n; n = n->elements)
    n->element->processFuncDecl(exec);
}

void SourceElementsNode::processVarDecls(ExecState *exec)
{
  for (SourceElementsNode *n = this; n; n = n->elements)
    n->element->processVarDecls(exec);
}
