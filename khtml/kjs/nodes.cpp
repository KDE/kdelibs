/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
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

#include <assert.h>
#include <stdio.h>

#include <qstring.h>
#include <qmessagebox.h>

#include "kjs.h"
#include "global.h"
#include "kjsstring.h"
#include "lexer.h"
#include "nodes.h"
#include "object.h"
#include "operations.h"

using namespace KJS;

int   Node::nodeCount = 0;
Node *Node::firstNode = 0L;

Node::Node()
{
  assert(KJSWorld::lexer);
  line = KJSWorld::lexer->yylineno;
  nodeCount++;
  //  cout << "Node()" << endl;

  // create a list of allocated objects. Makes
  // deleting (even after a parse error) quite easy
  nextNode = firstNode;
  firstNode = this;
}

Node::~Node()
{
  //  cout << "~Node()" << endl;
  nodeCount--;
}

void Node::deleteAllNodes()
{
  Node *tmp, *n = firstNode;

  while ((tmp = n)) {
    n = n->nextNode;
    delete tmp;
  }
  firstNode = 0L;
  assert(nodeCount == 0);
}

KJSO *NullNode::evaluate()
{
  return new KJSNull();
}

KJSO *BooleanNode::evaluate()
{
  return new KJSBoolean(value);
}

KJSO *NumberNode::evaluate()
{
  return new KJSNumber(value);
}

KJSO *StringNode::evaluate()
{
  return new KJSString(value);
}

// ECMA 11.1.1
KJSO *ThisNode::evaluate()
{
  return KJSWorld::context->thisValue;
}

// ECMA 11.1.2 & 10.1.4
KJSO *ResolveNode::evaluate()
{
  assert(KJSWorld::context);
  KJSScope *scope = KJSWorld::context->firstScope();

  while (scope) {
    KJSO *obj = scope->object;
    if (obj->hasProperty(ident)) {
      //      cout << "Resolve: found '" << ident.ascii() << "'" << endl;
      return new KJSReference(obj, ident);
    }
    scope = scope->next;
  }

  // identifier not found
  cout << "Resolve: didn't find '" << ident.ascii() << "'" << endl;
  return new KJSReference(zeroRef(new KJSNull()), ident); // TODO: use a global Null
}

// ECMA 11.1.4
KJSO *GroupNode::evaluate()
{
  return group->evaluate();
}

// ECMA 11.2.1a
KJSO *AccessorNode1::evaluate()
{
  Ptr e1 = expr1->evaluate();
  Ptr v1 = e1->getValue();
  Ptr e2 = expr2->evaluate();
  Ptr v2 = e2->getValue();
  Ptr o = toObject(v1);
  Ptr s = toString(v2);
  KJSReference *ref = new KJSReference(o, s->sVal().ascii());

  return ref;
}

// ECMA 11.2.1b
KJSO *AccessorNode2::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();
  Ptr o = toObject(v);
  KJSReference *ref = new KJSReference(o, ident);

  return ref;
}

// ECMA 11.2.2
KJSO *NewExprNode::evaluate()
{
  KJSO *v = expr->evaluate()->getValue();

  KJSO *a;
  if (args)
    a = args->evaluate();
  else
    a = 0L;

  if (!v->isObject())
    return new KJSError(ErrExprNoObject, this);
  if (!v->implementsConstruct())
    return new KJSError(ErrNoConstruct, this);

  KJSO *res = v->construct(a);
  if (!res->isObject())
    return new KJSError(ErrResNoObject, this);

  return res;
}

// ECMA 11.2.3
KJSO *FunctionCallNode::evaluate()
{
  Ptr e = expr->evaluate();

  KJSArgList *argList = args->evaluateList();

  Ptr v = e->getValue();

  if (!v->isObject()) {
    return new KJSError(ErrFuncNoObject, this);
  }

  if (!v->implementsCall()) {
    return new KJSError(ErrFuncNoCall, this);
  }

  Ptr o;
  if (e->isA(Reference))
    o = e->getBase();
  else
    o = new KJSNull();

  if (o->isA(Activation))
    o = new KJSNull();

  KJSO *result = v->executeCall(o, argList);

  delete argList;
  return result;
}

// ECMA 11.2.4
KJSArgList *ArgumentsNode::evaluateList()
{
  if (!list)
    return new KJSArgList();

  return list->evaluateList();
}

// ECMA 11.2.4
KJSArgList *ArgumentListNode::evaluateList()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();

  if (!list) {
    KJSArgList *l = new KJSArgList();
    return l->append(v);
  }

  KJSArgList *l = list->evaluateList();

  return l->append(v);
}

// ECMA 11.8
KJSO *RelationalNode::evaluate()
{
  Ptr e1 = expr1->evaluate();
  Ptr v1 = e1->getValue();
  Ptr e2 = expr2->evaluate();
  Ptr v2 = e2->getValue();
  /* TODO: abstract relational comparison */
  return new KJSBoolean(true);
}

// ECMA 11.9
KJSO *EqualNode::evaluate()
{
  Ptr e1 = expr1->evaluate();
  Ptr e2 = expr2->evaluate();
  Ptr v1 = e1->getValue();
  Ptr v2 = e2->getValue();

  return new KJSBoolean(compare(v1, oper, v2));
}

// ECMA 11.10
KJSO *BitOperNode::evaluate()
{
  Ptr e1 = expr1->evaluate();
  Ptr v1 = e1->getValue();
  Ptr e2 = expr2->evaluate();
  Ptr v2 = e2->getValue();
  int i1 = toInt32(v1);
  int i2 = toInt32(v2);
  int result;
  if (oper == OpBitAnd)
    result = i1 & i2;
  else if (oper == OpBitXOr)
    result = i1 ^ i2;
  else
    result = i1 | i2;

  return new KJSNumber(result);
}

// ECMA 11.11
KJSO *BinaryLogicalNode::evaluate()
{
  Ptr e1 = expr1->evaluate();
  Ptr v1 = e1->getValue();
  Ptr b1 = toBoolean(v1);
  if ((!b1->bVal() && oper == OpAnd) || (b1->bVal() && oper == OpOr))
    return v1->ref();

  Ptr e2 = expr2->evaluate();
  Ptr v2 = e2->getValue();

  return v2->ref();
}

// ECMA 11.12
KJSO *ConditionalNode::evaluate()
{
  Ptr e = logical->evaluate();
  Ptr v = e->getValue();
  Ptr b = toBoolean(v);

  if (b->bVal())
    e = expr1->evaluate();
  else
    e = expr2->evaluate();

  return e->getValue();
}

// ECMA 11.13
KJSO *AssignNode::evaluate()
{
  Ptr l, e, v;
  ErrorCode err;
  switch (oper)
    {
    case OpEqual:
      l = left->evaluate();
      e = expr->evaluate();
      v = e->getValue();
      err = l->putValue(v);
      if (err == ErrOK)
	return v.ref();
      else
	return new KJSError(err, this);
      break;
    default:
      assert(!"AssignNode: unhandled switch case");
      return 0L;
    };
}

// ECMA 11.3
KJSO *PostfixNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();
  Ptr n = toNumber(v);

  double newValue = (oper == OpPlusPlus) ? n->dVal() + 1 : n->dVal() - 1;
  Ptr n2 = new KJSNumber(newValue);

  e->putValue(n2);

  return n->ref();
}

// ECMA 11.4.1
KJSO *DeleteNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr b = e->getBase();
  CString n = e->getPropertyName(); /* TODO: runtime err if no ref */
  if (!b->isA(Object))
    return new KJSBoolean(true);
  /* TODO [delete] */
  return new KJSBoolean(!b->hasProperty(n));
}

// ECMA 11.4.2
KJSO *VoidNode::evaluate()
{
  Ptr dummy1 = expr->evaluate();
  Ptr dummy2 = dummy1->getValue();

  return new KJSUndefined();
}

// ECMA 11.4.3
KJSO *TypeOfNode::evaluate()
{
  const char *s = 0L;
  Ptr e = expr->evaluate();
  if (e->isA(Reference)) {
    Ptr b = e->getBase();
    if (b->isA(Null))
      return new KJSUndefined();
  }
  Ptr v = e->getValue();
  switch (v->type())
    {
    case Undefined:
      s = "undefined";
      break;
    case Null:
      s = "object";
      break;
    case Boolean:
      s = "boolean";
      break;
    case Number:
      s = "number";
      break;
    case String:
      s = "string";
      break;
    case Object:
      if (v->implementsCall())
	s = "function";
      else
	s = "object";
      break;
    default:
      assert(!"TypeOfNode::evaluate(): unhandled switch case");
    }

  return new KJSString(s);
}

// ECMA 11.4.4 and 11.4.5
KJSO *PrefixNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();
  Ptr n = toNumber(v);

  double newValue = (oper == OpPlusPlus) ? n->dVal() + 1 : n->dVal() - 1;
  Ptr n2 = new KJSNumber(newValue);

  e->putValue(n2);

  return n2->ref();
}

// ECMA 11.4.6
KJSO *UnaryPlusNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();
  
  return toNumber(v);
}

// ECMA 11.4.7
KJSO *NegateNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();
  Ptr n = toNumber(v);
  
  double d = -n->dVal();
  /* TODO: handle NaN */

  return new KJSNumber(d);
}

// ECMA 11.4.8
KJSO *BitwiseNotNode::evaluate()
{
  /* TODO */
}

// ECMA 11.4.9
KJSO *LogicalNotNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();
  Ptr b = toBoolean(v);

  return new KJSBoolean(!b->bVal());
}

KJSO *MultNode::evaluate()
{
  Ptr t1 = term1->evaluate();
  Ptr v1 = t1->getValue();

  Ptr t2 = term2->evaluate();
  Ptr v2 = t2->getValue();

  Ptr n1 = toNumber(v1);
  Ptr n2 = toNumber(v2);

  KJSNumber *result;

  if (oper == '*')
    result = new KJSNumber(n1->dVal() * n2->dVal()); // TODO: NaN, Infinity ...
  else // if (oper == '/')
    result = new KJSNumber(n1->dVal() / n2->dVal()); // TODO: NaN, Infinity ...
  //  else
  //    result = new KJSNumber(n1->dVal() % n2->dVal()); // TODO: Double, NaN ...

  return result;
}

// ECMA 11.7
KJSO *ShiftNode::evaluate()
{
  Ptr t1 = term1->evaluate();
  Ptr v1 = t1->getValue();
  Ptr t2 = term2->evaluate();
  Ptr v2 = t2->getValue();
  unsigned int i2 = toUInt32(v2);
  i2 &= 0x1f;

  long result;
  switch (oper) {
  case OpLShift:
    result = toInt32(v1) << i2;
    break;
  case OpRShift:
    result = toInt32(v1) >> i2;
    break;
  case OpURShift:
    result = toUInt32(v1) >> i2; /* TODO: unsigned shift */
    break;
  default:
    assert(!"ShiftNode: unhandled switch case");
    result = 0;
  }

  return new KJSNumber(static_cast<double>(result));
}

KJSO *AddNode::evaluate()
{
  Ptr t1 = term1->evaluate();
  Ptr v1 = t1->getValue();

  Ptr t2 = term2->evaluate();
  Ptr v2 = t2->getValue();

  Ptr p1 = toPrimitive(v1);
  Ptr p2 = toPrimitive(v2);

  if ((p1->isA(String) || p2->isA(String)) && oper == '+') {
    Ptr s1 = toString(p1);
    Ptr s2 = toString(p2);
 
    UString s = s1->sVal() + s2->sVal();

    KJSO *res = new KJSString(s);

    return res;
  }

  Ptr n1 = toNumber(p1);
  Ptr n2 = toNumber(p2);

  KJSNumber *result;
  if (oper == '+')
    result = new KJSNumber(n1->dVal() + n2->dVal());
  else
    result = new KJSNumber(n1->dVal() - n2->dVal());

  return result;
}

// ECMA 11.14
KJSO *CommaNode::evaluate()
{
  Ptr e = expr1->evaluate();
  Ptr dummy = e->getValue(); // ignore return value
  e = expr2->evaluate();

  return e->getValue();
}

// ECMA 12.1
KJSO *BlockNode::evaluate()
{
  if (!statlist)
    return new KJSCompletion(Normal);

  return statlist->evaluate();
}

// ECMA 12.1
KJSO *StatListNode::evaluate()
{
  if (!list)
    return statement->evaluate();

  Ptr l = list->evaluate();

  if (l->isA(Completion) && l->cVal() != Normal) // Completion check needed ?
    return l->ref();
  Ptr e = statement->evaluate();
  if (e->isValueCompletion())
    return e->ref();
  if (!l->isValueCompletion())
    return e->ref();
  Ptr v = l->getValue();
  if (e->isA(Completion) && e->cVal() == Break)
    return new KJSCompletion(Break, v);
  if (e->isA(Completion) && e->cVal() == Continue)
    return new KJSCompletion(Continue, v);

  return new KJSCompletion(Normal, v);
}

// ECMA 12.2
KJSO *VarStatementNode::evaluate()
{
  list->evaluate();

  return new KJSCompletion(Normal);
}

// ECMA 12.2
KJSO *VarDeclNode::evaluate()
{
  KJSO *variable = KJSWorld::context->variableObject();

  // TODO: coded with help of 10.1.3. Correct ?
  if (!variable->hasProperty(ident)) {
    //    KJSO *val;
    Ptr val, tmp;
    if (init) {
      tmp = init->evaluate();
      val = tmp->getValue();
    } else
      val = new KJSUndefined();
    variable->put(ident, val);
  }
}

// ECMA 12.2
KJSO *VarDeclListNode::evaluate()
{
  if (list)
    list->evaluate();

  var->evaluate();

  return new KJSUndefined();
}

// ECMA 12.2
KJSO *AssignExprNode::evaluate()
{
  return expr->evaluate();
}

// ECMA 12.3
KJSO *EmptyStatementNode::evaluate()
{
  return new KJSCompletion(Normal);
}

// ECMA 12.6.2
KJSO *ForNode::evaluate()
{
  Ptr e, v, b, cval;
  if (expr1) {
    e = expr1->evaluate();
    v = e->getValue();
  }
  while (1) {
    if (expr2) {
      e = expr2->evaluate();
      v = e->getValue();
      b = toBoolean(v);
      if (b->bVal() == false)
	return new KJSCompletion(Normal);
    }
    e = stat->evaluate();
    if (e->isValueCompletion())
      cval = e->complValue();
    if (e->cVal() == Break)
      return new KJSCompletion(Normal, cval);
    if (e->cVal() == ReturnValue)
      return e->ref();
    if (expr3) {
      e = expr3->evaluate();
      v = e->getValue();
    }
  }
}

// ECMA 12.6.3
KJSO *ForInNode::evaluate()
{
  /* TODO */
  return new KJSCompletion(Normal);
}

// ECMA 12.4
KJSO *ExprStatementNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();

  return new KJSCompletion(Normal, v);
}

// ECMA 12.5
KJSO *IfNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();
  Ptr b = toBoolean(v);

  // if ... then
  if (b->bVal())
    return statement1->evaluate();

  // no else
  if (!statement2)
    return new KJSCompletion(Normal);

  // else
  return statement2->evaluate();
}

// ECMA 12.6.1
KJSO *WhileNode::evaluate()
{
  Ptr b, be, bv, e;
  KJSO *value = 0L;

  while (1) {
    be = expr->evaluate();
    bv = be->getValue();
    b = toBoolean(bv);

    if (!b->bVal())
      break;

    e = statement->evaluate();
    if (e->isA(Completion)) { // really needed ? are all stat's completions ?
      if (e->isValueCompletion())
	value = e;
      if (e->cVal() == Break)
	break;
      if (e->cVal() == Continue)
	continue;
      if (e->cVal() == ReturnValue)
	return e->ref();
    }
  }

  return new KJSCompletion(Normal, value);
}

// ECMA 12.7
KJSO *ContinueNode::evaluate()
{
  return new KJSCompletion(Continue);
}

// ECMA 12.8
KJSO *BreakNode::evaluate()
{
  return new KJSCompletion(Break);
}

// ECMA 12.9
KJSO *ReturnNode::evaluate()
{
  if (!value)
    return new KJSCompletion(ReturnValue, zeroRef(new KJSUndefined()));

  Ptr e = value->evaluate();
  Ptr v = e->getValue();

  return new KJSCompletion(ReturnValue, v);
}

// ECMA 12.10
KJSO *WithNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();
  Ptr o = toObject(v);
  /* TODO: prepend scope */
  Ptr res = stat->evaluate();
  /* TODO: remove scope */
  return res->ref();
}

// ECMA 13
void FuncDeclNode::processFuncDecl()
{
  int num = 0;
  ParameterNode *p = param;
  while (p) {
    p = p->nextParam();
    num++;
  }
  KJSParamList *plist = new KJSParamList(num);
  p = param;
  for(int i = 0; i < num; i++, p = p->nextParam())
    plist->insert(num - i - 1, p->ident());

  Ptr f = new KJSDeclaredFunction(plist, block);

  /* TODO: decide between global and activation object */
  KJSWorld::global->put(ident, f);
}

// ECMA 13
KJSO *ParameterNode::evaluate()
{
  /* TODO */
}

// ECMA 14
KJSO *ProgramNode::evaluate()
{
  source->processFuncDecl();

  return source->evaluate();
}

// ECMA 14
KJSO *SourceElementsNode::evaluate()
{
  if (KJSWorld::error)
    return new KJSCompletion(ReturnValue, KJSWorld::error);

  if (!elements)
    return element->evaluate();

  Ptr res1 = elements->evaluate();
  if (KJSWorld::error)
    return new KJSCompletion(ReturnValue, KJSWorld::error);

  Ptr res2 = element->evaluate();
  if (KJSWorld::error)
    return new KJSCompletion(ReturnValue, KJSWorld::error);

  if (res2->isA(Completion))
    return res2.ref();

  return res1.ref();
}

// ECMA 14
void SourceElementsNode::processFuncDecl()
{
  element->processFuncDecl();

  if (elements)
    elements->processFuncDecl();
}

// ECMA 14
KJSO *SourceElementNode::evaluate()
{
  if (statement)
    return statement->evaluate();

  return new KJSCompletion(Normal);
}

// ECMA 14
void SourceElementNode::processFuncDecl()
{
  if (function)
    function->processFuncDecl();
}

// for debugging purposes
KJSO *PrintNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();
  Ptr s = toString(v);

  cout << "---> " << s->sVal().ascii() << endl;

  return new KJSCompletion(Normal);
}

// for debugging purposes
KJSO *AlertNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();
  Ptr s = toString(v);

  cout << "---> " << s->sVal().ascii() << endl;
  UString u = s->sVal();
  QString str = QString((QChar*) u.unicode(), u.length());
  QMessageBox::information(0L, "KJS", str);

  return new KJSCompletion(Normal);
}

ArgumentListNode::ArgumentListNode(Node *e) : list(0L), expr(e) {}

VarDeclNode::VarDeclNode(const CString &id, AssignExprNode *in)
    : ident(id), init(in) { }

ArgumentListNode::ArgumentListNode(ArgumentListNode *l, Node *e) :  
    list(l), expr(e) {}

ArgumentsNode::ArgumentsNode(ArgumentListNode *l) : list(l) {}
