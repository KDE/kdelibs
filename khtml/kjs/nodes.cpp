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

Node::Node()
{
  assert(KJSWorld::lexer);
  line = KJSWorld::lexer->yylineno;
  KJSWorld::nodeCount++;
  //  cout << "Node()" << endl;
}

Node::~Node()
{
  //  cout << "~Node()" << endl;
  KJSWorld::nodeCount--;
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
      cout << "Resolve: found '" << ident.ascii() << "'" << endl;
      return new KJSReference(obj, ident);
    }
    scope = scope->next;
  }

  // identifier not found
  cout << "Resolve: didn't find '" << ident.ascii() << "'" << endl;
  return new KJSReference(new KJSNull(), ident); // TODO: use a global Null
}

// ECMA 11.1.4
KJSO *GroupNode::evaluate()
{
  return group->evaluate();
}

// ECMA 11.2.1a
KJSO *AccessorNode1::evaluate()
{
  KJSO *e1 = expr1->evaluate();
  KJSO *v1 = e1->getValue();
  KJSO *e2 = expr2->evaluate();
  KJSO *v2 = e2->getValue();
  KJSO *o = toObject(v1);
  KJSO *s = toString(v2);
  KJSO *ref = new KJSReference(o, s->sVal().ascii());

  //  delete s;
  return ref;
}

// ECMA 11.2.1b
KJSO *AccessorNode2::evaluate()
{
  KJSO *e = expr->evaluate();
  KJSO *v = e->getValue();
  KJSO *o = toObject(v);
  KJSO *ref = new KJSReference(o, ident);

  // delete string
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

  if (!v->isObject()) {
    /* TODO: runtime error */
    assert(!"NewExprNode::evaluate(): RUNTIME ERROR. Expr is no object");
  }
  if (!v->implementsConstruct()) {
    /* TODO: runtime error */
    assert(!"NewExprNode::evaluate(): RUNTIME ERROR. No [[Construct]] impl.");
  }

  KJSO *res = v->construct(a);
  if (!res->isObject()) {
    /* TODO: runtime error */
    assert(!"NewExprNode::evaluate(): RUNTIME ERROR. Result is no object.");
  }

  return res;
}

// ECMA 11.2.3
KJSO *FunctionCallNode::evaluate()
{
  Debug("FunctionCallNode::evaluate()\n");

  KJSO *e = expr->evaluate();

  KJSO *tmp = args->evaluate();
  KJSArgList *argList = static_cast<KJSArgList*>(tmp);
  KJSO *v = e->getValue();

  if (!v->isObject()) {
    /* TODO: Runtime Error */
    assert(!"FunctionCallNode::evaluate(): Runtime Error I");
  }

  cout << "TYPE " << (int) v->type() << endl;
  if (!v->implementsCall()) {
    /* TODO: Runtime Error */
    assert(!"FunctionCallNode::evaluate(): Runtime Error II");
  }

  KJSO *o;
  if (e->isA(Reference))
    o = e->getBase();
  else
    o = new KJSNull();

  if (o->isA(Activation))
    o = new KJSNull(); // memory leak

  return v->executeCall(o, argList);
}

// ECMA 11.2.4
KJSO *ArgumentsNode::evaluate()
{
  if (!list)
    return new KJSArgList();

  return list->evaluate();
}

// ECMA 11.2.4
KJSO *ArgumentListNode::evaluate()
{
  if (!list) {
    KJSArgList *l = new KJSArgList();
    return l->append(expr->evaluate()->getValue());
  }

  KJSO *tmp = list->evaluate();
  KJSArgList *l = static_cast<KJSArgList*>(tmp);
  KJSO *v = expr->evaluate()->getValue();

  return l->append(v);
}

// ECMA 11.8
KJSO *RelationalNode::evaluate()
{
  /* TODO */
}

// ECMA 11.9
KJSO *EqualNode::evaluate()
{
  KJSO *v1 = expr1->evaluate()->getValue();
  KJSO *v2 = expr2->evaluate()->getValue();

  return new KJSBoolean(compare(v1, oper, v2));
}

// ECMA 11.10
KJSO *BitOperNode::evaluate()
{
  /* TODO */
}

// ECMA 11.11
KJSO *BinaryLogicalNode::evaluate()
{
  /* TODO */
}

// ECMA 11.12
KJSO *ConditionalNode::evaluate()
{
  KJSO *b = toBoolean(logical->evaluate()->getValue());

  if (b->bVal())
    return expr1->evaluate()->getValue();
  else
    return expr2->evaluate()->getValue();
}

// ECMA 11.13
KJSO *AssignNode::evaluate()
{
  KJSO *l, *e, *v;
  switch (oper)
    {
    case OpEqual:
      l = left->evaluate();
      e = expr->evaluate();
      v = e->getValue();
      l->putValue(v);
      return v;
      break;
    default:
      assert(!"AssignNode: unhandled switch case");
    };
}

// ECMA 11.3
KJSO *PostfixNode::evaluate()
{
  cout << "PostfixNode::evaluate()" << endl;
  /* TODO */
}

// ECMA 11.4.1
KJSO *DeleteNode::evaluate()
{
  /* TODO */
}

// ECMA 11.4.2
KJSO *VoidNode::evaluate()
{
  (void) expr->evaluate()->getValue();

  return new KJSUndefined();
}

// ECMA 11.4.3
KJSO *TypeOfNode::evaluate()
{
  const char *s = 0L;
  KJSO *e = expr->evaluate();
  if (e->isA(Reference) && e->getBase()->isA(Null))
    return new KJSUndefined();
  KJSO *v = e->getValue();
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
  cout << "PrefixNode::evaluate()" << endl;
  /* TODO */
}

// ECMA 11.4.6
KJSO *UnaryPlusNode::evaluate()
{
  KJSO *e = expr->evaluate();
  KJSO *v = e->getValue();
  
  return toNumber(v);
}

// ECMA 11.4.7
KJSO *NegateNode::evaluate()
{
  KJSO *e = expr->evaluate();
  KJSO *v = e->getValue();
  
  double d = -toNumber(v)->dVal();
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
  KJSO *v = expr->evaluate()->getValue();
  KJSO *b = toBoolean(v);

  return new KJSBoolean(!b->bVal());
}

KJSO *MultNode::evaluate()
{
  KJSO *t1 = term1->evaluate();
  KJSO *v1 = t1->getValue();

  KJSO *t2 = term2->evaluate();
  KJSO *v2 = t2->getValue();

  KJSO *n1 = toNumber(v1);
  KJSO *n2 = toNumber(v2);

  KJSNumber *result;

  if (oper == '*')
    result = new KJSNumber(n1->dVal() * n2->dVal()); // TODO: NaN, Infinity ...
  else // if (oper == '/')
    result = new KJSNumber(n1->dVal() / n2->dVal()); // TODO: NaN, Infinity ...
  //  else
  //    result = new KJSNumber(n1->dVal() % n2->dVal()); // TODO: Double, NaN ...

  //  delete n1;
  //  delete n2;
  return result;
}

// ECMA 11.7
KJSO *ShiftNode::evaluate()
{
  /* TODO */
}

KJSO *AddNode::evaluate()
{
  KJSO *t1 = term1->evaluate();
  KJSO *v1 = t1->getValue();

  KJSO *t2 = term2->evaluate();
  KJSO *v2 = t2->getValue();

  KJSO *p1 = toPrimitive(v1);
  KJSO *p2 = toPrimitive(v2);

  if ((p1->isA(String) || p2->isA(String)) && oper == '+') {
    KJSO *s1 = toString(p1);
    KJSO *s2 = toString(p2);
 
    UString s = s1->sVal() + s2->sVal();

    KJSO *res = new KJSString(s);

    return res;
  }

  KJSO *n1 = toNumber(p1);
  KJSO *n2 = toNumber(p2);

  KJSNumber *result;
  if (oper == '+')
    result = new KJSNumber(n1->dVal() + n2->dVal()); // TODO: NaN, Infinity ...
  else
    result = new KJSNumber(n1->dVal() - n2->dVal());

  //  delete n1;
  //  delete n2;
  return result;
}

// ECMA 11.14
KJSO *CommaNode::evaluate()
{
  KJSO *e = expr1->evaluate();
  e->getValue(); // ignore return value
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

  KJSO *l = list->evaluate();
  if (l->isA(Completion) && l->cVal() != Normal) // Completion check needed ?
    return l;
  KJSO *e = statement->evaluate();
  if (e->isValueCompletion())
    return e;
  if (!l->isValueCompletion())
    return e;
  KJSO *v = l->getValue();
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
    KJSO *v = new KJSUndefined();
    variable->put(ident, v);
    // TODO: this looks strange (and leaks memory)
    KJSO *r = new KJSReference(variable, ident);

    if (init) {
      KJSO *val = init->evaluate()->getValue();
      r->putValue(val);
    }
  }
}

// ECMA 12.2
KJSO *VarDeclListNode::evaluate()
{
  if (list)
    list->evaluate();

  var->evaluate();
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

KJSO *ForNode::evaluate()
{
  /* TODO */
}

// ECMA 12.4
KJSO *ExprStatementNode::evaluate()
{
  KJSO *e = expr->evaluate();
  KJSO *v = e->getValue();

  return new KJSCompletion(Normal, v);
}

// ECMA 12.5
KJSO *IfNode::evaluate()
{
  KJSO *e = expr->evaluate();
  KJSO *v = e->getValue();
  KJSO *b = toBoolean(v);

  // if ... then
  if (b->bVal())
    return statement1->evaluate();

  // no else
  if (!statement2)
    return new KJSCompletion(Normal);

  // else
  return statement2->evaluate();
}

// ECMA 12.6
KJSO *WhileNode::evaluate()
{
  KJSO *b, *e, *value = 0L;

  while (1) {
    b = toBoolean(expr->evaluate()->getValue());

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
	return e;
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
    return new KJSCompletion(ReturnValue, new KJSUndefined());

  KJSO *v = value->evaluate()->getValue();

  return new KJSCompletion(ReturnValue, v);
}

// ECMA 12.10
KJSO *WithNode::evaluate()
{
 /* TODO */
}

// ECMA 13
void FuncDeclNode::processFuncDecl()
{
  KJSO *f = new KJSDeclaredFunction(ident, param, block);

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
  fprintf(stderr , "ProgramNode::evaluate()\n");
  source->processFuncDecl();

  return source->evaluate();
}

// ECMA 14
KJSO *SourceElementsNode::evaluate()
{
  if (!elements)
    return element->evaluate();

  KJSO *res1 = elements->evaluate();
  KJSO *res2 = element->evaluate();

  if (res2->isA(Completion))
    return res2;

  return res1;
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
  KJSO *v = expr->evaluate()->getValue();
  KJSO *s = toString(v);

  cout << "---> " << s->sVal().ascii() << endl;
  QMessageBox::information(0L, "KJS", QString(s->sVal().ascii()));

  return new KJSCompletion(Normal);
}

// for debugging purposes
KJSO *AlertNode::evaluate()
{
  KJSO *v = expr->evaluate()->getValue();
  KJSO *s = toString(v);

  cout << "---> " << s->sVal().ascii() << endl;
  QMessageBox::information(0L, "KJS", QString(s->sVal().ascii()));

  return new KJSCompletion(Normal);
}
