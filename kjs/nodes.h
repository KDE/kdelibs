/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
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

#ifndef _NODES_H_
#define _NODES_H_

#include "internal.h"
#include "ustring.h"
#include "object.h"
#include "types.h"
#include "debugger.h"
#ifndef NDEBUG
#include <list>
#endif

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
    virtual ~Node();
    virtual KJSO evaluate(KJScriptImp *script, Context *context) = 0;
    virtual void processVarDecls(KJScriptImp */*script*/, Context */*context*/) {}
    int lineNo() const { return line; }

  public:
    // reference counting mechanism
    virtual void ref() { refcount++; }
    virtual bool deref() { return (!--refcount); }

#ifndef NDEBUG
    static void finalCheck();
#endif
  protected:
    KJSO throwError(ErrorType e, const char *msg);
    int line;
  private:
    unsigned int refcount;
#ifndef NDEBUG
    // List of all nodes, for debugging purposes. Don't remove!
    static std::list<Node *> s_nodes;
#endif
    // disallow assignment
    Node& operator=(const Node&);
    Node(const Node &other);
  };

  class StatementNode : public Node {
  public:
    StatementNode();
    ~StatementNode();
    void setLoc(int line0, int line1, int sourceId);
    int firstLine() const { return l0; }
    int lastLine() const { return l1; }
    int sourceId() const { return sid; }
    bool hitStatement(KJScriptImp *script, Context *context);
    bool abortStatement(KJScriptImp *script, Context *context);
    virtual Completion execute(KJScriptImp *script, Context *context) = 0;
    void pushLabel(const UString *id) {
      if (id) ls.push(*id);
    }
  protected:
    LabelStack ls;
  private:
    KJSO evaluate(KJScriptImp */*script*/, Context */*context*/) { return Undefined(); }
    int l0, l1;
    int sid;
    bool breakPoint;
  };

  class NullNode : public Node {
  public:
    NullNode() {}
    KJSO evaluate(KJScriptImp *script, Context *context);
  };

  class BooleanNode : public Node {
  public:
    BooleanNode(bool v) : value(v) {}
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    bool value;
  };

  class NumberNode : public Node {
  public:
    NumberNode(double v) : value(v) { }
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    double value;
  };

  class StringNode : public Node {
  public:
    StringNode(const UString *v) { value = *v; }
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    UString value;
  };

  class RegExpNode : public Node {
  public:
    RegExpNode(const UString &p, const UString &f)
      : pattern(p), flags(f) { }
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    UString pattern, flags;
  };

  class ThisNode : public Node {
  public:
    ThisNode() {}
    KJSO evaluate(KJScriptImp *script, Context *context);
  };

  class ResolveNode : public Node {
  public:
    ResolveNode(const UString *s) : ident(*s) { }
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    UString ident;
  };

  class GroupNode : public Node {
  public:
    GroupNode(Node *g) : group(g) { }
    virtual void ref();
    virtual bool deref();
    virtual ~GroupNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *group;
  };

  class ElisionNode : public Node {
  public:
    ElisionNode(ElisionNode *e) : elision(e) { }
    virtual void ref();
    virtual bool deref();
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
    virtual void ref();
    virtual bool deref();
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
    virtual void ref();
    virtual bool deref();
    virtual ~ArrayNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    ElementNode *element;
    ElisionNode *elision;
    bool opt;
  };

  class ObjectLiteralNode : public Node {
  public:
    ObjectLiteralNode(Node *l) : list(l) { }
    virtual void ref();
    virtual bool deref();
    virtual ~ObjectLiteralNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *list;
  };

  class PropertyValueNode : public Node {
  public:
    PropertyValueNode(Node *n, Node *a, Node *l = 0L)
      : name(n), assign(a), list(l) { }
    virtual void ref();
    virtual bool deref();
    virtual ~PropertyValueNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *name, *assign, *list;
  };

  class PropertyNode : public Node {
  public:
    PropertyNode(double d) : numeric(d) { }
    PropertyNode(const UString *s) : str(*s) { }
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    double numeric;
    UString str;
  };

  class AccessorNode1 : public Node {
  public:
    AccessorNode1(Node *e1, Node *e2) : expr1(e1), expr2(e2) {}
    virtual void ref();
    virtual bool deref();
    virtual ~AccessorNode1();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr1;
    Node *expr2;
  };

  class AccessorNode2 : public Node {
  public:
    AccessorNode2(Node *e, const UString *s) : expr(e), ident(*s) { }
    virtual void ref();
    virtual bool deref();
    virtual ~AccessorNode2();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
    UString ident;
  };

  class ArgumentListNode : public Node {
  public:
    ArgumentListNode(Node *e);
    ArgumentListNode(ArgumentListNode *l, Node *e);
    virtual void ref();
    virtual bool deref();
    virtual ~ArgumentListNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
    List *evaluateList(KJScriptImp *script, Context *context);
  private:
    ArgumentListNode *list;
    Node *expr;
  };

  class ArgumentsNode : public Node {
  public:
    ArgumentsNode(ArgumentListNode *l);
    virtual void ref();
    virtual bool deref();
    virtual ~ArgumentsNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
    List *evaluateList(KJScriptImp *script, Context *context);
  private:
    ArgumentListNode *list;
  };

  class NewExprNode : public Node {
  public:
    NewExprNode(Node *e) : expr(e), args(0L) {}
    NewExprNode(Node *e, ArgumentsNode *a) : expr(e), args(a) {}
    virtual void ref();
    virtual bool deref();
    virtual ~NewExprNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
    ArgumentsNode *args;
  };

  class FunctionCallNode : public Node {
  public:
    FunctionCallNode(Node *e, ArgumentsNode *a) : expr(e), args(a) {}
    virtual void ref();
    virtual bool deref();
    virtual ~FunctionCallNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
    ArgumentsNode *args;
  };

  class PostfixNode : public Node {
  public:
    PostfixNode(Node *e, Operator o) : expr(e), oper(o) {}
    virtual void ref();
    virtual bool deref();
    virtual ~PostfixNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
    Operator oper;
  };

  class DeleteNode : public Node {
  public:
    DeleteNode(Node *e) : expr(e) {}
    virtual void ref();
    virtual bool deref();
    virtual ~DeleteNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class VoidNode : public Node {
  public:
    VoidNode(Node *e) : expr(e) {}
    virtual void ref();
    virtual bool deref();
    virtual ~VoidNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class TypeOfNode : public Node {
  public:
    TypeOfNode(Node *e) : expr(e) {}
    virtual void ref();
    virtual bool deref();
    virtual ~TypeOfNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class PrefixNode : public Node {
  public:
    PrefixNode(Operator o, Node *e) : oper(o), expr(e) {}
    virtual void ref();
    virtual bool deref();
    virtual ~PrefixNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Operator oper;
    Node *expr;
  };

  class UnaryPlusNode : public Node {
  public:
    UnaryPlusNode(Node *e) : expr(e) {}
    virtual void ref();
    virtual bool deref();
    virtual ~UnaryPlusNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class NegateNode : public Node {
  public:
    NegateNode(Node *e) : expr(e) {}
    virtual void ref();
    virtual bool deref();
    virtual ~NegateNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class BitwiseNotNode : public Node {
  public:
    BitwiseNotNode(Node *e) : expr(e) {}
    virtual void ref();
    virtual bool deref();
    virtual ~BitwiseNotNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class LogicalNotNode : public Node {
  public:
    LogicalNotNode(Node *e) : expr(e) {}
    virtual void ref();
    virtual bool deref();
    virtual ~LogicalNotNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class MultNode : public Node {
  public:
    MultNode(Node *t1, Node *t2, char op) : term1(t1), term2(t2), oper(op) {}
    virtual void ref();
    virtual bool deref();
    virtual ~MultNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *term1, *term2;
    char oper;
  };

  class AddNode : public Node {
  public:
    AddNode(Node *t1, Node *t2, char op) : term1(t1), term2(t2), oper(op) {}
    virtual void ref();
    virtual bool deref();
    virtual ~AddNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *term1, *term2;
    char oper;
  };

  class ShiftNode : public Node {
  public:
    ShiftNode(Node *t1, Operator o, Node *t2)
      : term1(t1), term2(t2), oper(o) {}
    virtual void ref();
    virtual bool deref();
    virtual ~ShiftNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *term1, *term2;
    Operator oper;
  };

  class RelationalNode : public Node {
  public:
    RelationalNode(Node *e1, Operator o, Node *e2) :
      expr1(e1), expr2(e2), oper(o) {}
    virtual void ref();
    virtual bool deref();
    virtual ~RelationalNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr1, *expr2;
    Operator oper;
  };

  class EqualNode : public Node {
  public:
    EqualNode(Node *e1, Operator o, Node *e2)
      : expr1(e1), expr2(e2), oper(o) {}
    virtual void ref();
    virtual bool deref();
    virtual ~EqualNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr1, *expr2;
    Operator oper;
  };

  class BitOperNode : public Node {
  public:
    BitOperNode(Node *e1, Operator o, Node *e2) :
      expr1(e1), expr2(e2), oper(o) {}
    virtual void ref();
    virtual bool deref();
    virtual ~BitOperNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr1, *expr2;
    Operator oper;
  };

  class BinaryLogicalNode : public Node {
  public:
    BinaryLogicalNode(Node *e1, Operator o, Node *e2) :
      expr1(e1), expr2(e2), oper(o) {}
    virtual void ref();
    virtual bool deref();
    virtual ~BinaryLogicalNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr1, *expr2;
    Operator oper;
  };

  class ConditionalNode : public Node {
  public:
    ConditionalNode(Node *l, Node *e1, Node *e2) :
      logical(l), expr1(e1), expr2(e2) {}
    virtual void ref();
    virtual bool deref();
    virtual ~ConditionalNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *logical, *expr1, *expr2;
  };

  class AssignNode : public Node {
  public:
    AssignNode(Node *l, Operator o, Node *e) : left(l), oper(o), expr(e) {}
    virtual void ref();
    virtual bool deref();
    virtual ~AssignNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *left;
    Operator oper;
    Node *expr;
  };

  class CommaNode : public Node {
  public:
    CommaNode(Node *e1, Node *e2) : expr1(e1), expr2(e2) {}
    virtual void ref();
    virtual bool deref();
    virtual ~CommaNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr1, *expr2;
  };

  class StatListNode : public StatementNode {
  public:
    StatListNode(StatementNode *s) : statement(s), list(0L) { }
    StatListNode(StatListNode *l, StatementNode *s) : statement(s), list(l) { }
    virtual void ref();
    virtual bool deref();
    virtual ~StatListNode();
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    StatementNode *statement;
    StatListNode *list;
  };

  class AssignExprNode : public Node {
  public:
    AssignExprNode(Node *e) : expr(e) {}
    virtual void ref();
    virtual bool deref();
    virtual ~AssignExprNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class VarDeclNode : public Node {
  public:
    VarDeclNode(const UString *id, AssignExprNode *in);
    virtual void ref();
    virtual bool deref();
    virtual ~VarDeclNode();
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
    virtual void ref();
    virtual bool deref();
    virtual ~VarDeclListNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    Node *list;
    VarDeclNode *var;
  };

  class VarStatementNode : public StatementNode {
  public:
    VarStatementNode(VarDeclListNode *l) : list(l) {}
    virtual void ref();
    virtual bool deref();
    virtual ~VarStatementNode();
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    VarDeclListNode *list;
  };

  class BlockNode : public StatementNode {
  public:
    BlockNode(StatListNode *s) : statlist(s) {}
    virtual void ref();
    virtual bool deref();
    virtual ~BlockNode();
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    StatListNode *statlist;
  };

  class EmptyStatementNode : public StatementNode {
  public:
    EmptyStatementNode() { } // debug
    Completion execute(KJScriptImp *script, Context *context);
  };

  class ExprStatementNode : public StatementNode {
  public:
    ExprStatementNode(Node *e) : expr(e) { }
    virtual void ref();
    virtual bool deref();
    virtual ~ExprStatementNode();
    Completion execute(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class IfNode : public StatementNode {
  public:
    IfNode(Node *e, StatementNode *s1, StatementNode *s2)
      : expr(e), statement1(s1), statement2(s2) {}
    virtual void ref();
    virtual bool deref();
    virtual ~IfNode();
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    Node *expr;
    StatementNode *statement1, *statement2;
  };

  class DoWhileNode : public StatementNode {
  public:
    DoWhileNode(StatementNode *s, Node *e) : statement(s), expr(e) {}
    virtual void ref();
    virtual bool deref();
    virtual ~DoWhileNode();
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    StatementNode *statement;
    Node *expr;
  };

  class WhileNode : public StatementNode {
  public:
    WhileNode(Node *e, StatementNode *s) : expr(e), statement(s) {}
    virtual void ref();
    virtual bool deref();
    virtual ~WhileNode();
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    Node *expr;
    StatementNode *statement;
  };

  class ForNode : public StatementNode {
  public:
    ForNode(Node *e1, Node *e2, Node *e3, StatementNode *s) :
      expr1(e1), expr2(e2), expr3(e3), statement(s) {}
    virtual void ref();
    virtual bool deref();
    virtual ~ForNode();
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    Node *expr1, *expr2, *expr3;
    StatementNode *statement;
  };

  class ForInNode : public StatementNode {
  public:
    ForInNode(Node *l, Node *e, StatementNode *s) :
      init(0L), lexpr(l), expr(e), statement(s) {}
    ForInNode(const UString *i, AssignExprNode *in, Node *e, StatementNode *s)
      : ident(*i), init(in), lexpr(0L), expr(e), statement(s) {}
    virtual void ref();
    virtual bool deref();
    virtual ~ForInNode();
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    UString ident;
    AssignExprNode *init;
    Node *lexpr, *expr;
    StatementNode *statement;
  };

  class ContinueNode : public StatementNode {
  public:
    ContinueNode() { }
    ContinueNode(const UString *i) : ident(*i) { }
    Completion execute(KJScriptImp *script, Context *context);
  private:
    UString ident;
  };

  class BreakNode : public StatementNode {
  public:
    BreakNode() { }
    BreakNode(const UString *i) : ident(*i) { }
    Completion execute(KJScriptImp *script, Context *context);
  private:
    UString ident;
  };

  class ReturnNode : public StatementNode {
  public:
    ReturnNode(Node *v) : value(v) {}
    virtual void ref();
    virtual bool deref();
    virtual ~ReturnNode();
    Completion execute(KJScriptImp *script, Context *context);
  private:
    Node *value;
  };

  class WithNode : public StatementNode {
  public:
    WithNode(Node *e, StatementNode *s) : expr(e), statement(s) {}
    virtual void ref();
    virtual bool deref();
    virtual ~WithNode();
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    Node *expr;
    StatementNode *statement;
  };

  class CaseClauseNode: public Node {
  public:
    CaseClauseNode(Node *e, StatListNode *l) : expr(e), list(l) { }
    virtual void ref();
    virtual bool deref();
    virtual ~CaseClauseNode();
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
    virtual void ref();
    virtual bool deref();
    virtual ~ClauseListNode();
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
    virtual void ref();
    virtual bool deref();
    virtual ~CaseBlockNode();
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
    virtual void ref();
    virtual bool deref();
    virtual ~SwitchNode();
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    Node *expr;
    CaseBlockNode *block;
  };

  class LabelNode : public StatementNode {
  public:
    LabelNode(const UString *l, StatementNode *s) : label(*l), statement(s) { }
    virtual void ref();
    virtual bool deref();
    virtual ~LabelNode();
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    UString label;
    StatementNode *statement;
  };

  class ThrowNode : public StatementNode {
  public:
    ThrowNode(Node *e) : expr(e) {}
    virtual void ref();
    virtual bool deref();
    virtual ~ThrowNode();
    Completion execute(KJScriptImp *script, Context *context);
  private:
    Node *expr;
  };

  class CatchNode : public StatementNode {
  public:
    CatchNode(const UString *i, StatementNode *b) : ident(*i), block(b) {}
    virtual void ref();
    virtual bool deref();
    virtual ~CatchNode();
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
    virtual void ref();
    virtual bool deref();
    virtual ~FinallyNode();
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    StatementNode *block;
  };

  class TryNode : public StatementNode {
  public:
    TryNode(StatementNode *b, Node *c = 0L, Node *f = 0L)
      : block(b), _catch((CatchNode*)c), _final((FinallyNode*)f) {}
    virtual void ref();
    virtual bool deref();
    virtual ~TryNode();
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
    virtual void ref();
    virtual bool deref();
    virtual ~ParameterNode();
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
    virtual void ref();
    virtual bool deref();
    virtual ~FunctionBodyNode();
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  protected:
    SourceElementsNode *source;
  };

  class FuncDeclNode : public StatementNode {
  public:
    FuncDeclNode(const UString *i, ParameterNode *p, FunctionBodyNode *b)
      : ident(*i), param(p), body(b) { }
    virtual void ref();
    virtual bool deref();
    virtual ~FuncDeclNode();
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
    virtual void ref();
    virtual bool deref();
    virtual ~FuncExprNode();
    KJSO evaluate(KJScriptImp *script, Context *context);
  private:
    ParameterNode *param;
    FunctionBodyNode *body;
  };

  class SourceElementNode : public StatementNode {
  public:
    SourceElementNode(StatementNode *s) { statement = s; function = 0L; }
    SourceElementNode(FuncDeclNode *f) { function = f; statement = 0L;}
    virtual void ref();
    virtual bool deref();
    virtual ~SourceElementNode();
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processFuncDecl(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    StatementNode *statement;
    FuncDeclNode *function;
  };

  // A linked list of source element nodes
  class SourceElementsNode : public StatementNode {
  public:
    SourceElementsNode(SourceElementNode *s1) { element = s1; elements = 0L; }
    SourceElementsNode(SourceElementsNode *s1, SourceElementNode *s2)
      { elements = s1; element = s2; }
    virtual void ref();
    virtual bool deref();
    virtual ~SourceElementsNode();
    Completion execute(KJScriptImp *script, Context *context);
    virtual void processFuncDecl(KJScriptImp *script, Context *context);
    virtual void processVarDecls(KJScriptImp *script, Context *context);
  private:
    SourceElementNode *element; // 'this' element
    SourceElementsNode *elements; // pointer to next
  };

  class ProgramNode : public FunctionBodyNode {
  public:
    ProgramNode(SourceElementsNode *s) : FunctionBodyNode(s) { }
  private:
    // Disallow copy
    ProgramNode(const ProgramNode &other);
  };

}; // namespace

#endif
