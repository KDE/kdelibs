/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
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
#include <math.h>

#include "kjs.h"
#include "ustring.h"
#include "lexer.h"
#include "nodes.h"
#include "types.h"
#include "internal.h"
#include "operations.h"
#include "regexp_object.h"

using namespace KJS;

int   Node::nodeCount = 0;
ProgramNode *Node::prog = 0L;
Node *Node::firstNode = 0L;

Node::Node()
{
  assert(Lexer::curr());
  line = Lexer::curr()->lineNo();
  nodeCount++;
  //  cout << "Node()" << endl;

  // create a list of allocated objects. Makes
  // deleting (even after a parse error) quite easy
  next = firstNode;
  prev = 0L;
  if (firstNode)
    firstNode->prev = this;
  firstNode = this;
}

Node::~Node()
{
  //  cout << "~Node()" << endl;
  if (next)
    next->prev = prev;
  if (prev)
    prev->next = next;
  nodeCount--;
}

void Node::deleteAllNodes()
{
  Node *tmp, *n = firstNode;

  while ((tmp = n)) {
    n = n->next;
    delete tmp;
  }
  firstNode = 0L;
  prog = 0L;
  assert(nodeCount == 0);
}

KJSO Node::throwError(ErrorType e, const char *msg)
{
  return Error::create(e, msg, lineNo());
}

KJSO NullNode::evaluate()
{
  return Null();
}

KJSO BooleanNode::evaluate()
{
  return Boolean(value);
}

KJSO NumberNode::evaluate()
{
  return Number(value);
}

KJSO StringNode::evaluate()
{
  return String(value);
}

KJSO RegExpNode::evaluate()
{
  List list;
  String p(pattern);
  String f(flags);
  list.append(p);
  list.append(f);

  // very ugly
  KJSO r = Global::current().get("RegExp");
  RegExpObject *r2 = (RegExpObject*)r.imp();
  return r2->construct(list);
}

// ECMA 11.1.1
KJSO ThisNode::evaluate()
{
  return KJSO(const_cast<Imp*>(Context::current()->thisValue()));
}

// ECMA 11.1.2 & 10.1.4
KJSO ResolveNode::evaluate()
{
  assert(Context::current());
  const List *chain = Context::current()->pScopeChain();
  assert(chain);
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

// ECMA 11.1.4
KJSO GroupNode::evaluate()
{
  return group->evaluate();
}

// ECMA 11.1.5
KJSO ObjectLiteralNode::evaluate()
{
  if (list)
    return list->evaluate();

  return Object::create(ObjectClass);
}

// ECMA 11.1.5
KJSO PropertyValueNode::evaluate()
{
  KJSO obj;
  if (list)
    obj = list->evaluate();
  else
    obj = Object::create(ObjectClass);
  KJSO n = name->evaluate();
  KJSO a = assign->evaluate();
  KJSO v = a.getValue();

  obj.put(n.toString().value(), v);

  return obj;
}

// ECMA 11.1.5
KJSO PropertyNode::evaluate()
{
  KJSO s;

  if (str.isNull()) {
    s = String(UString::from(numeric));
  } else
    s = String(str);

  return s;
}

// ECMA 11.2.1a
KJSO AccessorNode1::evaluate()
{
  KJSO e1 = expr1->evaluate();
  KJSO v1 = e1.getValue();
  KJSO e2 = expr2->evaluate();
  KJSO v2 = e2.getValue();
  Object o = v1.toObject();
  String s = v2.toString();
  return Reference(o, s.value());
}

// ECMA 11.2.1b
KJSO AccessorNode2::evaluate()
{
  KJSO e = expr->evaluate();
  KJSO v = e.getValue();
  KJSO o = v.toObject();
  return Reference(o, ident);
}

// ECMA 11.2.2
KJSO NewExprNode::evaluate()
{
  KJSO e = expr->evaluate();
  KJSO v = e.getValue();

  List *argList = args ? args->evaluateList() : 0;

  if (!v.isObject()) {
    return throwError(TypeError, "Expression is no object.");
  }
  if (!v.isA(ConstructorType))
    return throwError(TypeError, "Expression is no constructor.");

  Constructor constr = Constructor::dynamicCast(v);

  if (!argList)
    argList = new List;
  
  KJSO res = constr.construct(*argList);

  delete argList;

  return res;
}

// ECMA 11.2.3
KJSO FunctionCallNode::evaluate()
{
  KJSO e = expr->evaluate();

  List *argList = args->evaluateList();

  KJSO v = e.getValue();

  if (!v.isObject()) {
    return throwError(TypeError, "Expression is no object.");
  }

  if (!v.implementsCall()) {
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

KJSO ArgumentsNode::evaluate()
{
  assert(0);
  return KJSO(); // dummy, see evaluateList()
}

// ECMA 11.2.4
List* ArgumentsNode::evaluateList()
{
  if (!list)
    return new List();

  return list->evaluateList();
}

KJSO ArgumentListNode::evaluate()
{
  assert(0);
  return KJSO(); // dummy, see evaluateList()
}

// ECMA 11.2.4
List* ArgumentListNode::evaluateList()
{
  KJSO e = expr->evaluate();
  KJSO v = e.getValue();

  if (!list) {
    List *l = new List();
    l->append(v);
    return l;
  }

  List *l = list->evaluateList();
  l->append(v);

  return l;
}

// ECMA 11.8
KJSO RelationalNode::evaluate()
{
  KJSO e1 = expr1->evaluate();
  KJSO v1 = e1.getValue();
  KJSO e2 = expr2->evaluate();
  KJSO v2 = e2.getValue();

  bool b;
  if (oper == OpLess || oper == OpGreaterEq) {
    int r = relation(v1, v2);
    if (r < 0)
      b = false; 
    else
      b = (oper == OpLess) ? r : !r;
  } else if (oper == OpGreater || oper == OpLessEq) {
    int r = relation(v2, v1);
    if (r < 0)
      b = false; 
    else
      b = (oper == OpGreater) ? r : !r;
  } else {
    /* TODO: instanceof */
    b = false;
  }

  return Boolean(b);
}

// ECMA 11.9
KJSO EqualNode::evaluate()
{
  KJSO e1 = expr1->evaluate();
  KJSO e2 = expr2->evaluate();
  KJSO v1 = e1.getValue();
  KJSO v2 = e2.getValue();

  bool eq = equal(v1, v2);

  return Boolean(oper == OpEqEq ? eq : !eq);
}

// ECMA 11.10
KJSO BitOperNode::evaluate()
{
  KJSO e1 = expr1->evaluate();
  KJSO v1 = e1.getValue();
  KJSO e2 = expr2->evaluate();
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

// ECMA 11.11
KJSO BinaryLogicalNode::evaluate()
{
  KJSO e1 = expr1->evaluate();
  KJSO v1 = e1.getValue();
  Boolean b1 = v1.toBoolean();
  if ((!b1.value() && oper == OpAnd) || (b1.value() && oper == OpOr))
    return v1;

  KJSO e2 = expr2->evaluate();
  KJSO v2 = e2.getValue();

  return v2;
}

// ECMA 11.12
KJSO ConditionalNode::evaluate()
{
  KJSO e = logical->evaluate();
  KJSO v = e.getValue();
  Boolean b = e.toBoolean();

  if (b.value())
    e = expr1->evaluate();
  else
    e = expr2->evaluate();

  return e.getValue();
}

// ECMA 11.13
KJSO AssignNode::evaluate()
{
  KJSO l, e, v;
  ErrorType err;
  if (oper == OpEqual) {
    l = left->evaluate();
    e = expr->evaluate();
    v = e.getValue();
  } else {
    l = left->evaluate();
    KJSO v1 = l.getValue();
    e = expr->evaluate();
    KJSO v2 = e.getValue();
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
      /* TODO */
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

// ECMA 11.3
KJSO PostfixNode::evaluate()
{
  KJSO e = expr->evaluate();
  KJSO v = e.getValue();
  Number n = v.toNumber();

  double newValue = (oper == OpPlusPlus) ? n.value() + 1 : n.value() - 1;
  KJSO n2 = Number(newValue);

  e.putValue(n2);

  return n;
}

// ECMA 11.4.1
KJSO DeleteNode::evaluate()
{
  KJSO e = expr->evaluate();
  KJSO b = e.getBase();
  UString n = e.getPropertyName(); /* TODO: runtime err if no ref */
  if (!b.isA(ObjectType))
    return Boolean(true);
  /* TODO [delete] */
  return Boolean(!b.hasProperty(n));
}

// ECMA 11.4.2
KJSO VoidNode::evaluate()
{
  KJSO dummy1 = expr->evaluate();
  KJSO dummy2 = dummy1.getValue();

  return Undefined();
}

// ECMA 11.4.3
KJSO TypeOfNode::evaluate()
{
  const char *s = 0L;
  KJSO e = expr->evaluate();
  if (e.isA(ReferenceType)) {
    KJSO b = e.getBase();
    if (b.isA(NullType))
      return Undefined();
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

// ECMA 11.4.4 and 11.4.5
KJSO PrefixNode::evaluate()
{
  KJSO e = expr->evaluate();
  KJSO v = e.getValue();
  Number n = v.toNumber();

  double newValue = (oper == OpPlusPlus) ? n.value() + 1 : n.value() - 1;
  KJSO n2 = Number(newValue);

  e.putValue(n2);

  return n2;
}

// ECMA 11.4.6
KJSO UnaryPlusNode::evaluate()
{
  KJSO e = expr->evaluate();
  KJSO v = e.getValue();

  return v.toNumber();
}

// ECMA 11.4.7
KJSO NegateNode::evaluate()
{
  KJSO e = expr->evaluate();
  KJSO v = e.getValue();
  Number n = v.toNumber();

  double d = -n.value();

  return Number(d);
}

// ECMA 11.4.8
KJSO BitwiseNotNode::evaluate()
{
  KJSO e = expr->evaluate();
  KJSO v = e.getValue();
  int i32 = v.toInt32();

  return Number(~i32);
}

// ECMA 11.4.9
KJSO LogicalNotNode::evaluate()
{
  KJSO e = expr->evaluate();
  KJSO v = e.getValue();
  Boolean b = v.toBoolean();

  return Boolean(!b.value());
}

// ECMA 11.5
KJSO MultNode::evaluate()
{
  KJSO t1 = term1->evaluate();
  KJSO v1 = t1.getValue();

  KJSO t2 = term2->evaluate();
  KJSO v2 = t2.getValue();

  return mult(v1, v2, oper);
}

// ECMA 11.7
KJSO ShiftNode::evaluate()
{
  KJSO t1 = term1->evaluate();
  KJSO v1 = t1.getValue();
  KJSO t2 = term2->evaluate();
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

// ECMA 11.6
KJSO AddNode::evaluate()
{
  KJSO t1 = term1->evaluate();
  KJSO v1 = t1.getValue();

  KJSO t2 = term2->evaluate();
  KJSO v2 = t2.getValue();

  return add(v1, v2, oper);
}

// ECMA 11.14
KJSO CommaNode::evaluate()
{
  KJSO e = expr1->evaluate();
  KJSO dummy = e.getValue(); // ignore return value
  e = expr2->evaluate();

  return e.getValue();
}

// ECMA 12.1
Completion BlockNode::execute()
{
  if (!statlist)
    return Completion(Normal);

  return statlist->execute();
}

// ECMA 12.1
Completion StatListNode::execute()
{
  if (!list)
    return statement->execute();

  Completion l = list->execute();
  if (l.complType() != Normal) // Completion check needed ?
    return l;
  Completion e = statement->execute();
  if (e.isValueCompletion())
    return e;
  if (!l.isValueCompletion())
    return e;
  KJSO v = l.getValue();
  if (e.isA(CompletionType) && e.complType() == Break)
    return Completion(Break, v);
  if (e.isA(CompletionType) && e.complType() == Continue)
    return Completion(Continue, v);

  return Completion(Normal, v);
}

// ECMA 12.2
Completion VarStatementNode::execute()
{
  (void) list->evaluate(); // returns 0L

  return Completion(Normal);
}

// ECMA 12.2
KJSO VarDeclNode::evaluate()
{
  KJSO variable = Context::current()->variableObject();

  // TODO: coded with help of 10.1.3. Correct ?
  if (!variable.hasProperty(ident)) {
    KJSO val, tmp;
    if (init) {
      tmp = init->evaluate();
      val = tmp.getValue();
    } else
      val = Undefined();
    variable.put(ident, val);
  }

  return Undefined();
}

// ECMA 12.2
KJSO VarDeclListNode::evaluate()
{
  if (list)
    list->evaluate();

  var->evaluate();

  return Undefined();
}

// ECMA 12.2
KJSO AssignExprNode::evaluate()
{
  return expr->evaluate();
}

// ECMA 12.3
Completion EmptyStatementNode::execute()
{
  return Completion(Normal);
}

// ECMA 12.6.2
Completion ForNode::execute()
{
  KJSO e, v, cval;
  Boolean b;
  if (expr1) {
    e = expr1->evaluate();
    v = e.getValue();
  }
  while (1) {
    if (expr2) {
      e = expr2->evaluate();
      v = e.getValue();
      b = v.toBoolean();
      if (b.value() == false)
	return Completion(Normal);
    }
    Completion c = stat->execute();
    if (c.isValueCompletion())
      cval = c.value();
    if (c.complType() == Break)
      return Completion(Normal, cval);
    if (c.complType() == ReturnValue)
      return c;
    if (expr3) {
      e = expr3->evaluate();
      v = e.getValue();
    }
  }
}

// ECMA 12.6.3
Completion ForInNode::execute()
{
  /* TODO */
  return Completion(Normal);
}

// ECMA 12.4
Completion ExprStatementNode::execute()
{
  KJSO e = expr->evaluate();
  KJSO v = e.getValue();

  return Completion(Normal, v);
}

// ECMA 12.5
Completion IfNode::execute()
{
  KJSO e = expr->evaluate();
  KJSO v = e.getValue();
  Boolean b = v.toBoolean();

  // if ... then
  if (b.value())
    return statement1->execute();

  // no else
  if (!statement2)
    return Completion(Normal);

  // else
  return statement2->execute();
}

// ECMA 12.6.1
Completion DoWhileNode::execute()
{
  KJSO be, bv;
  Completion c;
  Boolean b(false);
  KJSO value;

  do {
    c = statement->execute();
    /* TODO */
    be = expr->evaluate();
    bv = be.getValue();
    b = bv.toBoolean();
  } while (b.value());

  return Completion(Normal, value);
}

// ECMA 12.6.2
Completion WhileNode::execute()
{
  KJSO be, bv;
  Completion c;
  Boolean b(false);
  KJSO value;

  while (1) {
    be = expr->evaluate();
    bv = be.getValue();
    b = bv.toBoolean();

    if (!b.value())
      break;

    c = statement->execute();
    if (c.isValueCompletion())
      value = c.value();
    if (c.complType() == Break)
      break;
    if (c.complType() == Continue)
      continue;
    if (c.complType() == ReturnValue)
      return c;
  }

  return Completion(Normal, value);
}

// ECMA 12.7
Completion ContinueNode::execute()
{
  return Completion(Continue);
}

// ECMA 12.8
Completion BreakNode::execute()
{
  return Completion(Break);
}

// ECMA 12.9
Completion ReturnNode::execute()
{
  if (!value)
    return Completion(ReturnValue, Undefined());

  KJSO e = value->evaluate();
  KJSO v = e.getValue();

  return Completion(ReturnValue, v);
}

// ECMA 12.10
Completion WithNode::execute()
{
  KJSO e = expr->evaluate();
  KJSO v = e.getValue();
  Object o = v.toObject();
  Context::current()->pushScope(&o);
  Completion res = stat->execute();
  Context::current()->popScope();

  return res;
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

// ECMA 12.11
Completion SwitchNode::execute()
{
  KJSO e = expr->evaluate();
  KJSO v = e.getValue();
  Completion res = block->evalBlock(v);

  return res;
}

// ECMA 12.11
Completion CaseBlockNode::evalBlock(const KJSO& input)
{
  KJSO v;
  Completion res;
  ClauseListNode *a = list1, *b = list2;
  CaseClauseNode *clause;

  if (a) {
    while (a) {
      clause = a->clause();
      a = a->next();
      v = clause->evaluate();
      if (equal(input, v)) {
	res = clause->evalStatements();
	if (res.complType() != Normal)
	  return res;
	if (a) {
	  res = a->clause()->evalStatements();
	  if (res.complType() != Normal)
	    return res;
	}
	break;
      }
    }
  } else {
    while (b) {
      clause = b->clause();
      v = clause->evaluate();
      if (equal(input, v)) {
	res = clause->evalStatements();
	if (res.complType() != Normal)
	  return res;
	break;
      }
      b = b->next();
    }
  }
  // default clause
  if (def) {
    res = def->evalStatements();
    if (res.complType() != Normal)
      return res;
  }

  while (b) {
    clause = b->clause();
    v = clause->evaluate();
    if (equal(input, v)) {
      res = clause->evalStatements();
      if (res.complType() != Normal)
	return res;
      break;
    }
    b = b->next();
  }

  return Completion(Normal);
}

// ECMA 12.11
KJSO CaseClauseNode::evaluate()
{
  KJSO e = expr->evaluate();
  KJSO v = e.getValue();

  return v;
}

// ECMA 12.11
Completion CaseClauseNode::evalStatements()
{
  if (list)
    return list->execute();
  else
    return Completion(Normal, Undefined());
}

// ECMA 12.12
Completion LabelNode::execute()
{
  KJSO e;

  e = stat->execute();
  /* TODO */

  return Completion(Normal);
}

// ECMA 12.13
Completion ThrowNode::execute()
{
  /* TODO */
  return Completion(Normal);
}

// ECMA 12.14
Completion TryNode::execute()
{
  /* TODO */
  return Completion(Normal);
}

// ECMA 12.14
Completion CatchNode::execute()
{
  /* TODO */
  return Completion(Normal);
}

// ECMA 12.14
Completion FinallyNode::execute()
{
  return block->execute();
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
  ParamList *plist = new ParamList(num);
  p = param;
  for(int i = 0; i < num; i++, p = p->nextParam())
    plist->insert(num - i - 1, p->ident());

  Function f(new DeclaredFunctionImp(plist, block));

  /* TODO: decide between global and activation object */
  Global::current().put(ident, f);
}

// ECMA 13
KJSO ParameterNode::evaluate()
{
  /* TODO */
  return Undefined();
}

// ECMA 14
KJSO ProgramNode::evaluate()
{
  source->processFuncDecl();

  return source->evaluate();
}

void ProgramNode::deleteStatements()
{
  source->deleteStatements();
}

// ECMA 14
KJSO SourceElementsNode::evaluate()
{
  Context *context = Context::current();

  if (context->hadError())
    return Completion(ReturnValue, context->error());

  if (!elements)
    return element->evaluate();

  KJSO res1 = elements->evaluate();
  if (context->hadError())
    return Completion(ReturnValue, context->error());

  KJSO res2 = element->evaluate();
  if (context->hadError())
    return Completion(ReturnValue, context->error());

  if (res2.isA(CompletionType))
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

void SourceElementsNode::deleteStatements()
{
  element->deleteStatements();

  if (elements)
    elements->deleteStatements();
}

// ECMA 14
KJSO SourceElementNode::evaluate()
{
  if (statement)
    return statement->execute();

  return Completion(Normal);
}

// ECMA 14
void SourceElementNode::processFuncDecl()
{
  if (function)
    function->processFuncDecl();
}

void SourceElementNode::deleteStatements()
{
  delete statement;
}

ArgumentListNode::ArgumentListNode(Node *e) : list(0L), expr(e) {}

VarDeclNode::VarDeclNode(const UString *id, AssignExprNode *in)
    : ident(*id), init(in) { }

ArgumentListNode::ArgumentListNode(ArgumentListNode *l, Node *e) :
    list(l), expr(e) {}

ArgumentsNode::ArgumentsNode(ArgumentListNode *l) : list(l) {}
