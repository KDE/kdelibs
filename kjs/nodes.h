// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003, 2004, 2005, 2006, 2007 Apple Inc. All rights reserved.
 *  Copyright (C) 2007 Maksim Orlovich (maksim@kde.org)
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef NODES_H_
#define NODES_H_

#include "Parser.h"
#include "internal.h"
#include "operations.h"
#include "reference.h"
#include "SymbolTable.h"
#include "opcodes.h"
#include "bytecode/opargs.h"
#include <wtf/ListRefPtr.h>
#include <wtf/Vector.h>


namespace KJS {
  class ProgramNode;
  class PropertyNameNode;
  class PropertyListNode;
  class RegExp;
  class SourceElementsNode;
  class SourceStream;
  class PackageObject;
  class FuncDeclNode;
  class FunctionBodyNode;
  class Node;

  class VarDeclVisitor;
  class FuncDeclVisitor;
  class SemanticChecker;

  class CompileState;
  class CompileReference;

  class NodeVisitor {
  public:
    virtual ~NodeVisitor() {}
    /**
     This method should be overridden by subclasses to process nodes, and
     perhaps return pointers for replacement nodes. If the node should not be
     changed, return 0. Otherwise, return the replacement node.

     The default implementation asks the node to visit its kids, and do
     replacements on them if needed, but does not change anything for this node
    */
    virtual Node* visit(Node* node);
  };

  class Node {
  public:
      enum NodeType {
          UnknownNodeType,
          NullNodeType,
          BooleanNodeType,
          NumberNodeType,
          StringNodeType,
          RegExpNodeType,
          TryNodeType,
          GroupNodeType,
          LabelNodeType
      };

    Node();
    virtual ~Node();

    virtual NodeType type() const { return UnknownNodeType; }

    virtual JSValue *evaluate(ExecState *exec) = 0;
    UString toString() const;
    virtual void streamTo(SourceStream&) const = 0;
    int lineNo() const { return m_line; }

    void ref();
    void deref();
    unsigned refcount();
    static void clearNewNodes();

    virtual Node *nodeInsideAllParens();

    virtual bool isLocation() const       { return false; }
    virtual bool isVarAccessNode() const  { return false; }
    virtual bool isBracketAccessorNode() const { return false; }
    virtual bool isDotAccessorNode() const { return false; }
    bool isNumber() const { return type() == NumberNodeType; }
    bool isString() const { return type() == StringNodeType; }
    bool isGroupNode() const { return type() == GroupNodeType; }
    bool isTryNode() const { return type() == TryNodeType; }
    bool isLabelNode() const { return type() == LabelNodeType; }
    virtual bool introducesNewStaticScope () const { return false; }
    virtual bool introducesNewDynamicScope() const { return false; }
    virtual bool isIterationStatement()      const { return false; }
    virtual bool isSwitchStatement()         const { return false; }
    virtual bool isVarStatement()            const { return false; }

    bool introducesNewScope() const { return introducesNewStaticScope() || introducesNewDynamicScope(); }

    virtual void breakCycle() { }

    // Processes all function and variable declarations below this node,
    // adding them to symbol table or the current object depending on the
    // execution context..
    void processDecls(ExecState*);

    /*
      Implementations of this method should call visitor->visit on all the
      children nodes, and if they return value is non-0, update the link to the child.
      The recurseVisitLink helper takes care of this
    */
    virtual void recurseVisit(NodeVisitor * /*visitor*/) {}

    /*
      Nodes that can be optimized to take advantage of numeric binding to
      locals should override this method.
    */
    virtual Node* optimizeLocalAccess(ExecState *exec, FunctionBodyNode* node);

    template<typename T>
    static void recurseVisitLink(NodeVisitor* visitor, RefPtr<T>& link)
    {
        if (!link)
          return;

        T* newLink = static_cast<T*>(visitor->visit(link.get()));
        if (newLink)
          link = newLink;
    }

    template<typename T>
    static void recurseVisitLink(NodeVisitor* visitor, ListRefPtr<T>& link)
    {
        if (!link)
          return;

        T* newLink = static_cast<T*>(visitor->visit(link.get()));
        if (newLink)
          link = newLink;
    }

    Completion createErrorCompletion(ExecState *, ErrorType, const UString& msg);
    Completion createErrorCompletion(ExecState *, ErrorType, const UString& msg, const Identifier &);

    JSValue* throwError(ExecState*, ErrorType, const UString& msg);
    JSValue* throwError(ExecState*, ErrorType, const UString& msg, const char* string);
    JSValue* throwError(ExecState*, ErrorType, const UString& msg, JSValue*, Node*);
    JSValue* throwError(ExecState*, ErrorType, const UString& msg, const Identifier&);
    JSValue* throwError(ExecState*, ErrorType, const UString& msg, JSValue*, const Identifier&);
    JSValue* throwError(ExecState*, ErrorType, const UString& msg, JSValue*, Node*, Node*);
    JSValue* throwError(ExecState*, ErrorType, const UString& msg, JSValue*, Node*, const Identifier&);

    JSValue* throwUndefinedVariableError(ExecState*, const Identifier&);

    void handleException(ExecState*);
    void handleException(ExecState*, JSValue*);
    Completion rethrowException(ExecState*);

    void copyDebugInfo(Node* otherNode);

    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
  protected:
    /* Nodes that can do semantic checking should override this,
       and return an appropriate error node if appropriate. The reimplementations
       should call the parent class's checkSemantics method at point where
       recursion should occur
    */
    friend class SemanticChecker;
    virtual Node* checkSemantics(SemanticChecker* semanticChecker);

    int m_line;
  private:
    virtual void processVarDecl (ExecState* state);
    virtual void processFuncDecl(ExecState* state);
    friend class VarDeclVisitor;
    friend class FuncDeclVisitor;

    // disallow assignment
    Node& operator=(const Node&);
    Node(const Node &other);
  };

  class LocationNode : public Node {
  public:
    virtual bool isLocation() const { return true; }
    virtual Reference evaluateReference(ExecState* exec) = 0;

    // if we have a = foo, we call generateRefBegin before
    // evaluating 'foo', and then generateRefWrite after;
    // there are 2 reasons for this:
    // 1) the conceptual reference evaluation step could raise an exception,
    //    which should happen in the proper order, before evaluating the RHS
    // 2) when doing something like foo = bar(), the evaluation of
    //    bar() may introduce a new copy of foo earlier on in the scope
    //    chain, but that should not affect the scope.
    // Note that the client has to delete the return CompileReference itself
    virtual CompileReference* generateRefBegin (CompileState*, CodeBlock& block, bool errorOnFail) = 0;
    virtual OpValue generateRefBase(CompileState*, CodeBlock& block, CompileReference* ref) = 0;
    virtual OpValue generateRefRead(CompileState*, CodeBlock& block, CompileReference* ref) = 0;
    virtual void generateRefWrite  (CompileState*, CodeBlock& block,
                                    CompileReference* ref, OpValue& valToStore) = 0;
  };

  class StatementNode : public Node {
  public:
    StatementNode();
    void setLoc(int line0, int line1);
    int firstLine() const { return lineNo(); }
    int lastLine() const { return m_lastLine; }
    bool hitStatement(ExecState*);
    virtual Completion execute(ExecState *exec) = 0;

    void copyDebugInfo(StatementNode* otherNode);
    Node* createErrorNode(ErrorType e, const UString& msg);
    Node* createErrorNode(ErrorType e, const UString& msg, const Identifier &ident);

    virtual void generateExecCode(CompileState*, CodeBlock& block);
  protected:
    /* This implementation of checkSemantics applies the accumulated labels to
       this statement, manages the targets for labelless break and continue,
       and recurses.
    */
    virtual Node* checkSemantics(SemanticChecker* semanticChecker);
  private:
    JSValue *evaluate(ExecState*) { return jsUndefined(); }
    int m_lastLine;
  };

  class NullNode : public Node {
  public:
    NullNode() {}
    virtual NodeType type() const { return NullNodeType; }
    JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
  };

  class BooleanNode : public Node {
  public:
    BooleanNode(bool v) : val(v) {}
    bool value() const { return val; }

    virtual NodeType type() const { return BooleanNodeType; }
    JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
  private:
    bool val;
  };

  class NumberNode : public Node {
  public:
    NumberNode(double v) : val(v) {}
    double value() const { return val; }
    void setValue(double v) { val = v; }

    virtual NodeType type() const { return NumberNodeType; }
    JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
  private:
    double val;
  };

  class StringNode : public Node {
  public:
    StringNode(const UString *v) : val(*v) { }
    UString value() const { return val; }
    void setValue(const UString& v) { val = v; }

    virtual NodeType type() const { return StringNodeType; }
    JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
  private:
    UString val;
  };

  class RegExpNode : public Node {
  public:
    RegExpNode(const UString &p, const UString &f)
      : pattern(p), flags(f) { }
    virtual NodeType type() const { return RegExpNodeType; }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
  private:
    UString pattern, flags;
  };

  class ThisNode : public Node {
  public:
    ThisNode() {}
    JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
  };

  class VarAccessNode : public LocationNode {
  public:
    VarAccessNode(const Identifier& s) : ident(s) {}
    ~VarAccessNode(); // to avoid inst of ~OpValue here
    virtual Node* optimizeLocalAccess(ExecState *exec, FunctionBodyNode* node);

    virtual bool isVarAccessNode() const { return true; }
    virtual void streamTo(SourceStream&) const;
    virtual JSValue*  evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);

    virtual Reference evaluateReference(ExecState* exec);
    virtual CompileReference* generateRefBegin (CompileState*, CodeBlock& block, bool errorOnFail);
    virtual OpValue generateRefRead(CompileState*, CodeBlock& block, CompileReference* ref);
    virtual OpValue generateRefBase(CompileState* comp, CodeBlock& block, CompileReference* ref);
    virtual void generateRefWrite(CompileState*, CodeBlock& block,
                                        CompileReference* ref, OpValue& valToStore);

    // Returns the ID this variable should be accessed as, or
    // missingSymbolMarker(). maybeLocal will be set if the symbol
    // could potentially be found in the current scope.
    size_t localID(CompileState*, bool& dynamicLocal);
  protected:
    Identifier ident;
  };

  class GroupNode : public Node {
  public:
    GroupNode(Node *g) : group(g) { }
    virtual NodeType type() const { return GroupNodeType; }
    virtual JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual Node *nodeInsideAllParens();
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> group;
  };

  class ElementNode : public Node {
  public:
    // list pointer is tail of a circular list, cracked in the ArrayNode ctor
    ElementNode(int e, Node *n) : next(this), elision(e), node(n) { Parser::noteNodeCycle(this); }
    ElementNode(ElementNode *l, int e, Node *n)
      : next(l->next), elision(e), node(n) { l->next = this; }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    PassRefPtr<ElementNode> releaseNext() { return next.release(); }
    virtual void breakCycle();
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    friend class ArrayNode;
    ListRefPtr<ElementNode> next;
    int elision;
    RefPtr<Node> node;
  };

  class ArrayNode : public Node {
  public:
    ArrayNode(int e) : elision(e), opt(true) { }
    ArrayNode(ElementNode *ele)
      : element(ele->next.release()), elision(0), opt(false) { Parser::removeNodeCycle(element.get()); }
    ArrayNode(int eli, ElementNode *ele)
      : element(ele->next.release()), elision(eli), opt(true) { Parser::removeNodeCycle(element.get()); }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<ElementNode> element;
    int elision;
    bool opt;
  };

  class PropertyNameNode : public Node {
  public:
    PropertyNameNode(const Identifier &s) : str(s) { }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
  private:
    friend class ObjectLiteralNode;
    Identifier str;
  };

  class PropertyNode : public Node {
  public:
    enum Type { Constant, Getter, Setter };
    PropertyNode(PropertyNameNode *n, Node *a, Type t)
      : name(n), assign(a), type(t) { }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    friend class PropertyListNode;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    friend class ObjectLiteralNode;
    RefPtr<PropertyNameNode> name;
    RefPtr<Node> assign;
    Type type;
  };

  class PropertyListNode : public Node {
  public:
    // list pointer is tail of a circular list, cracked in the ObjectLiteralNode ctor
    PropertyListNode(PropertyNode *n)
      : node(n), next(this) { Parser::noteNodeCycle(this); }
    PropertyListNode(PropertyNode *n, PropertyListNode *l)
      : node(n), next(l->next) { l->next = this; }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    PassRefPtr<PropertyListNode> releaseNext() { return next.release(); }
    virtual void breakCycle();
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    friend class ObjectLiteralNode;
    RefPtr<PropertyNode> node;
    ListRefPtr<PropertyListNode> next;
  };

  class ObjectLiteralNode : public Node {
  public:
    ObjectLiteralNode() { }
    ObjectLiteralNode(PropertyListNode *l) : list(l->next.release()) { Parser::removeNodeCycle(list.get()); }
    JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<PropertyListNode> list;
  };

  class BracketAccessorNode : public LocationNode {
  public:
    BracketAccessorNode(Node *e1, Node *e2) : expr1(e1), expr2(e2) {}
    JSValue*  evaluate(ExecState*);
    Reference evaluateReference(ExecState*);
    virtual void streamTo(SourceStream&) const;

    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual CompileReference* generateRefBegin (CompileState*, CodeBlock& block, bool errorOnFail);
    virtual OpValue generateRefBase(CompileState*, CodeBlock& block, CompileReference* ref);
    virtual OpValue generateRefRead(CompileState*, CodeBlock& block, CompileReference* ref);
    virtual void generateRefWrite  (CompileState*, CodeBlock& block,
                                    CompileReference* ref, OpValue& valToStore);

    Node *base() { return expr1.get(); }
    Node *subscript() { return expr2.get(); }

    virtual void recurseVisit(NodeVisitor *visitor);
    virtual bool isBracketAccessorNode() const { return true; }
  protected:
    RefPtr<Node> expr1;
    RefPtr<Node> expr2;
  };

  class DotAccessorNode : public LocationNode {
  public:
    DotAccessorNode(Node *e, const Identifier &s) : expr(e), ident(s) { }
    JSValue*  evaluate(ExecState*);
    Reference evaluateReference(ExecState*);
    virtual void streamTo(SourceStream&) const;

    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);

    virtual CompileReference* generateRefBegin (CompileState*, CodeBlock& block, bool errorOnFail);
    virtual OpValue generateRefRead(CompileState*, CodeBlock& block, CompileReference* ref);
    virtual OpValue generateRefBase(CompileState* comp, CodeBlock& block, CompileReference* ref);
    virtual void generateRefWrite(CompileState*, CodeBlock& block,
                                        CompileReference* ref, OpValue& valToStore);


    Node *base() const { return expr.get(); }
    const Identifier& identifier() const { return ident; }

    virtual void recurseVisit(NodeVisitor *visitor);
    virtual bool isDotAccessorNode() const { return true; }
  protected:
    RefPtr<Node> expr;
    Identifier ident;
  };

  class ArgumentListNode : public Node {
  public:
    // list pointer is tail of a circular list, cracked in the ArgumentsNode ctor
    ArgumentListNode(Node *e) : next(this), expr(e) { Parser::noteNodeCycle(this); }
    ArgumentListNode(ArgumentListNode *l, Node *e)
      : next(l->next), expr(e) { l->next = this; }
    JSValue* evaluate(ExecState*);
    List evaluateList(ExecState*);
    virtual void streamTo(SourceStream&) const;
    PassRefPtr<ArgumentListNode> releaseNext() { return next.release(); }
    virtual void breakCycle();

    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    friend class ArgumentsNode;
    ListRefPtr<ArgumentListNode> next;
    RefPtr<Node> expr;
  };

  class ArgumentsNode : public Node {
  public:
    ArgumentsNode() { }
    ArgumentsNode(ArgumentListNode *l)
      : list(l->next.release()) { Parser::removeNodeCycle(list.get()); }
    JSValue* evaluate(ExecState*);
    List evaluateList(ExecState *exec) { return list ? list->evaluateList(exec) : List::empty(); }
    void generateEvalArguments(CompileState* comp, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;

    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<ArgumentListNode> list;
  };

  class NewExprNode : public Node {
  public:
    NewExprNode(Node *e) : expr(e) {}
    NewExprNode(Node *e, ArgumentsNode *a) : expr(e), args(a) {}
    JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
    RefPtr<ArgumentsNode> args;
  };

  class FunctionCallValueNode : public Node {
  public:
    FunctionCallValueNode(Node *e, ArgumentsNode *a) : expr(e), args(a) {}
    JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
    RefPtr<ArgumentsNode> args;
  };

  class FunctionCallReferenceNode : public Node {
  public:
    FunctionCallReferenceNode(LocationNode *e, ArgumentsNode *a) : expr(e), args(a) {}
    JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<LocationNode> expr;
    RefPtr<ArgumentsNode> args;
  };

  class FunctionCallBracketNode : public Node {
  public:
    FunctionCallBracketNode(Node *b, Node *s, ArgumentsNode *a) : base(b), subscript(s), args(a) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  protected:
    RefPtr<Node> base;
    RefPtr<Node> subscript;
    RefPtr<ArgumentsNode> args;
  };

  class FunctionCallParenBracketNode : public FunctionCallBracketNode {
  public:
    FunctionCallParenBracketNode(Node *b, Node *s, ArgumentsNode *a) : FunctionCallBracketNode(b, s, a) {}
    virtual void streamTo(SourceStream&) const;
  };

  class FunctionCallDotNode : public Node {
  public:
    FunctionCallDotNode(Node *b, const Identifier &i, ArgumentsNode *a) : base(b), ident(i), args(a) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  protected:
    RefPtr<Node> base;
    Identifier ident;
    RefPtr<ArgumentsNode> args;
  };

  class FunctionCallParenDotNode : public FunctionCallDotNode {
  public:
    FunctionCallParenDotNode(Node *b, const Identifier &i, ArgumentsNode *a) : FunctionCallDotNode(b, i, a) {}
    virtual void streamTo(SourceStream&) const;
  };

  class PostfixNode : public Node {
  public:
    PostfixNode(LocationNode *l, Operator o) : m_loc(l), m_oper(o) {}
    JSValue* evaluate(ExecState*);
    void streamTo(SourceStream&) const;
    void recurseVisit(NodeVisitor * visitor);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    Node* optimizeLocalAccess(ExecState *exec, FunctionBodyNode* node);
  protected:
    RefPtr<LocationNode> m_loc;
    Operator m_oper;
  };

  class PostfixErrorNode : public Node {
  public:
    PostfixErrorNode(Node* e, Operator o) : m_expr(e), m_oper(o) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
  private:
    RefPtr<Node> m_expr;
    Operator m_oper;
  };

  class DeleteReferenceNode : public Node {
  public:
    DeleteReferenceNode(LocationNode *l) : loc(l) {}
    JSValue* evaluate(ExecState*);
    void streamTo(SourceStream&) const;
    void recurseVisit(NodeVisitor * visitor);
  private:
    RefPtr<LocationNode> loc;
  };

  class DeleteValueNode : public Node {
  public:
    DeleteValueNode(Node *e) : m_expr(e) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> m_expr;
  };


  class VoidNode : public Node {
  public:
    VoidNode(Node *e) : expr(e) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
  };

  class TypeOfReferenceNode : public Node {
  public:
    TypeOfReferenceNode(LocationNode *l) : loc(l) {}
    JSValue* evaluate(ExecState*);
    void streamTo(SourceStream&) const;
    void recurseVisit(NodeVisitor * visitor);
  private:
    RefPtr<LocationNode> loc;
  };

  class TypeOfValueNode : public Node {
  public:
    TypeOfValueNode(Node *e) : m_expr(e) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> m_expr;
  };

  class PrefixErrorNode : public Node {
  public:
    PrefixErrorNode(Node* e, Operator o) : m_expr(e), m_oper(o) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
  private:
    RefPtr<Node> m_expr;
    Operator m_oper;
  };

  class PrefixNode : public Node {
  public:
    PrefixNode(LocationNode *l, Operator o) : m_loc(l), m_oper(o) {}
    JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    void streamTo(SourceStream&) const;
    void recurseVisit(NodeVisitor * visitor);
    Node* optimizeLocalAccess(ExecState *exec, FunctionBodyNode* node);
  protected:
    RefPtr<LocationNode> m_loc;
    Operator m_oper;
  };

  class UnaryPlusNode : public Node {
  public:
    UnaryPlusNode(Node *e) : expr(e) {}
    JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
  };

  class NegateNode : public Node {
  public:
    NegateNode(Node *e) : expr(e) {}
    JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
  };

  class BitwiseNotNode : public Node {
  public:
    BitwiseNotNode(Node *e) : expr(e) {}
    JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
  };

  class LogicalNotNode : public Node {
  public:
    LogicalNotNode(Node *e) : expr(e) {}
    JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
  };

  class BinaryOperatorNode : public Node {
  public:
    BinaryOperatorNode(Node* e1, Node* e2, Operator op)
      : expr1(e1), expr2(e2), oper(op) {}
    JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor* visitor);
  private:
    JSValue* operatorIn(ExecState* exec, JSValue* v1, JSValue* v2);
    JSValue* operatorInstanceOf(ExecState* exec, JSValue* v1, JSValue* v2);
    RefPtr<Node> expr1;
    RefPtr<Node> expr2;
    Operator oper;
  };

  /**
   * expr1 && expr2, expr1 || expr2
   */
  class BinaryLogicalNode : public Node {
  public:
    BinaryLogicalNode(Node *e1, Operator o, Node *e2) :
      expr1(e1), expr2(e2), oper(o) {}
    JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr1;
    RefPtr<Node> expr2;
    Operator oper;
  };

  /**
   * The ternary operator, "logical ? expr1 : expr2"
   */
  class ConditionalNode : public Node {
  public:
    ConditionalNode(Node *l, Node *e1, Node *e2) :
      logical(l), expr1(e1), expr2(e2) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> logical;
    RefPtr<Node> expr1;
    RefPtr<Node> expr2;
  };

  class AssignErrorNode : public Node {
  public:
    AssignErrorNode(Node* left, Operator oper, Node* right)
      : m_left(left), m_oper(oper), m_right(right) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
  protected:
    RefPtr<Node> m_left;
    Operator m_oper;
    RefPtr<Node> m_right;
  };

  class AssignNode : public Node {
  public:
    AssignNode(LocationNode* loc, Operator oper, Node *right)
      : m_loc(loc), m_oper(oper), m_right(right) {}
    void streamTo(SourceStream&) const;
    JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    void recurseVisit(NodeVisitor * visitor);
    Node* optimizeLocalAccess(ExecState *exec, FunctionBodyNode* node);
  protected:
    RefPtr<LocationNode> m_loc;
    Operator m_oper;
    RefPtr<Node> m_right;
  };

  class AssignBracketNode : public Node {
  public:
    AssignBracketNode(Node *base, Node *subscript, Operator oper, Node *right)
      : m_base(base), m_subscript(subscript), m_oper(oper), m_right(right) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  protected:
    RefPtr<Node> m_base;
    RefPtr<Node> m_subscript;
    Operator m_oper;
    RefPtr<Node> m_right;
  };

  class AssignDotNode : public Node {
  public:
    AssignDotNode(Node *base, const Identifier& ident, Operator oper, Node *right)
      : m_base(base), m_ident(ident), m_oper(oper), m_right(right) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  protected:
    RefPtr<Node> m_base;
    Identifier m_ident;
    Operator m_oper;
    RefPtr<Node> m_right;
  };

  class CommaNode : public Node {
  public:
    CommaNode(Node *e1, Node *e2) : expr1(e1), expr2(e2) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
  private:
    RefPtr<Node> expr1;
    RefPtr<Node> expr2;
  };

  class AssignExprNode : public Node {
  public:
    AssignExprNode(Node *e) : expr(e) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);

    Node* getExpr() { return expr.get(); }
  private:
    RefPtr<Node> expr;
  };

  class VarDeclNode : public Node {
  public:
    enum Type { Variable, Constant };
    VarDeclNode(const Identifier &id, AssignExprNode *in, Type t);
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);

    virtual void processVarDecl(ExecState*);
  private:
    friend class VarStatementNode;
    friend class VarDeclListNode;
    Type varType;
    Identifier ident;
    RefPtr<AssignExprNode> init;
  };

  class VarDeclListNode : public Node {
  public:
    // list pointer is tail of a circular list, cracked in the ForNode/VarStatementNode ctor
    VarDeclListNode(VarDeclNode *v) : next(this), var(v) { Parser::noteNodeCycle(this); }
    VarDeclListNode(VarDeclListNode *l, VarDeclNode *v)
      : next(l->next), var(v) { l->next = this; }
    JSValue* evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
    PassRefPtr<VarDeclListNode> releaseNext() { return next.release(); }
    virtual void breakCycle();
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    friend class ForNode;
    friend class VarStatementNode;
    ListRefPtr<VarDeclListNode> next;
    RefPtr<VarDeclNode> var;
  };

  class VarStatementNode : public StatementNode {
  public:
    virtual Node* optimizeLocalAccess(ExecState *exec, FunctionBodyNode* node);
    VarStatementNode(VarDeclListNode *l) : next(l->next.release()) { Parser::removeNodeCycle(next.get()); }
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
    virtual bool isVarStatement() const { return true; }
    virtual void generateExecCode(CompileState*, CodeBlock& block);
  private:
    RefPtr<VarDeclListNode> next;
  };

  class BlockNode : public StatementNode {
  public:
    BlockNode(SourceElementsNode *s);
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
    virtual void generateExecCode(CompileState*, CodeBlock& block);
  protected:
    RefPtr<SourceElementsNode> source;
  };

  class EmptyStatementNode : public StatementNode {
  public:
    EmptyStatementNode() { } // debug
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void generateExecCode(CompileState*, CodeBlock& block);
  };

  class ExprStatementNode : public StatementNode {
  public:
    ExprStatementNode(Node *e) : expr(e) { }
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
    virtual void generateExecCode(CompileState*, CodeBlock& block);
  private:
    RefPtr<Node> expr;
  };

  class IfNode : public StatementNode {
  public:
    IfNode(Node *e, StatementNode *s1, StatementNode *s2)
      : expr(e), statement1(s1), statement2(s2) {}
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
    virtual void generateExecCode(CompileState*, CodeBlock& block);
  private:
    RefPtr<Node> expr;
    RefPtr<StatementNode> statement1;
    RefPtr<StatementNode> statement2;
  };

  class DoWhileNode : public StatementNode {
  public:
    DoWhileNode(StatementNode *s, Node *e) : statement(s), expr(e) {}
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
    virtual void generateExecCode(CompileState*, CodeBlock& block);
    virtual bool isIterationStatement() const { return true; }
  private:
    RefPtr<StatementNode> statement;
    RefPtr<Node> expr;
  };

  class WhileNode : public StatementNode {
  public:
    WhileNode(Node *e, StatementNode *s) : expr(e), statement(s) {}
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
    virtual void generateExecCode(CompileState*, CodeBlock& block);
    virtual bool isIterationStatement() const { return true; }
  private:
    RefPtr<Node> expr;
    RefPtr<StatementNode> statement;
  };

  class ForNode : public StatementNode {
  public:
    ForNode(Node *e1, Node *e2, Node *e3, StatementNode *s) :
      expr1(e1), expr2(e2), expr3(e3), statement(s) {}
    ForNode(VarDeclListNode *e1, Node *e2, Node *e3, StatementNode *s) :
      expr1(e1->next.release()), expr2(e2), expr3(e3), statement(s) { Parser::removeNodeCycle(expr1.get()); }
    virtual Completion execute(ExecState*);
    virtual void generateExecCode(CompileState*, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
    virtual bool isIterationStatement() const { return true; }
  private:
    RefPtr<Node> expr1;
    RefPtr<Node> expr2;
    RefPtr<Node> expr3;
    RefPtr<StatementNode> statement;
  };

  class ForInNode : public StatementNode {
  public:
    ForInNode(Node *l, Node *e, StatementNode *s);
    ForInNode(const Identifier &i, AssignExprNode *in, Node *e, StatementNode *s);
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
    virtual bool isIterationStatement() const { return true; }
  private:
    Identifier ident;
    RefPtr<AssignExprNode> init;
    RefPtr<Node> lexpr;
    RefPtr<Node> expr;
    RefPtr<VarDeclNode> varDecl;
    RefPtr<StatementNode> statement;
  };

  class ContinueNode : public StatementNode {
  public:
    ContinueNode() : target(0) { }
    ContinueNode(const Identifier &i) : ident(i), target(0) { }
    virtual Completion execute(ExecState*);
    virtual void generateExecCode(CompileState*, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
  protected:
    virtual Node* checkSemantics(SemanticChecker* semanticChecker);
  private:
    Identifier  ident;
    const Node* target;
  };

  class BreakNode : public StatementNode {
  public:
    BreakNode() : target(0) { }
    BreakNode(const Identifier &i) : ident(i), target(0) { }
    virtual Completion execute(ExecState*);
    virtual void generateExecCode(CompileState*, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
  protected:
    virtual Node* checkSemantics(SemanticChecker* semanticChecker);
  private:
    Identifier ident;
    const Node* target;
  };

  class ReturnNode : public StatementNode {
  public:
    ReturnNode(Node *v) : value(v) {}
    virtual Completion execute(ExecState*);
    virtual void generateExecCode(CompileState*, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> value;
  };

  class WithNode : public StatementNode {
  public:
    WithNode(Node *e, StatementNode *s) : expr(e), statement(s) {}
    virtual Completion execute(ExecState*);
    virtual void generateExecCode(CompileState*, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
    virtual bool introducesNewDynamicScope() const { return true; }
  private:
    RefPtr<Node> expr;
    RefPtr<StatementNode> statement;
  };

  class LabelNode : public StatementNode {
  public:
    LabelNode(const Identifier &l, StatementNode *s) : label(l), statement(s) { }
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
    virtual void generateExecCode(CompileState*, CodeBlock& block);
    virtual NodeType type() const { return LabelNodeType; }
  protected:
    virtual Node* checkSemantics(SemanticChecker* semanticChecker);
  private:
    Identifier label;
    RefPtr<StatementNode> statement;
  };

  class ThrowNode : public StatementNode {
  public:
    ThrowNode(Node *e) : expr(e) {}
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
  };

  class TryNode : public StatementNode {
  public:
    TryNode(StatementNode *b, const Identifier &e, StatementNode *c, StatementNode *f)
      : tryBlock(b), exceptionIdent(e), catchBlock(c), finallyBlock(f) { }
    virtual NodeType type() const { return TryNodeType; }
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);

    void recurseVisitNonCatch(NodeVisitor *visitor);
  private:
    RefPtr<StatementNode> tryBlock;
    Identifier exceptionIdent;
    RefPtr<StatementNode> catchBlock;
    RefPtr<StatementNode> finallyBlock;
  };

  class ParameterNode : public Node {
  public:
    // list pointer is tail of a circular list, cracked in the FuncDeclNode/FuncExprNode ctor
    ParameterNode(const Identifier &i) : id(i), next(this) { Parser::noteNodeCycle(this); }
    ParameterNode(ParameterNode *next, const Identifier &i)
      : id(i), next(next->next) { next->next = this; }
    JSValue* evaluate(ExecState*);
    const Identifier& ident() const { return id; }
    ParameterNode *nextParam() const { return next.get(); }
    virtual void streamTo(SourceStream&) const;
    PassRefPtr<ParameterNode> releaseNext() { return next.release(); }
    virtual void breakCycle();

    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    friend class FuncDeclNode;
    friend class FuncExprNode;
    Identifier id;
    ListRefPtr<ParameterNode> next;
  };

  // inherited by ProgramNode

  /**
   This AST node corresponds to the function body in the AST, but is used to
   keep track of much of the information relevant to the whole function,
   such as parameter names and symbol tables. This is because there are both function
   declarations and expressions, so there is no natural single place to put this stuff
   above the body
  */
  class FunctionBodyNode : public BlockNode {
  private:
    struct Symbol {
      Symbol(const Identifier& _name, int _attr, FuncDeclNode* _funcDecl) : name(_name), funcDecl(_funcDecl), attr(_attr) {}
      Symbol() {}
      Identifier    name;
      FuncDeclNode* funcDecl;
      int           attr;
    };

    struct Parameter {
      Parameter() {}
      Parameter(const Identifier &n, size_t id) : name(n), symbolID(id) { }
      Identifier name;
      size_t     symbolID;
    };
  public:
    FunctionBodyNode(SourceElementsNode *);
    int sourceId() { return m_sourceId; }
    const UString& sourceURL() { return m_sourceURL; }

    void compileIfNeeded(CodeType ctype) { if (!m_compiled) compile(ctype); }
    bool isCompiled() const { return m_compiled; }

    virtual void generateExecCode(CompileState*, CodeBlock& block);
    //////////////////////////////////////////////////////////////////////
    // Symbol table functions
    //////////////////////////////////////////////////////////////////////
    SymbolTable& symbolTable() { return m_symbolTable; }
    size_t lookupSymbolID(const Identifier& id) const { return m_symbolTable.get(id.ustring().rep()); }

    int  numLocals()          const { return m_symbolList.size(); }
    int  getLocalAttr(size_t id)       const { return m_symbolList[id].attr; }
    Identifier getLocalName(size_t id) const { return m_symbolList[id].name; }
    FuncDeclNode* getLocalFuncDecl(size_t id) const { return m_symbolList[id].funcDecl; }

    //////////////////////////////////////////////////////////////////////
    // Parameter array functions
    //////////////////////////////////////////////////////////////////////
    void addParam(const Identifier& ident); //Also creates the symbol for the
                                            //parameter, called on creating
                                            //the enclosing expr or decl

    void addVarDecl(const Identifier& ident, int attr, ExecState* exec);
    void addFunDecl(const Identifier& ident, int attr, FuncDeclNode* funcDecl);

    int numParams() const { return m_paramList.size(); }
    size_t paramSymbolID(int pos) const { return m_paramList[pos].symbolID; }
    const Identifier& paramName(int pos) const { return m_paramList[pos].name; }

    Completion execute(ExecState *exec);
  private:
    void compile(CodeType ctype);

    UString m_sourceURL;
    int m_sourceId : 31;
    bool m_compiled : 1;

    size_t addSymbol(const Identifier& ident, int attr, FuncDeclNode* funcDecl = 0);

    // This maps id -> name / etc.
    WTF::Vector<Symbol> m_symbolList;

    // This maps name -> id
    SymbolTable m_symbolTable;

    // The list of parameters, and their local IDs in the
    WTF::Vector<Parameter> m_paramList;

    WTF::Vector<bool> m_shouldMark; // bits saying whether given index should be marked or not

    CodeBlock m_compiledCode;
  };

  class FuncExprNode : public Node {
  public:
    FuncExprNode(const Identifier &i, FunctionBodyNode *b, ParameterNode *p = 0)
      : ident(i), param(p ? p->next.release() : 0), body(b) { if (p) { Parser::removeNodeCycle(param.get()); } addParams(); }
    virtual JSValue *evaluate(ExecState*);
    virtual OpValue generateEvalCode(CompileState* comp, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
    virtual bool introducesNewStaticScope() const { return true; }
  protected:
    virtual Node* checkSemantics(SemanticChecker* semanticChecker);
  private:
    void addParams();
    // Used for streamTo
    friend class PropertyNode;
    Identifier ident;
    RefPtr<ParameterNode> param;
    RefPtr<FunctionBodyNode> body;
  };

  class FuncDeclNode : public StatementNode {
  public:
    FuncDeclNode(const Identifier &i, FunctionBodyNode *b)
      : ident(i), body(b) { addParams(); }
    FuncDeclNode(const Identifier &i, ParameterNode *p, FunctionBodyNode *b)
      : ident(i), param(p->next.release()), body(b) { Parser::removeNodeCycle(param.get()); addParams(); }
    virtual Completion execute(ExecState*);
    virtual void generateExecCode(CompileState*, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
    virtual bool introducesNewStaticScope() const { return true; }

    virtual void processFuncDecl(ExecState*);
    FunctionImp* makeFunctionObject(ExecState*);
  protected:
    virtual Node* checkSemantics(SemanticChecker* semanticChecker);
  private:
    void addParams();
    Identifier ident;
    RefPtr<ParameterNode> param;
    RefPtr<FunctionBodyNode> body;
  };

  // A linked list of source element nodes
  class SourceElementsNode : public StatementNode {
  public:
    // list pointer is tail of a circular list, cracked in the BlockNode (or subclass) ctor
    SourceElementsNode(StatementNode*);
    SourceElementsNode(SourceElementsNode *s1, StatementNode *s2);

    Completion execute(ExecState*);
    virtual void generateExecCode(CompileState*, CodeBlock& block);
    virtual void streamTo(SourceStream&) const;
    PassRefPtr<SourceElementsNode> releaseNext() { return next.release(); }
    virtual void breakCycle();
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    friend class BlockNode;
    friend class CaseClauseNode;
    RefPtr<StatementNode> node;
    ListRefPtr<SourceElementsNode> next;
  };

  class CaseClauseNode : public Node {
  public:
      CaseClauseNode(Node *e) : expr(e) { }
      CaseClauseNode(Node *e, SourceElementsNode *s)
      : expr(e), source(s->next.release()) { Parser::removeNodeCycle(source.get()); }
      JSValue* evaluate(ExecState*);
      Completion evalStatements(ExecState*);
      virtual void streamTo(SourceStream&) const;
      virtual void recurseVisit(NodeVisitor *visitor);
  private:
      RefPtr<Node> expr;
      RefPtr<SourceElementsNode> source;
  };

  class ClauseListNode : public Node {
  public:
      // list pointer is tail of a circular list, cracked in the CaseBlockNode ctor
      ClauseListNode(CaseClauseNode *c) : clause(c), next(this) { Parser::noteNodeCycle(this); }
      ClauseListNode(ClauseListNode *n, CaseClauseNode *c)
      : clause(c), next(n->next) { n->next = this; }
      JSValue* evaluate(ExecState*);
      CaseClauseNode *getClause() const { return clause.get(); }
      ClauseListNode *getNext() const { return next.get(); }
      virtual void streamTo(SourceStream&) const;
      PassRefPtr<ClauseListNode> releaseNext() { return next.release(); }
      virtual void breakCycle();
      virtual void recurseVisit(NodeVisitor *visitor);
  private:
      friend class CaseBlockNode;
      RefPtr<CaseClauseNode> clause;
      ListRefPtr<ClauseListNode> next;
  };

  class CaseBlockNode : public Node {
  public:
      CaseBlockNode(ClauseListNode *l1, CaseClauseNode *d, ClauseListNode *l2);
      JSValue* evaluate(ExecState*);
      Completion evalBlock(ExecState *exec, JSValue *input);
      virtual void streamTo(SourceStream&) const;
      virtual void recurseVisit(NodeVisitor *visitor);
  private:
      RefPtr<ClauseListNode> list1;
      RefPtr<CaseClauseNode> def;
      RefPtr<ClauseListNode> list2;
  };

  class SwitchNode : public StatementNode {
  public:
      SwitchNode(Node *e, CaseBlockNode *b) : expr(e), block(b) { }
      virtual Completion execute(ExecState*);
      virtual void streamTo(SourceStream&) const;
      virtual void recurseVisit(NodeVisitor *visitor);
      virtual bool isSwitchStatement() const { return true; }
  private:
      RefPtr<Node> expr;
      RefPtr<CaseBlockNode> block;
  };

  class ProgramNode : public FunctionBodyNode {
  public:
    ProgramNode(SourceElementsNode *s);
  };

  /**
   This node represents statically detectable errors that must be reported
   at runtime. The semantic checker splices it in above the original node
   in the program, and it does the error reporting; the child node is
   used for text serialization.

   Note: we do not visit kids of this node in the visitor interface,
   since they do not run, anyway
  */
  class ErrorNode : public StatementNode {
  public:
    ErrorNode(StatementNode* k, ErrorType e, const UString& m) :
        kid(k), errorType(e), message(m) { copyDebugInfo(k); }
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
  private:
    RefPtr<Node> kid;
    ErrorType    errorType;
    UString      message;
  };

  class PackageNameNode : public Node {
  public:
    PackageNameNode(const Identifier &i) : names(0), id(i) { }
    PackageNameNode(PackageNameNode *n,
                    const Identifier &i) : names(n), id(i) { }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);

    Completion loadSymbol(ExecState* exec, bool wildcard);
    PackageObject* resolvePackage(ExecState* exec);

  private:
    PackageObject* resolvePackage(ExecState* exec,
				  JSObject* baseObject, Package* basePackage);
    RefPtr<PackageNameNode> names;
    Identifier id;
  };

  class ImportStatement : public StatementNode {
  public:
    ImportStatement(PackageNameNode *n) : name(n), wld(false) {}
    void enableWildcard() { wld = true; }
    void setAlias(const Identifier &a) { al = a; }
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    virtual void processVarDecl (ExecState* state);
    RefPtr<PackageNameNode> name;
    Identifier al;
    bool wld;
  };

} // namespace

#endif
