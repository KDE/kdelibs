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
#include "operations.h"

using namespace KJS;

int   Node::nodeCount = 0;
ProgramNode *Node::prog = 0L;
Node *Node::firstNode = 0L;

Node::Node()
{
  assert(KJScript::lexer());
  line = KJScript::lexer()->lineNo();
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

KJSO *NullNode::evaluate()
{
  return KJSO::newNull();
}

KJSO *BooleanNode::evaluate()
{
  return KJSO::newBoolean(value);
}

KJSO *NumberNode::evaluate()
{
  return KJSO::newNumber(value);
}

KJSO *StringNode::evaluate()
{
  return KJSO::newString(value);
}

// ECMA 11.1.1
KJSO *ThisNode::evaluate()
{
  return KJScript::context()->thisValue;
}

// ECMA 11.1.2 & 10.1.4
KJSO *ResolveNode::evaluate()
{
  assert(KJScript::context());
  const List *chain = KJScript::context()->pScopeChain();
  ListIterator scope = chain->begin();

  while (scope != chain->end()) {
    if (scope->hasProperty(ident)) {
      //      cout << "Resolve: found '" << ident.ascii() << "'" << endl;
      return KJSO::newReference(scope, ident);
    }
    scope++;
  }

  // identifier not found
  //  cout << "Resolve: didn't find '" << ident.ascii() << "'" << endl;
  return KJSO::newReference(zeroRef(KJSO::newNull()), ident); // TODO: use a global Null
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
  KJSO *ref = KJSO::newReference(o, s->stringVal());

  return ref;
}

// ECMA 11.2.1b
KJSO *AccessorNode2::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();
  Ptr o = toObject(v);
  KJSO *ref = KJSO::newReference(o, ident);

  return ref;
}

// ECMA 11.2.2
KJSO *NewExprNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();

  Ptr argList;
  if (args)
    argList = args->evaluate();

  if (!v->isObject()) {
    return KJSO::newError(ErrExprNoObject, this);
  }
  if (!v->implementsConstruct()) {
    return KJSO::newError(ErrNoConstruct, this);
  }
  Constructor *constr = dynamic_cast<Constructor*>((KJSO*)v);
  assert(constr);

  List *tmp2;
  List nullList;
  if (args)
    tmp2 = static_cast<List*>((KJSO*)argList);
  else
    tmp2 = &nullList;

  KJSO *res = constr->construct(*tmp2);

  if (!res->isObject())
    return KJSO::newError(ErrResNoObject, this);

  return res;
}

// ECMA 11.2.3
KJSO *FunctionCallNode::evaluate()
{
  Ptr e = expr->evaluate();

  Ptr argList = args->evaluate();

  Ptr v = e->getValue();

  if (!v->isObject()) {
    return KJSO::newError(ErrFuncNoObject, this);
  }

  if (!v->implementsCall()) {
    return KJSO::newError(ErrFuncNoCall, this);
  }

  Ptr o;
  if (e->isA(ReferenceType))
    o = e->getBase();
  else
    o = KJSO::newNull();

  if (o->isA(ActivationType))
    o = KJSO::newNull();

  KJSO *tmp = argList;
  List *list = static_cast<List*>(tmp);
  KJSO *result = v->executeCall(o, list);

  return result;
}

// ECMA 11.2.4
KJSO *ArgumentsNode::evaluate()
{
  if (!list)
    return new List();

  return list->evaluate();
}

// ECMA 11.2.4
KJSO *ArgumentListNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();

  if (!list) {
    List *l = new List();
    l->append(v);
    return l;
  }

  List *l = static_cast<List*>(list->evaluate());
  l->append(v);

  return l;
}

// ECMA 11.8
KJSO *RelationalNode::evaluate()
{
  Ptr e1 = expr1->evaluate();
  Ptr v1 = e1->getValue();
  Ptr e2 = expr2->evaluate();
  Ptr v2 = e2->getValue();
  /* TODO: abstract relational comparison */
  return KJSO::newBoolean(true);
}

// ECMA 11.9
KJSO *EqualNode::evaluate()
{
  Ptr e1 = expr1->evaluate();
  Ptr e2 = expr2->evaluate();
  Ptr v1 = e1->getValue();
  Ptr v2 = e2->getValue();

  bool eq = equal(v1, v2);

  return KJSO::newBoolean(oper == OpEqEq ? eq : !eq);
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

  return KJSO::newNumber(result);
}

// ECMA 11.11
KJSO *BinaryLogicalNode::evaluate()
{
  Ptr e1 = expr1->evaluate();
  Ptr v1 = e1->getValue();
  Ptr b1 = toBoolean(v1);
  if ((!b1->boolVal() && oper == OpAnd) || (b1->boolVal() && oper == OpOr))
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

  if (b->boolVal())
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
	return KJSO::newError(err, this);
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

  double newValue = (oper == OpPlusPlus) ? n->doubleVal() + 1 : n->doubleVal() - 1;
  Ptr n2 = KJSO::newNumber(newValue);

  e->putValue(n2);

  return n->ref();
}

// ECMA 11.4.1
KJSO *DeleteNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr b = e->getBase();
  UString n = e->getPropertyName(); /* TODO: runtime err if no ref */
  if (!b->isA(ObjectType))
    return KJSO::newBoolean(true);
  /* TODO [delete] */
  return KJSO::newBoolean(!b->hasProperty(n));
}

// ECMA 11.4.2
KJSO *VoidNode::evaluate()
{
  Ptr dummy1 = expr->evaluate();
  Ptr dummy2 = dummy1->getValue();

  return KJSO::newUndefined();
}

// ECMA 11.4.3
KJSO *TypeOfNode::evaluate()
{
  const char *s = 0L;
  Ptr e = expr->evaluate();
  if (e->isA(ReferenceType)) {
    Ptr b = e->getBase();
    if (b->isA(NullType))
      return KJSO::newUndefined();
  }
  Ptr v = e->getValue();
  switch (v->type())
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
      if (v->implementsCall())
	s = "function";
      else
	s = "object";
      break;
    }

  return KJSO::newString(s);
}

// ECMA 11.4.4 and 11.4.5
KJSO *PrefixNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();
  Ptr n = toNumber(v);

  double newValue = (oper == OpPlusPlus) ? n->doubleVal() + 1 : n->doubleVal() - 1;
  Ptr n2 = KJSO::newNumber(newValue);

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

  double d = -n->doubleVal();

  return KJSO::newNumber(d);
}

// ECMA 11.4.8
KJSO *BitwiseNotNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();
  int i32 = toInt32(v);

  return KJSO::newNumber(~i32);
}

// ECMA 11.4.9
KJSO *LogicalNotNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();
  Ptr b = toBoolean(v);

  return KJSO::newBoolean(!b->boolVal());
}

// ECMA 11.5
KJSO *MultNode::evaluate()
{
  Ptr t1 = term1->evaluate();
  Ptr v1 = t1->getValue();

  Ptr t2 = term2->evaluate();
  Ptr v2 = t2->getValue();

  Ptr n1 = toNumber(v1);
  Ptr n2 = toNumber(v2);

  double result;

  if (oper == '*')
    result = n1->doubleVal() * n2->doubleVal();
  else if (oper == '/')
    result = n1->doubleVal() / n2->doubleVal();
  else
    result = fmod(n1->doubleVal(), n2->doubleVal());

  return KJSO::newNumber(result);
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
    result = toUInt32(v1) >> i2;
    break;
  default:
    assert(!"ShiftNode: unhandled switch case");
    result = 0;
  }

  return KJSO::newNumber(static_cast<double>(result));
}

// ECMA 11.6
KJSO *AddNode::evaluate()
{
  Ptr t1 = term1->evaluate();
  Ptr v1 = t1->getValue();

  Ptr t2 = term2->evaluate();
  Ptr v2 = t2->getValue();

  Ptr p1 = toPrimitive(v1);
  Ptr p2 = toPrimitive(v2);

  if ((p1->isA(StringType) || p2->isA(StringType)) && oper == '+') {
    Ptr s1 = toString(p1);
    Ptr s2 = toString(p2);

    UString s = s1->stringVal() + s2->stringVal();

    KJSO *res = KJSO::newString(s);

    return res;
  }

  Ptr n1 = toNumber(p1);
  Ptr n2 = toNumber(p2);

  KJSO *result;
  if (oper == '+')
    result = KJSO::newNumber(n1->doubleVal() + n2->doubleVal());
  else
    result = KJSO::newNumber(n1->doubleVal() - n2->doubleVal());

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
    return KJSO::newCompletion(Normal);

  return statlist->evaluate();
}

// ECMA 12.1
KJSO *StatListNode::evaluate()
{
  if (!list)
    return statement->evaluate();

  Ptr l = list->evaluate();

  if (l->isA(CompletionType) && l->complType() != Normal) // Completion check needed ?
    return l->ref();
  Ptr e = statement->evaluate();
  if (e->isValueCompletion())
    return e->ref();
  if (!l->isValueCompletion())
    return e->ref();
  Ptr v = l->getValue();
  if (e->isA(CompletionType) && e->complType() == Break)
    return KJSO::newCompletion(Break, v);
  if (e->isA(CompletionType) && e->complType() == Continue)
    return KJSO::newCompletion(Continue, v);

  return KJSO::newCompletion(Normal, v);
}

// ECMA 12.2
KJSO *VarStatementNode::evaluate()
{
  (void) list->evaluate(); // returns 0L

  return KJSO::newCompletion(Normal);
}

// ECMA 12.2
KJSO *VarDeclNode::evaluate()
{
  KJSO *variable = KJScript::context()->variableObject();

  // TODO: coded with help of 10.1.3. Correct ?
  if (!variable->hasProperty(ident)) {
    Ptr val, tmp;
    if (init) {
      tmp = init->evaluate();
      val = tmp->getValue();
    } else
      val = KJSO::newUndefined();
    variable->put(ident, val);
  }
  // TODO: I added this return to make it compile (Stephan)
  return 0L;
}

// ECMA 12.2
KJSO *VarDeclListNode::evaluate()
{
  if (list)
    list->evaluate();

  var->evaluate();

  return 0L;
}

// ECMA 12.2
KJSO *AssignExprNode::evaluate()
{
  return expr->evaluate();
}

// ECMA 12.3
KJSO *EmptyStatementNode::evaluate()
{
  return KJSO::newCompletion(Normal);
}

// ECMA 12.6.2
KJSO *ForNode::evaluate()
{
  Ptr e, v, b, cval;
  if (expr1) {
    e = expr1->evaluate();
    if (e)
      v = e->getValue();
  }
  while (1) {
    if (expr2) {
      e = expr2->evaluate();
      v = e->getValue();
      b = toBoolean(v);
      if (b->boolVal() == false)
	return KJSO::newCompletion(Normal);
    }
    e = stat->evaluate();
    if (e->isValueCompletion())
      cval = e->complValue();
    if (e->complType() == Break)
      return KJSO::newCompletion(Normal, cval);
    if (e->complType() == ReturnValue)
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
  return KJSO::newCompletion(Normal);
}

// ECMA 12.4
KJSO *ExprStatementNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();

  return KJSO::newCompletion(Normal, v);
}

// ECMA 12.5
KJSO *IfNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();
  Ptr b = toBoolean(v);

  // if ... then
  if (b->boolVal())
    return statement1->evaluate();

  // no else
  if (!statement2)
    return KJSO::newCompletion(Normal);

  // else
  return statement2->evaluate();
}

// ECMA 12.6.1
KJSO *DoWhileNode::evaluate()
{
  Ptr b, be, bv, s;
  KJSO *value = 0L;

  do {
    s = statement->evaluate();
    assert(s->isA(CompletionType));
    /* TODO */
    be = expr->evaluate();
    bv = be->getValue();
    b = toBoolean(bv);
  } while (b->boolVal());

  return KJSO::newCompletion(Normal, value);
}

// ECMA 12.6.2
KJSO *WhileNode::evaluate()
{
  Ptr b, be, bv, e;
  KJSO *value = 0L;

  while (1) {
    be = expr->evaluate();
    bv = be->getValue();
    b = toBoolean(bv);

    if (!b->boolVal())
      break;

    e = statement->evaluate();
    if (e->isA(CompletionType)) { // really needed ? are all stat's completions ?
      if (e->isValueCompletion())
	value = e;
      if (e->complType() == Break)
	break;
      if (e->complType() == Continue)
	continue;
      if (e->complType() == ReturnValue)
	return e->ref();
    }
  }

  return KJSO::newCompletion(Normal, value);
}

// ECMA 12.7
KJSO *ContinueNode::evaluate()
{
  return KJSO::newCompletion(Continue);
}

// ECMA 12.8
KJSO *BreakNode::evaluate()
{
  return KJSO::newCompletion(Break);
}

// ECMA 12.9
KJSO *ReturnNode::evaluate()
{
  if (!value)
    return KJSO::newCompletion(ReturnValue, zeroRef(KJSO::newUndefined()));

  Ptr e = value->evaluate();
  Ptr v = e->getValue();

  return KJSO::newCompletion(ReturnValue, v);
}

// ECMA 12.10
KJSO *WithNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();
  Ptr o = toObject(v);
  KJScript::context()->pushScope(o);
  Ptr res = stat->evaluate();
  KJScript::context()->popScope();

  return res->ref();
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
KJSO *SwitchNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();
  Ptr res = block->evalBlock(v);

  return res->ref();
}

// ECMA 12.11
KJSO *CaseBlockNode::evalBlock(KJSO *input)
{
  Ptr v, res;
  ClauseListNode *a = list1, *b = list2;
  CaseClauseNode *clause;

  if (a) {
    while (a) {
      clause = a->clause();
      a = a->next();
      v = clause->evaluate();
      if (equal(input, v)) {
	res = clause->evalStatements();
	if (res->complType() != Normal)
	  return res->ref();
	if (a) {
	  res = a->clause()->evalStatements();
	  if (res->complType() != Normal)
	    return res->ref();
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
	if (res->complType() != Normal)
	  return res->ref();
	break;
      }
      b = b->next();
    }
  }
  // default clause
  if (def) {
    res = def->evalStatements();
    if (res->complType() != Normal)
      return res->ref();
  }

  while (b) {
    clause = b->clause();
    v = clause->evaluate();
    if (equal(input, v)) {
      res = clause->evalStatements();
      if (res->complType() != Normal)
	return res->ref();
      break;
    }
    b = b->next();
  }

  return KJSO::newCompletion(Normal);
}

// ECMA 12.11
KJSO *CaseClauseNode::evaluate()
{
  Ptr e = expr->evaluate();
  Ptr v = e->getValue();

  return v->ref();
}

// ECMA 12.11
KJSO *CaseClauseNode::evalStatements()
{
  KJSO *res;

  if (list)
    res = list->evaluate();
  else
    res = KJSO::newUndefined();

  return res;
}

// ECMA 12.12
KJSO *LabelNode::evaluate()
{
  Ptr e;

  e = stat->evaluate();
  /* TODO */

  return KJSO::newCompletion(Normal);
}

// ECMA 12.13
KJSO *ThrowNode::evaluate()
{
  /* TODO */
  return 0L;
}

// ECMA 12.14
KJSO *TryNode::evaluate()
{
  /* TODO */
  return 0L;
}

// ECMA 12.14
KJSO *CatchNode::evaluate()
{
  /* TODO */
  return 0L;
}

// ECMA 12.14
KJSO *FinallyNode::evaluate()
{
  return block->evaluate();
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

  Ptr f = new DeclaredFunction(plist, block);

  /* TODO: decide between global and activation object */
  KJScript::global()->put(ident, f);
}

// ECMA 13
KJSO *ParameterNode::evaluate()
{
  /* TODO */
  return 0L;
}

// ECMA 14
KJSO *ProgramNode::evaluate()
{
  source->processFuncDecl();

  return source->evaluate();
}

void ProgramNode::deleteStatements()
{
  source->deleteStatements();
}

// ECMA 14
KJSO *SourceElementsNode::evaluate()
{
  if (KJScript::error())
    return KJSO::newCompletion(ReturnValue, KJScript::error());

  if (!elements)
    return element->evaluate();

  Ptr res1 = elements->evaluate();
  if (KJScript::error())
    return KJSO::newCompletion(ReturnValue, KJScript::error());

  Ptr res2 = element->evaluate();
  if (KJScript::error())
    return KJSO::newCompletion(ReturnValue, KJScript::error());

  if (res2->isA(CompletionType))
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

void SourceElementsNode::deleteStatements()
{
  element->deleteStatements();

  if (elements)
    elements->deleteStatements();
}

// ECMA 14
KJSO *SourceElementNode::evaluate()
{
  if (statement)
    return statement->evaluate();

  return KJSO::newCompletion(Normal);
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
