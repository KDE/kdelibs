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

#ifndef _INTERNAL_H_
#define _INTERNAL_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kjs.h"
#include "object.h"
#include "function.h"
#include "debugger.h"

#define I18N_NOOP(s) s

namespace KJS {

  class Boolean;
  class Number;
  class String;
  class Object;
  class RegExp;
  class Node;
  class FunctionBodyNode;
  class ProgramNode;
  class Debugger;

  class UndefinedImp : public Imp {
  public:
    UndefinedImp();
    virtual ~UndefinedImp() { }
    virtual KJSO toPrimitive(Type preferred = UndefinedType) const;
    virtual Boolean toBoolean() const;
    virtual Number toNumber() const;
    virtual String toString() const;
    virtual Object toObject() const;

    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;

    static UndefinedImp *staticUndefined;
  };

  class NullImp : public Imp {
  public:
    NullImp();
    virtual ~NullImp() { }
    virtual KJSO toPrimitive(Type preferred = UndefinedType) const;
    virtual Boolean toBoolean() const;
    virtual Number toNumber() const;
    virtual String toString() const;
    virtual Object toObject() const;

    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;

    static NullImp *staticNull;
  };

  class BooleanImp : public Imp {
  public:
    virtual ~BooleanImp() { }
    BooleanImp(bool v = false) : val(v) { }
    bool value() const { return val; }
    virtual KJSO toPrimitive(Type preferred = UndefinedType) const;
    virtual Boolean toBoolean() const;
    virtual Number toNumber() const;
    virtual String toString() const;
    virtual Object toObject() const;

    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;

    static BooleanImp *staticTrue, *staticFalse;
  private:
    bool val;
  };

  class NumberImp : public Imp {
  public:
    NumberImp(double v);
    virtual ~NumberImp() { }
    double value() const { return val; }
    virtual KJSO toPrimitive(Type preferred = UndefinedType) const;
    virtual Boolean toBoolean() const;
    virtual Number toNumber() const;
    virtual String toString() const;
    virtual Object toObject() const;

    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    double val;
  };

  class StringImp : public Imp {
  public:
    StringImp(const UString& v);
    virtual ~StringImp() { }
    UString value() const { return val; }
    virtual KJSO toPrimitive(Type preferred = UndefinedType) const;
    virtual Boolean toBoolean() const;
    virtual Number toNumber() const;
    virtual String toString() const;
    virtual Object toObject() const;

    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    UString val;
  };

  class ReferenceImp : public Imp {
  public:
    ReferenceImp(const KJSO& b, const UString& p);
    virtual ~ReferenceImp() { }
    virtual void mark(Imp*);
    KJSO getBase() const { return base; }
    UString getPropertyName() const { return prop; }

    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    KJSO base;
    UString prop;
  };

  class CompletionImp : public Imp {
  public:
    CompletionImp(Compl c, const KJSO& v, const UString& t);
    virtual ~CompletionImp() { }
    virtual void mark(Imp*);
    Compl completion() const { return comp; }
    KJSO value() const { return val; }
    UString target() const { return tar; }

    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    Compl comp;
    KJSO val;
    UString tar;
  };

  class RegExpImp : public ObjectImp {
  public:
    RegExpImp();
    ~RegExpImp();
    void setRegExp(RegExp *r) { reg = r; }
    RegExp* regExp() const { return reg; }
  private:
    RegExp *reg;
  };

  class StatementNode;
  class UString;

  class Reference : public KJSO {
  public:
    Reference(const KJSO& b, const UString &p);
    virtual ~Reference();
  };

  /**
   * @short The "label set" in Ecma-262 spec
   */
  class LabelStack {
  public:
    LabelStack(): tos(0L) {}
    ~LabelStack();

    LabelStack(const LabelStack &other);
    LabelStack &operator=(const LabelStack &other);

    /**
     * If id is not empty and is not in the stack already, puts it on top of
     * the stack and returns true, otherwise returns false
     */
    bool push(const UString &id);
    /**
     * Is the id in the stack?
     */
    bool contains(const UString &id) const;
    /**
     * Removes from the stack the last pushed id (what else?)
     */
    void pop();
  private:
    struct StackElem {
      UString id;
      StackElem *prev;
    };

    StackElem *tos;
    void clear();
  };

  /**
   * @short Execution context.
   */
  class Context {
  public:
    Context(CodeType type = GlobalCode, Context *_callingContext = 0L,
	       FunctionImp *func = 0L, const List *args = 0L, Imp *thisV = 0L);
    virtual ~Context();
    const List *pScopeChain() const { return scopeChain; }
    void pushScope(const KJSO &s);
    void popScope();
    List *copyOfChain();
    KJSO variableObject() const { return variable; }
    void setVariableObject( const KJSO &obj ) { variable = obj; }
    KJSO thisValue() const { return thisVal; }
    void setThisValue(const KJSO &t) { thisVal = t; }
    LabelStack *seenLabels() { return &ls; }
    Context *callingContext() { return callingCon; }
  private:
    LabelStack ls;
    KJSO thisVal;
    KJSO activation;
    KJSO variable;
    List *scopeChain;
    CodeType codeType;
    Context *callingCon;
  };

  class DeclaredFunctionImp : public ConstructorImp {
  public:
    DeclaredFunctionImp(const UString &n, FunctionBodyNode *b,
			const List *sc);
    ~DeclaredFunctionImp();
    Completion execute(const List &);
    Object construct(const List &);
    CodeType codeType() const { return FunctionCode; }
    List *scopeChain() const { return scopes; }
    virtual void processVarDecls();
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
    FunctionBodyNode *body;
  private:
    List *scopes;
  };

  class AnonymousFunction : public Function {
  public:
    AnonymousFunction();
    Completion execute(const List &);
    CodeType codeType() const { return AnonymousCode; }
  };

  class ArgumentsImp : public ObjectImp {
  public:
    ArgumentsImp(FunctionImp *func, const List *args);
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  };

  class ActivationImp : public Imp {
  public:
    ActivationImp(FunctionImp *f, const List *args);
    void cleanup();
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    FunctionImp *func;
  };

  class Parser {
  public:
    static ProgramNode *parse(const UChar *code, unsigned int length, int sourceId = -1,
			      int *errType = 0, int *errLine = 0, UString *errMsg = 0);
    static ProgramNode *progNode;
    static int sid;
  };

  class DebuggerImp {
    friend class Debugger;
  public:
    DebuggerImp(Debugger *_dbg);
    ~DebuggerImp();
    bool hitStatement(KJScriptImp *script, Context *context, int sid, int line0, int line1);

  protected:
    int dmode;
    Debugger *dbg;
    Context *stepFromContext;
  };

  class KJScriptImp {
    friend class ::KJScript;
    friend class Lexer;
    friend class Context;
    friend class Global;
    friend class Collector;
  public:
    KJScriptImp(KJScript *s, KJSO global = KJSO());
    ~KJScriptImp();
    void mark();
    // ### remove current
    static KJScriptImp *current() { return curr; }
    static void setCurrent(KJScriptImp *s) { curr = s; }
    static void setException(Imp *e);
    static void setException(const char *msg);
    bool hadException() { return (exMsg != 0); }
    static KJSO exception();
    static void clearException();

    Context *context() const { return con; }
    void setContext(Context *c) { con = c; }

    Debugger *debugger() const { return dbg; }
    void setDebugger(Debugger *debugger);
    static int nextSourceId() { return nextSid++; }
  private:
    /**
     * Initialize global object and context. For internal use only.
     */
    void init();
    void clear();
    /**
     * Called when the first interpreter is instanciated. Initializes
     * global pointers.
     */
    void globalInit();
    /**
     * Called when the last interpreter instance is destroyed. Frees
     * globally allocated memory.
     */
    void globalClear();
    bool evaluate(const UChar *code, unsigned int length, const KJSO &thisV = KJSO(),
		  bool onlyCheckSyntax = false);
    bool call(const KJSO &scope, const UString &func, const List &args);
    bool call(const KJSO &func, const KJSO &thisV,
	      const List &args, const List &extraScope);

  public:
    KJScriptImp *next, *prev;
    KJScript *scr;

    static void initGlobal(Imp *global);
    // The global object, whatever it is
    KJSO globalObject() const { return glob; }
    // The global object, assuming it was a default GlobalImp (otherwise 0L)
    // ### TODO better name
    Global globalObjectAsGlobal() const;

    KJSO objectPrototype() const;
    KJSO functionPrototype() const;
    void abortExecution();
    void setDebuggingEnabled(bool enabled) { debugEnabled = enabled; }
    bool debuggingEnabled() const { return debugEnabled; }

  private:
    static KJScriptImp *curr, *hook;
    static int instances; // total number of instances
    static int running;	// total number running
    bool initialized;
    Lexer *lex;
    Context *con;
    KJSO glob;
    int errType, errLine;
    UString errMsg;
    Debugger *dbg;
    const char *exMsg;
    Imp *exVal;
    Imp *retVal;
    int recursion;

    static int nextSid;
    bool aborted;
    bool debugEnabled;
  };

  /**
   * @short Struct used to return the property names of an object
   */
  class PropList {
  public:
    PropList(UString nm = UString::null, PropList *nx = 0) :
			  name(nm), next(nx) {};
    ~PropList() {
      if(next) delete next;
    }
    /**
     * The property name
     */
    UString name;
    /**
     * The next property
     */
    PropList *next;
    bool contains(const UString &name);
  };

  /* TODO just temporary until functions are objects and this becomes
     a member function. Called by RelationNode for 'instanceof' operator. */
  KJSO hasInstance(const KJSO &F, const KJSO &V);

#ifndef NDEBUG
  void printInfo( const char *s, const KJSO &o, int lineno = -1 );
#endif

}; // namespace


#endif
