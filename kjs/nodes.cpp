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
 */

#include "nodes.h"

#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <typeinfo>

#include "kjs.h"
#include "ustring.h"
#include "lexer.h"
#include "types.h"
#include "internal.h"
#include "operations.h"
#include "regexp_object.h"
#include "debugger.h"

using namespace KJS;

#define KJS_BREAKPOINT if (!hitStatement(script,context)) return Completion(Normal);
#define KJS_ABORTPOINT if (abortStatement(script,context)) return Completion(Normal);

#ifndef NDEBUG
std::list<Node *> Node::s_nodes;
#endif

// ------------------------------ Node -----------------------------------------

Node::Node()
{
  //  assert(Lexer::curr());
  //  line = Lexer::curr()->lineNo();
  line = 0; // ### FIXME
  refcount = 0;
#ifndef NDEBUG
  s_nodes.push_back( this );
#endif
}

Node::~Node()
{
#ifndef NDEBUG
  s_nodes.remove( this );
#endif
}

#ifndef NDEBUG
void Node::finalCheck()
{
  fprintf( stderr, "Node::finalCheck(): list count       : %d\n", s_nodes.size() );
  std::list<Node *>::iterator it = s_nodes.begin();
  for ( uint i = 0; it != s_nodes.end() ; ++it, ++i )
    fprintf( stderr, "[%d] Still having node %p (%s)\n", i, *it, typeid( **it ).name() );
}
#endif

KJSO Node::throwError(ErrorType e, const char *msg)
{
  return Error::create(e, msg, lineNo());
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

bool StatementNode::hitStatement(KJScriptImp *script, Context *context)
{
  if (script->debugger())
    return script->debugger()->rep->hitStatement(script,context,sid,l0,l1);
  else
    return script;
}

// return true if the debugger wants us to stop at this point
bool StatementNode::abortStatement(KJScriptImp *script, Context */*context*/)
{
  // ###
  /*
  if (script->debugger() &&
      script->debugger()->mode() == Debugger::Stop)
      return true;
  */

  return false;
}

// ------------------------------ NullNode -------------------------------------

KJSO NullNode::evaluate(KJScriptImp */*script*/, Context */*context*/)
{
  return Null();
}

// ------------------------------ BooleanNode ----------------------------------

KJSO BooleanNode::evaluate(KJScriptImp */*script*/, Context */*context*/)
{
  return Boolean(value);
}

// ------------------------------ NumberNode -----------------------------------

KJSO NumberNode::evaluate(KJScriptImp */*script*/, Context */*context*/)
{
  return Number(value);
}

// ------------------------------ StringNode -----------------------------------

KJSO StringNode::evaluate(KJScriptImp */*script*/, Context */*context*/)
{
  return String(value);
}

// ------------------------------ RegExpNode -----------------------------------

KJSO RegExpNode::evaluate(KJScriptImp *script, Context */*context*/)
{
  List list;
  String p(pattern);
  String f(flags);
  list.append(p);
  list.append(f);

  // ### use built-in regexp object instead ? (script->regExp() or something)
  KJSO r = script->globalObject().get("RegExp");
  RegExpObject *r2 = (RegExpObject*)r.imp();
  return r2->construct(list);
}

// ------------------------------ ThisNode -------------------------------------

// ECMA 11.1.1
KJSO ThisNode::evaluate(KJScriptImp */*script*/, Context *context)
{
  return context->thisValue();
}

// ------------------------------ ResolveNode ----------------------------------

// ECMA 11.1.2 & 10.1.4
KJSO ResolveNode::evaluate(KJScriptImp */*script*/, Context *context)
{
  //  assert(context);
  const List *chain = context->pScopeChain();
  //  assert(chain);
  ListIterator scope = chain->begin();

  while (scope != chain->end()) {
    if (scope->hasProperty(ident)) {
//        cout << "Resolve: found '" << ident.ascii() << "'"
// 	    << " type " << scope->get(ident).imp()->typeInfo()->name << endl;
      return Reference(*scope, ident);
    }
    scope++;
  }

  // identifier not found
//  cout << "Resolve: didn't find '" << ident.ascii() << "'" << endl;
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
KJSO GroupNode::evaluate(KJScriptImp *script, Context *context)
{
  return group->evaluate(script,context);
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
KJSO ElisionNode::evaluate(KJScriptImp *script, Context *context)
{
  if (elision)
    return Number(elision->evaluate(script,context).toNumber().value() + 1);
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
KJSO ElementNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO array, val;
  int length = 0;
  int elisionLen = elision ? elision->evaluate(script,context).toInt32() : 0;

  if (list) {
    array = list->evaluate(script,context);
    val = node->evaluate(script,context).getValue();
    length = array.get("length").toInt32();
  } else {
    array = Object::create(ArrayClass);
    val = node->evaluate(script,context).getValue();
  }

  array.putArrayElement(UString::from(elisionLen + length), val);

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
KJSO ArrayNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO array;
  int length;
  int elisionLen = elision ? elision->evaluate(script,context).toInt32() : 0;

  if (element) {
    array = element->evaluate(script,context);
    length = opt ? array.get("length").toInt32() : 0;
  } else {
    array = Object::create(ArrayClass);
    length = 0;
  }

  if (opt)
    array.put("length", Number(elisionLen + length), DontEnum | DontDelete);

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
KJSO ObjectLiteralNode::evaluate(KJScriptImp *script, Context *context)
{
  if (list)
    return list->evaluate(script,context);

  return Object::create(ObjectClass);
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
KJSO PropertyValueNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO obj;
  if (list)
    obj = list->evaluate(script,context);
  else
    obj = Object::create(ObjectClass);
  KJSO n = name->evaluate(script,context);
  KJSO a = assign->evaluate(script,context);
  KJSO v = a.getValue();

  obj.put(n.toString().value(), v);

  return obj;
}

// ------------------------------ PropertyNode ---------------------------------

// ECMA 11.1.5
KJSO PropertyNode::evaluate(KJScriptImp */*script*/, Context */*context*/)
{
  KJSO s;

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
KJSO AccessorNode1::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e1 = expr1->evaluate(script,context);
  KJSO v1 = e1.getValue();
  KJSO e2 = expr2->evaluate(script,context);
  KJSO v2 = e2.getValue();
  Object o = v1.toObject();
  String s = v2.toString();
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
KJSO AccessorNode2::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e = expr->evaluate(script,context);
  KJSO v = e.getValue();
  KJSO o = v.toObject();
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

KJSO ArgumentListNode::evaluate(KJScriptImp */*script*/, Context */*context*/)
{
  assert(0);
  return KJSO(); // dummy, see evaluateList()
}

// ECMA 11.2.4
List* ArgumentListNode::evaluateList(KJScriptImp *script, Context *context)
{
  KJSO e = expr->evaluate(script,context);
  KJSO v = e.getValue();

  if (!list) {
    List *l = new List();
    l->append(v);
    return l;
  }

  List *l = list->evaluateList(script,context);
  l->append(v);

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

KJSO ArgumentsNode::evaluate(KJScriptImp */*script*/, Context */*context*/)
{
  assert(0);
  return KJSO(); // dummy, see evaluateList()
}

// ECMA 11.2.4
List* ArgumentsNode::evaluateList(KJScriptImp *script, Context *context)
{
  if (!list)
    return new List();

  return list->evaluateList(script,context);
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

KJSO NewExprNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e = expr->evaluate(script,context);
  KJSO v = e.getValue();

  List *argList = args ? args->evaluateList(script,context) : 0;

  if (!v.isObject()) {
    delete argList;
    return throwError(TypeError, "Expression is no object. Cannot be new'ed");
  }

  Constructor constr = Constructor::dynamicCast(v);
  if (constr.isNull()) {
    delete argList;
    return throwError(TypeError, "Expression is no constructor.");
  }

  if (!argList)
    argList = new List;

  KJSO res = constr.construct(*argList);

  delete argList;

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
KJSO FunctionCallNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e = expr->evaluate(script,context);

  List *argList = args->evaluateList(script,context);

  KJSO v = e.getValue();

  if (!v.isObject()) {
#ifndef NDEBUG
    printInfo("Failed function call attempt on", e, line);
#endif
    delete argList;
    return throwError(TypeError, "Expression is no object. Cannot be called.");
  }

  if (!v.implementsCall()) {
#ifndef NDEBUG
    printInfo("Failed function call attempt on", e, line);
#endif
    delete argList;
    return throwError(TypeError, "Expression does not allow calls.");
  }

  KJSO o;
  if (e.isA(ReferenceType))
    o = e.getBase();
  else
    o = Null();

  if (o.isA(ActivationType))
    o = Null();

  KJSO result = v.executeCall(o, argList);

  delete argList;

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
KJSO PostfixNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e = expr->evaluate(script,context);
  KJSO v = e.getValue();
  Number n = v.toNumber();

  double newValue = (oper == OpPlusPlus) ? n.value() + 1 : n.value() - 1;
  KJSO n2 = Number(newValue);

  e.putValue(n2);

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
KJSO DeleteNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e = expr->evaluate(script,context);
  if (!e.isA(ReferenceType))
    return Boolean(true);
  KJSO b = e.getBase();
  UString n = e.getPropertyName();
  bool ret = b.deleteProperty(n);

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
KJSO VoidNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO dummy1 = expr->evaluate(script,context);
  KJSO dummy2 = dummy1.getValue();

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
KJSO TypeOfNode::evaluate(KJScriptImp *script, Context *context)
{
  const char *s = 0L;
  KJSO e = expr->evaluate(script,context);
  if (e.isA(ReferenceType)) {
    KJSO b = e.getBase();
    if (b.isA(NullType))
      return String("undefined");
  }
  KJSO v = e.getValue();
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
      if (v.implementsCall())
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
KJSO PrefixNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e = expr->evaluate(script,context);
  KJSO v = e.getValue();
  Number n = v.toNumber();

  double newValue = (oper == OpPlusPlus) ? n.value() + 1 : n.value() - 1;
  KJSO n2 = Number(newValue);

  e.putValue(n2);

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
KJSO UnaryPlusNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e = expr->evaluate(script,context);
  KJSO v = e.getValue();

  return v.toNumber();
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
KJSO NegateNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e = expr->evaluate(script,context);
  KJSO v = e.getValue();
  Number n = v.toNumber();

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
KJSO BitwiseNotNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e = expr->evaluate(script,context);
  KJSO v = e.getValue();
  int i32 = v.toInt32();

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
KJSO LogicalNotNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e = expr->evaluate(script,context);
  KJSO v = e.getValue();
  Boolean b = v.toBoolean();

  return Boolean(!b.value());
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
KJSO MultNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO t1 = term1->evaluate(script,context);
  KJSO v1 = t1.getValue();

  KJSO t2 = term2->evaluate(script,context);
  KJSO v2 = t2.getValue();

  return mult(v1, v2, oper);
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
KJSO AddNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO t1 = term1->evaluate(script,context);
  KJSO v1 = t1.getValue();

  KJSO t2 = term2->evaluate(script,context);
  KJSO v2 = t2.getValue();

  return add(v1, v2, oper);
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
KJSO ShiftNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO t1 = term1->evaluate(script,context);
  KJSO v1 = t1.getValue();
  KJSO t2 = term2->evaluate(script,context);
  KJSO v2 = t2.getValue();
  unsigned int i2 = v2.toUInt32();
  i2 &= 0x1f;

  long result;
  switch (oper) {
  case OpLShift:
    result = v1.toInt32() << i2;
    break;
  case OpRShift:
    result = v1.toInt32() >> i2;
    break;
  case OpURShift:
    result = v1.toUInt32() >> i2;
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
KJSO RelationalNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e1 = expr1->evaluate(script,context);
  KJSO v1 = e1.getValue();
  KJSO e2 = expr2->evaluate(script,context);
  KJSO v2 = e2.getValue();

  bool b;
  if (oper == OpLess || oper == OpGreaterEq) {
    int r = relation(v1, v2);
    if (r < 0)
      b = false;
    else
      b = (oper == OpLess) ? (r == 1) : (r == 0);
  } else if (oper == OpGreater || oper == OpLessEq) {
    int r = relation(v2, v1);
    if (r < 0)
      b = false;
    else
      b = (oper == OpGreater) ? (r == 1) : (r == 0);
  } else if (oper == OpIn) {
      /* Is all of this OK for host objects? */
      if (!v2.isObject())
          return throwError( TypeError,
                             "Shift expression not an object into IN expression." );
      b = v2.hasProperty(v1.toString().value());
  } else {
    /* TODO: should apply to Function _objects_ only */
    if (!v2.derivedFrom("Function"))
      return throwError(TypeError,
			"Called instanceof operator on non-function object." );
    return hasInstance(v2, v1);	/* TODO: make object member function */
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
KJSO EqualNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e1 = expr1->evaluate(script,context);
  KJSO e2 = expr2->evaluate(script,context);
  KJSO v1 = e1.getValue();
  KJSO v2 = e2.getValue();

  bool result;
  if (oper == OpEqEq || oper == OpNotEq) {
    // == and !=
    bool eq = equal(v1, v2);
    result = oper == OpEqEq ? eq : !eq;
  } else {
    // === and !==
    bool eq = strictEqual(v1, v2);
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
KJSO BitOperNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e1 = expr1->evaluate(script,context);
  KJSO v1 = e1.getValue();
  KJSO e2 = expr2->evaluate(script,context);
  KJSO v2 = e2.getValue();
  int i1 = v1.toInt32();
  int i2 = v2.toInt32();
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
KJSO BinaryLogicalNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e1 = expr1->evaluate(script,context);
  KJSO v1 = e1.getValue();
  Boolean b1 = v1.toBoolean();
  if ((!b1.value() && oper == OpAnd) || (b1.value() && oper == OpOr))
    return v1;

  KJSO e2 = expr2->evaluate(script,context);
  KJSO v2 = e2.getValue();

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
KJSO ConditionalNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e = logical->evaluate(script,context);
  KJSO v = e.getValue();
  Boolean b = v.toBoolean();

  if (b.value())
    e = expr1->evaluate(script,context);
  else
    e = expr2->evaluate(script,context);

  return e.getValue();
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
KJSO AssignNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO l, e, v;
  ErrorType err;
  if (oper == OpEqual) {
    l = left->evaluate(script,context);
    e = expr->evaluate(script,context);
    v = e.getValue();
  } else {
    l = left->evaluate(script,context);
    KJSO v1 = l.getValue();
    e = expr->evaluate(script,context);
    KJSO v2 = e.getValue();
    int i1 = v1.toInt32();
    int i2 = v2.toInt32();
    switch (oper) {
    case OpMultEq:
      v = mult(v1, v2, '*');
      break;
    case OpDivEq:
      v = mult(v1, v2, '/');
      break;
    case OpPlusEq:
      v = add(v1, v2, '+');
      break;
    case OpMinusEq:
      v = add(v1, v2, '-');
      break;
    case OpLShift:
      v = Number(i1 <<= i2);
      break;
    case OpRShift:
      v = Number(i1 >>= i2);
      break;
    case OpURShift:
      i1 = v1.toUInt32();
      v = Number(i1 >>= i2);
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
    case OpModEq:
      v = Number(i1 %= i2);
      break;
    default:
      v = Undefined();
    }
    err = l.putValue(v);
  };

  err = l.putValue(v);
  if (err == NoError)
    return v;
  else
    return throwError(err, "Invalid reference.");
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
KJSO CommaNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e = expr1->evaluate(script,context);
  KJSO dummy = e.getValue(); // ignore return value
  e = expr2->evaluate(script,context);

  return e.getValue();
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
Completion StatListNode::execute(KJScriptImp *script, Context *context)
{
  if (!list) {
    Completion c = statement->execute(script,context);
    KJS_ABORTPOINT
    if (script->hadException()) {
      KJSO ex = KJScriptImp::exception();
      KJScriptImp::clearException();
      return Completion(Throw, ex);
    } else
      return c;
  }

  Completion l = list->execute(script,context);
  KJS_ABORTPOINT
  if (l.complType() != Normal)
    return l;
  Completion e = statement->execute(script,context);
  KJS_ABORTPOINT;
  if (script->hadException()) {
    KJSO ex = KJScriptImp::exception();
    KJScriptImp::clearException();
    return Completion(Throw, ex);
  }

  KJSO v = e.isValueCompletion() ? e.value() : l.value();

  return Completion(e.complType(), v, e.target() );
}

void StatListNode::processVarDecls(KJScriptImp *script, Context *context)
{
  statement->processVarDecls(script,context);

  if (list)
    list->processVarDecls(script,context);
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
KJSO AssignExprNode::evaluate(KJScriptImp *script, Context *context)
{
  return expr->evaluate(script,context);
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
KJSO VarDeclNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO variable = context->variableObject();

  KJSO val, tmp;
  if (init) {
      tmp = init->evaluate(script,context);
      val = tmp.getValue();
  } else {
      if ( variable.hasProperty( ident ) ) // already declared ?
          return KJSO();
      val = Undefined();
  }
  variable.put(ident, val, DontDelete);

  // spec wants to return ident. But what for ? Will be ignored above.
  return KJSO();
}

void VarDeclNode::processVarDecls(KJScriptImp */*script*/, Context *context)
{
  KJSO variable = context->variableObject();
  variable.put(ident, Undefined(), DontDelete);
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
KJSO VarDeclListNode::evaluate(KJScriptImp *script, Context *context)
{
  if (list)
    (void) list->evaluate(script,context);

  (void) var->evaluate(script,context);

  return KJSO();
}

void VarDeclListNode::processVarDecls(KJScriptImp *script, Context *context)
{
  if (list)
    list->processVarDecls(script,context);

  var->processVarDecls(script,context);
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
Completion VarStatementNode::execute(KJScriptImp *script, Context *context)
{
  KJS_BREAKPOINT;

  (void) list->evaluate(script,context); // returns 0L

  return Completion(Normal);
}

void VarStatementNode::processVarDecls(KJScriptImp *script, Context *context)
{
  list->processVarDecls(script,context);
}

// ------------------------------ BlockNode ------------------------------------

BlockNode::~BlockNode()
{
}

void BlockNode::ref()
{
  Node::ref();
  if ( statlist )
    statlist->ref();
}

bool BlockNode::deref()
{
  if ( statlist && statlist->deref() )
    delete statlist;
  return Node::deref();
}

// ECMA 12.1
Completion BlockNode::execute(KJScriptImp *script, Context *context)
{
  if (!statlist)
    return Completion(Normal);

  return statlist->execute(script,context);
}

void BlockNode::processVarDecls(KJScriptImp *script, Context *context)
{
  if (statlist)
    statlist->processVarDecls(script,context);
}

// ------------------------------ EmptyStatementNode ---------------------------

// ECMA 12.3
Completion EmptyStatementNode::execute(KJScriptImp */*script*/, Context */*context*/)
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
Completion ExprStatementNode::execute(KJScriptImp *script, Context *context)
{
  KJS_BREAKPOINT;

  KJSO e = expr->evaluate(script,context);
  KJSO v = e.getValue();

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
Completion IfNode::execute(KJScriptImp *script, Context *context)
{
  KJS_BREAKPOINT;

  KJSO e = expr->evaluate(script,context);
  KJSO v = e.getValue();
  Boolean b = v.toBoolean();

  // if ... then
  if (b.value())
    return statement1->execute(script,context);

  // no else
  if (!statement2)
    return Completion(Normal);

  // else
  return statement2->execute(script,context);
}

void IfNode::processVarDecls(KJScriptImp *script, Context *context)
{
  statement1->processVarDecls(script,context);

  if (statement2)
    statement2->processVarDecls(script,context);
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
Completion DoWhileNode::execute(KJScriptImp *script, Context *context)
{
  KJS_BREAKPOINT;

  KJSO be, bv;
  Completion c;
  KJSO value;

  do {
    // bail out on error
    if (script->hadException())
      return Completion(Throw, KJScriptImp::exception());

    c = statement->execute(script,context);
    if (!((c.complType() == Continue) && ls.contains(c.target()))) {
      if ((c.complType() == Break) && ls.contains(c.target()))
        return Completion(Normal, value);
      if (c.complType() != Normal)
        return c;
    }
    be = expr->evaluate(script,context);
    bv = be.getValue();
  } while (bv.toBoolean().value());

  return Completion(Normal, value);
}

void DoWhileNode::processVarDecls(KJScriptImp *script, Context *context)
{
  statement->processVarDecls(script,context);
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
Completion WhileNode::execute(KJScriptImp *script, Context *context)
{
  KJS_BREAKPOINT;

  KJSO be, bv;
  Completion c;
  Boolean b(false);
  KJSO value;

  while (1) {
    be = expr->evaluate(script,context);
    bv = be.getValue();
    b = bv.toBoolean();

    // bail out on error
    if (script->hadException())
      return Completion(Throw, KJScriptImp::exception());

    if (!b.value())
      return Completion(Normal, value);

    c = statement->execute(script,context);
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

void WhileNode::processVarDecls(KJScriptImp *script, Context *context)
{
  statement->processVarDecls(script,context);
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
Completion ForNode::execute(KJScriptImp *script, Context *context)
{
  KJSO e, v, cval;
  Boolean b;

  if (expr1) {
    e = expr1->evaluate(script,context);
    v = e.getValue();
  }
  while (1) {
    if (expr2) {
      e = expr2->evaluate(script,context);
      v = e.getValue();
      b = v.toBoolean();
      if (b.value() == false)
	return Completion(Normal, cval);
    }
    // bail out on error
    if (script->hadException())
      return Completion(Throw, KJScriptImp::exception());

    Completion c = statement->execute(script,context);
    if (c.isValueCompletion())
      cval = c.value();
    if (!((c.complType() == Continue) && ls.contains(c.target()))) {
      if ((c.complType() == Break) && ls.contains(c.target()))
        return Completion(Normal, cval);
      if (c.complType() != Normal)
      return c;
    }
    if (expr3) {
      e = expr3->evaluate(script,context);
      v = e.getValue();
    }
  }
}

void ForNode::processVarDecls(KJScriptImp *script, Context *context)
{
  if (expr1)
    expr1->processVarDecls(script,context);

  statement->processVarDecls(script,context);
}

// ------------------------------ ForInNode ------------------------------------

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
  return Node::deref();
}

// ECMA 12.6.4
Completion ForInNode::execute(KJScriptImp *script, Context *context)
{
  KJSO e, v, retval;
  Completion c;
  VarDeclNode *vd = 0;
  const PropList *lst, *curr;

  // This should be done in the constructor
  if (!lexpr) { // for( var foo = bar in baz )
    vd = new VarDeclNode(&ident, init);
    vd->evaluate(script,context);

    lexpr = new ResolveNode(&ident);
  }

  e = expr->evaluate(script,context);
  v = e.getValue().toObject();
  curr = lst = v.imp()->propList();

  while (curr) {
    if (!v.hasProperty(curr->name)) {
      curr = curr->next;
      continue;
    }

    e = lexpr->evaluate(script,context);
    e.putValue(String(curr->name));

    c = statement->execute(script,context);
    if (c.isValueCompletion())
      retval = c.value();

    if (!((c.complType() == Continue) && ls.contains(c.target()))) {
      if ((c.complType() == Break) && ls.contains(c.target()))
        break;
      if (c.complType() != Normal) {
        delete lst;
        return c;
      }
    }

    curr = curr->next;
  }

  delete lst;
  return Completion(Normal, retval);
}

void ForInNode::processVarDecls(KJScriptImp *script, Context *context)
{
  statement->processVarDecls(script,context);
}

// ------------------------------ ContinueNode ---------------------------------

// ECMA 12.7
Completion ContinueNode::execute(KJScriptImp *script, Context *context)
{
  KJS_BREAKPOINT;

  KJSO dummy;
  return context->seenLabels()->contains(ident) ?
    Completion(Continue, dummy, ident) :
    Completion(Throw,
	       throwError(SyntaxError, "Label not found in containing block"));
}

// ------------------------------ BreakNode ------------------------------------

// ECMA 12.8
Completion BreakNode::execute(KJScriptImp *script, Context *context)
{
  KJS_BREAKPOINT;

  KJSO dummy;
  return context->seenLabels()->contains(ident) ?
    Completion(Break, dummy, ident) :
    Completion(Throw,
	       throwError(SyntaxError, "Label not found in containing block"));
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
Completion ReturnNode::execute(KJScriptImp *script, Context *context)
{
  KJS_BREAKPOINT;

  if (!value)
    return Completion(ReturnValue, Undefined());

  KJSO e = value->evaluate(script,context);
  KJSO v = e.getValue();

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
Completion WithNode::execute(KJScriptImp *script, Context *context)
{
  KJS_BREAKPOINT;

  KJSO e = expr->evaluate(script,context);
  KJSO v = e.getValue();
  Object o = v.toObject();
  context->pushScope(o);
  Completion res = statement->execute(script,context);
  context->popScope();

  return res;
}

void WithNode::processVarDecls(KJScriptImp *script, Context *context)
{
  statement->processVarDecls(script,context);
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
KJSO CaseClauseNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e = expr->evaluate(script,context);
  KJSO v = e.getValue();
  return v;
}

// ECMA 12.11
Completion CaseClauseNode::evalStatements(KJScriptImp *script, Context *context)
{
  if (list)
    return list->execute(script,context);
  else
    return Completion(Normal, Undefined());
}

void CaseClauseNode::processVarDecls(KJScriptImp *script, Context *context)
{
  if (list)
    list->processVarDecls(script,context);
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

KJSO ClauseListNode::evaluate(KJScriptImp */*script*/, Context */*context*/)
{
  /* should never be called */
  assert(false);
  return KJSO();
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

void ClauseListNode::processVarDecls(KJScriptImp *script, Context *context)
{
  if (cl)
    cl->processVarDecls(script,context);
  if (nx)
    nx->processVarDecls(script,context);
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

KJSO CaseBlockNode::evaluate(KJScriptImp */*script*/, Context */*context*/)
{
  /* should never be called */
  assert(false);
  return KJSO();
}

// ECMA 12.11
Completion CaseBlockNode::evalBlock(KJScriptImp *script, Context *context, const KJSO& input)
{
  KJSO v;
  Completion res;
  ClauseListNode *a = list1, *b = list2;
  CaseClauseNode *clause;

  if (a) {
    while (a) {
      clause = a->clause();
      a = a->next();
      v = clause->evaluate(script,context);
      if (strictEqual(input, v)) {
	res = clause->evalStatements(script,context);
	if (res.complType() != Normal)
	  return res;
	while (a) {
	  res = a->clause()->evalStatements(script,context);
	  if (res.complType() != Normal)
	    return res;
	  a = a->next();
	}
	break;
      }
    }
  }

  while (b) {
    clause = b->clause();
    b = b->next();
    v = clause->evaluate(script,context);
    if (strictEqual(input, v)) {
      res = clause->evalStatements(script,context);
      if (res.complType() != Normal)
	return res;
      goto step18;
    }
  }

  // default clause
  if (def) {
    res = def->evalStatements(script,context);
    if (res.complType() != Normal)
      return res;
  }
  b = list2;
 step18:
  while (b) {
    clause = b->clause();
    res = clause->evalStatements(script,context);
    if (res.complType() != Normal)
      return res;
    b = b->next();
  }

  return Completion(Normal);
}

void CaseBlockNode::processVarDecls(KJScriptImp *script, Context *context)
{
  if (list1)
    list1->processVarDecls(script,context);
  if (def)
    def->processVarDecls(script,context);
  if (list2)
    list2->processVarDecls(script,context);
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
Completion SwitchNode::execute(KJScriptImp *script, Context *context)
{
  KJS_BREAKPOINT;

  KJSO e = expr->evaluate(script,context);
  KJSO v = e.getValue();
  Completion res = block->evalBlock(script,context,v);

  if ((res.complType() == Break) && ls.contains(res.target()))
    return Completion(Normal, res.value());
  else
    return res;
}

void SwitchNode::processVarDecls(KJScriptImp *script, Context *context)
{
  block->processVarDecls(script,context);
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
Completion LabelNode::execute(KJScriptImp *script, Context *context)
{
  Completion e;

  if (!context->seenLabels()->push(label)) {
    return Completion( Throw,
		       throwError(SyntaxError, "Duplicated label found" ));
  };
  e = statement->execute(script,context);
  context->seenLabels()->pop();

  if ((e.complType() == Break) && (e.target() == label))
    return Completion(Normal, e.value());
  else
    return e;
}

void LabelNode::processVarDecls(KJScriptImp *script, Context *context)
{
  statement->processVarDecls(script,context);
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
Completion ThrowNode::execute(KJScriptImp *script, Context *context)
{
  KJS_BREAKPOINT;

  KJSO v = expr->evaluate(script,context).getValue();

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

Completion CatchNode::execute(KJScriptImp */*script*/, Context */*context*/)
{
  // should never be reached. execute(const KJS &arg) is used instead
  assert(0L);
  return Completion();
}

// ECMA 12.14
Completion CatchNode::execute(KJScriptImp *script, Context *context, const KJSO &arg)
{
  /* TODO: correct ? Not part of the spec */
  KJScriptImp::clearException();

  Object obj;
  obj.put(ident, arg, DontDelete);
  context->pushScope(obj);
  Completion c = block->execute(script,context);
  context->popScope();

  return c;
}

void CatchNode::processVarDecls(KJScriptImp *script, Context *context)
{
  block->processVarDecls(script,context);
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
Completion FinallyNode::execute(KJScriptImp *script, Context *context)
{
  return block->execute(script,context);
}

void FinallyNode::processVarDecls(KJScriptImp *script, Context *context)
{
  block->processVarDecls(script,context);
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
Completion TryNode::execute(KJScriptImp *script, Context *context)
{
  KJS_BREAKPOINT;

  Completion c, c2;

  c = block->execute(script,context);

  if (!_final) {
    if (c.complType() != Throw)
      return c;
    return _catch->execute(script,context,c.value());
  }

  if (!_catch) {
    c2 = _final->execute(script,context);
    return (c2.complType() == Normal) ? c : c2;
  }

  if (c.complType() == Throw)
    c = _catch->execute(script,context,c.value());

  c2 = _final->execute(script,context);
  return (c2.complType() == Normal) ? c : c2;
}

void TryNode::processVarDecls(KJScriptImp *script, Context *context)
{
  block->processVarDecls(script,context);
  if (_final)
    _final->processVarDecls(script,context);
  if (_catch)
    _catch->processVarDecls(script,context);
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
KJSO ParameterNode::evaluate(KJScriptImp */*script*/, Context */*context*/)
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
Completion FunctionBodyNode::execute(KJScriptImp *script, Context *context)
{
  /* TODO: workaround for empty body which I don't see covered by the spec */
  if (!source)
    return Completion(ReturnValue, Undefined());

  source->processFuncDecl(script,context);

  return source->execute(script,context);
}

void FunctionBodyNode::processVarDecls(KJScriptImp *script, Context *context)
{
  if (source)
    source->processVarDecls(script,context);
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
void FuncDeclNode::processFuncDecl(KJScriptImp */*script*/, Context *context)
{
  const List *sc = context->pScopeChain();
  /* TODO: let this be an object with [[Class]] property "Function" */
  FunctionImp *fimp = new DeclaredFunctionImp(ident, body, sc);
  Function func(fimp); // protect from GC
  fimp->put("prototype", Object::create(ObjectClass), DontDelete);

  int plen = 0;
  for(ParameterNode *p = param; p != 0L; p = p->nextParam(), plen++)
    fimp->addParameter(p->ident());

  fimp->setLength(plen);

  context->variableObject().put(ident, func);
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
KJSO FuncExprNode::evaluate(KJScriptImp */*script*/, Context *context)
{
  const List *sc = context->pScopeChain();
  FunctionImp *fimp = new DeclaredFunctionImp(UString::null, body, sc);
  Function ret(fimp);

  int plen = 0;
  for(ParameterNode *p = param; p != 0L; p = p->nextParam(), plen++)
    fimp->addParameter(p->ident());
  fimp->setLength(plen);

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
Completion SourceElementNode::execute(KJScriptImp *script, Context *context)
{
  if (statement)
    return statement->execute(script,context);

  return Completion(Normal);
}

// ECMA 14
void SourceElementNode::processFuncDecl(KJScriptImp *script, Context *context)
{
  if (function)
    function->processFuncDecl(script,context);
}

void SourceElementNode::processVarDecls(KJScriptImp *script, Context *context)
{
  if (statement)
    statement->processVarDecls(script,context);
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
Completion SourceElementsNode::execute(KJScriptImp *script, Context *context)
{
  if (script->hadException())
    return Completion(Throw, KJScriptImp::exception());

  if (!elements)
    return element->execute(script,context);

  Completion c1 = elements->execute(script,context);
  if (script->hadException())
    return Completion(Throw, KJScriptImp::exception());
  if (c1.complType() != Normal)
    return c1;

  Completion c2 = element->execute(script,context);
  if (script->hadException())
    return Completion(Throw, KJScriptImp::exception());

  return c2;
}

// ECMA 14
void SourceElementsNode::processFuncDecl(KJScriptImp *script, Context *context)
{
  if (elements)
    elements->processFuncDecl(script,context);

  element->processFuncDecl(script,context);
}

void SourceElementsNode::processVarDecls(KJScriptImp *script, Context *context)
{
  if (elements)
    elements->processVarDecls(script,context);

  element->processVarDecls(script,context);
}
