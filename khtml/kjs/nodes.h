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

#ifndef _NODES_H_
#define _NODES_H_

#include <stdlib.h> // for free()
#include <strings.h>
#include <iostream.h>

#include "global.h"
#include "kjsstring.h"

namespace KJS {

class KJSO;

class Node {
public:
  Node();
  virtual ~Node();
  virtual KJSO *evaluate() = 0;

private:
  // disallow assignment and copy-construction
  Node(const Node &);
  Node& operator=(const Node&);
  int line;
};

class StatementNode : public Node {
public:
  KJSO *evaluate() = 0;
};

class NullNode : public Node {
public:
  KJSO *evaluate();
};

class BooleanNode : public Node {
public:
  BooleanNode(bool v) : value(v) {}
  KJSO *evaluate();
private:
  bool value;
};

class NumberNode : public Node {
public:
  NumberNode(int v) : value((double)v) { }
  NumberNode(double v) : value(v) { }
  ~NumberNode() { }
  KJSO *evaluate();
private:
  double value;
};

class StringNode : public Node {
public:
  StringNode(const UString &v) { value = v; }
  KJSO *evaluate();
private:
  UString value;
};

class ThisNode : public Node {
public:
  KJSO *evaluate();
};

class ResolveNode : public Node {
public:
  ResolveNode(const CString &s) : ident(s) { }
  ~ResolveNode() { }
  KJSO *evaluate();
private:
  CString ident;
};

class GroupNode : public Node {
public:
  GroupNode(Node *g) : group(g) { }
  ~GroupNode() { delete group; }
  KJSO *evaluate();
private:
  Node *group;
};

class AccessorNode1 : public Node {
public:
  AccessorNode1(Node *e1, Node *e2) : expr1(e1), expr2(e2) {}
  ~AccessorNode1() { delete expr1; delete expr2; }
  KJSO *evaluate();
private:
  Node *expr1;
  Node *expr2;
};

class AccessorNode2 : public Node {
public:
  AccessorNode2(Node *e, const CString &s) : expr(e), ident(s) { }
  ~AccessorNode2() { delete expr; }
  KJSO *evaluate();
private:
  Node *expr;
  CString ident;
};

class ArgumentListNode : public Node {
public:
  ArgumentListNode(Node *e) : expr(e), list(0L) {}
  ArgumentListNode(ArgumentListNode *l, Node *e) :  list(l), expr(e) {}
  ~ArgumentListNode() { delete expr; delete list; }
  KJSO *evaluate();
private:
  Node *expr;
  ArgumentListNode *list;
};

class ArgumentsNode : public Node {
public:
  ArgumentsNode(ArgumentListNode *l) : list(l) {}
  ~ArgumentsNode() { delete list; }
  KJSO *evaluate();
private:
  ArgumentListNode *list;
};

class NewExprNode : public Node {
public:
  NewExprNode(Node *e) : expr(e), args(0L) {}
  NewExprNode(Node *e, ArgumentsNode *a) : expr(e), args(a) {}
  ~NewExprNode() { delete expr; delete args; }
  KJSO *evaluate();
private:
  Node *expr;
  ArgumentsNode *args;
};

class FunctionCallNode : public Node {
public:
  FunctionCallNode(Node *e, ArgumentsNode *a) : expr(e), args(a) {}
  ~FunctionCallNode() { delete expr; delete args; }
  KJSO *evaluate();
private:
  Node *expr;
  ArgumentsNode *args;
};

class PostfixNode : public Node {
public:
  PostfixNode(Node *e, Operator o) : expr(e), oper(o) {}
  ~PostfixNode() { delete expr; }
  KJSO *evaluate();
private:
  Node *expr;
  Operator oper;
};

class DeleteNode : public Node {
public:
  DeleteNode(Node *e) : expr(e) {}
  ~DeleteNode() { delete expr; }
  KJSO *evaluate();
private:
  Node *expr;
};

class VoidNode : public Node {
public:
  VoidNode(Node *e) : expr(e) {}
  ~VoidNode() { delete expr; }
  KJSO *evaluate();
private:
  Node *expr;
};

class TypeOfNode : public Node {
public:
  TypeOfNode(Node *e) : expr(e) {}
  ~TypeOfNode() { delete expr; }
  KJSO *evaluate();
private:
  Node *expr;
};

class PrefixNode : public Node {
public:
  PrefixNode(Operator o, Node *e) : oper(o), expr(e) {}
  ~PrefixNode() { delete expr; }
  KJSO *evaluate();
private:
  Operator oper;
  Node *expr;
};

class UnaryPlusNode : public Node {
public:
  UnaryPlusNode(Node *e) : expr(e) {}
  ~UnaryPlusNode() { delete expr; }
  KJSO *evaluate();
private:
  Node *expr;
};

class NegateNode : public Node {
public:
  NegateNode(Node *e) : expr(e) {}
  ~NegateNode() { delete expr; }
  KJSO *evaluate();
private:
  Node *expr;
};

class BitwiseNotNode : public Node {
public:
  BitwiseNotNode(Node *e) : expr(e) {}
  ~BitwiseNotNode() { delete expr; }
  KJSO *evaluate();
private:
  Node *expr;
};

class LogicalNotNode : public Node {
public:
  LogicalNotNode(Node *e) : expr(e) {}
  ~LogicalNotNode() { delete expr; }
  KJSO *evaluate();
private:
  Node *expr;
};

class MultNode : public Node {
public:
  MultNode(Node *t1, Node *t2, int op) : term1(t1), term2(t2), oper(op) {}
  ~MultNode() { delete term1; delete term2; }
  KJSO *evaluate();
private:
  Node *term1, *term2;
  int oper;
};

class AddNode : public Node {
public:
  AddNode(Node *t1, Node *t2, int op) : term1(t1), term2(t2), oper(op) {}
  ~AddNode() { delete term1; delete term2; }
  KJSO *evaluate();
private:
  Node *term1, *term2;
  int oper;
};

class ShiftNode : public Node {
public:
  ShiftNode(Node *t1, Operator o, Node *t2) : term1(t1), term2(t2), oper(o) {}
  ~ShiftNode() { delete term1; delete term2; }
  KJSO *evaluate();
private:
  Node *term1, *term2;
  Operator oper;
};

class RelationalNode : public Node {
public:
  RelationalNode(Node *e1, Operator o, Node *e2) :
    expr1(e1), oper(o), expr2(e2) {}
  ~RelationalNode() { delete expr1; delete expr2; }
  KJSO *evaluate();
private:
  Node *expr1, *expr2;
  Operator oper;
};

class EqualNode : public Node {
public:
  EqualNode(Node *e1, Operator o, Node *e2) : expr1(e1), oper(o), expr2(e2) {}
  ~EqualNode() { delete expr1; delete expr2; }
  KJSO *evaluate();
private:
  Node *expr1, *expr2;
  Operator oper;
};

class BitOperNode : public Node {
public:
  BitOperNode(Node *e1, Operator o, Node *e2) :
    expr1(e1), oper(o), expr2(e2) {}
  ~BitOperNode() { delete expr1; delete expr2; }
  KJSO *evaluate();
private:
  Node *expr1, *expr2;
  Operator oper;
};

class BinaryLogicalNode : public Node {
public:
  BinaryLogicalNode(Node *e1, Operator o, Node *e2) :
    expr1(e1), oper(o), expr2(e2) {}
  ~BinaryLogicalNode() { delete expr1; delete expr2; }
  KJSO *evaluate();
private:
  Node *expr1, *expr2;
  Operator oper;
};

class ConditionalNode : public Node {
public:
  ConditionalNode(Node *l, Node *e1, Node *e2) :
    logical(l), expr1(e1), expr2(e2) {}
  ~ConditionalNode() { delete logical; delete expr1; delete expr2; }
  KJSO *evaluate();
private:
  Node *logical, *expr1, *expr2;
};

class AssignNode : public Node {
public:
  AssignNode(Node *l, Operator o, Node *e) : left(l), oper(o), expr(e) {}
  ~AssignNode() { delete left; delete expr; }
  KJSO *evaluate();
private:
  Node *left;
  Operator oper;
  Node *expr;
};

class CommaNode : public Node {
public:
  CommaNode(Node *e1, Node *e2) : expr1(e1), expr2(e2) {}
  ~CommaNode() { delete expr1; delete expr2; }
  KJSO *evaluate();
private:
  Node *expr1, *expr2;
};

class StatListNode : public Node {
public:
  StatListNode(StatementNode *s) : statement(s), list(0L) { }
  StatListNode(StatListNode *l, StatementNode *s) : list(l), statement(s) { }
  ~StatListNode() { delete statement; delete list; }
  KJSO *evaluate();
private:
  StatementNode *statement;
  StatListNode *list;
};

class AssignExprNode : public Node {
public:
  AssignExprNode(Node *e) : expr(e) {}
  ~AssignExprNode() { delete expr; }
  KJSO *evaluate();
private:
  Node *expr;
};

class VarDeclNode : public Node {
public:
  VarDeclNode(const CString &id) : ident(id), init(0L) { }
  VarDeclNode(const CString &id, AssignExprNode *in) : ident(id), init(in) { }
  ~VarDeclNode() { delete init; }
  KJSO *evaluate();
private:
  CString ident;
  AssignExprNode *init;
};

class VarDeclListNode : public Node {
public:
  VarDeclListNode(VarDeclNode *v) : var(v), list(0L) {}
  VarDeclListNode(Node *l, VarDeclNode *v) : list(l), var(v) {}
  ~VarDeclListNode() { delete list; delete var; }
  KJSO *evaluate();
private:
  Node *list;
  VarDeclNode *var;
};

class VarStatementNode : public StatementNode {
public:
  VarStatementNode(VarDeclListNode *l) : list(l) {}
  ~VarStatementNode() { delete list; }
  KJSO *evaluate();
private:
  VarDeclListNode *list;
};

class BlockNode : public StatementNode {
public:
  BlockNode(StatListNode *s) : statlist(s) {}
  ~BlockNode() { delete statlist; }
  KJSO *evaluate();
private:
  StatListNode *statlist;
};

class EmptyStatementNode : public StatementNode {
public:
  EmptyStatementNode() { } // debug
  ~EmptyStatementNode() { } // debug
public:
  KJSO *evaluate();
};

class ExprStatementNode : public StatementNode {
public:
  ExprStatementNode(Node *e) : expr(e) { }
  ~ExprStatementNode() { delete expr; }
  KJSO *evaluate();
private:
  Node *expr;
};

class IfNode : public StatementNode {
public:
  IfNode(Node *e, StatementNode *s1, StatementNode *s2)
    : expr(e), statement1(s1), statement2(s2) {}
  ~IfNode() { delete expr; delete statement1; delete statement2; }
  KJSO *evaluate();
private:
  Node *expr;
  StatementNode *statement1, *statement2;
};

class WhileNode : public StatementNode {
public:
  WhileNode(Node *e, StatementNode *s) : expr(e), statement(s) {}
  ~WhileNode() { delete expr; delete statement; }
  KJSO *evaluate();
private:
  Node *expr;
  StatementNode *statement;
};

class ForNode : public StatementNode {
public:
  ForNode(Node *e1, Node *e2, Node *e3, StatementNode *s) :
    expr1(e1), expr2(e2), expr3(e3), stat(s) {}
  ForNode(VarDeclListNode *, Node *, Node *, StatementNode *) {}
  ForNode(Node *, Node *, StatementNode *) {}
  ForNode(const CString &, AssignExprNode*, Node *, StatementNode *) {}
  ~ForNode() { delete expr1; delete expr2; delete expr3; delete stat; }
  KJSO *evaluate();
private:
  Node *expr1, *expr2, *expr3;
  StatementNode *stat;
};

class ContinueNode : public StatementNode {
public:
  KJSO *evaluate();
};

class BreakNode : public StatementNode {
public:
  KJSO *evaluate();
};


class ReturnNode : public StatementNode {
public:
  ReturnNode(Node *v) : value(v) {}
  ~ReturnNode() { delete value; }
  KJSO *evaluate();
private:
  Node *value;
};

class WithNode : public StatementNode {
public:
  WithNode(const Node *e, const StatementNode *s) : expr(e), stat(s) {}
  ~WithNode() { delete expr; delete stat; }
  KJSO *evaluate() { /* TODO */ }
private:
  const Node *expr;
  const StatementNode *stat;
};

class ParameterNode : public Node {
public:
  ParameterNode(const CString &i) : ident(i) { }
  ParameterNode *append(const CString &i) { /* TODO */ return this; };
  ~ParameterNode() { }
  KJSO *evaluate() { /* TODO */ }
private:
  CString ident;
};

class FuncDeclNode : public StatementNode {
public:
  FuncDeclNode(const CString &i, ParameterNode *p, StatementNode *b)
    : ident(i), param(p), block(b) { }
  ~FuncDeclNode() { delete param; delete block; }
  KJSO *evaluate() { /* empty */ }
  void processFuncDecl();
private:
  CString ident;
  ParameterNode *param;
  StatementNode *block;
};

class SourceElementNode : public Node {
public:
  SourceElementNode(StatementNode *s) { statement = s; function = 0L; }
  SourceElementNode(FuncDeclNode *f) { function = f; statement = 0L;}
  ~SourceElementNode() { delete statement; delete function; }
  KJSO *evaluate();
  virtual void processFuncDecl();
private:
  StatementNode *statement;
  FuncDeclNode *function;
};

class SourceElementsNode : public Node {
public:
  SourceElementsNode(SourceElementNode *s1) { element = s1; elements = 0L; }
  SourceElementsNode(SourceElementsNode *s1, SourceElementNode *s2)
    { elements = s1; element = s2; }
  ~SourceElementsNode() { delete element; delete elements; }
  KJSO *evaluate();
  virtual void processFuncDecl();
private:
  SourceElementNode *element;
  SourceElementsNode *elements;
};

class ProgramNode : public Node {
public:
  ProgramNode(SourceElementsNode *s) : source(s) { }
  ~ProgramNode() { delete source; }
  KJSO *evaluate();
private:
  SourceElementsNode *source;
};

// for debugging
class PrintNode : public StatementNode {
public:
  PrintNode(Node *e) : expr(e) { }
  ~PrintNode() { delete expr; }
  KJSO *evaluate();
private:
  Node *expr;
};

// for debugging
class AlertNode : public StatementNode {
public:
  AlertNode(Node *e) : expr(e) { }
  ~AlertNode() { delete expr; }
  KJSO *evaluate();
private:
  Node *expr;
};

};

#endif
