// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2002 Apple Computer, Inc.
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
 *
 */

#ifndef _INTERNAL_H_
#define _INTERNAL_H_

#include "ustring.h"
#include "value.h"
#include "object.h"
#include "types.h"
#include "interpreter.h"

#ifndef I18N_NOOP
#define I18N_NOOP(s) s
#endif

namespace KJS {

  static const double D16 = 65536.0;
  static const double D32 = 4294967296.0;

  class ProgramNode;
  class FunctionBodyNode;
  class FunctionPrototypeImp;
  class FunctionImp;
  class Debugger;

  // ---------------------------------------------------------------------------
  //                            Primitive impls
  // ---------------------------------------------------------------------------

  class UndefinedImp : public ValueImp {
  public:
    Type type() const { return UndefinedType; }

    Value toPrimitive(ExecState *exec, Type preferred = UnspecifiedType) const;
    bool toBoolean(ExecState *exec) const;
    double toNumber(ExecState *exec) const;
    UString toString(ExecState *exec) const;
    Object toObject(ExecState *exec) const;

    static UndefinedImp *staticUndefined;
  };

  inline Undefined::Undefined(UndefinedImp *imp) : Value(imp) { }

  class NullImp : public ValueImp {
  public:
    Type type() const { return NullType; }

    Value toPrimitive(ExecState *exec, Type preferred = UnspecifiedType) const;
    bool toBoolean(ExecState *exec) const;
    double toNumber(ExecState *exec) const;
    UString toString(ExecState *exec) const;
    Object toObject(ExecState *exec) const;

    static NullImp *staticNull;
  };

  inline Null::Null(NullImp *imp) : Value(imp) { }

  class BooleanImp : public ValueImp {
  public:
    BooleanImp(bool v = false) : val(v) { }
    bool value() const { return val; }

    Type type() const { return BooleanType; }

    Value toPrimitive(ExecState *exec, Type preferred = UnspecifiedType) const;
    bool toBoolean(ExecState *exec) const;
    double toNumber(ExecState *exec) const;
    UString toString(ExecState *exec) const;
    Object toObject(ExecState *exec) const;

    static BooleanImp *staticTrue;
    static BooleanImp *staticFalse;
  private:
    bool val;
  };

  inline Boolean::Boolean(BooleanImp *imp) : Value(imp) { }

  class StringImp : public ValueImp {
  public:
    StringImp(const UString& v) : val(v) { }
    UString value() const { return val; }

    Type type() const { return StringType; }

    Value toPrimitive(ExecState *exec, Type preferred = UnspecifiedType) const;
    bool toBoolean(ExecState *exec) const;
    double toNumber(ExecState *exec) const;
    UString toString(ExecState *exec) const;
    Object toObject(ExecState *exec) const;

  private:
    UString val;
  };

  inline String::String(StringImp *imp) : Value(imp) { }

  class NumberImp : public ValueImp {
  public:
    NumberImp(double v);
    double value() const { return val; }

    Type type() const { return NumberType; }

    Value toPrimitive(ExecState *exec, Type preferred = UnspecifiedType) const;
    bool toBoolean(ExecState *exec) const;
    double toNumber(ExecState *exec) const;
    UString toString(ExecState *exec) const;
    Object toObject(ExecState *exec) const;
    static NumberImp *staticNaN;

  private:
    double val;
  };

  // ---------------------------------------------------------------------------
  //                            Internal type impls
  // ---------------------------------------------------------------------------

  // TODO: remove. replaced by light-weight new Reference2 class
  class ReferenceImp : public ValueImp {
  public:

    ReferenceImp(const Value& v, const UString& p);
    virtual ~ReferenceImp() { }
    virtual void mark();

    Value toPrimitive(ExecState *exec, Type preferred = UnspecifiedType) const;
    bool toBoolean(ExecState *exec) const;
    double toNumber(ExecState *exec) const;
    UString toString(ExecState *exec) const;
    Object toObject(ExecState *exec) const;

    Value getBase() const { return Value(base); }
    UString getPropertyName() const { return prop; }

    Type type() const { return ReferenceType; }

  private:
    ValueImp *base;
    UString prop;
  };

  class CompletionImp : public ValueImp {
  public:
    Type type() const { return CompletionType; }

    CompletionImp(ComplType c, const Value& v, const UString& t);
    virtual ~CompletionImp();
    virtual void mark();

    Value toPrimitive(ExecState *exec, Type preferred = UnspecifiedType) const;
    bool toBoolean(ExecState *exec) const;
    double toNumber(ExecState *exec) const;
    UString toString(ExecState *exec) const;
    Object toObject(ExecState *exec) const;

    ComplType complType() const { return comp; }
    Value value() const { return Value(val); }
    UString target() const { return tar; }

  private:
    ComplType comp;
    ValueImp * val;
    UString tar;
  };

  /**
   * @internal
   */
  /* TODO: KDE 4.0: delete Reference in types.h and rename this one. */
  class Reference2 {
  public:
    /**
     * Constructs an invalid reference
     */
    Reference2() { }
    /**
     * Constructs an invalid reference containing a value instead.
     */
    Reference2(const Value& v) : bs(v) { }
    /**
     * Constructs an reference with the specified base and property.
     */
    Reference2(const Value& v, const UString& p) : bs(v), prop(p) { }

    bool isValid() const { return bs.isValid() && !prop.isNull(); }


    // ECMA 8.7.1
    Value base() const { return Value(bs); }
    // ECMA 8.7.2
    UString propertyName() const { return prop; }

    // ECMA 8.7.1
    Value getValue(ExecState *exec) const;
    void putValue(ExecState *exec, const Value& w);

  private:
    Value bs;
    UString prop;
  };

  /**
   * @internal
   */
  class ListNode {
    friend class List;
    friend class ListImp;
    friend class ListIterator;
    ListNode(Value val, ListNode *p, ListNode *n)
      : member(val.imp()), prev(p), next(n) {};
    ValueImp *member;
    ListNode *prev, *next;
  };

  class ListImp : public ValueImp {
    friend class ListIterator;
    friend class List;
    friend class InterpreterImp;
  public:
    ListImp();
    ~ListImp();

    Type type() const { return ListType; }

    virtual void mark();

    Value toPrimitive(ExecState *exec, Type preferred = UnspecifiedType) const;
    bool toBoolean(ExecState *exec) const;
    double toNumber(ExecState *exec) const;
    UString toString(ExecState *exec) const;
    Object toObject(ExecState *exec) const;

    void append(const Value& val);
    void prepend(const Value& val);
    void appendList(const List& lst);
    void prependList(const List& lst);
    void removeFirst();
    void removeLast();
    void remove(const Value &val);
    void clear();
    ListImp *copy() const;
    ListIterator begin() const { return ListIterator(hook->next); }
    ListIterator end() const { return ListIterator(hook); }
    //    bool isEmpty() const { return (hook->prev == hook); }
    bool isEmpty() const;
    int size() const;
    Value at(int i) const;
    Value operator[](int i) const { return at(i); }
    static ListImp* empty();

#ifdef KJS_DEBUG_MEM
    static int count;
#endif
  private:
    void erase(ListNode *n);
    ListNode *hook;
    static ListImp *emptyList;
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


  // ---------------------------------------------------------------------------
  //                            Parsing & evaluateion
  // ---------------------------------------------------------------------------

  /**
   * @short Execution context.
   */
  class ContextImp {
    friend class Context;
    friend class StatementNode;
  public:
    // TODO: remove glob parameter. deducable from exec.
    ContextImp(Object &glob, ExecState *exec, Object &thisV, int _sourceId, CodeType type = GlobalCode,
               ContextImp *_callingContext = 0L, FunctionImp *func = 0L, const List &_args = List());
    virtual ~ContextImp();

    const List scopeChain() const { return scope; }
    Object variableObject() const { return variable; }
    void setVariableObject(const Object &v) { variable = v; }
    Object thisValue() const { return thisVal; }
    ContextImp *callingContext() { return callingCon; }
    Object activationObject() { return activation; }

    void pushScope(const Object &s);
    void popScope();
    LabelStack *seenLabels() { return &ls; }

    void pushTryCatch() { tryCatch++; };
    void popTryCatch() { tryCatch--; };
    bool inTryCatch() const;

    void setLines(int l0, int l1) { line0 = l0; line1 = l1; }

  private:

    List scope;
    Object activation;
    Object variable;
    Object thisVal;

    ContextImp *callingCon;

    LabelStack ls;
    CodeType codeType;
    int tryCatch;

    int sourceId;
    int line0;
    int line1;
    Object function;
    UString functionName;
    List args;
  };

  class SourceCode {
  public:
    SourceCode(int _sid)
      : sid(_sid), interpreter(0), refcount(0), next(0) {}

    void ref() { refcount++; }
    void deref() { if (!--refcount) cleanup(); }
    void cleanup();

    int sid;
    InterpreterImp *interpreter;
    int refcount;
    SourceCode *next;
  };

  /**
   * @internal
   *
   * Parses ECMAScript source code and converts into ProgramNode objects, which
   * represent the root of a parse tree. This class provides a conveniant workaround
   * for the problem of the bison parser working in a static context.
   */
  class Parser {
  public:
    static ProgramNode *parse(const UChar *code, unsigned int length, SourceCode **src,
			      int *errLine = 0, UString *errMsg = 0);

    static ProgramNode *progNode;
    static SourceCode *source;
    static int sid;
  private:
  };

  class InterpreterImp {
    friend class Collector;
  public:
    static void globalInit();
    static void globalClear();

    InterpreterImp(Interpreter *interp, const Object &glob);
    ~InterpreterImp();

    Object globalObject() const { return global; }
    Interpreter* interpreter() const { return m_interpreter; }

    void initGlobalObject();

    void mark();

    ExecState *globalExec() { return globExec; }
    bool checkSyntax(const UString &code);
    Completion evaluate(const UString &code, const Value &thisV);
    Debugger *debugger() const { return dbg; }
    void setDebugger(Debugger *d);

    Object builtinObject() const { return b_Object; }
    Object builtinFunction() const { return b_Function; }
    Object builtinArray() const { return b_Array; }
    Object builtinBoolean() const { return b_Boolean; }
    Object builtinString() const { return b_String; }
    Object builtinNumber() const { return b_Number; }
    Object builtinDate() const { return b_Date; }
    Object builtinRegExp() const { return b_RegExp; }
    Object builtinError() const { return b_Error; }

    Object builtinObjectPrototype() const { return b_ObjectPrototype; }
    Object builtinFunctionPrototype() const { return b_FunctionPrototype; }
    Object builtinArrayPrototype() const { return b_ArrayPrototype; }
    Object builtinBooleanPrototype() const { return b_BooleanPrototype; }
    Object builtinStringPrototype() const { return b_StringPrototype; }
    Object builtinNumberPrototype() const { return b_NumberPrototype; }
    Object builtinDatePrototype() const { return b_DatePrototype; }
    Object builtinRegExpPrototype() const { return b_RegExpPrototype; }
    Object builtinErrorPrototype() const { return b_ErrorPrototype; }

    Object builtinEvalError() const { return b_evalError; }
    Object builtinRangeError() const { return b_rangeError; }
    Object builtinReferenceError() const { return b_referenceError; }
    Object builtinSyntaxError() const { return b_syntaxError; }
    Object builtinTypeError() const { return b_typeError; }
    Object builtinURIError() const { return b_uriError; }

    Object builtinEvalErrorPrototype() const { return b_evalErrorPrototype; }
    Object builtinRangeErrorPrototype() const { return b_rangeErrorPrototype; }
    Object builtinReferenceErrorPrototype() const { return b_referenceErrorPrototype; }
    Object builtinSyntaxErrorPrototype() const { return b_syntaxErrorPrototype; }
    Object builtinTypeErrorPrototype() const { return b_typeErrorPrototype; }
    Object builtinURIErrorPrototype() const { return b_uriErrorPrototype; }

    void setCompatMode(Interpreter::CompatMode mode) { m_compatMode = mode; }
    Interpreter::CompatMode compatMode() const { return m_compatMode; }

    // Chained list of interpreters (ring)
    static InterpreterImp* firstInterpreter() { return s_hook; }
    InterpreterImp *nextInterpreter() const { return next; }
    InterpreterImp *prevInterpreter() const { return prev; }

    void addSourceCode(SourceCode *code);
    void removeSourceCode(SourceCode *code);

  private:
    void clear();
    Interpreter *m_interpreter;
    Object global;
    Debugger *dbg;

    // Built-in properties of the object prototype. These are accessible
    // from here even if they are replaced by js code (e.g. assigning to
    // Array.prototype)

    Object b_Object;
    Object b_Function;
    Object b_Array;
    Object b_Boolean;
    Object b_String;
    Object b_Number;
    Object b_Date;
    Object b_RegExp;
    Object b_Error;

    Object b_ObjectPrototype;
    Object b_FunctionPrototype;
    Object b_ArrayPrototype;
    Object b_BooleanPrototype;
    Object b_StringPrototype;
    Object b_NumberPrototype;
    Object b_DatePrototype;
    Object b_RegExpPrototype;
    Object b_ErrorPrototype;

    Object b_evalError;
    Object b_rangeError;
    Object b_referenceError;
    Object b_syntaxError;
    Object b_typeError;
    Object b_uriError;

    Object b_evalErrorPrototype;
    Object b_rangeErrorPrototype;
    Object b_referenceErrorPrototype;
    Object b_syntaxErrorPrototype;
    Object b_typeErrorPrototype;
    Object b_uriErrorPrototype;

    ExecState *globExec;
    Interpreter::CompatMode m_compatMode;

    // Chained list of interpreters (ring) - for collector
    static InterpreterImp* s_hook;
    InterpreterImp *next, *prev;

    int recursion;
    SourceCode *sources;
  };

  class ExecStateImp {
  public:
    ExecStateImp(Interpreter *interp, ContextImp *con)
      : interpreter(interp), context(con) {};
    Interpreter *interpreter;
    ContextImp *context;
    Value exception;
  };

  class AttachedInterpreter;
  class DebuggerImp {
  public:

    DebuggerImp() {
      interps = 0;
      isAborted = false;
    }

    void abort() { isAborted = true; }
    bool aborted() const { return isAborted; }

    AttachedInterpreter *interps;
    bool isAborted;
  };



  class InternalFunctionImp : public ObjectImp {
  public:
    InternalFunctionImp(FunctionPrototypeImp *funcProto);
    bool implementsHasInstance() const;
    Boolean hasInstance(ExecState *exec, const Value &value);

    virtual const ClassInfo *classInfo() const { return &info; }
    static const ClassInfo info;
  };

  // helper function for toInteger, toInt32, toUInt32 and toUInt16
  double roundValue(ExecState *exec, const Value &v);

#ifndef NDEBUG
  void printInfo(ExecState *exec, const char *s, const Value &o, int lineno = -1);
#endif

} // namespace


#endif //  _INTERNAL_H_
