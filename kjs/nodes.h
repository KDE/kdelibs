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

#include "internal.h"
#include "ustring.h"
#include "object.h"
#include "types.h"
#include "debugger.h"

namespace KJS {

  class KJSO;
  class RegExp;
  class SourceElementsNode;
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
    Node(const Node &other);
    virtual ~Node();
    virtual KJSO evaluate(KJScriptImp *script, Context *context) = 0;
    virtual void processVarDecls(KJScriptImp */*script*/, Context */*context*/) {}
    int lineNo() const { return line; }
    virtual Node *copy() const = 0;
#ifdef KJS_DEBUGGER
    static bool setBreakpoint(Node *firstNode, int id, int line, bool set);
    virtual bool setBreakpoint(int, int, bool) { return false; }
#endif
  protected:
    KJSO throwError(ErrorType e, const char *msg);
    int line;
  private:
    // disallow assignment
    Node& operator=(const Node&);
  };

  class StatementNode : public Node {
  public:
#ifdef KJS_DEBUGGER
    StatementNode() : l0(-1), l1(-1), sid(-1), breakPoint(false) { }
    void setLoc(int line0, int line1);
    int firstLine() const { return l0; }
    int lastLine() const { return l1; }
    int sourceId() const { return sid; }
    bool hitStatement();
    bool abortStatement();
    virtual bool setBreakpoint(int id, int line, bool set);
#endif
    StatementNode() {}
    StatementNode(const StatementNode &other);
    virtual Completion execute(KJScriptImp *script, Context *context) = 0;
    void pushLabel(const UString *id) {
      if (id) ls.push(*id);
    }
  protected:
    LabelStack ls;
  private:
    KJSO evaluate(KJScriptImp */*script*/, Context */*context*/) { return Undefined(); }
#ifdef KJS_DEBUGGER
    int l0, l1;
    int sid;
    bool breakPoint;
#endif
  };

  class NullNode : public Node {
  public:
    NullNode() {}
    NullNode(const NullNode &other);
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  };

  class BooleanNode : public Node {
  public:
    BooleanNode(bool v) : value(v) {}
    BooleanNode(const BooleanNode &other);
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    bool value;
  };

  class NumberNode : public Node {
  public:
    NumberNode(double v) : value(v) { }
    NumberNode(const NumberNode &other);
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    double value;
  };

  class StringNode : public Node {
  public:
    StringNode(const UString *v) { value = *v; }
    StringNode(const StringNode &other);
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    UString value;
  };

  class RegExpNode : public Node {
  public:
    RegExpNode(const UString &p, const UString &f)
      : pattern(p), flags(f) { }
    RegExpNode(const RegExpNode &other);
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    UString pattern, flags;
  };

  class ThisNode : public Node {
  public:
    ThisNode() {}
    ThisNode(const ThisNode &other);
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  };

  class ResolveNode : public Node {
  public:
    ResolveNode(const UString *s) : ident(*s) { }
    ResolveNode(const ResolveNode &other);
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    UString ident;
  };

  class GroupNode : public Node {
  public:
    GroupNode(Node *g) : group(g) { }
    GroupNode(const GroupNode &other);
    virtual Node *copy() const;
    virtual ~GroupNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *group;
  };

  class ElisionNode : public Node {
  public:
    ElisionNode(ElisionNode *e) : elision(e) { }
    ElisionNode(const ElisionNode &other);
    virtual Node *copy() const;
    virtual ~ElisionNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    ElisionNode *elision;
  };

  class ElementNode : public Node {
  public:
    ElementNode(ElisionNode *e, Node *n) : list(0l), elision(e), node(n) { }
    ElementNode(ElementNode *l, ElisionNode *e, Node *n)
      : list(l), elision(e), node(n) { }
    ElementNode(const ElementNode &other);
    virtual Node *copy() const;
    virtual ~ElementNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    ElementNode *list;
    ElisionNode *elision;
    Node *node;
  };

  class ArrayNode : public Node {
  public:
    ArrayNode(ElisionNode *e) : element(0L), elision(e), opt(true) { }
    ArrayNode(ElementNode *ele)
      : element(ele), elision(0), opt(false) { }
    ArrayNode(ElisionNode *eli, ElementNode *ele)
      : element(ele), elision(eli), opt(true) { }
    ArrayNode(const ArrayNode &other);
    virtual ~ArrayNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    ElementNode *element;
    ElisionNode *elision;
    bool opt;
  };

  class ObjectLiteralNode : public Node {
  public:
    ObjectLiteralNode(Node *l) : list(l) { }
    ObjectLiteralNode(const ObjectLiteralNode &other);
    virtual ~ObjectLiteralNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *list;
  };

  class PropertyValueNode : public Node {
  public:
    PropertyValueNode(Node *n, Node *a, Node *l = 0L)
      : name(n), assign(a), list(l) { }
    PropertyValueNode(const PropertyValueNode &other);
    virtual ~PropertyValueNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *name, *assign, *list;
  };

  class PropertyNode : public Node {
  public:
    PropertyNode(double d) : numeric(d) { }
    PropertyNode(const UString *s) : str(*s) { }
    PropertyNode(const PropertyNode &other);
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    double numeric;
    UString str;
  };

  class AccessorNode1 : public Node {
  public:
    AccessorNode1(Node *e1, Node *e2) : expr1(e1), expr2(e2) {}
    AccessorNode1(const AccessorNode1 &other);
    virtual ~AccessorNode1();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr1;
    Node *expr2;
  };

  class AccessorNode2 : public Node {
  public:
    AccessorNode2(Node *e, const UString *s) : expr(e), ident(*s) { }
    AccessorNode2(const AccessorNode2 &other);
    virtual ~AccessorNode2();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
    UString ident;
  };

  class ArgumentListNode : public Node {
  public:
    ArgumentListNode(Node *e);
    ArgumentListNode(ArgumentListNode *l, Node *e);
    ArgumentListNode(const ArgumentListNode &other);
    virtual ~ArgumentListNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
    List *evaluateList(KJScriptImp *script, Context *context);
  private:
    ArgumentListNode *list;
    Node *expr;
  };

  class ArgumentsNode : public Node {
  public:
    ArgumentsNode(ArgumentListNode *l);
    ArgumentsNode(const ArgumentsNode &other);
    virtual ~ArgumentsNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
    List *evaluateList(KJScriptImp *script, Context *context);
  private:
    ArgumentListNode *list;
  };

  class NewExprNode : public Node {
  public:
    NewExprNode(Node *e) : expr(e), args(0L) {}
    NewExprNode(Node *e, ArgumentsNode *a) : expr(e), args(a) {}
    NewExprNode(const NewExprNode &other);
    virtual Node *copy() const;
    virtual ~NewExprNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
    ArgumentsNode *args;
  };

  class FunctionCallNode : public Node {
  public:
    FunctionCallNode(Node *e, ArgumentsNode *a) : expr(e), args(a) {}
    FunctionCallNode(const FunctionCallNode &other);
    virtual ~FunctionCallNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
#ifdef KJS_DEBUGGER
    void steppingInto(bool in);
    Debugger::Mode previousMode;
#endif
  private:
    Node *expr;
    ArgumentsNode *args;
  };

  class PostfixNode : public Node {
  public:
    PostfixNode(Node *e, Operator o) : expr(e), oper(o) {}
    PostfixNode(const PostfixNode &other);
    virtual ~PostfixNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
    Operator oper;
  };

  class DeleteNode : public Node {
  public:
    DeleteNode(Node *e) : expr(e) {}
    DeleteNode(const DeleteNode &other);
    virtual ~DeleteNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class VoidNode : public Node {
  public:
    VoidNode(Node *e) : expr(e) {}
    VoidNode(const VoidNode &other);
    virtual ~VoidNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class TypeOfNode : public Node {
  public:
    TypeOfNode(Node *e) : expr(e) {}
    TypeOfNode(const TypeOfNode &other);
    virtual ~TypeOfNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class PrefixNode : public Node {
  public:
    PrefixNode(Operator o, Node *e) : oper(o), expr(e) {}
    PrefixNode(const PrefixNode &other);
    virtual ~PrefixNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Operator oper;
    Node *expr;
  };

  class UnaryPlusNode : public Node {
  public:
    UnaryPlusNode(Node *e) : expr(e) {}
    UnaryPlusNode(const UnaryPlusNode &other);
    virtual ~UnaryPlusNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class NegateNode : public Node {
  public:
    NegateNode(Node *e) : expr(e) {}
    NegateNode(const NegateNode &other);
    virtual ~NegateNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class BitwiseNotNode : public Node {
  public:
    BitwiseNotNode(Node *e) : expr(e) {}
    BitwiseNotNode(const BitwiseNotNode &other);
    virtual ~BitwiseNotNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class LogicalNotNode : public Node {
  public:
    LogicalNotNode(Node *e) : expr(e) {}
    LogicalNotNode(const LogicalNotNode &other);
    virtual ~LogicalNotNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class MultNode : public Node {
  public:
    MultNode(Node *t1, Node *t2, char op) : term1(t1), term2(t2), oper(op) {}
    MultNode(const MultNode &other);
    virtual ~MultNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *term1, *term2;
    char oper;
  };

  class AddNode : public Node {
  public:
    AddNode(Node *t1, Node *t2, char op) : term1(t1), term2(t2), oper(op) {}
    AddNode(const AddNode &other);
    virtual ~AddNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *term1, *term2;
    char oper;
  };

  class ShiftNode : public Node {
  public:
    ShiftNode(Node *t1, Operator o, Node *t2)
      : term1(t1), term2(t2), oper(o) {}
    ShiftNode(const ShiftNode &other);
    virtual ~ShiftNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *term1, *term2;
    Operator oper;
  };

  class RelationalNode : public Node {
  public:
    RelationalNode(Node *e1, Operator o, Node *e2) :
      expr1(e1), expr2(e2), oper(o) {}
    RelationalNode(const RelationalNode &other);
    virtual ~RelationalNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr1, *expr2;
    Operator oper;
  };

  class EqualNode : public Node {
  public:
    EqualNode(Node *e1, Operator o, Node *e2)
      : expr1(e1), expr2(e2), oper(o) {}
    EqualNode(const EqualNode &other);
    virtual ~EqualNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr1, *expr2;
    Operator oper;
  };

  class BitOperNode : public Node {
  public:
    BitOperNode(Node *e1, Operator o, Node *e2) :
      expr1(e1), expr2(e2), oper(o) {}
    BitOperNode(const BitOperNode &other);
    virtual ~BitOperNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr1, *expr2;
    Operator oper;
  };

  class BinaryLogicalNode : public Node {
  public:
    BinaryLogicalNode(Node *e1, Operator o, Node *e2) :
      expr1(e1), expr2(e2), oper(o) {}
    BinaryLogicalNode(const BinaryLogicalNode &other);
    virtual ~BinaryLogicalNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr1, *expr2;
    Operator oper;
  };

  class ConditionalNode : public Node {
  public:
    ConditionalNode(Node *l, Node *e1, Node *e2) :
      logical(l), expr1(e1), expr2(e2) {}
    ConditionalNode(const ConditionalNode &other);
    virtual ~ConditionalNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *logical, *expr1, *expr2;
  };

  class AssignNode : public Node {
  public:
    AssignNode(Node *l, Operator o, Node *e) : left(l), oper(o), expr(e) {}
    AssignNode(const AssignNode &other);
    virtual ~AssignNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *left;
    Operator oper;
    Node *expr;
  };

  class CommaNode : public Node {
  public:
    CommaNode(Node *e1, Node *e2) : expr1(e1), expr2(e2) {}
    CommaNode(const CommaNode &other);
    virtual ~CommaNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr1, *expr2;
  };

  class StatListNode : public StatementNode {
  public:
    StatListNode(StatementNode *s) : statement(s), list(0L) { }
    StatListNode(StatListNode *l, StatementNode *s) : statement(s), list(l) { }
    StatListNode(const StatListNode &other);
    virtual ~StatListNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    StatementNode *statement;
    StatListNode *list;
  };

  class AssignExprNode : public Node {
  public:
    AssignExprNode(Node *e) : expr(e) {}
    AssignExprNode(const AssignExprNode &other);
    virtual ~AssignExprNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class VarDeclNode : public Node {
  public:
    VarDeclNode(const UString *id, AssignExprNode *in);
    VarDeclNode(const VarDeclNode &other);
    virtual ~VarDeclNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    UString ident;
    AssignExprNode *init;
  };

  class VarDeclListNode : public Node {
  public:
    VarDeclListNode(VarDeclNode *v) : list(0L), var(v) {}
    VarDeclListNode(Node *l, VarDeclNode *v) : list(l), var(v) {}
    VarDeclListNode(const VarDeclListNode &other);
    virtual ~VarDeclListNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    Node *list;
    VarDeclNode *var;
  };

  class VarStatementNode : public StatementNode {
  public:
    VarStatementNode(VarDeclListNode *l) : list(l) {}
    VarStatementNode(const VarStatementNode &other);
    virtual ~VarStatementNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    VarDeclListNode *list;
  };

  class BlockNode : public StatementNode {
  public:
    BlockNode(StatListNode *s) : statlist(s) {}
    BlockNode(const BlockNode &other);
    virtual ~BlockNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    StatListNode *statlist;
  };

  class EmptyStatementNode : public StatementNode {
  public:
    EmptyStatementNode() { } // debug
    EmptyStatementNode(const EmptyStatementNode &other);
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
  };

  class ExprStatementNode : public StatementNode {
  public:
    ExprStatementNode(Node *e) : expr(e) { }
    ExprStatementNode(const ExprStatementNode &other);
    virtual ~ExprStatementNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class IfNode : public StatementNode {
  public:
    IfNode(Node *e, StatementNode *s1, StatementNode *s2)
      : expr(e), statement1(s1), statement2(s2) {}
    IfNode(const IfNode &other);
    virtual ~IfNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    Node *expr;
    StatementNode *statement1, *statement2;
  };

  class DoWhileNode : public StatementNode {
  public:
    DoWhileNode(StatementNode *s, Node *e) : statement(s), expr(e) {}
    DoWhileNode(const DoWhileNode &other);
    virtual ~DoWhileNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    StatementNode *statement;
    Node *expr;
  };

  class WhileNode : public StatementNode {
  public:
    WhileNode(Node *e, StatementNode *s) : expr(e), statement(s) {}
    WhileNode(const WhileNode &other);
    virtual ~WhileNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    Node *expr;
    StatementNode *statement;
  };

  class ForNode : public StatementNode {
  public:
    ForNode(Node *e1, Node *e2, Node *e3, StatementNode *s) :
      expr1(e1), expr2(e2), expr3(e3), stat(s) {}
    ForNode(const ForNode &other);
    virtual ~ForNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
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
    ForInNode(const ForInNode &other);
    virtual ~ForInNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
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
    ContinueNode(const ContinueNode &other);
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
  private:
    UString ident;
  };

  class BreakNode : public StatementNode {
  public:
    BreakNode() { }
    BreakNode(const UString *i) : ident(*i) { }
    BreakNode(const BreakNode &other);
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
  private:
    UString ident;
  };

  class ReturnNode : public StatementNode {
  public:
    ReturnNode(Node *v) : value(v) {}
    ReturnNode(const ReturnNode &other);
    virtual ~ReturnNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
  private:
    Node *value;
  };

  class WithNode : public StatementNode {
  public:
    WithNode(Node *e, StatementNode *s) : expr(e), stat(s) {}
    WithNode(const WithNode &other);
    virtual ~WithNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    Node *expr;
    StatementNode *stat;
  };

  class CaseClauseNode: public Node {
  public:
    CaseClauseNode(Node *e, StatListNode *l) : expr(e), list(l) { }
    CaseClauseNode(const CaseClauseNode &other);
    virtual ~CaseClauseNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
    Completion evalStatements(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    Node *expr;
    StatListNode *list;
  };

  class ClauseListNode : public Node {
  public:
    ClauseListNode(CaseClauseNode *c) : cl(c), nx(0L) { }
    ClauseListNode(const ClauseListNode &other);
    virtual ~ClauseListNode();
    virtual Node *copy() const;
    ClauseListNode* append(CaseClauseNode *c);
    KJSO evaluate(KJScriptImp *script, Context *context);
    CaseClauseNode *clause() const { return cl; }
    ClauseListNode *next() const { return nx; }
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    CaseClauseNode *cl;
    ClauseListNode *nx;
  };

  class CaseBlockNode: public Node {
  public:
    CaseBlockNode(ClauseListNode *l1, CaseClauseNode *d, ClauseListNode *l2)
      : list1(l1), def(d), list2(l2) { }
    CaseBlockNode(const CaseBlockNode &other);
    virtual ~CaseBlockNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
    Completion evalBlock(KJScriptImp *script, Context *context, const KJSO& input);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    ClauseListNode *list1;
    CaseClauseNode *def;
    ClauseListNode *list2;
  };

  class SwitchNode : public StatementNode {
  public:
    SwitchNode(Node *e, CaseBlockNode *b) : expr(e), block(b) { }
    SwitchNode(const SwitchNode &other);
    virtual ~SwitchNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    Node *expr;
    CaseBlockNode *block;
  };

  class LabelNode : public StatementNode {
  public:
    LabelNode(const UString *l, StatementNode *s) : label(*l), stat(s) { }
    LabelNode(const LabelNode &other);
    virtual ~LabelNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    UString label;
    StatementNode *stat;
  };

  class ThrowNode : public StatementNode {
  public:
    ThrowNode(Node *e) : expr(e) {}
    ThrowNode(const ThrowNode &other);
    virtual ~ThrowNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class CatchNode : public StatementNode {
  public:
    CatchNode(const UString *i, StatementNode *b) : ident(*i), block(b) {}
    CatchNode(const CatchNode &other);
    virtual ~CatchNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
    Completion execute(KJScriptImp *script, Context *context, const KJSO &arg);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    UString ident;
    StatementNode *block;
  };

  class FinallyNode : public StatementNode {
  public:
    FinallyNode(StatementNode *b) : block(b) {}
    FinallyNode(const FinallyNode &other);
    virtual ~FinallyNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    StatementNode *block;
  };

  class TryNode : public StatementNode {
  public:
    TryNode(StatementNode *b, Node *c = 0L, Node *f = 0L)
      : block(b), _catch((CatchNode*)c), _final((FinallyNode*)f) {}
    TryNode(const TryNode &other);
    virtual ~TryNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    StatementNode *block;
    CatchNode *_catch;
    FinallyNode *_final;
  };

  class ParameterNode : public Node {
  public:
    ParameterNode(const UString *i) : id(*i), next(0L) { }
    ParameterNode *append(const UString *i);
    ParameterNode(const ParameterNode &other);
    virtual ~ParameterNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
    UString ident() { return id; }
    ParameterNode *nextParam() { return next; }
  private:
    UString id;
    ParameterNode *next;
  };

  // inherited by ProgramNode
  class FunctionBodyNode : public StatementNode {
  public:
    FunctionBodyNode(SourceElementsNode *s);
    FunctionBodyNode(const FunctionBodyNode &other);
    virtual ~FunctionBodyNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  protected:
    SourceElementsNode *source;
  };

  class FuncDeclNode : public StatementNode {
  public:
    FuncDeclNode(const UString *i, ParameterNode *p, FunctionBodyNode *b)
      : ident(*i), param(p), body(b) { }
    FuncDeclNode(const FuncDeclNode &other);
    virtual ~FuncDeclNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp */*script*/, Context */*context*/)
      { /* empty */ return Completion(); }
    void processFuncDecl(KJScriptImp *script, Context *context);
  private:
    UString ident;
    ParameterNode *param;
    FunctionBodyNode *body;
  };

  class FuncExprNode : public Node {
  public:
    FuncExprNode(ParameterNode *p, FunctionBodyNode *b)
	: param(p), body(b) { }
    FuncExprNode(const FuncExprNode &other);
    virtual ~FuncExprNode();
    virtual Node *copy() const;
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    ParameterNode *param;
    FunctionBodyNode *body;
  };

  class SourceElementNode : public StatementNode {
  public:
    SourceElementNode(StatementNode *s) { statement = s; function = 0L; }
    SourceElementNode(FuncDeclNode *f) { function = f; statement = 0L;}
    SourceElementNode(const SourceElementNode &other);
    virtual ~SourceElementNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processFuncDecl(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    StatementNode *statement;
    FuncDeclNode *function;
  };

  class SourceElementsNode : public StatementNode {
  public:
    SourceElementsNode(SourceElementNode *s1) { element = s1; elements = 0L; }
    SourceElementsNode(SourceElementsNode *s1, SourceElementNode *s2)
      { elements = s1; element = s2; }
    SourceElementsNode(const SourceElementsNode &other);
    virtual ~SourceElementsNode();
    virtual Node *copy() const;
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processFuncDecl(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    SourceElementNode *element;
    SourceElementsNode *elements;
  };

  class ProgramNode : public FunctionBodyNode {
  public:
    ProgramNode(SourceElementsNode *s) : FunctionBodyNode(s) { }
    ProgramNode(const ProgramNode &other);
    virtual Node *copy() const;
  };

}; // namespace

#endif
