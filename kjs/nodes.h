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

#ifndef _NODES_H_
#define _NODES_H_

#include "ustring.h"
#include "object.h"
#include "types.h"

namespace KJS {

  class KJSO;
  class RegExp;
  class ProgramNode;

  enum Operator { OpEqual,
		  OpEqEq,
		  OpNotEq,
		  OpStrEq,
		  OpStrNEq,
		  OpPlusEq,
		  OpMinusEq,
		  OpMultEq,
		  OpDivEq,
                  OpPlusPlus,
		  OpMinusMinus,
		  OpLess,
		  OpLessEq,
		  OpGreater,
		  OpGreaterEq,
		  OpAndEq,
		  OpXOrEq,
		  OpOrEq,
		  OpModEq,
                  OpAnd,
                  OpOr,
		  OpBitAnd,
		  OpBitXOr,
		  OpBitOr,
		  OpLShift,
		  OpRShift,
		  OpURShift,
		  OpIn,
		  OpInstanceOf
  };

  class Node {
  public:
    Node();
    virtual ~Node();
    virtual KJSO evaluate() = 0;
    int lineNo() const { return line; }
    static ProgramNode *progNode() { return prog; }
    static void deleteAllNodes();
  protected:
    KJSO throwError(ErrorType e, const char *msg);
    static ProgramNode *prog;
  private:
    // disallow assignment and copy-construction
    Node(const Node &);
    Node& operator=(const Node&);
    int line;
    static  int nodeCount;
    Node *next, *prev;
    static Node *firstNode;
  };

  class StatementNode : public Node {
  public:
    virtual Completion execute() = 0;
  private:
    KJSO evaluate() { return Undefined(); }
  };

  class NullNode : public Node {
  public:
    KJSO evaluate();
  };

  class BooleanNode : public Node {
  public:
    BooleanNode(bool v) : value(v) {}
    KJSO evaluate();
  private:
    bool value;
  };

  class NumberNode : public Node {
  public:
    NumberNode(double v) : value(v) { }
    KJSO evaluate();
  private:
    double value;
  };

  class StringNode : public Node {
  public:
    StringNode(const UString *v) { value = *v; }
    KJSO evaluate();
  private:
    UString value;
  };

  class RegExpNode : public Node {
  public:
    RegExpNode(const UString &p, const UString &f)
      : pattern(p), flags(f) { }
    KJSO evaluate();
  private:
    UString pattern, flags;
  };
  
  class ThisNode : public Node {
  public:
    KJSO evaluate();
  };

  class ResolveNode : public Node {
  public:
    ResolveNode(const UString *s) : ident(*s) { }
    KJSO evaluate();
  private:
    UString ident;
  };

  class GroupNode : public Node {
  public:
    GroupNode(Node *g) : group(g) { }
    KJSO evaluate();
  private:
    Node *group;
  };

  class ElisionNode : public Node {
  public:
    ElisionNode(ElisionNode *) { /* TODO */ }
    KJSO evaluate() { /* TODO */ return KJSO(); }
  };

  class ElementNode : public Node {
  public:
    ElementNode(ElisionNode *, Node *) { /* TODO */ }
    ElementNode *append(ElisionNode *, Node *) { /* TODO */ return 0L; }
    KJSO evaluate() { /* TODO */ return KJSO(); }
  };

  class ArrayNode : public Node {
  public:
    ArrayNode(ElisionNode *) { /* TODO */ }
    ArrayNode(ElementNode *, ElisionNode *) { /* TODO */ }
    KJSO evaluate() { /* TODO */ return KJSO(); }
  };

  class ObjectLiteralNode : public Node {
  public:
    ObjectLiteralNode(Node *l) : list(l) { }
    KJSO evaluate();
  private:
    Node *list;
  };

  class PropertyValueNode : public Node {
  public:
    PropertyValueNode(Node *n, Node *a, Node *l = 0L)
      : name(n), assign(a), list(l) { }
    KJSO evaluate();
  private:
    Node *name, *assign, *list;
  };
  
  class PropertyNode : public Node {
  public:
    PropertyNode(double d) : numeric(d) { }
    PropertyNode(const UString *s) : str(*s) { }
    KJSO evaluate();
  private:
    double numeric;
    UString str;
  };

  class AccessorNode1 : public Node {
  public:
    AccessorNode1(Node *e1, Node *e2) : expr1(e1), expr2(e2) {}
    KJSO evaluate();
  private:
    Node *expr1;
    Node *expr2;
  };

  class AccessorNode2 : public Node {
  public:
    AccessorNode2(Node *e, const UString *s) : expr(e), ident(*s) { }
    KJSO evaluate();
  private:
    Node *expr;
    UString ident;
  };

  class ArgumentListNode : public Node {
  public:
    ArgumentListNode(Node *e);
    ArgumentListNode(ArgumentListNode *l, Node *e);
    KJSO evaluate();
    List *evaluateList();
  private:
    ArgumentListNode *list;
    Node *expr;
  };

  class ArgumentsNode : public Node {
  public:
    ArgumentsNode(ArgumentListNode *l);
    KJSO evaluate();
    List *evaluateList();
  private:
    ArgumentListNode *list;
  };

  class NewExprNode : public Node {
  public:
    NewExprNode(Node *e) : expr(e), args(0L) {}
    NewExprNode(Node *e, ArgumentsNode *a) : expr(e), args(a) {}
    KJSO evaluate();
  private:
    Node *expr;
    ArgumentsNode *args;
  };

  class FunctionCallNode : public Node {
  public:
    FunctionCallNode(Node *e, ArgumentsNode *a) : expr(e), args(a) {}
    KJSO evaluate();
  private:
    Node *expr;
    ArgumentsNode *args;
  };

  class PostfixNode : public Node {
  public:
    PostfixNode(Node *e, Operator o) : expr(e), oper(o) {}
    KJSO evaluate();
  private:
    Node *expr;
    Operator oper;
  };

  class DeleteNode : public Node {
  public:
    DeleteNode(Node *e) : expr(e) {}
    KJSO evaluate();
  private:
    Node *expr;
  };

  class VoidNode : public Node {
  public:
    VoidNode(Node *e) : expr(e) {}
    KJSO evaluate();
  private:
    Node *expr;
  };

  class TypeOfNode : public Node {
  public:
    TypeOfNode(Node *e) : expr(e) {}
    KJSO evaluate();
  private:
    Node *expr;
  };

  class PrefixNode : public Node {
  public:
    PrefixNode(Operator o, Node *e) : oper(o), expr(e) {}
    KJSO evaluate();
  private:
    Operator oper;
    Node *expr;
  };

  class UnaryPlusNode : public Node {
  public:
    UnaryPlusNode(Node *e) : expr(e) {}
    KJSO evaluate();
  private:
    Node *expr;
  };

  class NegateNode : public Node {
  public:
    NegateNode(Node *e) : expr(e) {}
    KJSO evaluate();
  private:
    Node *expr;
  };

  class BitwiseNotNode : public Node {
  public:
    BitwiseNotNode(Node *e) : expr(e) {}
    KJSO evaluate();
  private:
    Node *expr;
  };

  class LogicalNotNode : public Node {
  public:
    LogicalNotNode(Node *e) : expr(e) {}
    KJSO evaluate();
  private:
    Node *expr;
  };

  class MultNode : public Node {
  public:
    MultNode(Node *t1, Node *t2, char op) : term1(t1), term2(t2), oper(op) {}
    KJSO evaluate();
  private:
    Node *term1, *term2;
    char oper;
  };

  class AddNode : public Node {
  public:
    AddNode(Node *t1, Node *t2, char op) : term1(t1), term2(t2), oper(op) {}
    KJSO evaluate();
  private:
    Node *term1, *term2;
    char oper;
  };

  class ShiftNode : public Node {
  public:
    ShiftNode(Node *t1, Operator o, Node *t2)
      : term1(t1), term2(t2), oper(o) {}
    KJSO evaluate();
  private:
    Node *term1, *term2;
    Operator oper;
  };

  class RelationalNode : public Node {
  public:
    RelationalNode(Node *e1, Operator o, Node *e2) :
      expr1(e1), expr2(e2), oper(o) {}
    KJSO evaluate();
  private:
    Node *expr1, *expr2;
    Operator oper;
  };

  class EqualNode : public Node {
  public:
    EqualNode(Node *e1, Operator o, Node *e2)
      : expr1(e1), expr2(e2), oper(o) {}
    KJSO evaluate();
  private:
    Node *expr1, *expr2;
    Operator oper;
  };

  class BitOperNode : public Node {
  public:
    BitOperNode(Node *e1, Operator o, Node *e2) :
      expr1(e1), expr2(e2), oper(o) {}
    KJSO evaluate();
  private:
    Node *expr1, *expr2;
    Operator oper;
  };

  class BinaryLogicalNode : public Node {
  public:
    BinaryLogicalNode(Node *e1, Operator o, Node *e2) :
      expr1(e1), expr2(e2), oper(o) {}
    KJSO evaluate();
  private:
    Node *expr1, *expr2;
    Operator oper;
  };

  class ConditionalNode : public Node {
  public:
    ConditionalNode(Node *l, Node *e1, Node *e2) :
      logical(l), expr1(e1), expr2(e2) {}
    KJSO evaluate();
  private:
    Node *logical, *expr1, *expr2;
  };

  class AssignNode : public Node {
  public:
    AssignNode(Node *l, Operator o, Node *e) : left(l), oper(o), expr(e) {}
    KJSO evaluate();
  private:
    Node *left;
    Operator oper;
    Node *expr;
  };

  class CommaNode : public Node {
  public:
    CommaNode(Node *e1, Node *e2) : expr1(e1), expr2(e2) {}
    KJSO evaluate();
  private:
    Node *expr1, *expr2;
  };

  class StatListNode : public Node {
  public:
    StatListNode(StatementNode *s) : statement(s), list(0L) { }
    StatListNode(StatListNode *l, StatementNode *s) : statement(s), list(l) { }
    Completion execute();
  private:
    KJSO evaluate() { return Null(); }
    StatementNode *statement;
    StatListNode *list;
  };

  class AssignExprNode : public Node {
  public:
    AssignExprNode(Node *e) : expr(e) {}
    KJSO evaluate();
  private:
    Node *expr;
  };

  class VarDeclNode : public Node {
  public:
    VarDeclNode(const UString *id, AssignExprNode *in);
    KJSO evaluate();
  private:
    UString ident;
    AssignExprNode *init;
  };

  class VarDeclListNode : public Node {
  public:
    VarDeclListNode(VarDeclNode *v) : list(0L), var(v) {}
    VarDeclListNode(Node *l, VarDeclNode *v) : list(l), var(v) {}
    KJSO evaluate();
  private:
    Node *list;
    VarDeclNode *var;
  };

  class VarStatementNode : public StatementNode {
  public:
    VarStatementNode(VarDeclListNode *l) : list(l) {}
    Completion execute();
  private:
    VarDeclListNode *list;
  };

  class BlockNode : public StatementNode {
  public:
    BlockNode(StatListNode *s) : statlist(s) {}
    Completion execute();
  private:
    StatListNode *statlist;
  };

  class EmptyStatementNode : public StatementNode {
  public:
    EmptyStatementNode() { } // debug
    Completion execute();
  };

  class ExprStatementNode : public StatementNode {
  public:
    ExprStatementNode(Node *e) : expr(e) { }
    Completion execute();
  private:
    Node *expr;
  };

  class IfNode : public StatementNode {
  public:
    IfNode(Node *e, StatementNode *s1, StatementNode *s2)
      : expr(e), statement1(s1), statement2(s2) {}
    Completion execute();
  private:
    Node *expr;
    StatementNode *statement1, *statement2;
  };

  class DoWhileNode : public StatementNode {
  public:
    DoWhileNode(StatementNode *s, Node *e) : statement(s), expr(e) {}
    Completion execute();
  private:
    StatementNode *statement;
    Node *expr;
  };

  class WhileNode : public StatementNode {
  public:
    WhileNode(Node *e, StatementNode *s) : expr(e), statement(s) {}
    Completion execute();
  private:
    Node *expr;
    StatementNode *statement;
  };

  class ForNode : public StatementNode {
  public:
    ForNode(Node *e1, Node *e2, Node *e3, StatementNode *s) :
      expr1(e1), expr2(e2), expr3(e3), stat(s) {}
    Completion execute();
  private:
    Node *expr1, *expr2, *expr3;
    StatementNode *stat;
  };

  class ForInNode : public StatementNode {
  public:
    ForInNode(Node *l, Node *e, StatementNode *s) :
      init(0L), lexpr(l), expr(e), stat(s) {}
    ForInNode(const UString *i, AssignExprNode *in, Node *e, StatementNode *s)
      : ident(*i), init(in), lexpr(0L), expr(e), stat(s) {}
    Completion execute();
  private:
    UString ident;
    AssignExprNode *init;
    Node *lexpr, *expr;
    StatementNode *stat;
  };

  class ContinueNode : public StatementNode {
  public:
    ContinueNode() { }
    ContinueNode(const UString *i) : ident(*i) { }
    Completion execute();
  private:
    UString ident;
  };

  class BreakNode : public StatementNode {
  public:
    BreakNode() { }
    BreakNode(const UString *i) : ident(*i) { }
    Completion execute();
  private:
    UString ident;
  };

  class ReturnNode : public StatementNode {
  public:
    ReturnNode(Node *v) : value(v) {}
    Completion execute();
  private:
    Node *value;
  };

  class WithNode : public StatementNode {
  public:
    WithNode(Node *e, StatementNode *s) : expr(e), stat(s) {}
    Completion execute();
  private:
    Node *expr;
    StatementNode *stat;
  };

  class CaseClauseNode: public Node {
  public:
    CaseClauseNode(Node *e, StatListNode *l) : expr(e), list(l) { }
    KJSO evaluate();
    Completion evalStatements();
  private:
    Node *expr;
    StatListNode *list;
  };

  class ClauseListNode : public Node {
  public:
    ClauseListNode(CaseClauseNode *c) : cl(c), nx(0L) { }
    ClauseListNode* append(CaseClauseNode *c);
    KJSO evaluate() { /* should never be called */ return KJSO(); }
    CaseClauseNode *clause() const { return cl; }
    ClauseListNode *next() const { return nx; }
  private:
    CaseClauseNode *cl;
    ClauseListNode *nx;
  };

  class CaseBlockNode: public Node {
  public:
    CaseBlockNode(ClauseListNode *l1, CaseClauseNode *d, ClauseListNode *l2)
      : list1(l1), def(d), list2(l2) { }
    KJSO evaluate() { /* should never be called */ return KJSO(); }
    Completion evalBlock(const KJSO& input);
  private:
    ClauseListNode *list1;
    CaseClauseNode *def;
    ClauseListNode *list2;
  };

  class SwitchNode : public StatementNode {
  public:
    SwitchNode(Node *e, CaseBlockNode *b) : expr(e), block(b) { }
    Completion execute();
  private:
    Node *expr;
    CaseBlockNode *block;
  };

  class LabelNode : public StatementNode {
  public:
    LabelNode(const UString *l, StatementNode *s) : label(*l), stat(s) { }
    Completion execute();
  private:
    UString label;
    StatementNode *stat;
  };

  class ThrowNode : public StatementNode {
  public:
    ThrowNode(Node *e) : expr(e) {}
    Completion execute();
  private:
    Node *expr;
  };

  class CatchNode : public StatementNode {
  public:
    CatchNode() {}
    Completion execute();
  };

  class FinallyNode : public StatementNode {
  public:
    FinallyNode(StatementNode *b) : block(b) {}
    Completion execute();
  private:
    StatementNode *block;
  };

  class TryNode : public StatementNode {
  public:
    TryNode() {}
    Completion execute();
  };

  class ParameterNode : public Node {
  public:
    ParameterNode(const UString *i) : id(*i), next(0L) { }
    ParameterNode(ParameterNode *l, const UString *i) : id(*i), next(l) { }
    KJSO evaluate();
    UString ident() { return id; }
    ParameterNode *nextParam() { return next; }
  private:
    UString id;
    ParameterNode *next;
  };

  class FuncDeclNode : public StatementNode {
  public:
    FuncDeclNode(const UString *i, ParameterNode *p, StatementNode *b)
      : ident(*i), param(p), block(b) { }
    Completion execute() { /* empty */ return Completion(); }
    void processFuncDecl();
  private:
    UString ident;
    ParameterNode *param;
    StatementNode *block;
  };

  class SourceElementNode : public Node {
  public:
    SourceElementNode(StatementNode *s) { statement = s; function = 0L; }
    SourceElementNode(FuncDeclNode *f) { function = f; statement = 0L;}
    KJSO evaluate();
    virtual void processFuncDecl();
    void deleteStatements();
  private:
    StatementNode *statement;
    FuncDeclNode *function;
  };

  class SourceElementsNode : public Node {
  public:
    SourceElementsNode(SourceElementNode *s1) { element = s1; elements = 0L; }
    SourceElementsNode(SourceElementsNode *s1, SourceElementNode *s2)
      { elements = s1; element = s2; }
    KJSO evaluate();
    virtual void processFuncDecl();
    void deleteStatements();
  private:
    SourceElementNode *element;
    SourceElementsNode *elements;
  };

  class ProgramNode : public Node {
  public:
    ProgramNode(SourceElementsNode *s) : source(s) { Node::prog = this; }
    KJSO evaluate();
    void deleteStatements();
  private:
    SourceElementsNode *source;
  };

}; // namespace

#endif
