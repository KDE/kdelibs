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

// ------------------------------ Node -----------------------------------------

Node::Node()
{
  //  assert(Lexer::curr());
  //  line = Lexer::curr()->lineNo();
  line = 0; // ### FIXME
}

Node::Node(const Node &other)
{
  line = other.line;
}

Node::~Node()
{
}

KJSO Node::throwError(ErrorType e, const char *msg)
{
  return Error::create(e, msg, lineNo());
}


// ------------------------------ StatementNode --------------------------------

StatementNode::StatementNode(const StatementNode &other) : Node(other)
{
  ls = other.ls; // ### write copy-constructor for ls
  l0 = other.l0;
  l1 = other.l1;
  sid = other.sid;
  breakPoint = other.breakPoint;
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

Completion StatementNode::execute(KJScriptImp */*script*/, Context */*context*/)
{
  // subclasses should always override execute9)
  assert(false);
}

// ------------------------------ NullNode -------------------------------------

NullNode::NullNode(const NullNode &other) : Node(other)
{
}

Node *NullNode::copy() const
{
  return new NullNode(*this);
}

KJSO NullNode::evaluate(KJScriptImp */*script*/, Context */*context*/)
{
  return Null();
}

// ------------------------------ BooleanNode ----------------------------------

BooleanNode::BooleanNode(const BooleanNode &other) : Node(other)
{ 
  value = other.value;
}

Node *BooleanNode::copy() const
{
  return new BooleanNode(*this);
}

KJSO BooleanNode::evaluate(KJScriptImp */*script*/, Context */*context*/)
{
  return Boolean(value);
}

// ------------------------------ NumberNode -----------------------------------

NumberNode::NumberNode(const NumberNode &other) : Node(other)
{
  value = other.value;
}

Node *NumberNode::copy() const
{
  return new NumberNode(*this);
}

KJSO NumberNode::evaluate(KJScriptImp */*script*/, Context */*context*/)
{
  return Number(value);
}

// ------------------------------ StringNode -----------------------------------

StringNode::StringNode(const StringNode &other) : Node(other)
{
  value = other.value;
}

Node *StringNode::copy() const
{
  return new StringNode(*this);
}

KJSO StringNode::evaluate(KJScriptImp */*script*/, Context */*context*/)
{
  return String(value);
}

// ------------------------------ RegExpNode -----------------------------------

RegExpNode::RegExpNode(const RegExpNode &other) : Node(other)
{
  pattern = other.pattern;
  flags = other.flags;
}

Node *RegExpNode::copy() const
{
  return new RegExpNode(*this);
}

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

ThisNode::ThisNode(const ThisNode &other) : Node(other)
{
}

Node *ThisNode::copy() const
{
  return new ThisNode(*this);
}

// ECMA 11.1.1
KJSO ThisNode::evaluate(KJScriptImp */*script*/, Context *context)
{
  return context->thisValue();
}

// ------------------------------ ResolveNode ----------------------------------

ResolveNode::ResolveNode(const ResolveNode &other) : Node(other)
{
  ident = other.ident;
}

Node *ResolveNode::copy() const
{
  return new ResolveNode(*this);
}

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

GroupNode::GroupNode(const GroupNode &other) : Node(other)
{
  group = other.group ? other.group->copy() : 0;
}

GroupNode::~GroupNode()
{
  if (group)
    delete group;
}

Node *GroupNode::copy() const
{
  return new GroupNode(*this);
}

// ECMA 11.1.6
KJSO GroupNode::evaluate(KJScriptImp *script, Context *context)
{
  return group->evaluate(script,context);
}

// ------------------------------ ElisionNode ----------------------------------

ElisionNode::ElisionNode(const ElisionNode &other) : Node(other)
{
  elision = other.elision ? static_cast<ElisionNode*>(other.elision->copy()) : 0;
}

ElisionNode::~ElisionNode()
{
  if (elision)
    delete elision;
}

Node *ElisionNode::copy() const
{
  return new ElisionNode(*this);
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

ElementNode::ElementNode(const ElementNode &other) : Node(other)
{
  list = other.list ? static_cast<ElementNode*>(other.list->copy()) : 0;
  elision = other.elision ? static_cast<ElisionNode*>(other.elision->copy()) : 0;
  node = other.node ? other.node->copy() : 0;
}

ElementNode::~ElementNode()
{
  if (list)
    delete list;
  if (elision)
    delete elision;
  if (node)
    delete node;
}

Node *ElementNode::copy() const
{
  return new ElementNode(*this);
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

ArrayNode::ArrayNode(const ArrayNode &other) : Node(other)
{
  element = other.element ? static_cast<ElementNode*>(other.element->copy()) : 0;
  elision = other.elision ? static_cast<ElisionNode*>(other.elision->copy()) : 0;
  opt = other.opt;
}

ArrayNode::~ArrayNode()
{
  if (element)
    delete element;
  if (elision)
    delete elision;
}

Node *ArrayNode::copy() const
{
  return new ArrayNode(*this);
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

ObjectLiteralNode::ObjectLiteralNode(const ObjectLiteralNode &other) : Node (other)
{
  list = other.list ? other.list->copy() : 0;
}

ObjectLiteralNode::~ObjectLiteralNode()
{
  if (list)
    delete list;
}

Node *ObjectLiteralNode::copy() const
{
  return new ObjectLiteralNode(*this);
}

// ECMA 11.1.5
KJSO ObjectLiteralNode::evaluate(KJScriptImp *script, Context *context)
{
  if (list)
    return list->evaluate(script,context);

  return Object::create(ObjectClass);
}

// ------------------------------ PropertyValueNode ----------------------------

PropertyValueNode::PropertyValueNode(const PropertyValueNode &other) : Node (other)
{
  name = other.name ? other.name->copy() : 0;
  assign = other.assign ? other.assign->copy() : 0;
  list = other.list ? other.list->copy() : 0;
}

PropertyValueNode::~PropertyValueNode()
{
  if (name)
    delete name;
  if (assign)
    delete assign;
  if (list)
    delete list;
}

Node *PropertyValueNode::copy() const
{
  return new PropertyValueNode(*this);
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

PropertyNode::PropertyNode(const PropertyNode &other) : Node(other)
{
  numeric = other.numeric;
  str = other.str;
}

Node *PropertyNode::copy() const
{
  return new PropertyNode(*this);
}

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

AccessorNode1::AccessorNode1(const AccessorNode1 &other) : Node(other)
{
  expr1 = other.expr1 ? other.expr1->copy() : 0;
  expr2 = other.expr2 ? other.expr2->copy() : 0;
}

AccessorNode1::~AccessorNode1()
{
  if (expr1)
    delete expr1;
  if (expr2)
    delete expr2;
}

Node *AccessorNode1::copy() const
{
  return new AccessorNode1(*this);
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

AccessorNode2::AccessorNode2(const AccessorNode2 &other) : Node (other)
{
  expr = other.expr ? other.expr->copy() : 0;
  ident = other.ident;
}

AccessorNode2::~AccessorNode2()
{
  delete expr;
}

Node *AccessorNode2::copy() const
{
  return new AccessorNode2(*this);
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

ArgumentListNode::ArgumentListNode(const ArgumentListNode &other) : Node(other)
{
  list = other.list ? static_cast<ArgumentListNode*>(other.list->copy()) : 0;
  expr = other.expr ? other.expr->copy() : 0;
}

ArgumentListNode::~ArgumentListNode()
{
  if (list)
    delete list;
  if (expr)
    delete expr;
}

Node *ArgumentListNode::copy() const
{
  return new ArgumentListNode(*this);
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

ArgumentsNode::ArgumentsNode(const ArgumentsNode &other) : Node(other)
{
  list = other.list ? static_cast<ArgumentListNode*>(other.list->copy()) : 0;
}

ArgumentsNode::~ArgumentsNode()
{
  if (list)
    delete list;
}

Node *ArgumentsNode::copy() const
{
  return new ArgumentsNode(*this);
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

NewExprNode::NewExprNode(const NewExprNode &other) : Node(other)
{
  expr = other.expr ? other.expr->copy() : 0;
  args = other.args ? static_cast<ArgumentsNode*>(other.args->copy()) : 0;
}

NewExprNode::~NewExprNode()
{
  if (expr)
    delete expr;
  if (args)
    delete args;
}

Node *NewExprNode::copy() const
{
  return new NewExprNode(*this);
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

FunctionCallNode::FunctionCallNode(const FunctionCallNode &other) : Node(other)
{
  expr = other.expr ? other.expr->copy() : 0;
  args = other.args ? static_cast<ArgumentsNode*>(other.args->copy()) : 0;
}

FunctionCallNode::~FunctionCallNode()
{
  if (expr)
    delete expr;
  if (args)
    delete args;
}

Node *FunctionCallNode::copy() const
{
  return new FunctionCallNode(*this);
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

PostfixNode::PostfixNode(const PostfixNode &other) : Node(other)
{
  expr = other.expr ? other.expr->copy() : 0;
  oper = other.oper;
}

PostfixNode::~PostfixNode()
{
  if (expr)
    delete expr;
}

Node *PostfixNode::copy() const
{
  return new PostfixNode(*this);
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

DeleteNode::DeleteNode(const DeleteNode &other) : Node(other)
{
  expr = other.expr ? other.expr->copy() : 0;
}

DeleteNode::~DeleteNode()
{
  if (expr)
    delete expr;
}

Node *DeleteNode::copy() const
{
  return new DeleteNode(*this);
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

VoidNode::VoidNode(const VoidNode &other) : Node(other)
{
  expr = other.expr ? other.expr->copy() : 0;
}

VoidNode::~VoidNode()
{
  if (expr)
    delete expr;
}

Node *VoidNode::copy() const
{
  return new VoidNode(*this);
}

// ECMA 11.4.2
KJSO VoidNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO dummy1 = expr->evaluate(script,context);
  KJSO dummy2 = dummy1.getValue();

  return Undefined();
}

// ------------------------------ TypeOfNode -----------------------------------

TypeOfNode::TypeOfNode(const TypeOfNode &other) : Node(other)
{
  expr = other.expr ? other.expr->copy() : 0;
}

TypeOfNode::~TypeOfNode()
{
  if (expr)
    delete expr;
}

Node *TypeOfNode::copy() const
{
  return new TypeOfNode(*this);
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

PrefixNode::PrefixNode(const PrefixNode &other) : Node(other)
{
  oper = other.oper;
  expr = other.expr ? other.expr->copy() : 0;
}

PrefixNode::~PrefixNode()
{
  delete expr;
}

Node *PrefixNode::copy() const
{
  return new PrefixNode(*this);
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

UnaryPlusNode::UnaryPlusNode(const UnaryPlusNode &other) : Node(other)
{
  expr = other.expr ? other.expr->copy() : 0;
}

UnaryPlusNode::~UnaryPlusNode()
{
  if (expr)
    delete expr;
}

Node *UnaryPlusNode::copy() const
{
  return new UnaryPlusNode(*this);
}

// ECMA 11.4.6
KJSO UnaryPlusNode::evaluate(KJScriptImp *script, Context *context)
{
  KJSO e = expr->evaluate(script,context);
  KJSO v = e.getValue();

  return v.toNumber();
}

// ------------------------------ NegateNode -----------------------------------

NegateNode::NegateNode(const NegateNode &other) : Node(other)
{
  expr = other.expr ? other.expr->copy() : 0;
}

NegateNode::~NegateNode()
{
  if (expr)
    delete expr;
}

Node *NegateNode::copy() const
{
  return new NegateNode(*this);
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

BitwiseNotNode::BitwiseNotNode(const BitwiseNotNode &other) : Node(other)
{
  expr = other.expr ? other.expr->copy() : 0;
}

BitwiseNotNode::~BitwiseNotNode()
{
  delete expr;
}

Node *BitwiseNotNode::copy() const
{
  return new BitwiseNotNode(*this);
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

LogicalNotNode::LogicalNotNode(const LogicalNotNode &other) : Node(other)
{
  expr = other.expr ? other.expr->copy() : 0;
}

LogicalNotNode::~LogicalNotNode()
{
  if (expr)
    delete expr;
}

Node *LogicalNotNode::copy() const
{
  return new LogicalNotNode(*this);
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

MultNode::MultNode(const MultNode &other) : Node(other)
{
  term1 = other.term1 ? other.term1->copy() : 0;
  term2 = other.term2 ? other.term2->copy() : 0;
  oper = other.oper;
}

MultNode::~MultNode()
{
  if (term1)
    delete term1;
  if (term2)
    delete term2;
}

Node *MultNode::copy() const
{
  return new MultNode(*this);
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

AddNode::AddNode(const AddNode &other) : Node(other)
{
  term1 = other.term1 ? other.term1->copy() : 0;
  term2 = other.term2 ? other.term2->copy() : 0;
  oper = other.oper;
}

AddNode::~AddNode()
{
  if (term1)
    delete term1;
  if (term2)
    delete term2;
}

Node *AddNode::copy() const
{
  return new AddNode(*this);
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

ShiftNode::ShiftNode(const ShiftNode &other) : Node(other)
{
  term1 = other.term1 ? other.term1->copy() : 0;
  term2 = other.term2 ? other.term2->copy() : 0;
  oper = other.oper;
}

ShiftNode::~ShiftNode()
{
  if (term1)
    delete term1;
  if (term2)
    delete term2;
}

Node *ShiftNode::copy() const
{
  return new ShiftNode(*this);
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

RelationalNode::RelationalNode(const RelationalNode &other) : Node(other)
{
  expr1 = other.expr1 ? other.expr1->copy() : 0;
  expr2 = other.expr2 ? other.expr2->copy() : 0;
  oper = other.oper;
}

RelationalNode::~RelationalNode()
{
  if (expr1)
    delete expr1;
  if (expr2)
    delete expr2;
}

Node *RelationalNode::copy() const
{
  return new RelationalNode(*this);
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

EqualNode::EqualNode(const EqualNode &other) : Node(other)
{
  expr1 = other.expr1 ? other.expr1->copy() : 0;
  expr2 = other.expr2 ? other.expr2->copy() : 0;
  oper = other.oper;
}

EqualNode::~EqualNode()
{
  if (expr1)
    delete expr1;
  if (expr2)
    delete expr2;
}

Node *EqualNode::copy() const
{
  return new EqualNode(*this);
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

BitOperNode::BitOperNode(const BitOperNode &other) : Node(other)
{
  expr1 = other.expr1 ? other.expr1->copy() : 0;
  expr2 = other.expr2 ? other.expr2->copy() : 0;
  oper = other.oper;
}

BitOperNode::~BitOperNode()
{
  if (expr1)
    delete expr1;
  if (expr2)
    delete expr2;
}

Node *BitOperNode::copy() const
{
  return new BitOperNode(*this);
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

BinaryLogicalNode::BinaryLogicalNode(const BinaryLogicalNode &other) : Node(other)
{
  expr1 = other.expr1 ? other.expr1->copy() : 0;
  expr2 = other.expr2 ? other.expr2->copy() : 0;
  oper = other.oper;
}

BinaryLogicalNode::~BinaryLogicalNode()
{
  if (expr1)
    delete expr1;
  if (expr2)
    delete expr2;
}

Node *BinaryLogicalNode::copy() const
{
  return new BinaryLogicalNode(*this);
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

ConditionalNode::ConditionalNode(const ConditionalNode &other) : Node(other)
{
  logical = other.logical ? other.logical->copy() : 0;
  expr1 = other.expr1 ? other.expr1->copy() : 0;
  expr2 = other.expr2 ? other.expr2->copy() : 0;
}

ConditionalNode::~ConditionalNode()
{
  if (logical)
    delete logical;
  if (expr1)
    delete expr1;
  if (expr2)
    delete expr2;
}

Node *ConditionalNode::copy() const
{
  return new ConditionalNode(*this);
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

AssignNode::AssignNode(const AssignNode &other) : Node(other)
{
  left = other.left ? other.left->copy() : 0;
  oper = other.oper;
  expr = other.expr ? other.expr->copy() : 0;
}

AssignNode::~AssignNode()
{
  if (left)
    delete left;
  if (expr)
    delete expr;
}

Node *AssignNode::copy() const
{
  return new AssignNode(*this);
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

CommaNode::CommaNode(const CommaNode &other) : Node(other)
{
  expr1 = other.expr1 ? other.expr1->copy() : 0;
  expr2 = other.expr2 ? other.expr2->copy() : 0;
}

CommaNode::~CommaNode()
{
  if (expr1)
    delete expr1;
  if (expr2)
    delete expr2;
}

Node *CommaNode::copy() const
{
  return new CommaNode(*this);
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

StatListNode::StatListNode(const StatListNode &other) : StatementNode(other)
{
  statement = other.statement ? static_cast<StatementNode*>(other.statement->copy()) : 0;
  list = other.list ? static_cast<StatListNode*>(other.list->copy()) : 0;
}

StatListNode::~StatListNode()
{
  if (statement)
    delete statement;
  if (list)
    delete list;
}

Node *StatListNode::copy() const
{
  return new StatListNode(*this);
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

AssignExprNode::AssignExprNode(const AssignExprNode &other) : Node(other)
{
  expr = other.expr ? other.expr->copy() : 0;
}

AssignExprNode::~AssignExprNode()
{
  if (expr)
    delete expr;
}

Node *AssignExprNode::copy() const
{
  return new AssignExprNode(*this);
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

VarDeclNode::VarDeclNode(const VarDeclNode &other) : Node(other)
{
  ident = other.ident;
  init = other.init ? static_cast<AssignExprNode*>(other.init->copy()) : 0;
}

VarDeclNode::~VarDeclNode()
{
  if (init)
    delete init;
}

Node *VarDeclNode::copy() const
{
  return new VarDeclNode(*this);
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

VarDeclListNode::VarDeclListNode(const VarDeclListNode &other) : Node(other)
{
  list = other.list ? other.list->copy() : 0;
  var = other.var ? static_cast<VarDeclNode*>(other.var->copy()) : 0;
}

VarDeclListNode::~VarDeclListNode()
{
  if (list)
    delete list;
  if (var)
    delete var;
}

Node *VarDeclListNode::copy() const
{
  return new VarDeclListNode(*this);
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

VarStatementNode::VarStatementNode(const VarStatementNode &other) : StatementNode(other)
{
  list = other.list ? static_cast<VarDeclListNode*>(other.list->copy()) : 0;
}

VarStatementNode::~VarStatementNode()
{
  if (list)
    delete list;
}

Node *VarStatementNode::copy() const
{
  return new VarStatementNode(*this);
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

BlockNode::BlockNode(const BlockNode &other) : StatementNode(other)
{
  statlist = other.statlist ? static_cast<StatListNode*>(other.statlist->copy()) : 0;
}

BlockNode::~BlockNode()
{
  if (statlist)
    delete statlist;
}

Node *BlockNode::copy() const
{
  return new BlockNode(*this);
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

EmptyStatementNode::EmptyStatementNode(const EmptyStatementNode &other) : StatementNode(other)
{
}

Node *EmptyStatementNode::copy() const
{
  return new EmptyStatementNode(*this);
}

// ECMA 12.3
Completion EmptyStatementNode::execute(KJScriptImp */*script*/, Context */*context*/)
{
  return Completion(Normal);
}

// ------------------------------ ExprStatementNode ----------------------------

ExprStatementNode::ExprStatementNode(const ExprStatementNode &other) : StatementNode(other)
{
  expr = other.expr ? other.expr->copy() : 0;
}

ExprStatementNode::~ExprStatementNode()
{
  if (expr)
    delete expr;
}

Node *ExprStatementNode::copy() const
{
  return new ExprStatementNode(*this);
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

IfNode::IfNode(const IfNode &other) : StatementNode(other)
{
  expr = other.expr ? other.expr->copy() : 0;
  statement1 = other.statement1 ? static_cast<StatementNode*>(other.statement1->copy()) : 0;
  statement2 = other.statement2 ? static_cast<StatementNode*>(other.statement2->copy()) : 0;
}

IfNode::~IfNode()
{
  if (expr)
    delete expr;
  if (statement1)
    delete statement1;
  if (statement2)
    delete statement2;
}

Node *IfNode::copy() const
{
  return new IfNode(*this);
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

DoWhileNode::DoWhileNode(const DoWhileNode &other) : StatementNode(other)
{
  statement = other.statement ? static_cast<StatementNode*>(other.statement->copy()) : 0;
  expr = other.expr ? other.expr->copy() : 0;
}

DoWhileNode::~DoWhileNode()
{
  if (statement)
    delete statement;
  if (expr)
    delete expr;
}

Node *DoWhileNode::copy() const
{
  return new DoWhileNode(*this);
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

WhileNode::WhileNode(const WhileNode &other) : StatementNode(other)
{
  expr = other.expr ? other.expr->copy() : 0;
  statement = other.statement ? static_cast<StatementNode*>(other.statement->copy()) : 0;
}

WhileNode::~WhileNode()
{
  if (expr)
    delete expr;
  if (statement)
    delete statement;
}

Node *WhileNode::copy() const
{
  return new WhileNode(*this);
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

ForNode::ForNode(const ForNode &other) : StatementNode(other)
{
  expr1 = other.expr1 ? other.expr1->copy() : 0;
  expr2 = other.expr2 ? other.expr2->copy() : 0;
  expr3 = other.expr3 ? other.expr3->copy() : 0;
  stat = other.stat ? static_cast<StatementNode*>(other.stat->copy()) : 0;
}

ForNode::~ForNode()
{
  if (expr1)
    delete expr1;
  if (expr2)
    delete expr2;
  if (expr3)
    delete expr3;
  if (stat)
    delete stat;
}

Node *ForNode::copy() const
{
  return new ForNode(*this);
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

    Completion c = stat->execute(script,context);
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

  stat->processVarDecls(script,context);
}

// ------------------------------ ForInNode ------------------------------------

ForInNode::ForInNode(const ForInNode &other) : StatementNode(other)
{
  ident = other.ident;
  init = other.init ? static_cast<AssignExprNode*>(other.init->copy()) : 0;
  lexpr = other.lexpr ? other.lexpr->copy() : 0;
  expr = other.expr ? other.expr->copy() : 0;
  stat = other.stat ? static_cast<StatementNode*>(other.stat->copy()) : 0;
}

ForInNode::~ForInNode()
{
  if (init)
    delete init;
  if (lexpr)
    delete lexpr;
  if (expr)
    delete expr;
  if (stat)
    delete stat;
}

Node *ForInNode::copy() const
{
  return new ForInNode(*this);
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

    c = stat->execute(script,context);
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
  stat->processVarDecls(script,context);
}

// ------------------------------ ContinueNode ---------------------------------

ContinueNode::ContinueNode(const ContinueNode &other) : StatementNode(other)
{
  ident = other.ident;
}

Node *ContinueNode::copy() const
{
  return new ContinueNode(*this);
}

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

BreakNode::BreakNode(const BreakNode &other) : StatementNode(other)
{
  ident = other.ident;
}

Node *BreakNode::copy() const
{
  return new BreakNode(*this);
}

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

ReturnNode::ReturnNode(const ReturnNode &other) : StatementNode(other)
{
  value = other.value ? other.value->copy() : 0;
}

ReturnNode::~ReturnNode()
{
  if (value)
    delete value;
}

Node *ReturnNode::copy() const
{
  return new ReturnNode(*this);
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

WithNode::WithNode(const WithNode &other) : StatementNode(other)
{
  expr = other.expr ? other.expr->copy() : 0;
  stat = other.stat ? static_cast<StatementNode*>(other.stat->copy()) : 0;
}

WithNode::~WithNode()
{
  if (expr)
    delete expr;
  if (stat)
    delete stat;
}

Node *WithNode::copy() const
{
  return new WithNode(*this);
}

// ECMA 12.10
Completion WithNode::execute(KJScriptImp *script, Context *context)
{
  KJS_BREAKPOINT;

  KJSO e = expr->evaluate(script,context);
  KJSO v = e.getValue();
  Object o = v.toObject();
  context->pushScope(o);
  Completion res = stat->execute(script,context);
  context->popScope();

  return res;
}

void WithNode::processVarDecls(KJScriptImp *script, Context *context)
{
  stat->processVarDecls(script,context);
}

// ------------------------------ CaseClauseNode -------------------------------

CaseClauseNode::CaseClauseNode(const CaseClauseNode &other) : Node(other)
{
  expr = other.expr ? other.expr->copy() : 0;
  list = other.list ? static_cast<StatListNode*>(other.list->copy()) : 0;
}

CaseClauseNode::~CaseClauseNode()
{
  if (expr)
    delete expr;
  if (list)
    delete list;
}

Node *CaseClauseNode::copy() const
{
  return new CaseClauseNode(*this);
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

ClauseListNode::ClauseListNode(const ClauseListNode &other) : Node(other)
{
  cl = other.cl ? static_cast<CaseClauseNode*>(other.cl->copy()) : 0;
  nx = other.nx ? static_cast<ClauseListNode*>(other.nx->copy()) : 0;
}

ClauseListNode::~ClauseListNode()
{
  if (cl)
    delete cl;
  if (nx)
    delete nx;
}

Node *ClauseListNode::copy() const
{
  return new ClauseListNode(*this);
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

CaseBlockNode::CaseBlockNode(const CaseBlockNode &other) : Node(other)
{
  list1 = other.list1 ? static_cast<ClauseListNode*>(other.list1->copy()) : 0;
  def = other.def ? static_cast<CaseClauseNode*>(other.def->copy()) : 0;
  list2 = other.list2 ? static_cast<ClauseListNode*>(other.list2->copy()) : 0;
}

CaseBlockNode::~CaseBlockNode()
{
  if (list1)
    delete list1;
  if (def)
    delete def;
  if (list2)
    delete list2;
}

Node *CaseBlockNode::copy() const
{
  return new CaseBlockNode(*this);
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

SwitchNode::SwitchNode(const SwitchNode &other) : StatementNode(other)
{
  expr = other.expr ? other.expr->copy() : 0;
  block = other.block ? static_cast<CaseBlockNode*>(other.block->copy()) : 0;
}

SwitchNode::~SwitchNode()
{
  if (expr)
    delete expr;
  if (block)
    delete block;
}

Node *SwitchNode::copy() const
{
  return new SwitchNode(*this);
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

LabelNode::LabelNode(const LabelNode &other) : StatementNode(other)
{
  label = other.label;
  stat = other.stat ? static_cast<StatementNode*>(other.stat->copy()) : 0;
}

LabelNode::~LabelNode()
{
  if (stat)
    delete stat;
}

Node *LabelNode::copy() const
{
  return new LabelNode(*this);
}

// ECMA 12.12
Completion LabelNode::execute(KJScriptImp *script, Context *context)
{
  Completion e;

  if (!context->seenLabels()->push(label)) {
    return Completion( Throw,
		       throwError(SyntaxError, "Duplicated label found" ));
  };
  e = stat->execute(script,context);
  context->seenLabels()->pop();

  if ((e.complType() == Break) && (e.target() == label))
    return Completion(Normal, e.value());
  else
    return e;
}

void LabelNode::processVarDecls(KJScriptImp *script, Context *context)
{
  stat->processVarDecls(script,context);
}

// ------------------------------ ThrowNode ------------------------------------

ThrowNode::ThrowNode(const ThrowNode &other) : StatementNode(other)
{
  expr = other.expr ? other.expr->copy() : 0;
}

ThrowNode::~ThrowNode()
{
  if (expr)
    delete expr;
}

Node *ThrowNode::copy() const
{
  return new ThrowNode(*this);
}

// ECMA 12.13
Completion ThrowNode::execute(KJScriptImp *script, Context *context)
{
  KJS_BREAKPOINT;

  KJSO v = expr->evaluate(script,context).getValue();

  return Completion(Throw, v);
}

// ------------------------------ CatchNode ------------------------------------

CatchNode::CatchNode(const CatchNode &other) : StatementNode(other)
{
  ident = other.ident;
  block = other.block ? static_cast<StatementNode*>(other.block->copy()) : 0;
}

CatchNode::~CatchNode()
{
  if (block)
    delete block;
}

Node *CatchNode::copy() const
{
  return new CatchNode(*this);
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

FinallyNode::FinallyNode(const FinallyNode &other) : StatementNode(other)
{
  block = other.block ? static_cast<StatementNode*>(other.block->copy()) : 0;
}

FinallyNode::~FinallyNode()
{
  if (block)
    delete block;
}

Node *FinallyNode::copy() const
{
  return new FinallyNode(*this);
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

TryNode::TryNode(const TryNode &other) : StatementNode(other)
{
  block = other.block ? static_cast<StatementNode*>(other.block->copy()) : 0;
  _catch = other._catch ? static_cast<CatchNode*>(other._catch->copy()) : 0;
  _final = other._final ? static_cast<FinallyNode*>(other._final->copy()) : 0;
}

TryNode::~TryNode()
{
  if (block)
    delete block;
  if (_final)
    delete _final;
  if (_catch)
    delete _catch;
}

Node *TryNode::copy() const
{
  return new TryNode(*this);
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

ParameterNode::ParameterNode(const ParameterNode &other) : Node(other)
{
  id = other.id;
  next = other.next ? static_cast<ParameterNode*>(other.next->copy()) : 0;
}

ParameterNode::~ParameterNode()
{
  if (next)
    delete next;
}

Node *ParameterNode::copy() const
{
  return new ParameterNode(*this);
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
}

FunctionBodyNode::FunctionBodyNode(const FunctionBodyNode &other) : StatementNode(other)
{
  source = other.source ? static_cast<SourceElementsNode*>(other.source->copy()) : 0;
}

FunctionBodyNode::~FunctionBodyNode()
{
  if (source)
    delete source;
}

Node *FunctionBodyNode::copy() const
{
  return new FunctionBodyNode(*this);
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

FuncDeclNode::FuncDeclNode(const FuncDeclNode &other) : StatementNode(other)
{
  ident = other.ident;
  param = other.param ? static_cast<ParameterNode*>(other.param->copy()) : 0;
  body = other.body ? static_cast<FunctionBodyNode*>(other.body->copy()) : 0;
}

FuncDeclNode::~FuncDeclNode()
{
  if (param)
    delete param;
  if (body)
    delete body;
}

Node *FuncDeclNode::copy() const
{
  return new FuncDeclNode(*this);
}

// ECMA 13
void FuncDeclNode::processFuncDecl(KJScriptImp */*script*/, Context *context)
{
  const List *sc = context->pScopeChain();
  /* TODO: let this be an object with [[Class]] property "Function" */
  FunctionImp *fimp = new DeclaredFunctionImp(ident, static_cast<FunctionBodyNode*>(body->copy()), sc);
  Function func(fimp); // protect from GC
  fimp->put("prototype", Object::create(ObjectClass), DontDelete);

  int plen = 0;
  for(ParameterNode *p = param; p != 0L; p = p->nextParam(), plen++)
    fimp->addParameter(p->ident());

  fimp->setLength(plen);

  context->variableObject().put(ident, func);
}

// ------------------------------ FuncExprNode ---------------------------------

FuncExprNode::FuncExprNode(const FuncExprNode &other) : Node(other)
{
  param = other.param ? static_cast<ParameterNode*>(other.param->copy()) : 0;
  body = other.body ? static_cast<FunctionBodyNode*>(other.body->copy()) : 0;
}

FuncExprNode::~FuncExprNode()
{
  if (param)
    delete param;
  if (body)
    delete body;
}

Node *FuncExprNode::copy() const
{
  return new FuncExprNode(*this);
}

// ECMA 13
KJSO FuncExprNode::evaluate(KJScriptImp */*script*/, Context *context)
{
  const List *sc = context->pScopeChain();
  FunctionImp *fimp = new DeclaredFunctionImp(UString::null, body, sc ? sc->copy() : 0);
  Function ret(fimp);

  int plen = 0;
  for(ParameterNode *p = param; p != 0L; p = p->nextParam(), plen++)
    fimp->addParameter(p->ident());
  fimp->setLength(plen);

  return ret;
}

// ------------------------------ SourceElementNode ----------------------------

SourceElementNode::SourceElementNode(const SourceElementNode &other) : StatementNode(other)
{
  statement = other.statement ? static_cast<StatementNode*>(other.statement->copy()) : 0;
  function = other.function ? static_cast<FuncDeclNode*>(other.function->copy()) : 0;
}

SourceElementNode::~SourceElementNode()
{
  if (function)
    delete function;
  if (statement)
    delete statement;
}

Node *SourceElementNode::copy() const
{
  return new SourceElementNode(*this);
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

SourceElementsNode::SourceElementsNode(const SourceElementsNode &other) : StatementNode(other)
{
  element = other.element ? static_cast<SourceElementNode*>(other.element->copy()) : 0;
  elements = other.elements ? static_cast<SourceElementsNode*>(other.elements->copy()) : 0;
}

SourceElementsNode::~SourceElementsNode()
{
  if (elements)
    delete elements;
  if (element)
    delete element;
}

Node *SourceElementsNode::copy() const
{
  return new SourceElementsNode(*this);
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

// ------------------------------ ProgramNode ----------------------------------

ProgramNode::ProgramNode(const ProgramNode &other) : FunctionBodyNode(other)
{
}

Node *ProgramNode::copy() const
{
  return new ProgramNode(*this);
}

