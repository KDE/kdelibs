/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2002 Harri Porten (porten@kde.org)
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

#include "nodes.h"
#include <config.h>
#include "scriptfunction.h"

#include <math.h>
#include <stdio.h>
#ifdef KJS_DEBUG_MEM
#include <typeinfo>
#endif

//#include <iostream>

#include "context.h"
#include "debugger.h"
#include "function_object.h"
#include "lexer.h"
#include "operations.h"
#include "package.h"
#include "PropertyNameArray.h"
#include <wtf/AlwaysInline.h>
#include <wtf/HashSet.h>
#include <wtf/HashCountedSet.h>
#include <wtf/MathExtras.h>


namespace KJS {

#define KJS_BREAKPOINT \
  if (Debugger::debuggersPresent > 0 && !hitStatement(exec)) \
    return Completion(Normal);

#define KJS_ABORTPOINT \
  if (Debugger::debuggersPresent > 0 && \
      exec->dynamicInterpreter()->imp()->debugger() && \
      exec->dynamicInterpreter()->imp()->debugger()->imp()->aborted()) \
    return Completion(Normal);

#define KJS_CHECKEXCEPTION \
  if (exec->hadException()) { \
    JSValue *ex = exec->exception(); \
    exec->clearException(); \
    handleException(exec, ex); \
    return Completion(Throw, ex); \
  } \
  if (Collector::isOutOfMemory()) \
    return Completion(Throw, Error::create(exec, GeneralError, "Out of memory"));

#define KJS_CHECKEXCEPTIONVALUE \
  if (exec->hadException()) { \
    handleException(exec); \
    return jsUndefined(); \
  } \
  if (Collector::isOutOfMemory()) \
    return jsUndefined(); // will be picked up by KJS_CHECKEXCEPTION

#define KJS_CHECKEXCEPTIONREFERENCE \
  if (exec->hadException()) { \
    handleException(exec); \
    ref.found = true; \
    return ref; \
  } \
  if (Collector::isOutOfMemory()) { \
    ref.found = true; \
    return ref; \
  }


#define KJS_CHECKEXCEPTIONLIST \
  if (exec->hadException()) { \
    handleException(exec); \
    return List(); \
  } \
  if (Collector::isOutOfMemory()) \
    return List(); // will be picked up by KJS_CHECKEXCEPTION

// ------------------------------ Node -----------------------------------------


#ifndef NDEBUG
struct NodeCounter {
    static unsigned count;
    ~NodeCounter()
    {
        if (count)
            fprintf(stderr, "LEAK: %d KJS::Node\n", count);
    }
};
unsigned NodeCounter::count = 0;
static NodeCounter nodeCounter;
#endif

static HashSet<Node*>* newNodes;
static HashCountedSet<Node*>* nodeExtraRefCounts;

Node::Node()
{
#ifndef NDEBUG
    ++NodeCounter::count;
#endif
  m_line = lexer().lineNo();
  if (!newNodes)
      newNodes = new HashSet<Node*>;
  newNodes->add(this);
}

Node::~Node()
{
#ifndef NDEBUG
    --NodeCounter::count;
#endif
}

void Node::ref()
{
    // bumping from 0 to 1 is just removing from the new nodes set
    if (newNodes) {
        HashSet<Node*>::iterator it = newNodes->find(this);
        if (it != newNodes->end()) {
            newNodes->remove(it);
            ASSERT(!nodeExtraRefCounts || !nodeExtraRefCounts->contains(this));
            return;
        }
    }

    ASSERT(!newNodes || !newNodes->contains(this));

    if (!nodeExtraRefCounts)
        nodeExtraRefCounts = new HashCountedSet<Node*>;
    nodeExtraRefCounts->add(this);
}

void Node::deref()
{
    ASSERT(!newNodes || !newNodes->contains(this));

    if (!nodeExtraRefCounts) {
        delete this;
        return;
    }

    HashCountedSet<Node*>::iterator it = nodeExtraRefCounts->find(this);
    if (it == nodeExtraRefCounts->end())
        delete this;
    else
        nodeExtraRefCounts->remove(it);
}

unsigned Node::refcount()
{
    if (newNodes && newNodes->contains(this)) {
        ASSERT(!nodeExtraRefCounts || !nodeExtraRefCounts->contains(this));
        return 0;
    }

    ASSERT(!newNodes || !newNodes->contains(this));

    if (!nodeExtraRefCounts)
        return 1;

    return 1 + nodeExtraRefCounts->count(this);
}

void Node::clearNewNodes()
{
    if (!newNodes)
        return;

#ifndef NDEBUG
    HashSet<Node*>::iterator end = newNodes->end();
    for (HashSet<Node*>::iterator it = newNodes->begin(); it != end; ++it)
        ASSERT(!nodeExtraRefCounts || !nodeExtraRefCounts->contains(*it));
#endif
    deleteAllValues(*newNodes);
    delete newNodes;
    newNodes = 0;
}

static void substitute(UString &string, const UString &substring)
{
    int position = string.find("%s");
    assert(position != -1);
    string = string.substr(0, position) + substring + string.substr(position + 2);
}

static inline int currentSourceId(ExecState* exec)
{
    return exec->context()->currentBody()->sourceId();
}

static inline const UString& currentSourceURL(ExecState* exec)
{
    return exec->context()->currentBody()->sourceURL();
}

Completion Node::createErrorCompletion(ExecState* exec, ErrorType e, const UString& msg)
{
    return Completion(Throw, Error::create(exec, e, msg, lineNo(), currentSourceId(exec), currentSourceURL(exec)));
}

Completion Node::createErrorCompletion(ExecState *exec, ErrorType e, const UString& msg, const Identifier &ident)
{
    UString message = msg;
    substitute(message, ident.ustring());
    return Completion(Throw, Error::create(exec, e, message, lineNo(), currentSourceId(exec), currentSourceURL(exec)));
}

JSValue* Node::throwError(ExecState* exec, ErrorType e, const UString& msg)
{
    return KJS::throwError(exec, e, msg, lineNo(), currentSourceId(exec), currentSourceURL(exec));
}

JSValue* Node::throwError(ExecState* exec, ErrorType e, const UString& msg, const char* string) 
{ 
    UString message = msg; 
    substitute(message, string); 
    return KJS::throwError(exec, e, message, lineNo(), currentSourceId(exec), currentSourceURL(exec)); 
}

JSValue* Node::throwError(ExecState* exec, ErrorType e, const UString& msg, JSValue* v, Node* expr)
{
    UString message = msg;
    substitute(message, v->toString(exec));
    substitute(message, expr->toString());
    return KJS::throwError(exec, e, message, lineNo(), currentSourceId(exec), currentSourceURL(exec));
}


JSValue* Node::throwError(ExecState* exec, ErrorType e, const UString& msg, const Identifier& label)
{
    UString message = msg;
    substitute(message, label.ustring());
    return KJS::throwError(exec, e, message, lineNo(), currentSourceId(exec), currentSourceURL(exec));
}

JSValue* Node::throwError(ExecState* exec, ErrorType e, const UString& msg, JSValue* v, Node* e1, Node* e2)
{
    UString message = msg;
    substitute(message, v->toString(exec));
    substitute(message, e1->toString());
    substitute(message, e2->toString());
    return KJS::throwError(exec, e, message, lineNo(), currentSourceId(exec), currentSourceURL(exec));
}

JSValue* Node::throwError(ExecState* exec, ErrorType e, const UString& msg, JSValue* v, Node* expr, const Identifier& label)
{
    UString message = msg;
    substitute(message, v->toString(exec));
    substitute(message, expr->toString());
    substitute(message, label.ustring());
    return KJS::throwError(exec, e, message, lineNo(), currentSourceId(exec), currentSourceURL(exec));
}

JSValue* Node::throwError(ExecState* exec, ErrorType e, const UString& msg, JSValue* v, const Identifier& label)
{
    UString message = msg;
    substitute(message, v->toString(exec));
    substitute(message, label.ustring());
    return KJS::throwError(exec, e, message, lineNo(), currentSourceId(exec), currentSourceURL(exec));
}

JSValue* Node::throwUndefinedVariableError(ExecState* exec, const Identifier& ident)
{
    return throwError(exec, ReferenceError, "Cannot find variable: %s", ident);
}

void Node::handleException(ExecState *exec)
{
    handleException(exec, exec->exception());
}

void Node::handleException(ExecState* exec, JSValue* exceptionValue)
{
    Debugger* dbg = exec->dynamicInterpreter()->debugger();
    if (dbg && !dbg->hasHandledException(exec, exceptionValue)) {
        bool cont = dbg->exception(exec, currentSourceId(exec), m_line, exceptionValue);
        if (!cont)
            dbg->imp()->abort();
    }
}

Node *Node::nodeInsideAllParens()
{
    return this;
}

void Node::copyDebugInfo(Node* otherNode)
{
    m_line = otherNode->m_line;
}

class VarDeclVisitor: public NodeVisitor {
  private:
    ExecState* m_exec;
  public:
    VarDeclVisitor(ExecState* exec) : m_exec(exec)
    {}

    virtual Node* visit(Node* node) {
      node->processVarDecl(m_exec);

      //Do not recurse inside function bodies...
      if (node->introducesNewStaticScope())
        return 0;

      return NodeVisitor::visit(node);
    }
};

class FuncDeclVisitor: public NodeVisitor {
  private:
    ExecState* m_exec;
  public:
    FuncDeclVisitor(ExecState* exec) : m_exec(exec)
    {}

    virtual Node* visit(Node* node) {
      node->processFuncDecl(m_exec);

      if (node->introducesNewStaticScope())
        return 0;

      return NodeVisitor::visit(node);
    }
};

void Node::processDecls(ExecState *exec) {
  VarDeclVisitor vVisit(exec);
  vVisit.visit(this);

  FuncDeclVisitor fVisit(exec);
  fVisit.visit(this);
}

void Node::processVarDecl (ExecState*)
{}

void Node::processFuncDecl(ExecState*)
{}

Node* Node::optimizeLocalAccess(ExecState* /*exec*/, FunctionBodyNode* /*node*/)
{
  return 0;
}

// ------------------------------ NodeVisitor ----------------------------------
Node* NodeVisitor::visit(Node *node) {
  node->recurseVisit(this);
  return 0;
}

// ------------------------------ StatementNode --------------------------------

StatementNode::StatementNode()
    : m_lastLine(-1)
{
    m_line = -1;
}

void StatementNode::setLoc(int firstLine, int lastLine)
{
    m_line = firstLine;
    m_lastLine = lastLine;
}

void StatementNode::copyDebugInfo(StatementNode* otherNode)
{
    m_line     = otherNode->m_line;
    m_lastLine = otherNode->m_lastLine;
}

// return true if the debugger wants us to stop at this point
bool StatementNode::hitStatement(ExecState* exec)
{
  Debugger *dbg = exec->dynamicInterpreter()->debugger();
  if (dbg)
    return dbg->atStatement(exec, currentSourceId(exec), firstLine(), lastLine());
  else
    return true; // continue
}

Node* StatementNode::createErrorNode(ErrorType e, const UString& msg)
{
    return new ErrorNode(this, e, msg);
}

Node* StatementNode::createErrorNode(ErrorType e, const UString& msg, const Identifier &ident)
{
    UString message = msg;
    substitute(message, ident.ustring());
    return new ErrorNode(this, e, message);
}



// ------------------------------ NullNode -------------------------------------

JSValue *NullNode::evaluate(ExecState *)
{
  return jsNull();
}

// ------------------------------ BooleanNode ----------------------------------

JSValue *BooleanNode::evaluate(ExecState *)
{
    return jsBoolean(value());
}

// ------------------------------ NumberNode -----------------------------------

JSValue *NumberNode::evaluate(ExecState *)
{
    return jsNumber(value());
}

// ------------------------------ StringNode -----------------------------------

JSValue *StringNode::evaluate(ExecState *)
{
    return jsString(value());
}

// ------------------------------ RegExpNode -----------------------------------

JSValue *RegExpNode::evaluate(ExecState *exec)
{
  List list;
  list.append(jsString(pattern));
  list.append(jsString(flags));

  JSObject *reg = exec->lexicalInterpreter()->builtinRegExp();
  return reg->construct(exec,list);
}

// ------------------------------ ThisNode -------------------------------------

// ECMA 11.1.1
JSValue *ThisNode::evaluate(ExecState *exec)
{
  return exec->context()->thisValue();
}

// ------------------------------ LocalVarAccessNode ----------------------------

JSValue* LocalVarAccessNode::evaluate(ExecState *exec)
{
  ActivationImp* scope = static_cast<ActivationImp*>(exec->context()->activationObject());
  return *scope->getLocalDirect(index);
}

Reference LocalVarAccessNode::evaluateReference(ExecState *exec)
{
  ActivationImp* scope = static_cast<ActivationImp*>(exec->context()->activationObject());
  Reference ref(ident); //### FIXME: the ident is for delete, but it makes it take the super-slow path..
  ref.base  = scope;
  ref.found = true;
  ref.propertySlot.setPotentiallyWriteable();
  ref.propertySlot.setValueSlot(scope, scope->getLocalDirect(index), scope->isLocalReadOnly(index));
  return ref;
}

// ------------------------------ VarAccessNode --------------------------------

JSValue* VarAccessNode::evaluate(ExecState* exec) {
  const ScopeChain& chain = exec->context()->scopeChain();
  ScopeChainIterator iter = chain.begin();
  ScopeChainIterator end = chain.end();

  // we must always have something in the scope chain
  assert(iter != end);

  PropertySlot slot;
  JSObject *scopeObj;
  do {
    scopeObj = *iter;

    if (scopeObj->getPropertySlot(exec, ident, slot))
      return slot.getValue(exec, scopeObj, ident);

    ++iter;
  } while (iter != end);

  return throwUndefinedVariableError(exec, ident);
}

Reference VarAccessNode::evaluateReference(ExecState* exec) {
  Reference ref(ident);
  ref.found = true;

  const ScopeChain& chain = exec->context()->scopeChain();
  ScopeChainIterator iter = chain.begin();
  ScopeChainIterator end = chain.end();

  // we must always have something in the scope chain
  assert(iter != end);

  JSObject *scopeObj;
  do {
    scopeObj = *iter;
    ref.base = scopeObj;

    if (scopeObj->getPropertySlot(exec, ident, ref.propertySlot))
      return ref;

    ++iter;
  } while (iter != end);

  ref.found = false;
  ref.propertySlot.setUndefined(scopeObj); //outermost scope.
  return ref;
}

Node* VarAccessNode::optimizeLocalAccess(ExecState *exec, FunctionBodyNode* node)
{
  int index = node->lookupSymbolID(ident);
  Node* out = 0;

  // If the symbol isn't local, we can still optimize 
  // a bit, and skip the local scope for lookup.
  // (unless 'eval' injects a new local, but NonLocalResolver
  // is careful about that)
  if (index == -1) {
    if (ident == exec->propertyNames().arguments)
      out = 0; //Dynamic property..
    else
      out = new NonLocalVarAccessNode(ident);
  } else {
      out = new LocalVarAccessNode(ident, index);
  }

  if (out)
    out->copyDebugInfo(this);
  return out;
}

int VarAccessNode::localID(FunctionBodyNode* node)
{
  return node->lookupSymbolID(ident);
}

// ------------------------------ NonLocalVarAccessNode ------------------------

JSValue* NonLocalVarAccessNode::evaluate(ExecState* exec) {
  const ScopeChain& chain = exec->context()->scopeChain();
  ScopeChainIterator iter = chain.begin();
  ScopeChainIterator end = chain.end();

  // we must always have something in the scope chain
  assert(iter != end);

  PropertySlot slot;
  JSObject *scopeObj = *iter;

  if (!scopeObj->isLocalInjected()) {
   // Unless eval introduced new variables dynamically,
   // we know this isn't in the top scope
   ++iter;
  }

  do {
    scopeObj = *iter;

    if (scopeObj->getPropertySlot(exec, ident, slot))
      return slot.getValue(exec, scopeObj, ident);

    ++iter;
  } while (iter != end);

  return throwUndefinedVariableError(exec, ident);
}

Reference NonLocalVarAccessNode::evaluateReference(ExecState* exec) {
  Reference ref(ident);
  ref.found = true;

  const ScopeChain& chain = exec->context()->scopeChain();
  ScopeChainIterator iter = chain.begin();
  ScopeChainIterator end = chain.end();

  // we must always have something in the scope chain
  assert(iter != end);

  JSObject *scopeObj = *iter;
  if (!scopeObj->isLocalInjected()) {
   // Unless eval introduced new variables dynamically,
   // we know this isn't in the top scope
   ++iter;
  }

  do {
    scopeObj = *iter;
    ref.base = scopeObj;

    if (scopeObj->getPropertySlot(exec, ident, ref.propertySlot))
      return ref;

    ++iter;
  } while (iter != end);

  ref.base = scopeObj;
  ref.found = false;
  ref.propertySlot.setUndefined(scopeObj); //outermost scope.
  return ref;
}

// ------------------------------ GroupNode ------------------------------------

// ECMA 11.1.6
JSValue *GroupNode::evaluate(ExecState *exec)
{
  return group->evaluate(exec);
}

Node *GroupNode::nodeInsideAllParens()
{
    Node *n = this;
    do
        n = static_cast<GroupNode *>(n)->group.get();
    while (n->isGroupNode());
    return n;
}

void GroupNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, group);
}


// ------------------------------ ElementNode ----------------------------------

// ECMA 11.1.4
JSValue *ElementNode::evaluate(ExecState *exec)
{
  JSObject *array = exec->lexicalInterpreter()->builtinArray()->construct(exec, List::empty());
  int length = 0;
  for (ElementNode *n = this; n; n = n->next.get()) {
    JSValue *val = n->node->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE
    length += n->elision;
    array->put(exec, length++, val);
  }
  return array;
}

void ElementNode::breakCycle()
{
    next = 0;
}

void ElementNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, next);
    recurseVisitLink(visitor, node);
}


// ------------------------------ ArrayNode ------------------------------------

// ECMA 11.1.4
JSValue *ArrayNode::evaluate(ExecState *exec)
{
  JSObject *array;
  int length;

  if (element) {
    array = static_cast<JSObject*>(element->evaluate(exec));
    KJS_CHECKEXCEPTIONVALUE
    length = opt ? array->get(exec, exec->propertyNames().length)->toInt32(exec) : 0;
  } else {
    JSValue *newArr = exec->lexicalInterpreter()->builtinArray()->construct(exec,List::empty());
    array = static_cast<JSObject*>(newArr);
    length = 0;
  }

  if (opt)
    array->put(exec, exec->propertyNames().length, jsNumber(elision + length), DontEnum | DontDelete);

  return array;
}

void ArrayNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, element);
}

// ------------------------------ ObjectLiteralNode ----------------------------

// ECMA 11.1.5
JSValue *ObjectLiteralNode::evaluate(ExecState *exec)
{
  if (list)
    return list->evaluate(exec);

  return exec->lexicalInterpreter()->builtinObject()->construct(exec,List::empty());
}

void ObjectLiteralNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, list);
}

// ------------------------------ PropertyListNode -----------------------------

// ECMA 11.1.5
JSValue *PropertyListNode::evaluate(ExecState *exec)
{
  JSObject *obj = exec->lexicalInterpreter()->builtinObject()->construct(exec, List::empty());

  for (PropertyListNode *p = this; p; p = p->next.get()) {
    JSValue *n = p->node->name->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE
    JSValue *v = p->node->assign->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE

    Identifier propertyName = Identifier(n->toString(exec));
    switch (p->node->type) {
      case PropertyNode::Getter:
        assert(v->isObject());
        obj->defineGetter(exec, propertyName, static_cast<JSObject *>(v));
        break;
      case PropertyNode::Setter:
        assert(v->isObject());
        obj->defineSetter(exec, propertyName, static_cast<JSObject *>(v));
        break;
      case PropertyNode::Constant:
        obj->put(exec, propertyName, v);
        break;
    }
  }

  return obj;
}

void PropertyListNode::breakCycle()
{
    next = 0;
}

void PropertyListNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, node);
    recurseVisitLink(visitor, next);
}


// ------------------------------ PropertyNode -----------------------------
// ECMA 11.1.5
JSValue *PropertyNode::evaluate(ExecState*)
{
  assert(false);
  return jsNull();
}

void PropertyNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, name);
    recurseVisitLink(visitor, assign);
}

// ---------------------------- PropertyNameNode -------------------------------

// ECMA 11.1.5
JSValue *PropertyNameNode::evaluate(ExecState*)
{
  JSValue *s;

  if (str.isNull()) {
    s = jsString(UString::from(numeric));
  } else {
    s = jsString(str.ustring());
  }

  return s;
}

// ------------------------------ BracketAccessorNode --------------------------------

// ECMA 11.2.1a
JSValue *BracketAccessorNode::evaluate(ExecState *exec)
{
  JSValue *v1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  JSValue *v2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  JSObject *o = v1->toObject(exec);
  uint32_t i;
  if (v2->getUInt32(i))
    return o->get(exec, i);
  return o->get(exec, Identifier(v2->toString(exec)));
}

Reference BracketAccessorNode::evaluateReference(ExecState* exec)
{
  Reference  ref;
  JSValue *v1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONREFERENCE

  JSValue *v2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONREFERENCE

  JSObject *o = v1->toObject(exec);
  ref.base  = o;
  ref.found = true; //Always for these sorts of things..
  
  // Special case for indices, to avoid converting double->string
  uint32_t i;
  if (v2->getUInt32(i)) {
    ref.ident = Identifier::from(i);
    if (!o->getPropertySlot(exec, i, ref.propertySlot))
      ref.propertySlot.setUndefined(o);
    return ref;
  }

  //Otherwise do it symbolically..
  Identifier ident(v2->toString(exec));
  ref.ident = ident;
  if (!o->getPropertySlot(exec, ident, ref.propertySlot))
    ref.propertySlot.setUndefined(o);
  return ref;
}

void BracketAccessorNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr1);
  recurseVisitLink(visitor, expr2);
}

// ------------------------------ DotAccessorNode --------------------------------

// ECMA 11.2.1b
JSValue *DotAccessorNode::evaluate(ExecState *exec)
{
  JSValue *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  return v->toObject(exec)->get(exec, ident);
}

Reference DotAccessorNode::evaluateReference(ExecState* exec)
{
  Reference  ref(ident);
  JSValue *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONREFERENCE

  JSObject *o = v->toObject(exec);

  ref.found = true; //Always for these sorts of things..
  ref.base  = o;
  if (!o->getPropertySlot(exec, ident, ref.propertySlot)) {
    //Currently undefined, note that, but don't trigger
    //a "not found" error
    ref.propertySlot.setUndefined(o);
  }
  return ref;
}


void DotAccessorNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, expr);
}

// ------------------------------ ArgumentListNode -----------------------------

JSValue *ArgumentListNode::evaluate(ExecState *)
{
  assert(0);
  return 0; // dummy, see evaluateList()
}

// ECMA 11.2.4
List ArgumentListNode::evaluateList(ExecState *exec)
{
  List l;

  for (ArgumentListNode *n = this; n; n = n->next.get()) {
    JSValue *v = n->expr->evaluate(exec);
    KJS_CHECKEXCEPTIONLIST
    l.append(v);
  }

  return l;
}

void ArgumentListNode::breakCycle()
{
    next = 0;
}

void ArgumentListNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, next);
    recurseVisitLink(visitor, expr);
}

// ------------------------------ ArgumentsNode --------------------------------

JSValue *ArgumentsNode::evaluate(ExecState *)
{
  assert(0);
  return 0; // dummy, see evaluateList()
}

void ArgumentsNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, list);
}


// ------------------------------ NewExprNode ----------------------------------

// ECMA 11.2.2

JSValue *NewExprNode::evaluate(ExecState *exec)
{
  JSValue *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  List argList;
  if (args) {
    argList = args->evaluateList(exec);
    KJS_CHECKEXCEPTIONVALUE
  }

  if (!v->isObject()) {
    return throwError(exec, TypeError, "Value %s (result of expression %s) is not an object. Cannot be used with new.", v, expr.get());
  }

  JSObject *constr = static_cast<JSObject*>(v);
  if (!constr->implementsConstruct()) {
    return throwError(exec, TypeError, "Value %s (result of expression %s) is not a constructor. Cannot be used with new.", v, expr.get());
  }

  return constr->construct(exec, argList);
}

void NewExprNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, expr);
    recurseVisitLink(visitor, args);
}

static const char* notAnObjectMessage()
{
  return "Value %s (result of expression %s) is not an object.";
}

static const char* notCallableMessage()
{
  return "Object %s (result of expression %s) does not allow calls.";
}

// ECMA 11.2.3
JSValue *FunctionCallValueNode::evaluate(ExecState *exec)
{
  JSValue *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  if (!v->isObject()) {
    return throwError(exec, TypeError, notAnObjectMessage(), v, expr.get());
  }

  JSObject *func = static_cast<JSObject*>(v);

  if (!func->implementsCall()) {
    return throwError(exec, TypeError, notCallableMessage(), v, expr.get());
  }

  List argList = args->evaluateList(exec);
  KJS_CHECKEXCEPTIONVALUE

  JSObject *thisObj =  exec->dynamicInterpreter()->globalObject();

  return func->call(exec, thisObj, argList);
}

void FunctionCallValueNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, expr);
    recurseVisitLink(visitor, args);
}

JSValue *FunctionCallReferenceNode::evaluate(ExecState *exec)
{
  //Evaluate the location
  Reference ref = expr->evaluateReference(exec);

  //handle error..
  if (!ref.found)
    return throwUndefinedVariableError(exec, ref.ident);

  KJS_CHECKEXCEPTIONVALUE

  JSValue *v = ref.read(exec);
  KJS_CHECKEXCEPTIONVALUE

  if (!v->isObject()) {
    return throwError(exec, TypeError, notAnObjectMessage(), v, expr.get());
  }

  JSObject *func = static_cast<JSObject*>(v);

  if (!func->implementsCall()) {
    return throwError(exec, TypeError, notCallableMessage(), v, expr.get());
  }

  List argList = args->evaluateList(exec);
  KJS_CHECKEXCEPTIONVALUE

  JSObject *thisObj = ref.base;
  if (thisObj->isActivation())
    thisObj =  exec->dynamicInterpreter()->globalObject();

  return func->call(exec, thisObj, argList);
}

void FunctionCallReferenceNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, expr);
    recurseVisitLink(visitor, args);
}

// ECMA 11.2.3
JSValue *FunctionCallBracketNode::evaluate(ExecState *exec)
{
  JSValue *baseVal = base->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  JSValue *subscriptVal = subscript->evaluate(exec);

  JSObject *baseObj = baseVal->toObject(exec);
  uint32_t i;
  PropertySlot slot;

  JSValue *funcVal;
  if (subscriptVal->getUInt32(i)) {
    if (baseObj->getPropertySlot(exec, i, slot))
      funcVal = slot.getValue(exec, baseObj, i);
    else
      funcVal = jsUndefined();
  } else {
    Identifier ident(subscriptVal->toString(exec));
    if (baseObj->getPropertySlot(exec, ident, slot))
      funcVal = baseObj->get(exec, ident);
    else
      funcVal = jsUndefined();
  }

  KJS_CHECKEXCEPTIONVALUE

  if (!funcVal->isObject()) {
    return throwError(exec, TypeError, "Value %s (result of expression %s[%s]) is not object.", funcVal, base.get(), subscript.get());
  }

  JSObject *func = static_cast<JSObject*>(funcVal);

  if (!func->implementsCall()) {
    return throwError(exec, TypeError, "Object %s (result of expression %s[%s]) does not allow calls.", funcVal, base.get(), subscript.get());
  }

  List argList = args->evaluateList(exec);
  KJS_CHECKEXCEPTIONVALUE

  JSObject *thisObj = baseObj;
  assert(thisObj);
  assert(thisObj->isObject());
  assert(!thisObj->isActivation());

  return func->call(exec, thisObj, argList);
}

void FunctionCallBracketNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, base);
    recurseVisitLink(visitor, subscript);
    recurseVisitLink(visitor, args);
}


static const char *dotExprNotAnObjectString()
{
  return "Value %s (result of expression %s.%s) is not object.";
}

static const char *dotExprDoesNotAllowCallsString()
{
  return "Object %s (result of expression %s.%s) does not allow calls.";
}

// ECMA 11.2.3
JSValue *FunctionCallDotNode::evaluate(ExecState *exec)
{
  JSValue *baseVal = base->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  JSObject *baseObj = baseVal->toObject(exec);
  PropertySlot slot;
  JSValue *funcVal = baseObj->getPropertySlot(exec, ident, slot) ? slot.getValue(exec, baseObj, ident) : jsUndefined();
  KJS_CHECKEXCEPTIONVALUE

  if (!funcVal->isObject())
    return throwError(exec, TypeError, dotExprNotAnObjectString(), funcVal, base.get(), ident);

  JSObject *func = static_cast<JSObject*>(funcVal);

  if (!func->implementsCall())
    return throwError(exec, TypeError, dotExprDoesNotAllowCallsString(), funcVal, base.get(), ident);

  List argList = args->evaluateList(exec);
  KJS_CHECKEXCEPTIONVALUE

  JSObject *thisObj = baseObj;
  assert(thisObj);
  assert(thisObj->isObject());
  assert(!thisObj->isActivation());

  return func->call(exec, thisObj, argList);
}

void FunctionCallDotNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, base);
    recurseVisitLink(visitor, args);
}

// ECMA 11.3

// ------------------------------ PostfixNode ----------------------------------

class LocalPostfixNode : public PostfixNode
{
public:
  LocalPostfixNode(LocationNode *l, Operator o, int idx) : PostfixNode(l, o), index(idx) {}

  JSValue* evaluate(ExecState* exec) {
    ActivationImp* scope = static_cast<ActivationImp*>(exec->context()->activationObject());
    JSValue* v = *scope->getLocalDirect(index);
    double n = v->toNumber(exec);
    double newValue = (m_oper == OpPlusPlus) ? n + 1 : n - 1;
    scope->putLocalChecked(index, jsNumber(newValue));
    return jsNumber(n);
  }
private:
  int index;
};

JSValue* PostfixNode::evaluate(ExecState* exec)
{
  //Evaluate the location
  Reference ref = m_loc->evaluateReference(exec);

  //handle error..
  if (!ref.found)
    return throwUndefinedVariableError(exec, ref.ident);

  KJS_CHECKEXCEPTIONVALUE

  //do the op
  JSValue *v = ref.read(exec);
  double n = v->toNumber(exec);
  double newValue = (m_oper == OpPlusPlus) ? n + 1 : n - 1;
  ref.write(exec, jsNumber(newValue));
  return jsNumber(n);
}

void PostfixNode::recurseVisit(NodeVisitor *visitor)
{
   Node::recurseVisitLink(visitor, m_loc);
}

Node* PostfixNode::optimizeLocalAccess(ExecState* /*exec*/, FunctionBodyNode* node)
{
  if (m_loc->isVarAccessNode()) {
    VarAccessNode* ident = static_cast<VarAccessNode*>(m_loc.get());
    int id = ident->localID(node);
    if (id != -1) {
      Node* out = new LocalPostfixNode(m_loc.get(), m_oper, id);
      out->copyDebugInfo(this);
      return out;
    }
  }
  return 0;
}

JSValue* PostfixErrorNode::evaluate(ExecState* exec) 
{ 
    throwError(exec, ReferenceError, "Postfix %s operator applied to value that is not a reference.", 
	       m_oper == OpPlusPlus ? "++" : "--"); 
    handleException(exec); 
    return jsUndefined(); 
} 

// ECMA 11.4.1

// ------------------------------ DeleteReferenceNode -------------------------------
JSValue* DeleteReferenceNode::evaluate(ExecState* exec)
{
  //Evaluate the location
  Reference ref = loc->evaluateReference(exec);
  KJS_CHECKEXCEPTIONVALUE

  //handle error..
  if (!ref.found)
    return jsBoolean(true);
  else
    return jsBoolean(ref.deleteProperty(exec));
}

void DeleteReferenceNode::recurseVisit(NodeVisitor *visitor)
{
   Node::recurseVisitLink(visitor, loc);
}

// ------------------------------ DeleteValueNode -----------------------------------
JSValue *DeleteValueNode::evaluate(ExecState *exec)
{
  m_expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  // delete on a non-location expression ignores the value and returns true
  return jsBoolean(true);
}

void DeleteValueNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, m_expr);
}

// ------------------------------ VoidNode -------------------------------------

// ECMA 11.4.2
JSValue *VoidNode::evaluate(ExecState *exec)
{
  expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return jsUndefined();
}

void VoidNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr);
}

// ECMA 11.4.3

// ------------------------------ TypeOfValueNode -----------------------------------

static JSValue *typeStringForValue(JSValue *v)
{
    switch (v->type()) {
    case UndefinedType:
        return jsString("undefined");
    case NullType:
        return jsString("object");
    case BooleanType:
        return jsString("boolean");
    case NumberType:
        return jsString("number");
    case StringType:
        return jsString("string");
    default:
        if (v->isObject()) {
            // Return "undefined" for objects that should be treated
            // as null when doing comparisons.
            if (static_cast<JSObject*>(v)->masqueradeAsUndefined())
                return jsString("undefined");
            else if (static_cast<JSObject*>(v)->implementsCall())
                return jsString("function");
        }

        return jsString("object");
    }
}

JSValue* TypeOfReferenceNode::evaluate(ExecState* exec)
{
  //Evaluate the location
  Reference ref = loc->evaluateReference(exec);
  KJS_CHECKEXCEPTIONVALUE

  //handle error..
  if (!ref.found)
    return jsString("undefined");
  else
    return typeStringForValue(ref.read(exec));
}

void TypeOfReferenceNode::recurseVisit(NodeVisitor *visitor)
{
   Node::recurseVisitLink(visitor, loc);
}

// ------------------------------ TypeOfValueNode -----------------------------------

JSValue *TypeOfValueNode::evaluate(ExecState *exec)
{
  JSValue *v = m_expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return typeStringForValue(v);
}

void TypeOfValueNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, m_expr);
}


// ECMA 11.4.4 and 11.4.5

// ------------------------------ PrefixNode ----------------------------------------

class LocalPrefixNode : public PrefixNode
{
public:
  LocalPrefixNode(LocationNode *l, Operator o, int idx) : PrefixNode(l, o), index(idx) {}

  JSValue* evaluate(ExecState* exec) {
    ActivationImp* scope = static_cast<ActivationImp*>(exec->context()->activationObject());
    JSValue* v = *scope->getLocalDirect(index);
    double n = v->toNumber(exec);
    double newValue = (m_oper == OpPlusPlus) ? n + 1 : n - 1;
    JSValue *n2 = jsNumber(newValue);
    scope->putLocalChecked(index, n2);
    return n2;
  }
private:
  int index;
};


JSValue* PrefixNode::evaluate(ExecState* exec)
{
  //Evaluate the location
  Reference ref = m_loc->evaluateReference(exec);
  KJS_CHECKEXCEPTIONVALUE

  //handle error..
  if (!ref.found)
    return throwUndefinedVariableError(exec, ref.ident);

  JSValue *v = ref.read(exec);
  double n = v->toNumber(exec);
  double newValue = (m_oper == OpPlusPlus) ? n + 1 : n - 1;
  JSValue *n2 = jsNumber(newValue);
  ref.write(exec, n2);
  return n2;
}

void PrefixNode::recurseVisit(NodeVisitor *visitor)
{
   Node::recurseVisitLink(visitor, m_loc);
}

Node* PrefixNode::optimizeLocalAccess(ExecState* /*exec*/, FunctionBodyNode* node)
{
  if (m_loc->isVarAccessNode()) {
    VarAccessNode* ident = static_cast<VarAccessNode*>(m_loc.get());
    int id = ident->localID(node);
    if (id != -1) {
      Node* out = new LocalPrefixNode(m_loc.get(), m_oper, id);
      out->copyDebugInfo(this);
      return out;
    }
  }
  return 0;
}

JSValue* PrefixErrorNode::evaluate(ExecState* exec) 
{ 
    throwError(exec, ReferenceError, "Prefix %s operator applied to value that is not a reference.", 
	       m_oper == OpPlusPlus ? "++" : "--"); 
    handleException(exec); 
    return jsUndefined(); 
}

// ------------------------------ UnaryPlusNode --------------------------------

// ECMA 11.4.6
JSValue *UnaryPlusNode::evaluate(ExecState *exec)
{
  JSValue *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return jsNumber(v->toNumber(exec));
}

void UnaryPlusNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr);
}


// ------------------------------ NegateNode -----------------------------------

// ECMA 11.4.7
JSValue *NegateNode::evaluate(ExecState *exec)
{
  JSValue *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  double n = v->toNumber(exec);
  return jsNumber(-n);
}

void NegateNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr);
}

// ------------------------------ BitwiseNotNode -------------------------------

// ECMA 11.4.8
JSValue *BitwiseNotNode::evaluate(ExecState *exec)
{
  JSValue *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  return jsNumber(~v->toInt32(exec));
}

void BitwiseNotNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr);
}


// ------------------------------ LogicalNotNode -------------------------------

// ECMA 11.4.9
JSValue *LogicalNotNode::evaluate(ExecState *exec)
{
  JSValue *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  return jsBoolean(!v->toBoolean(exec));
}

void LogicalNotNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr);
}

// ------------------------ BinaryOperatorNode -------------------------------

static inline JSValue *add(ExecState *exec, JSValue *v1, JSValue *v2)
{
    // exception for the Date exception in defaultValue()
    JSValue *p1 = v1->toPrimitive(exec, UnspecifiedType);
    JSValue *p2 = v2->toPrimitive(exec, UnspecifiedType);
    
    if (p1->isString() || p2->isString()) {
        UString value = p1->toString(exec) + p2->toString(exec);
        if (value.isNull()) {
            JSObject *error = Error::create(exec, GeneralError, "Out of memory");
            exec->setException(error);
            return error;
        } else
            return jsString(value);
    }
    
    return jsNumber(p1->toNumber(exec) + p2->toNumber(exec));
}

static inline JSValue *sub(ExecState *exec, JSValue *v1, JSValue *v2)
{
    JSValue *p1 = v1->toPrimitive(exec, NumberType);
    JSValue *p2 = v2->toPrimitive(exec, NumberType);

    return jsNumber(p1->toNumber(exec) - p2->toNumber(exec));
}

JSValue* BinaryOperatorNode::evaluate(ExecState* exec)
{
  JSValue* v1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  JSValue* v2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  switch (oper) {
  case OpMult:
      // operator *
      return jsNumber(v1->toNumber(exec) * v2->toNumber(exec));
  case OpDiv:
      // operator /
      return jsNumber(v1->toNumber(exec) / v2->toNumber(exec));
  case OpMod:
      // operator %
      return jsNumber(fmod(v1->toNumber(exec), v2->toNumber(exec)));
  case OpPlus:
      // operator +
      return add(exec, v1, v2);
  case OpMinus:
      // operator -
      return sub(exec, v1, v2);
  case OpLShift:
      // operator <<
      return jsNumber(v1->toInt32(exec) << (v2->toUInt32(exec) & 0x1f));
  case OpRShift:
      // operator >>
      return jsNumber(v1->toInt32(exec) >> (v2->toUInt32(exec) & 0x1f));
  case OpURShift:
      // operator >>>
      return jsNumber(v1->toUInt32(exec) >> (v2->toUInt32(exec) & 0x1f));
  case OpLess:
      // operator <
      // -1: NaN, undefined => false
      //  0: v1 >= v2       => false
      //  1: v1 <  v2       => true;
      return jsBoolean(relation(exec, v1, v2) == 1);
  case OpGreaterEq:
      // operator >=
      // -1: NaN, undefined => false
      //  0: v1 >= v2       => true
      //  1: v1 <  v2       => false;
      return jsBoolean(relation(exec, v1, v2) == 0);
  case OpGreater:
      // operator >
      // -1: NaN, undefined => false
      //  0: v2 >= v1       => false
      //  1: v2 <  v1       => true
      return jsBoolean(relation(exec, v2, v1) == 1);
  case OpLessEq:
      // operator <=
      // -1: NaN, undefined => false
      //  0: v2 >= v1       => true
      //  1: v2 <  v1       => false
      return jsBoolean(relation(exec, v2, v1) == 0);
  case OpIn:
      // operator in
      return operatorIn(exec, v1, v2);
  case OpInstanceOf:
      // operator instanceof
      return operatorInstanceOf(exec, v1, v2);
  case OpEqEq:
      // operator ==
      return jsBoolean(equal(exec,v1, v2));
  case OpNotEq:
      // operator !=
      return jsBoolean(!equal(exec,v1, v2));
  case OpStrEq:
      // operator ===
      return jsBoolean(strictEqual(exec,v1, v2));
  case OpStrNEq:
      // operator !==
      return jsBoolean(!strictEqual(exec,v1, v2));
  case OpBitAnd:
      // operator &
      return jsNumber(v1->toInt32(exec) & v2->toInt32(exec));
  case OpBitXOr:
      // operator ^
      return jsNumber(v1->toInt32(exec) ^ v2->toInt32(exec));
  case OpBitOr:
      // operator |
      return jsNumber(v1->toInt32(exec) | v2->toInt32(exec));
  default:
      assert(!"BinaryOperatorNode: unhandled switch case");
      return jsUndefined();
  }
}

JSValue* BinaryOperatorNode::operatorIn(ExecState* exec,
                                        JSValue* v1, JSValue* v2)
{
    if (!v2->isObject())
        return throwError(exec,  TypeError,
                          "Value %s (result of expression %s) is not an "
                          "object. Cannot be used with IN expression.",
                          v2, expr2.get());
    JSObject* o2 = static_cast<JSObject*>(v2);
    return jsBoolean(o2->hasProperty(exec, Identifier(v1->toString(exec))));
}

JSValue* BinaryOperatorNode::operatorInstanceOf(ExecState* exec,
                                                JSValue* v1, JSValue* v2)
{
    if (!v2->isObject())
        return throwError(exec,  TypeError,
                          "Value %s (result of expression %s) is not an "
                          "object. Cannot be used with instanceof operator.",
                          v2, expr2.get());

    JSObject* o2 = static_cast<JSObject*>(v2);
    if (!o2->implementsHasInstance())
      // According to the spec, only some types of objects "implement" the [[HasInstance]] property.
      // But we are supposed to throw an exception where the object does not "have" the [[HasInstance]]
      // property. It seems that all object have the property, but not all implement it, so in this
      // case we return false (consistent with mozilla)
      return jsBoolean(false);
    return jsBoolean(o2->hasInstance(exec, v1));
}

void BinaryOperatorNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr1);
  recurseVisitLink(visitor, expr2);
}

// ------------------------------ BinaryLogicalNode ----------------------------

// ECMA 11.11
JSValue *BinaryLogicalNode::evaluate(ExecState *exec)
{
  JSValue *v1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  bool b1 = v1->toBoolean(exec);
  if ((!b1 && oper == OpAnd) || (b1 && oper == OpOr))
    return v1;

  JSValue *v2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return v2;
}

void BinaryLogicalNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr1);
  recurseVisitLink(visitor, expr2);
}


// ------------------------------ ConditionalNode ------------------------------

// ECMA 11.12
JSValue *ConditionalNode::evaluate(ExecState *exec)
{
  JSValue *v = logical->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  bool b = v->toBoolean(exec);

  if (b)
    v = expr1->evaluate(exec);
  else
    v = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return v;
}

void ConditionalNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, logical);
  recurseVisitLink(visitor, expr1);
  recurseVisitLink(visitor, expr2);
}


// ECMA 11.13

static ALWAYS_INLINE JSValue *valueForReadModifyAssignment(ExecState * exec, JSValue *v1, JSValue *v2, Operator oper)
{
  JSValue *v;
  int i1;
  int i2;
  unsigned int ui;
  switch (oper) {
  case OpMultEq:
    v = jsNumber(v1->toNumber(exec) * v2->toNumber(exec));
    break;
  case OpDivEq:
    v = jsNumber(v1->toNumber(exec) / v2->toNumber(exec));
    break;
  case OpPlusEq:
    v = add(exec, v1, v2);
    break;
  case OpMinusEq:
    v = sub(exec, v1, v2);
    break;
  case OpLShift:
    i1 = v1->toInt32(exec);
    i2 = v2->toInt32(exec);
    v = jsNumber(i1 << i2);
    break;
  case OpRShift:
    i1 = v1->toInt32(exec);
    i2 = v2->toInt32(exec);
    v = jsNumber(i1 >> i2);
    break;
  case OpURShift:
    ui = v1->toUInt32(exec);
    i2 = v2->toInt32(exec);
    v = jsNumber(ui >> i2);
    break;
  case OpAndEq:
    i1 = v1->toInt32(exec);
    i2 = v2->toInt32(exec);
    v = jsNumber(i1 & i2);
    break;
  case OpXOrEq:
    i1 = v1->toInt32(exec);
    i2 = v2->toInt32(exec);
    v = jsNumber(i1 ^ i2);
    break;
  case OpOrEq:
    i1 = v1->toInt32(exec);
    i2 = v2->toInt32(exec);
    v = jsNumber(i1 | i2);
    break;
  case OpModEq: {
    double d1 = v1->toNumber(exec);
    double d2 = v2->toNumber(exec);
    v = jsNumber(fmod(d1, d2));
  }
    break;
  default:
    assert(0);
    v = jsUndefined();
  }

  return v;
}

// ------------------------------ AssignNode -----------------------------------

// An special version where LHS is always local and where no 
// read-modify-write is possible.
class LocalAssignNode : public AssignNode {
public:
  LocalAssignNode(LocationNode* loc, Node *right, int idx) : 
      AssignNode(loc, OpEqual, right), index(idx) {}

  JSValue* evaluate(ExecState* exec) {
    ActivationImp* scope = static_cast<ActivationImp*>(exec->context()->activationObject());
    JSValue *v = m_right->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE
    scope->putLocalChecked(index, v);
    return v;
  }
private:
  int index;
};

JSValue* AssignNode::evaluate(ExecState* exec) {
  Reference ref = m_loc->evaluateReference(exec);

  KJS_CHECKEXCEPTIONVALUE

  //Lookup failure is OK for assignments, but not 
  //read-modify-write ops, e.g. +=
  if (!ref.found && m_oper != OpEqual)
    return throwUndefinedVariableError(exec, ref.ident);

  JSValue *v;

  if (m_oper == OpEqual) {
    v = m_right->evaluate(exec);
  } else {
    JSValue *v1 = ref.read(exec);
    KJS_CHECKEXCEPTIONVALUE
    JSValue *v2 = m_right->evaluate(exec);
    v = valueForReadModifyAssignment(exec, v1, v2, m_oper);
  }

  KJS_CHECKEXCEPTIONVALUE

  ref.write(exec, v);
  return v;
}

void AssignNode::recurseVisit(NodeVisitor *visitor)
{
   Node::recurseVisitLink(visitor, m_loc);
   Node::recurseVisitLink(visitor, m_right);
}


Node* AssignNode::optimizeLocalAccess(ExecState* /*exec*/, FunctionBodyNode* node)
{
  if (m_loc->isVarAccessNode() && m_oper == OpEqual) {
    VarAccessNode* ident = static_cast<VarAccessNode*>(m_loc.get());
    int id = ident->localID(node);
    if (id != -1) {
      Node* out = new LocalAssignNode(m_loc.get(), m_right.get(), id);
      out->copyDebugInfo(this);
      return out;
    }
  }
  return 0;
}

// ------------------------------ AssignDotNode -----------------------------------

JSValue *AssignDotNode::evaluate(ExecState *exec)
{
  JSValue *baseValue = m_base->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  JSObject *base = baseValue->toObject(exec);

  JSValue *v;

  if (m_oper == OpEqual) {
    v = m_right->evaluate(exec);
  } else {
    PropertySlot slot;
    JSValue *v1 = base->getPropertySlot(exec, m_ident, slot) ? slot.getValue(exec, base, m_ident) : jsUndefined();
    KJS_CHECKEXCEPTIONVALUE
    JSValue *v2 = m_right->evaluate(exec);
    v = valueForReadModifyAssignment(exec, v1, v2, m_oper);
    
    //### can do direct write here..
  }

  KJS_CHECKEXCEPTIONVALUE

  base->put(exec, m_ident, v);
  return v;
}

void AssignDotNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, m_base);
  recurseVisitLink(visitor, m_right);
}


// ------------------------------ AssignBracketNode -----------------------------------

JSValue *AssignBracketNode::evaluate(ExecState *exec)
{
  JSValue *baseValue = m_base->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  JSValue *subscript = m_subscript->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  JSObject *base = baseValue->toObject(exec);

  uint32_t propertyIndex;
  if (subscript->getUInt32(propertyIndex)) {
    JSValue *v;
    if (m_oper == OpEqual) {
      v = m_right->evaluate(exec);
    } else {
      PropertySlot slot;
      JSValue *v1 = base->getPropertySlot(exec, propertyIndex, slot) ? slot.getValue(exec, base, propertyIndex) : jsUndefined();
      KJS_CHECKEXCEPTIONVALUE
      JSValue *v2 = m_right->evaluate(exec);
      v = valueForReadModifyAssignment(exec, v1, v2, m_oper);

      //### can do direct write here..
    }

    KJS_CHECKEXCEPTIONVALUE

    base->put(exec, propertyIndex, v);
    return v;
  }

  Identifier propertyName(subscript->toString(exec));
  JSValue *v;

  if (m_oper == OpEqual) {
    v = m_right->evaluate(exec);
  } else {
    PropertySlot slot;
    JSValue *v1 = base->getPropertySlot(exec, propertyName, slot) ? slot.getValue(exec, base, propertyName) : jsUndefined();
    KJS_CHECKEXCEPTIONVALUE
    JSValue *v2 = m_right->evaluate(exec);
    v = valueForReadModifyAssignment(exec, v1, v2, m_oper);

    //### can do direct write here..
  }

  KJS_CHECKEXCEPTIONVALUE

  base->put(exec, propertyName, v);
  return v;
}

void AssignBracketNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, m_base);
  recurseVisitLink(visitor, m_subscript);
  recurseVisitLink(visitor, m_right);
}

JSValue* AssignErrorNode::evaluate(ExecState* exec) 
{ 
    throwError(exec, ReferenceError, "Left side of assignment is not a reference."); 
    handleException(exec); 
    return jsUndefined(); 
} 


// ------------------------------ CommaNode ------------------------------------

// ECMA 11.14
JSValue *CommaNode::evaluate(ExecState *exec)
{
  expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  JSValue *v = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return v;
}

void CommaNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr1);
  recurseVisitLink(visitor, expr2);
}


// ------------------------------ AssignExprNode -------------------------------

// ECMA 12.2
JSValue *AssignExprNode::evaluate(ExecState *exec)
{
  return expr->evaluate(exec);
}

void AssignExprNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr);
}


// ------------------------------ VarDeclNode ----------------------------------


VarDeclNode::VarDeclNode(const Identifier &id, AssignExprNode *in, Type t)
    : varType(t), ident(id), init(in)
{
}

// ECMA 12.2
JSValue *VarDeclNode::evaluate(ExecState *exec)
{
  JSObject* variable = exec->context()->variableObject();

  JSValue* val;
  if (init) {
      val = init->evaluate(exec);
      KJS_CHECKEXCEPTIONVALUE
  } else {
      // already declared? - check with getDirect so you can override
      // built-in properties of the global object with var declarations.
      // Also check for 'arguments' property. The 'arguments' cannot be found with
      // getDirect, because it's created lazily by
      // ActivationImp::getOwnPropertySlot.
      // Since variable declarations are always in function scope, 'variable'
      // will always contain instance of ActivationImp and ActivationImp will
      // always have 'arguments' property
      if (variable->getDirect(ident) || ident == exec->propertyNames().arguments)
          return 0;
      val = jsUndefined();
  }

#ifdef KJS_VERBOSE
  printInfo(exec,(UString("new variable ")+ident.ustring()).cstring().c_str(),val);
#endif
  // We use Internal to bypass all checks in derived objects, e.g. so that
  // "var location" creates a dynamic property instead of activating window.location.
  int flags = Internal;
  if (exec->context()->codeType() != EvalCode)
    flags |= DontDelete;
  if (varType == VarDeclNode::Constant)
    flags |= ReadOnly;
  variable->put(exec, ident, val, flags);

  return 0; //No useful value, not a true expr
}

void VarDeclNode::processVarDecl(ExecState *exec)
{
  JSObject* variable = exec->context()->variableObject();
  
  // First, determine which flags we want to use..
  int flags = DontDelete; 
  if (varType == VarDeclNode::Constant)
    flags |= ReadOnly;

  // Are we inside a function? If so, we fill in the symbol table
  switch (exec->context()->codeType()) {
    case FunctionCode:
      // Inside a function, we're just computing static information.
      // so, just fill in the symbol table.
      exec->context()->currentBody()->addVarDecl(ident, flags, exec);
      return;
    case EvalCode:
      // eval-injected variables can be deleted..
      flags &= ~DontDelete;

      // If a variable by this name already exists, don't clobber it -
      // eval may be trying to inject a variable that already exists..
      if (!variable->hasProperty(exec, ident)) {
        variable->put(exec, ident, jsUndefined(), flags);
        // eval injected a new local into scope! Better mark that down, 
        // so that NonLocalResolver stops skipping the local scope
        variable->setLocalInjected();
      }
      break;
    case GlobalCode:
      // If a variable by this name already exists, don't clobber it -
      // ### I am not sue this is needed for GlobalCode
      if (!variable->hasProperty(exec, ident))
        variable->put(exec, ident, jsUndefined(), flags);
  };
}

void VarDeclNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, init);
}


// ------------------------------ VarDeclListNode ------------------------------

// ECMA 12.2
JSValue *VarDeclListNode::evaluate(ExecState *exec)
{
  for (VarDeclListNode *n = this; n; n = n->next.get()) {
    n->var->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE
  }
  return jsUndefined();
}

void VarDeclListNode::breakCycle()
{
    next = 0;
}

void VarDeclListNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, var);
  recurseVisitLink(visitor, next);
}


// ------------------------------ VarStatementNode -----------------------------

// ECMA 12.2
Completion VarStatementNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  (void) next->evaluate(exec);
  KJS_CHECKEXCEPTION

  return Completion(Normal);
}

void VarStatementNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, next);
}

Node* VarStatementNode::optimizeLocalAccess(ExecState*, FunctionBodyNode* funcBody)
{
  StaticVarStatementNode* staticVer = new StaticVarStatementNode();
  staticVer->originalStatement = this;

  for (VarDeclListNode *n = next.get(); n; n = n->next.get()) {
    // We only care about things which have an initializer --- 
    // everything else is a no-op at execution time, 
    // and only makes a difference at processVarDecl time
    if (n->var->init) {
      StaticVarStatementNode::Initializer init;
      init.initExpr = n->var->init->getExpr(); //Skip the AssignExprNode as well...
      init.localID  = funcBody->lookupSymbolID(n->var->ident);
      if (init.localID == -1) { //Ouch, a dynamic property! Bail out, bail out!
        newNodes->remove(staticVer);
        delete staticVer;
        return 0;
      }
      staticVer->initializers.append(init);
    }
  }
  staticVer->copyDebugInfo(this);
  return staticVer;
}


// ------------------------------ StaticVarStatementNode -----------------------

Completion StaticVarStatementNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;
  ActivationImp* scope = static_cast<ActivationImp*>(exec->context()->activationObject());

  for (unsigned int i = 0; i < initializers.size(); ++i) {
    JSValue* val = initializers[i].initExpr->evaluate(exec);
    KJS_CHECKEXCEPTION
    scope->putLocal(initializers[i].localID, val);
    // Note: flags are already taken care of at declaration time
  }
  return Completion(Normal);
}

void StaticVarStatementNode::recurseVisit(NodeVisitor *visitor)
{
  for (unsigned int i = 0; i < initializers.size(); ++i) {
   Node* oldInitializer = initializers[i].initExpr.get();
   Node* newInitializer = visitor->visit(oldInitializer);
   if (newInitializer)
    initializers[i].initExpr = newInitializer;
  }
}

// ------------------------------ BlockNode ------------------------------------

BlockNode::BlockNode(SourceElementsNode *s)
{
  if (s) {
    source = s->next.release();
    Parser::removeNodeCycle(source.get());
    setLoc(s->firstLine(), s->lastLine());
  } else {
    source = 0;
  }
}

// ECMA 12.1
Completion BlockNode::execute(ExecState *exec)
{
  if (!source)
    return Completion(Normal);

  return source->execute(exec);
}

void BlockNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, source);
}

// ------------------------------ EmptyStatementNode ---------------------------

// ECMA 12.3
Completion EmptyStatementNode::execute(ExecState *)
{
  return Completion(Normal);
}

// ------------------------------ ExprStatementNode ----------------------------

// ECMA 12.4
Completion ExprStatementNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  JSValue *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTION

  return Completion(Normal, v);
}

void ExprStatementNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr);
}


// ------------------------------ IfNode ---------------------------------------

// ECMA 12.5
Completion IfNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  JSValue *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTION
  bool b = v->toBoolean(exec);

  // if ... then
  if (b)
    return statement1->execute(exec);

  // no else
  if (!statement2)
    return Completion(Normal);

  // else
  return statement2->execute(exec);
}

void IfNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr);
  recurseVisitLink(visitor, statement1);
  recurseVisitLink(visitor, statement2);
}


// ------------------------------ DoWhileNode ----------------------------------

// ECMA 12.6.1
Completion DoWhileNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  JSValue *bv;
  Completion c;
  JSValue* value = 0;

  do {
    // bail out on error
    KJS_CHECKEXCEPTION

    c = statement->execute(exec);

    if (exec->dynamicInterpreter()->checkTimeout())
        return Completion(Interrupted);

    if (c.isValueCompletion())
	value = c.value();

    if (!((c.complType() == Continue) && (c.target() == this))) {
      if ((c.complType() == Break) && (c.target() == this))
        return Completion(Normal, value);
      if (c.complType() != Normal)
        return c;
    }
    bv = expr->evaluate(exec);
    KJS_CHECKEXCEPTION
  } while (bv->toBoolean(exec));

  return Completion(Normal, value);
}

void DoWhileNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr);
  recurseVisitLink(visitor, statement);
}

// ------------------------------ WhileNode ------------------------------------

// ECMA 12.6.2
Completion WhileNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  JSValue *bv;
  Completion c;
  bool b(false);
  JSValue *value = 0;

  while (1) {
    bv = expr->evaluate(exec);
    KJS_CHECKEXCEPTION
    b = bv->toBoolean(exec);

    // bail out on error
    KJS_CHECKEXCEPTION

    if (!b)
      return Completion(Normal, value);

    c = statement->execute(exec);

    if (exec->dynamicInterpreter()->checkTimeout())
        return Completion(Interrupted);

    if (c.isValueCompletion())
      value = c.value();

    if ((c.complType() == Continue) && (c.target() == this))
      continue;
    if ((c.complType() == Break) && (c.target() == this))
      return Completion(Normal, value);
    if (c.complType() != Normal)
      return c;
  }

  return Completion(); // work around gcc 4.0 bug
}

void WhileNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr);
  recurseVisitLink(visitor, statement);
}

// ------------------------------ ForNode --------------------------------------

// ECMA 12.6.3
Completion ForNode::execute(ExecState *exec)
{
  JSValue *v, *cval = 0;

  if (expr1) {
    v = expr1->evaluate(exec);
    KJS_CHECKEXCEPTION
  }
  while (1) {
    if (expr2) {
      v = expr2->evaluate(exec);
      KJS_CHECKEXCEPTION
      if (!v->toBoolean(exec))
        return Completion(Normal, cval);
    }
    // bail out on error
    KJS_CHECKEXCEPTION

    Completion c = statement->execute(exec);
    if (c.isValueCompletion())
      cval = c.value();
    if (!((c.complType() == Continue) && (c.target() == this))) {
      if ((c.complType() == Break) && (c.target() == this))
        return Completion(Normal, cval);
      if (c.complType() != Normal)
        return c;
    }

    if (exec->dynamicInterpreter()->checkTimeout())
        return Completion(Interrupted);

    if (expr3) {
      v = expr3->evaluate(exec);
      KJS_CHECKEXCEPTION
    }
  }

  return Completion(); // work around gcc 4.0 bug
}

void ForNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr1);
  recurseVisitLink(visitor, expr2);
  recurseVisitLink(visitor, expr3);
  recurseVisitLink(visitor, statement);
}


// ------------------------------ ForInNode ------------------------------------

ForInNode::ForInNode(Node *l, Node *e, StatementNode *s)
  : init(0L), lexpr(l), expr(e), varDecl(0L), statement(s)
{
}

ForInNode::ForInNode(const Identifier &i, AssignExprNode *in, Node *e, StatementNode *s)
  : ident(i), init(in), expr(e), statement(s)
{
  // for( var foo = bar in baz )
  varDecl = new VarDeclNode(ident, init.get(), VarDeclNode::Variable);
  lexpr   = new VarAccessNode(ident);
}

// ECMA 12.6.4
Completion ForInNode::execute(ExecState *exec)
{
  JSValue *e;
  JSValue *retval = 0;
  JSObject *v;
  Completion c;
  PropertyNameArray propertyNames;

  if (varDecl) {
    varDecl->evaluate(exec);
    KJS_CHECKEXCEPTION
  }

  e = expr->evaluate(exec);

  // for Null and Undefined, we want to make sure not to go through
  // the loop at all, because their object wrappers will have a
  // property list but will throw an exception if you attempt to
  // access any property.
  if (e->isUndefinedOrNull()) {
    return Completion(Normal, 0);
  }

  KJS_CHECKEXCEPTION
  v = e->toObject(exec);
  v->getPropertyNames(exec, propertyNames);

  PropertyNameArrayIterator end = propertyNames.end();
  for (PropertyNameArrayIterator it = propertyNames.begin(); it != end; ++it) {
    const Identifier &name = *it;
    if (!v->hasProperty(exec, name))
        continue;

    JSValue *str = jsString(name.ustring());

    assert (lexpr->isLocation());
    LocationNode* loc = static_cast<LocationNode*>(lexpr.get());

    Reference     ref = loc->evaluateReference(exec);
    KJS_CHECKEXCEPTION
    ref.write(exec, str);

    c = statement->execute(exec);
    if (c.isValueCompletion())
      retval = c.value();

    if (!((c.complType() == Continue) && (c.target() == this))) {
      if ((c.complType() == Break) && (c.target() == this))
        break;
      if (c.complType() != Normal) {
        return c;
      }
    }
  }

  // bail out on error
  KJS_CHECKEXCEPTION

  return Completion(Normal, retval);
}

void ForInNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, init);
  recurseVisitLink(visitor, lexpr); 
  recurseVisitLink(visitor, expr);
  recurseVisitLink(visitor, varDecl);
  recurseVisitLink(visitor, statement);
}

// ------------------------------ ContinueNode ---------------------------------

// ECMA 12.7
Completion ContinueNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  return Completion(Continue, 0, target);
}

// ------------------------------ BreakNode ------------------------------------

// ECMA 12.8
Completion BreakNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  return Completion(Break, 0, target);
}

// ------------------------------ ReturnNode -----------------------------------

// ECMA 12.9
Completion ReturnNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  CodeType codeType = exec->context()->codeType();
  if (codeType != FunctionCode) {
    return createErrorCompletion(exec, SyntaxError, "Invalid return statement.");
  }

  if (!value)
    return Completion(ReturnValue, jsUndefined());

  JSValue *v = value->evaluate(exec);
  KJS_CHECKEXCEPTION

  return Completion(ReturnValue, v);
}

void ReturnNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, value);
}


// ------------------------------ WithNode -------------------------------------

// ECMA 12.10
Completion WithNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  JSValue *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTION
  JSObject *o = v->toObject(exec);
  KJS_CHECKEXCEPTION
  exec->context()->pushScope(o);
  Completion res = statement->execute(exec);
  exec->context()->popScope();

  return res;
}

void WithNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr);
  recurseVisitLink(visitor, statement);
}


// ------------------------------ CaseClauseNode -------------------------------

// ECMA 12.11
JSValue *CaseClauseNode::evaluate(ExecState *exec)
{
  JSValue *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return v;
}

// ECMA 12.11
Completion CaseClauseNode::evalStatements(ExecState *exec)
{
  if (source)
    return source->execute(exec);
  else
    return Completion(Normal, jsUndefined());
}

void CaseClauseNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr);
  recurseVisitLink(visitor, source);
}


// ------------------------------ ClauseListNode -------------------------------

JSValue *ClauseListNode::evaluate(ExecState *)
{
  // should never be called
  assert(false);
  return 0;
}

void ClauseListNode::breakCycle()
{
    next = 0;
}

void ClauseListNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, clause);
  recurseVisitLink(visitor, next);
}

// ------------------------------ CaseBlockNode --------------------------------

CaseBlockNode::CaseBlockNode(ClauseListNode *l1, CaseClauseNode *d,
                             ClauseListNode *l2)
{
  if (l1) {
    list1 = l1->next.release();
    Parser::removeNodeCycle(list1.get());
  } else {
    list1 = 0;
  }

  def = d;

  if (l2) {
    list2 = l2->next.release();
    Parser::removeNodeCycle(list2.get());
  } else {
    list2 = 0;
  }
}

JSValue *CaseBlockNode::evaluate(ExecState *)
{
  // should never be called
  assert(false);
  return 0;
}

// ECMA 12.11
Completion CaseBlockNode::evalBlock(ExecState *exec, JSValue *input)
{
  JSValue *v;
  Completion res;
  ClauseListNode *a = list1.get();
  ClauseListNode *b = list2.get();
  CaseClauseNode *clause;

    while (a) {
      clause = a->getClause();
      a = a->getNext();
      v = clause->evaluate(exec);
      KJS_CHECKEXCEPTION
      if (strictEqual(exec, input, v)) {
	res = clause->evalStatements(exec);
	if (res.complType() != Normal)
	  return res;
	while (a) {
	  res = a->getClause()->evalStatements(exec);
	  if (res.complType() != Normal)
	    return res;
	  a = a->getNext();
	}
	break;
      }
    }

  while (b) {
    clause = b->getClause();
    b = b->getNext();
    v = clause->evaluate(exec);
    KJS_CHECKEXCEPTION
    if (strictEqual(exec, input, v)) {
      res = clause->evalStatements(exec);
      if (res.complType() != Normal)
        return res;
      goto step18;
    }
  }

  // default clause
  if (def) {
    res = def->evalStatements(exec);
    if (res.complType() != Normal)
      return res;
  }
  b = list2.get();
 step18:
  while (b) {
    clause = b->getClause();
    res = clause->evalStatements(exec);
    if (res.complType() != Normal)
      return res;
    b = b->getNext();
  }

  // bail out on error
  KJS_CHECKEXCEPTION

  return Completion(Normal);
}

void CaseBlockNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, list1);
  recurseVisitLink(visitor, def);
  recurseVisitLink(visitor, list2);
}

// ------------------------------ SwitchNode -----------------------------------

// ECMA 12.11
Completion SwitchNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  JSValue *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTION

  Completion res = block->evalBlock(exec,v);

  if ((res.complType() == Break) && (res.target() == this))
    return Completion(Normal, res.value());
  return res;
}

void SwitchNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr);
  recurseVisitLink(visitor, block);
}

// ------------------------------ LabelNode ------------------------------------

// ECMA 12.12
Completion LabelNode::execute(ExecState *exec)
{
  Completion e = statement->execute(exec);

  if ((e.complType() == Break) && (e.target() == this))
    return Completion(Normal, e.value());
  return e;
}

void LabelNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, statement);
}

// ------------------------------ ThrowNode ------------------------------------

// ECMA 12.13
Completion ThrowNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  JSValue *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTION

  handleException(exec, v);

  return Completion(Throw, v);
}

void ThrowNode::recurseVisit(NodeVisitor *visitor)
{
  recurseVisitLink(visitor, expr);
}


// ------------------------------ TryNode --------------------------------------

// ECMA 12.14
Completion TryNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Completion c = tryBlock->execute(exec);

  if (catchBlock && c.complType() == Throw) {
    JSObject *obj = new JSObject;
    obj->put(exec, exceptionIdent, c.value(), DontDelete);
    exec->context()->pushScope(obj);
    c = catchBlock->execute(exec);
    exec->context()->popScope();
  }

  if (finallyBlock) {
    Completion c2 = finallyBlock->execute(exec);
    if (c2.complType() != Normal)
      c = c2;
  }

  return c;
}

void TryNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, tryBlock);
    recurseVisitLink(visitor, catchBlock);
    recurseVisitLink(visitor, finallyBlock);
}

void TryNode::recurseVisitNonCatch(NodeVisitor* visitor)
{
    recurseVisitLink(visitor, tryBlock);
    recurseVisitLink(visitor, finallyBlock); //### check this?
}

// ------------------------------ ParameterNode --------------------------------

// ECMA 13
JSValue *ParameterNode::evaluate(ExecState *)
{
  return jsUndefined();
}

void ParameterNode::breakCycle()
{
    next = 0;
}

void ParameterNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, next);
}

// ------------------------------ FunctionBodyNode -----------------------------

FunctionBodyNode::FunctionBodyNode(SourceElementsNode *s)
    : BlockNode(s)
    , m_sourceURL(lexer().sourceURL())
    , m_sourceId(parser().sourceId())
    , m_builtSymbolList(false)
{

  setLoc(-1, -1);
}


void FunctionBodyNode::addVarDecl(const Identifier& ident, int attr, ExecState* exec)
{
  // There is one nasty special case: ignore a 'var' declaration of 'arguments';
  // it effectively doesn't do anything since the magic 'arguments' is already 
  // in scope anyway, and if we allocated a local, we would have to worry about
  // keeping track of whether it was initialized or not on what is supposed to be the 
  // fast path. So we just make this go through the property map instead.
  // Note that this does not matter for parameters or function declarations,
  // since those overwrite the magic 'arguments' anyway.
  if (ident == exec->propertyNames().arguments)
    return;

  (void)addSymbol(ident, attr);
}

void FunctionBodyNode::addFunDecl(const Identifier& ident, int attr, FuncDeclNode* funcDecl)
{
  (void)addSymbol(ident, attr, funcDecl);
}

int FunctionBodyNode::addSymbol(const Identifier& ident, int flags, FuncDeclNode* funcDecl)
{

  // We get symbols in the order specified in 10.1.3, but sometimes
  // the later ones are supposed to lose. This -mostly- does not 
  // matter for us --- we primarily concern ourselves with name/ID
  // mapping, but there is an issue of attributes and funcDecl's.
  // However, the only flag that matters here is ReadOnly -- 
  // everything else just has DontDelete set; and it's from const, 
  // so we can just ignore it on repetitions, since var/const should lose
  // and are at the end.
  //
  // And for funcDecl, since functions win over everything, we always set it if non-zero
  if (JSValue* oldIdVal = m_symbolTable.get(ident)) {
    int oldId = (int)oldIdVal->getNumber();
    if (funcDecl)
      m_symbolList[oldId - 1].funcDecl = funcDecl;
    return oldId;
  }
    
  int id = m_symbolList.size() + 1;         //First entry gets 1, etc.
  m_symbolTable.put(ident, jsNumber(id), 0);
  m_symbolList.append (Symbol(ident, flags, funcDecl));
  return id;
}

int FunctionBodyNode::lookupSymbolID(const Identifier& ident) const
{
  JSValue* val = m_symbolTable.get(ident);
  if (!val)
    return -1;
  return int(val->getNumber());
}


void FunctionBodyNode::addParam(const Identifier& ident)
{
  int id = addSymbol(ident, DontDelete);
  m_paramList.append(Parameter(ident, id));
}


// ------------------------------ FuncDeclNode ---------------------------------

// ECMA 13
void FuncDeclNode::processFuncDecl(ExecState *exec)
{
  Context *context = exec->context();

  // See whether we just need to fill in the symbol table, 
  // or actually fiddle with objects.
  int flags = Internal | DontDelete;
  switch (exec->context()->codeType()) {
    case FunctionCode:
      // Inside a function, just need symbol info
      exec->context()->currentBody()->addFunDecl(ident, flags, this);
      return;
    case EvalCode:
      // eval-injected symbols can be deleted...
      flags &= ~DontDelete;
      
      // fallthrough intentional
    case GlobalCode:
      context->variableObject()->put(exec, ident, makeFunctionObject(exec), flags);
  };
}

void FuncDeclNode::addParams() 
{
  for (ParameterNode *p = param.get(); p != 0L; p = p->nextParam())
    body->addParam(p->ident());
}

FunctionImp* FuncDeclNode::makeFunctionObject(ExecState *exec)
{
  // TODO: let this be an object with [[Class]] property "Function"
  Context *context = exec->context();
  FunctionImp *func = new DeclaredFunctionImp(exec, ident, body.get(), context->scopeChain());

  JSObject *proto = exec->lexicalInterpreter()->builtinObject()->construct(exec, List::empty());
  proto->put(exec, exec->propertyNames().constructor, func, DontEnum);
  func->put(exec, exec->propertyNames().prototype, proto, Internal|DontDelete);

  func->put(exec, exec->propertyNames().length, jsNumber(body->numParams()), ReadOnly|DontDelete|DontEnum);

  return func;
}

Completion FuncDeclNode::execute(ExecState *)
{
    return Completion(Normal);
}

void FuncDeclNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, param);
    recurseVisitLink(visitor, body);
}


// ------------------------------ FuncExprNode ---------------------------------

// ECMA 13
JSValue *FuncExprNode::evaluate(ExecState *exec)
{
  Context *context = exec->context();
  bool named = !ident.isNull();
  JSObject *functionScopeObject = 0;

  if (named) {
    // named FunctionExpressions can recursively call themselves,
    // but they won't register with the current scope chain and should
    // be contained as single property in an anonymous object.
    functionScopeObject = new JSObject;
    context->pushScope(functionScopeObject);
  }

  FunctionImp *func = new DeclaredFunctionImp(exec, ident, body.get(), context->scopeChain());
  JSObject *proto = exec->lexicalInterpreter()->builtinObject()->construct(exec, List::empty());
  proto->put(exec, exec->propertyNames().constructor, func, DontEnum);
  func->put(exec, exec->propertyNames().prototype, proto, Internal|DontDelete);

  if (named) {
    functionScopeObject->put(exec, ident, func, Internal | ReadOnly | (context->codeType() == EvalCode ? 0 : DontDelete));
    context->popScope();
  }

  return func;
}

void FuncExprNode::addParams()
{
  for(ParameterNode *p = param.get(); p != 0L; p = p->nextParam())
    body->addParam(p->ident());
}


void FuncExprNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, param);
    recurseVisitLink(visitor, body);
}


// ------------------------------ SourceElementsNode ---------------------------

SourceElementsNode::SourceElementsNode(StatementNode *s1)
  : node(s1), next(this)
{
    Parser::noteNodeCycle(this);
    setLoc(s1->firstLine(), s1->lastLine());
}

SourceElementsNode::SourceElementsNode(SourceElementsNode *s1, StatementNode *s2)
  : node(s2), next(s1->next)
{
  s1->next = this;
  setLoc(s1->firstLine(), s2->lastLine());
}

// ECMA 14
Completion SourceElementsNode::execute(ExecState *exec)
{
  KJS_CHECKEXCEPTION

  Completion c1 = node->execute(exec);
  KJS_CHECKEXCEPTION;
  if (c1.complType() != Normal)
    return c1;

  for (SourceElementsNode *n = next.get(); n; n = n->next.get()) {
    Completion c2 = n->node->execute(exec);
    if (c2.complType() != Normal) {
	if (c2.complType() == Throw)
	    return c2;
	if (c2.isValueCompletion())
	    return c2;
	return Completion(c2.complType(), c1.value(), c2.target());
    }
    if (c2.value())
      c1 = c2;
  }

  return c1;
}

// ECMA 14
void SourceElementsNode::breakCycle()
{
    next = 0;
}

void SourceElementsNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, node);
    recurseVisitLink(visitor, next);
}

// ------------------------------ ProgramNode ----------------------------------

ProgramNode::ProgramNode(SourceElementsNode *s) : FunctionBodyNode(s)
{
}

// ------------------------------ ErrorNode ------------------------------------

Completion ErrorNode::execute(ExecState *exec)
{
    return createErrorCompletion(exec, SyntaxError, message);
}

// ------------------------------ PackageNameNode ------------------------------
JSValue* PackageNameNode::evaluate(ExecState*)
{
    // should never get here.
    return 0;
}

void PackageNameNode::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, names);
}

Completion PackageNameNode::loadSymbol(ExecState* exec, bool wildcard)
{
    Package* basePackage;
    JSObject* baseObject;
    if (names) {
	PackageObject *pobj = names->resolvePackage(exec);
	if (pobj == 0)
	    return Completion(Normal);
	basePackage = pobj->package();
	baseObject = pobj;
    } else {
	Interpreter* ip = exec->lexicalInterpreter();
	basePackage = ip->globalPackage();
	baseObject = ip->globalObject();
    }

    if (wildcard) {
	// if a .* is specified the last identifier should
	// denote another package name
	PackageObject* pobj = resolvePackage(exec, baseObject, basePackage);
	if (!pobj)
	    return Completion(Normal);
	basePackage = pobj->package();
	baseObject = pobj;
	basePackage->loadAllSymbols(exec, baseObject);
    } else {
	basePackage->loadSymbol(exec, baseObject, id);
    }

    return Completion(Normal);
}

PackageObject* PackageNameNode::resolvePackage(ExecState* exec)
{
    JSObject* baseObject;
    Package* basePackage;
    if (names) {
	PackageObject* basePackageObject = names->resolvePackage(exec);
	if (basePackageObject == 0)
	    return 0;
	baseObject = basePackageObject;
	basePackage = basePackageObject->package();
    } else {
	// first identifier is looked up in global object
	Interpreter* ip = exec->lexicalInterpreter();
	baseObject = ip->globalObject();
	basePackage = ip->globalPackage();
    }
    
    return resolvePackage(exec, baseObject, basePackage);
}

PackageObject* PackageNameNode::resolvePackage(ExecState* exec,
					       JSObject* baseObject,
					       Package* basePackage)
{
    PackageObject* res = 0;

    // Let's see whether the package was already resolved previously.
    JSValue* v = baseObject->get(exec, id);
    if (v && !v->isUndefined()) {
	if (!v->isObject()) {
	    // Symbol conflict
	    throwError(exec, GeneralError, "Invalid type of package %s", id);
	    return 0;
	}
	res = static_cast<PackageObject*>(v);
    } else {
	UString err;
	Package *newBase = basePackage->loadSubPackage(id, &err);
	if (newBase == 0) {
	    if (err.isEmpty()) {
		throwError(exec, GeneralError, "Package not found");
	    } else {
		throwError(exec, GeneralError, err);
	    }
	    return 0;
	}
	res = new PackageObject(newBase);
	baseObject->put(exec, id, res);
    }

    return res;
}

Completion ImportStatement::execute(ExecState*)
{
    return Completion(Normal);
}


void ImportStatement::processVarDecl(ExecState* exec)
{
    // error out if package support is not activated
    Package* glob = exec->lexicalInterpreter()->globalPackage();
    if (!glob) {
	throwError(exec, GeneralError,
		   "Package support disabled. Import failed.");
	return;
    }

    // also error out if not used on top-level
    if (exec->context()->codeType() != GlobalCode) {
      throwError(exec, GeneralError,
                  "Package imports may only occur at top level.");
      return;
    }

    name->loadSymbol(exec, wld);
}

void ImportStatement::recurseVisit(NodeVisitor *visitor)
{
    recurseVisitLink(visitor, name);
}

} //namespace KJS
