// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
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
 *
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#ifndef NDEBUG
#include <strings.h>      // for strdup
#endif

#include "array_object.h"
#include "bool_object.h"
#include "collector.h"
#include "date_object.h"
#include "debugger.h"
#include "error_object.h"
#include "function_object.h"
#include "internal.h"
#include "lexer.h"
#include "math_object.h"
#include "nodes.h"
#include "number_object.h"
#include "object.h"
#include "object_object.h"
#include "operations.h"
#include "regexp_object.h"
#include "string_object.h"

#define I18N_NOOP(s) s

extern int kjsyyparse();

using namespace KJS;

namespace KJS {
#ifdef WORDS_BIGENDIAN
  unsigned char NaN_Bytes[] = { 0x7f, 0xf8, 0, 0, 0, 0, 0, 0 };
  unsigned char Inf_Bytes[] = { 0x7f, 0xf0, 0, 0, 0, 0, 0, 0 };
#elif defined(arm)
  unsigned char NaN_Bytes[] = { 0, 0, 0xf8, 0x7f, 0, 0, 0, 0 };
  unsigned char Inf_Bytes[] = { 0, 0, 0xf0, 0x7f, 0, 0, 0, 0 };
#else
  unsigned char NaN_Bytes[] = { 0, 0, 0, 0, 0, 0, 0xf8, 0x7f };
  unsigned char Inf_Bytes[] = { 0, 0, 0, 0, 0, 0, 0xf0, 0x7f };
#endif

  const double NaN = *(const double*) NaN_Bytes;
  const double Inf = *(const double*) Inf_Bytes;
};

// ------------------------------ UndefinedImp ---------------------------------

UndefinedImp *UndefinedImp::staticUndefined = 0;

Value UndefinedImp::toPrimitive(ExecState */*exec*/, Type) const
{
  return Value((ValueImp*)this);
}

bool UndefinedImp::toBoolean(ExecState */*exec*/) const
{
  return false;
}

double UndefinedImp::toNumber(ExecState */*exec*/) const
{
  return NaN;
}

UString UndefinedImp::toString(ExecState */*exec*/) const
{
  return "undefined";
}

Object UndefinedImp::toObject(ExecState *exec) const
{
  Object err = Error::create(exec, TypeError, I18N_NOOP("Undefined value"));
  exec->setException(err);
  return err;
}

// ------------------------------ NullImp --------------------------------------

NullImp *NullImp::staticNull = 0;

Value NullImp::toPrimitive(ExecState */*exec*/, Type) const
{
  return Value((ValueImp*)this);
}

bool NullImp::toBoolean(ExecState */*exec*/) const
{
  return false;
}

double NullImp::toNumber(ExecState */*exec*/) const
{
  return 0.0;
}

UString NullImp::toString(ExecState */*exec*/) const
{
  return "null";
}

Object NullImp::toObject(ExecState *exec) const
{
  Object err = Error::create(exec, TypeError, I18N_NOOP("Null value"));
  exec->setException(err);
  return err;
}

// ------------------------------ BooleanImp -----------------------------------

BooleanImp* BooleanImp::staticTrue = 0;
BooleanImp* BooleanImp::staticFalse = 0;

Value BooleanImp::toPrimitive(ExecState */*exec*/, Type) const
{
  return Value((ValueImp*)this);
}

bool BooleanImp::toBoolean(ExecState */*exec*/) const
{
  return val;
}

double BooleanImp::toNumber(ExecState */*exec*/) const
{
  return val ? 1.0 : 0.0;
}

UString BooleanImp::toString(ExecState */*exec*/) const
{
  return val ? "true" : "false";
}

Object BooleanImp::toObject(ExecState *exec) const
{
  List args;
  args.append(Boolean(const_cast<BooleanImp*>(this)));
  return Object::dynamicCast(exec->interpreter()->builtinBoolean().construct(exec,args));
}

// ------------------------------ StringImp ------------------------------------

StringImp::StringImp(const UString& v)
  : val(v)
{
}

Value StringImp::toPrimitive(ExecState */*exec*/, Type) const
{
  return Value((ValueImp*)this);
}

bool StringImp::toBoolean(ExecState */*exec*/) const
{
  return (val.size() > 0);
}

double StringImp::toNumber(ExecState */*exec*/) const
{
  return val.toDouble();
}

UString StringImp::toString(ExecState */*exec*/) const
{
  return val;
}

Object StringImp::toObject(ExecState *exec) const
{
  List args;
  args.append(String(const_cast<StringImp*>(this)));
  return Object::dynamicCast(exec->interpreter()->builtinString().construct(exec,args));
}

// ------------------------------ NumberImp ------------------------------------

NumberImp::NumberImp(double v)
  : val(v)
{
}

Value NumberImp::toPrimitive(ExecState *, Type) const
{
  return Number((NumberImp*)this);
}

bool NumberImp::toBoolean(ExecState *) const
{
  return !((val == 0) /* || (iVal() == N0) */ || isNaN(val));
}

double NumberImp::toNumber(ExecState *) const
{
  return val;
}

UString NumberImp::toString(ExecState *) const
{
  return UString::from(val);
}

Object NumberImp::toObject(ExecState *exec) const
{
  List args;
  args.append(Number(const_cast<NumberImp*>(this)));
  return Object::dynamicCast(exec->interpreter()->builtinNumber().construct(exec,args));
}

// ------------------------------ ReferenceImp ---------------------------------

ReferenceImp::ReferenceImp(const Value& v, const UString& p)
  : base(v.imp()), prop(p)
{
}

void ReferenceImp::mark()
{
  ValueImp::mark();
  if (base && !base->marked())
    base->mark();
}

Value ReferenceImp::toPrimitive(ExecState */*exec*/, Type /*preferredType*/) const
{
  // invalid for Reference
  assert(false);
  return Value();
}

bool ReferenceImp::toBoolean(ExecState */*exec*/) const
{
  // invalid for Reference
  assert(false);
  return false;
}

double ReferenceImp::toNumber(ExecState */*exec*/) const
{
  // invalid for Reference
  assert(false);
  return 0;
}

UString ReferenceImp::toString(ExecState */*exec*/) const
{
  // invalid for Reference
  assert(false);
  return UString::null;
}

Object ReferenceImp::toObject(ExecState */*exec*/) const
{
  // invalid for Reference
  assert(false);
  return Object();
}

// ------------------------------ LabelStack -----------------------------------

LabelStack::LabelStack(const LabelStack &other)
{
  tos = 0;
  *this = other;
}

LabelStack &LabelStack::operator=(const LabelStack &other)
{
  clear();
  tos = 0;
  StackElem *cur = 0;
  StackElem *se = other.tos;
  while (se) {
    StackElem *newPrev = new StackElem;
    newPrev->prev = 0;
    newPrev->id = se->id;
    if (cur)
      cur->prev = newPrev;
    else
      tos = newPrev;
    cur = newPrev;
    se = se->prev;
  }
  return *this;
}

bool LabelStack::push(const UString &id)
{
  if (id.isEmpty() || contains(id))
    return false;

  StackElem *newtos = new StackElem;
  newtos->id = id;
  newtos->prev = tos;
  tos = newtos;
  return true;
}

bool LabelStack::contains(const UString &id) const
{
  if (id.isEmpty())
    return true;

  for (StackElem *curr = tos; curr; curr = curr->prev)
    if (curr->id == id)
      return true;

  return false;
}

void LabelStack::pop()
{
  if (tos) {
    StackElem *prev = tos->prev;
    delete tos;
    tos = prev;
  }
}

LabelStack::~LabelStack()
{
  clear();
}

void LabelStack::clear()
{
  StackElem *prev;

  while (tos) {
    prev = tos->prev;
    delete tos;
    tos = prev;
  }
}

// ------------------------------ CompletionImp --------------------------------

CompletionImp::CompletionImp(ComplType c, const Value& v, const UString& t)
  : comp(c), val(v.imp()), tar(t)
{
}

CompletionImp::~CompletionImp()
{
}

void CompletionImp::mark()
{
  ValueImp::mark();

  if (val && !val->marked())
    val->mark();
}

Value CompletionImp::toPrimitive(ExecState */*exec*/, Type /*preferredType*/) const
{
  // invalid for Completion
  assert(false);
  return Value();
}

bool CompletionImp::toBoolean(ExecState */*exec*/) const
{
  // invalid for Completion
  assert(false);
  return false;
}

double CompletionImp::toNumber(ExecState */*exec*/) const
{
  // invalid for Completion
  assert(false);
  return 0;
}

UString CompletionImp::toString(ExecState */*exec*/) const
{
  // invalid for Completion
  assert(false);
  return UString::null;
}

Object CompletionImp::toObject(ExecState */*exec*/) const
{
  // invalid for Completion
  assert(false);
  return Object();
}

// ------------------------------ ListImp --------------------------------------

#ifdef KJS_DEBUG_MEM
int ListImp::count = 0;
#endif

Value ListImp::toPrimitive(ExecState */*exec*/, Type /*preferredType*/) const
{
  // invalid for List
  assert(false);
  return Value();
}

bool ListImp::toBoolean(ExecState */*exec*/) const
{
  // invalid for List
  assert(false);
  return false;
}

double ListImp::toNumber(ExecState */*exec*/) const
{
  // invalid for List
  assert(false);
  return 0;
}

UString ListImp::toString(ExecState */*exec*/) const
{
  // invalid for List
  assert(false);
  return UString::null;
}

Object ListImp::toObject(ExecState */*exec*/) const
{
  // invalid for List
  assert(false);
  return Object();
}

ListImp::ListImp()
{
#ifdef KJS_DEBUG_MEM
  count++;
#endif

  hook = new ListNode(Null(), 0L, 0L);
  hook->next = hook;
  hook->prev = hook;
  //fprintf(stderr,"ListImp::ListImp %p hook=%p\n",this,hook);
}

ListImp::~ListImp()
{
  //fprintf(stderr,"ListImp::~ListImp %p\n",this);
#ifdef KJS_DEBUG_MEM
  count--;
#endif

  clear();
  delete hook;

  if ( emptyList == this )
    emptyList = 0L;
}

void ListImp::mark()
{
  ListNode *n = hook->next;
  while (n != hook) {
    if (!n->member->marked())
      n->member->mark();
    n = n->next;
  }
  ValueImp::mark();
}

void ListImp::append(const Value& obj)
{
  ListNode *n = new ListNode(obj, hook->prev, hook);
  hook->prev->next = n;
  hook->prev = n;
}

void ListImp::prepend(const Value& obj)
{
  ListNode *n = new ListNode(obj, hook, hook->next);
  hook->next->prev = n;
  hook->next = n;
}

void ListImp::appendList(const List& lst)
{
  ListIterator it = lst.begin();
  ListIterator e = lst.end();
  while(it != e) {
    append(*it);
    ++it;
  }
}

void ListImp::prependList(const List& lst)
{
  ListIterator it = lst.end();
  ListIterator e = lst.begin();
  while(it != e) {
    --it;
    prepend(*it);
  }
}

void ListImp::removeFirst()
{
  erase(hook->next);
}

void ListImp::removeLast()
{
  erase(hook->prev);
}

void ListImp::remove(const Value &obj)
{
  if (obj.isNull())
    return;
  ListNode *n = hook->next;
  while (n != hook) {
    if (n->member == obj.imp()) {
      erase(n);
      return;
    }
    n = n->next;
  }
}

void ListImp::clear()
{
  ListNode *n = hook->next;
  while (n != hook) {
    n = n->next;
    delete n->prev;
  }

  hook->next = hook;
  hook->prev = hook;
}

ListImp *ListImp::copy() const
{
  ListImp* newList = new ListImp;

  ListIterator e = end();
  ListIterator it = begin();

  while(it != e) {
    newList->append(*it);
    ++it;
  }

  //fprintf( stderr, "ListImp::copy returning newList=%p\n", newList );
  return newList;
}

void ListImp::erase(ListNode *n)
{
  if (n != hook) {
    n->next->prev = n->prev;
    n->prev->next = n->next;
    delete n;
  }
}

bool ListImp::isEmpty() const
{
  return (hook->prev == hook);
}

int ListImp::size() const
{
  int s = 0;
  ListNode *node = hook;
  while ((node = node->next) != hook)
    s++;

  return s;
}

Value ListImp::at(int i) const
{
  if (i < 0 || i >= size())
    return Undefined();

  ListIterator it = begin();
  int j = 0;
  while ((j++ < i))
    it++;

  return *it;
}

ListImp *ListImp::emptyList = 0L;

ListImp *ListImp::empty()
{
  if (!emptyList)
    emptyList = new ListImp();
  return emptyList;
}

// ------------------------------ ContextImp -----------------------------------


// ECMA 10.2
ContextImp::ContextImp(Object &glob, ExecState *exec, Object &thisV, CodeType type,
                       ContextImp *_callingContext, FunctionImp *func, const List &args)
{
  codeType = type;
  callingCon = _callingContext;

  // create and initialize activation object (ECMA 10.1.6)
  if (type == FunctionCode || type == AnonymousCode ) {
    activation = Object(new ActivationImp(exec,func,args));
    variable = activation;
  } else {
    activation = Object();
    variable = glob;
  }

  // ECMA 10.2
  switch(type) {
    case EvalCode:
      if (callingCon) {
	scope = callingCon->scopeChain().copy();
	variable = callingCon->variableObject();
	thisVal = callingCon->thisValue();
	break;
      } // else same as GlobalCode
    case GlobalCode:
      scope = List();
      scope.append(glob);
      thisVal = Object(static_cast<ObjectImp*>(glob.imp()));
      break;
    case FunctionCode:
    case AnonymousCode:
      if (type == FunctionCode) {
	scope = func->scope().copy();
	scope.prepend(activation);
      } else {
	scope = List();
	scope.append(activation);
	scope.append(glob);
      }
      variable = activation; // TODO: DontDelete ? (ECMA 10.2.3)
      thisVal = thisV;
      break;
    }

}

ContextImp::~ContextImp()
{
}

void ContextImp::pushScope(const Object &s)
{
  scope.prepend(s);
}

void ContextImp::popScope()
{
  scope.removeFirst();
}

// ------------------------------ Parser ---------------------------------------

ProgramNode *Parser::progNode = 0;
int Parser::sid = 0;

ProgramNode *Parser::parse(const UChar *code, unsigned int length, int *sourceId,
			   int *errLine, UString *errMsg)
{
  if (errLine)
    *errLine = -1;
  if (errMsg)
    *errMsg = 0;

  Lexer::curr()->setCode(code, length);
  progNode = 0;
  sid++;
  if (sourceId)
    *sourceId = sid;
  // Enable this (and the #define YYDEBUG in grammar.y) to debug a parse error
  //extern int kjsyydebug;
  //kjsyydebug=1;
  int parseError = kjsyyparse();
  ProgramNode *prog = progNode;
  progNode = 0;
  sid = -1;

  if (parseError) {
    int eline = Lexer::curr()->lineNo();
    if (errLine)
      *errLine = eline;
    if (errMsg)
      *errMsg = "Parse error at line " + UString::from(eline);
#ifndef NDEBUG
    fprintf(stderr, "KJS: JavaScript parse error at line %d.\n", eline);
#endif
    delete prog;
    return 0;
  }

  return prog;
}

// ------------------------------ InterpreterImp -------------------------------

InterpreterImp* InterpreterImp::s_hook = 0L;

void InterpreterImp::globalInit()
{
  //fprintf( stderr, "InterpreterImp::globalInit()\n" );
  UndefinedImp::staticUndefined = new UndefinedImp();
  UndefinedImp::staticUndefined->ref();
  NullImp::staticNull = new NullImp();
  NullImp::staticNull->ref();
  BooleanImp::staticTrue = new BooleanImp(true);
  BooleanImp::staticTrue->ref();
  BooleanImp::staticFalse = new BooleanImp(false);
  BooleanImp::staticFalse->ref();
}

void InterpreterImp::globalClear()
{
  //fprintf( stderr, "InterpreterImp::globalClear()\n" );
  UndefinedImp::staticUndefined->deref();
  UndefinedImp::staticUndefined->setGcAllowed();
  UndefinedImp::staticUndefined = 0L;
  NullImp::staticNull->deref();
  NullImp::staticNull->setGcAllowed();
  NullImp::staticNull = 0L;
  BooleanImp::staticTrue->deref();
  BooleanImp::staticTrue->setGcAllowed();
  BooleanImp::staticTrue = 0L;
  BooleanImp::staticFalse->deref();
  BooleanImp::staticFalse->setGcAllowed();
  BooleanImp::staticFalse = 0L;
}

InterpreterImp::InterpreterImp(Interpreter *interp, const Object &glob)
{
  // add this interpreter to the global chain
  // as a root set for garbage collection
  if (s_hook) {
    prev = s_hook;
    next = s_hook->next;
    s_hook->next->prev = this;
    s_hook->next = this;
  } else {
    // This is the first interpreter
    s_hook = next = prev = this;
    globalInit();
  }

  m_interpreter = interp;
  global = glob;
  globExec = new ExecState(m_interpreter,0);
  dbg = 0;
  m_compatMode = Interpreter::NativeMode;

  // initialize properties of the global object
  initGlobalObject();

  recursion = 0;
}

void InterpreterImp::initGlobalObject()
{
  // Contructor prototype objects (Object.prototype, Array.prototype etc)

  FunctionPrototypeImp *funcProto = new FunctionPrototypeImp(globExec);
  b_FunctionPrototype = Object(funcProto);
  ObjectPrototypeImp *objProto = new ObjectPrototypeImp(globExec,funcProto);
  b_ObjectPrototype = Object(objProto);
  funcProto->setPrototype(b_ObjectPrototype);

  ArrayPrototypeImp *arrayProto = new ArrayPrototypeImp(globExec,objProto);
  b_ArrayPrototype = Object(arrayProto);
  StringPrototypeImp *stringProto = new StringPrototypeImp(globExec,objProto);
  b_StringPrototype = Object(stringProto);
  BooleanPrototypeImp *booleanProto = new BooleanPrototypeImp(globExec,objProto,funcProto);
  b_BooleanPrototype = Object(booleanProto);
  NumberPrototypeImp *numberProto = new NumberPrototypeImp(globExec,objProto,funcProto);
  b_NumberPrototype = Object(numberProto);
  DatePrototypeImp *dateProto = new DatePrototypeImp(globExec,objProto);
  b_DatePrototype = Object(dateProto);
  RegExpPrototypeImp *regexpProto = new RegExpPrototypeImp(globExec,objProto,funcProto);
  b_RegExpPrototype = Object(regexpProto);
  ErrorPrototypeImp *errorProto = new ErrorPrototypeImp(globExec,objProto,funcProto);
  b_ErrorPrototype = Object(errorProto);

  static_cast<ObjectImp*>(global.imp())->setPrototype(b_ObjectPrototype);

  // Constructors (Object, Array, etc.)

  ObjectObjectImp *objectObj = new ObjectObjectImp(globExec,objProto,funcProto);
  b_Object = Object(objectObj);
  FunctionObjectImp *funcObj = new FunctionObjectImp(globExec,funcProto);
  b_Function = Object(funcObj);
  ArrayObjectImp *arrayObj = new ArrayObjectImp(globExec,funcProto,arrayProto);
  b_Array = Object(arrayObj);
  StringObjectImp *stringObj = new StringObjectImp(globExec,funcProto,stringProto);
  b_String = Object(stringObj);
  BooleanObjectImp *booleanObj = new BooleanObjectImp(globExec,funcProto,booleanProto);
  b_Boolean = Object(booleanObj);
  NumberObjectImp *numberObj = new NumberObjectImp(globExec,funcProto,numberProto);
  b_Number = Object(numberObj);
  DateObjectImp *dateObj = new DateObjectImp(globExec,funcProto,dateProto);
  b_Date = Object(dateObj);
  RegExpObjectImp *regexpObj = new RegExpObjectImp(globExec,regexpProto,funcProto);
  b_RegExp = Object(regexpObj);
  ErrorObjectImp *errorObj = new ErrorObjectImp(globExec,funcProto,errorProto);
  b_Error = Object(errorObj);

  // Error object prototypes
  b_evalErrorPrototype = Object(new NativeErrorPrototypeImp(globExec,errorProto,EvalError,
                                                            "EvalError","EvalError"));
  b_rangeErrorPrototype = Object(new NativeErrorPrototypeImp(globExec,errorProto,RangeError,
                                                            "RangeError","RangeError"));
  b_referenceErrorPrototype = Object(new NativeErrorPrototypeImp(globExec,errorProto,ReferenceError,
                                                            "ReferenceError","ReferenceError"));
  b_syntaxErrorPrototype = Object(new NativeErrorPrototypeImp(globExec,errorProto,SyntaxError,
                                                            "SyntaxError","SyntaxError"));
  b_typeErrorPrototype = Object(new NativeErrorPrototypeImp(globExec,errorProto,TypeError,
                                                            "TypeError","TypeError"));
  b_uriErrorPrototype = Object(new NativeErrorPrototypeImp(globExec,errorProto,URIError,
                                                            "URIError","URIError"));

  // Error objects
  b_evalError = Object(new NativeErrorImp(globExec,funcProto,b_evalErrorPrototype));
  b_rangeError = Object(new NativeErrorImp(globExec,funcProto,b_rangeErrorPrototype));
  b_referenceError = Object(new NativeErrorImp(globExec,funcProto,b_referenceErrorPrototype));
  b_syntaxError = Object(new NativeErrorImp(globExec,funcProto,b_syntaxErrorPrototype));
  b_typeError = Object(new NativeErrorImp(globExec,funcProto,b_typeErrorPrototype));
  b_uriError = Object(new NativeErrorImp(globExec,funcProto,b_uriErrorPrototype));

  // ECMA 15.3.4.1
  funcProto->put(globExec,"constructor", b_Function, DontEnum);

  global.put(globExec,"Object", b_Object, DontEnum);
  global.put(globExec,"Function", b_Function, DontEnum);
  global.put(globExec,"Array", b_Array, DontEnum);
  global.put(globExec,"Boolean", b_Boolean, DontEnum);
  global.put(globExec,"String", b_String, DontEnum);
  global.put(globExec,"Number", b_Number, DontEnum);
  global.put(globExec,"Date", b_Date, DontEnum);
  global.put(globExec,"RegExp", b_RegExp, DontEnum);
  global.put(globExec,"Error", b_Error, DontEnum);
  // Using Internal for those to have something != 0
  // (see kjs_window). Maybe DontEnum would be ok too ?
  global.put(globExec,"EvalError",b_evalError, Internal);
  global.put(globExec,"RangeError",b_rangeError, Internal);
  global.put(globExec,"ReferenceError",b_referenceError, Internal);
  global.put(globExec,"SyntaxError",b_syntaxError, Internal);
  global.put(globExec,"TypeError",b_typeError, Internal);
  global.put(globExec,"URIError",b_uriError, Internal);

  // Set the "constructor" property of all builtin constructors
  objProto->put(globExec, "constructor", b_Object, DontEnum | DontDelete | ReadOnly);
  funcProto->put(globExec, "constructor", b_Function, DontEnum | DontDelete | ReadOnly);
  arrayProto->put(globExec, "constructor", b_Array, DontEnum | DontDelete | ReadOnly);
  booleanProto->put(globExec, "constructor", b_Boolean, DontEnum | DontDelete | ReadOnly);
  stringProto->put(globExec, "constructor", b_String, DontEnum | DontDelete | ReadOnly);
  numberProto->put(globExec, "constructor", b_Number, DontEnum | DontDelete | ReadOnly);
  dateProto->put(globExec, "constructor", b_Date, DontEnum | DontDelete | ReadOnly);
  regexpProto->put(globExec, "constructor", b_RegExp, DontEnum | DontDelete | ReadOnly);
  errorProto->put(globExec, "constructor", b_Error, DontEnum | DontDelete | ReadOnly);
  b_evalErrorPrototype.put(globExec, "constructor", b_evalError, DontEnum | DontDelete | ReadOnly);
  b_rangeErrorPrototype.put(globExec, "constructor", b_rangeError, DontEnum | DontDelete | ReadOnly);
  b_referenceErrorPrototype.put(globExec, "constructor", b_referenceError, DontEnum | DontDelete | ReadOnly);
  b_syntaxErrorPrototype.put(globExec, "constructor", b_syntaxError, DontEnum | DontDelete | ReadOnly);
  b_typeErrorPrototype.put(globExec, "constructor", b_typeError, DontEnum | DontDelete | ReadOnly);
  b_uriErrorPrototype.put(globExec, "constructor", b_uriError, DontEnum | DontDelete | ReadOnly);

  // built-in values
  global.put(globExec, "NaN",        Number(NaN), DontEnum|DontDelete);
  global.put(globExec, "Infinity",   Number(Inf), DontEnum|DontDelete);
  global.put(globExec, "undefined",  Undefined(), DontEnum|DontDelete);

  // built-in functions
  global.put(globExec,"eval",       Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::Eval,       1)), DontEnum);
  global.put(globExec,"parseInt",   Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::ParseInt,   2)), DontEnum);
  global.put(globExec,"parseFloat", Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::ParseFloat, 1)), DontEnum);
  global.put(globExec,"isNaN",      Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::IsNaN,      1)), DontEnum);
  global.put(globExec,"isFinite",   Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::IsFinite,   1)), DontEnum);
  global.put(globExec,"escape",     Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::Escape,     1)), DontEnum);
  global.put(globExec,"unescape",   Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::UnEscape,   1)), DontEnum);

  // built-in objects
  global.put(globExec,"Math", Object(new MathObjectImp(globExec,objProto)), DontEnum);
}

InterpreterImp::~InterpreterImp()
{
  if (dbg)
    dbg->detach(m_interpreter);
  delete globExec;
  globExec = 0L;
  clear();
}

void InterpreterImp::clear()
{
  //fprintf(stderr,"InterpreterImp::clear\n");
  // remove from global chain (see init())
  next->prev = prev;
  prev->next = next;
  s_hook = next;
  if (s_hook == this)
  {
    // This was the last interpreter
    s_hook = 0L;
    globalClear();
  }
}

void InterpreterImp::mark()
{
  //if (exVal && !exVal->marked())
  //  exVal->mark();
  //if (retVal && !retVal->marked())
  //  retVal->mark();
  if (UndefinedImp::staticUndefined && !UndefinedImp::staticUndefined->marked())
    UndefinedImp::staticUndefined->mark();
  if (NullImp::staticNull && !NullImp::staticNull->marked())
    NullImp::staticNull->mark();
  if (BooleanImp::staticTrue && !BooleanImp::staticTrue->marked())
    BooleanImp::staticTrue->mark();
  if (BooleanImp::staticFalse && !BooleanImp::staticFalse->marked())
    BooleanImp::staticFalse->mark();
  if (ListImp::emptyList && !ListImp::emptyList->marked())
    ListImp::emptyList->mark();
  //fprintf( stderr, "InterpreterImp::mark this=%p global.imp()=%p\n", this, global.imp() );
  if (global.imp())
    global.imp()->mark();
  if (m_interpreter)
    m_interpreter->mark();
}

bool InterpreterImp::checkSyntax(const UString &code)
{
  // Parser::parse() returns 0 in a syntax error occurs, so we just check for that
  ProgramNode *progNode = Parser::parse(code.data(),code.size(),0,0,0);
  bool ok = (progNode != 0);
  delete progNode;
  return ok;
}

Completion InterpreterImp::evaluate(const UString &code, const Value &thisV)
{
  // prevent against infinite recursion
  if (recursion >= 20) {
    return Completion(Throw,Error::create(globExec,GeneralError,"Recursion too deep"));
  }

  // parse the source code
  int sid;
  int errLine;
  UString errMsg;
  ProgramNode *progNode = Parser::parse(code.data(),code.size(),&sid,&errLine,&errMsg);

  // notify debugger that source has been parsed
  if (dbg) {
    bool cont = dbg->sourceParsed(globExec,sid,code,errLine);
    if (!cont)
      return Completion(Break);
  }

  // no program node means a syntax occurred
  if (!progNode) {
    Object err = Error::create(globExec,SyntaxError,errMsg.ascii(),errLine);
    err.put(globExec,"sid",Number(sid));
    return Completion(Throw,err);
  }

  globExec->clearException();

  recursion++;
  progNode->ref();

  Object globalObj = globalObject();
  Object thisObj = globalObject();

  if (!thisV.isNull()) {
    // "this" must be an object... use same rules as Function.prototype.apply()
    if (thisV.isA(NullType) || thisV.isA(UndefinedType))
      thisObj = globalObject();
    else {
      thisObj = thisV.toObject(globExec);
    }
  }

  Completion res;
  if (globExec->hadException()) {
    // the thisArg.toObject() conversion above might have thrown an exception - if so,
    // propagate it back
    res = Completion(Throw,globExec->exception());
  }
  else {
    // execute the code
    ExecState *exec1 = 0;
    ContextImp *ctx = new ContextImp(globalObj, exec1, thisObj);
    ExecState *newExec = new ExecState(m_interpreter,ctx);

    res = progNode->execute(newExec);

    delete newExec;
    delete ctx;
  }

  if (progNode->deref())
    delete progNode;
  recursion--;

  return res;
}

void InterpreterImp::setDebugger(Debugger *d)
{
  if (d)
    d->detach(m_interpreter);
  dbg = d;
}

// ------------------------------ InternalFunctionImp --------------------------

const ClassInfo InternalFunctionImp::info = {"Function", 0, 0, 0};

InternalFunctionImp::InternalFunctionImp(FunctionPrototypeImp *funcProto)
  : ObjectImp(Object(funcProto))
{
}

bool InternalFunctionImp::implementsHasInstance() const
{
  return true;
}

Boolean InternalFunctionImp::hasInstance(ExecState *exec, const Value &value)
{
  if (value.type() != ObjectType)
    return Boolean(false);

  Value prot = get(exec,"prototype");
  if (prot.type() != ObjectType && prot.type() != NullType) {
    Object err = Error::create(exec, TypeError, "Invalid prototype encountered "
                               "in instanceof operation.");
    exec->setException(err);
    return Boolean(false);
  }

  Object v = Object(static_cast<ObjectImp*>(value.imp()));
  while ((v = Object::dynamicCast(v.prototype())).imp()) {
    if (v.imp() == prot.imp())
      return Boolean(true);
  }
  return Boolean(false);
}

// ------------------------------ global functions -----------------------------

double KJS::roundValue(ExecState *exec, const Value &v)
{
  if (v.type() == UndefinedType) /* TODO: see below */
    return 0.0;
  Number n = v.toNumber(exec);
  if (n.value() == 0.0)   /* TODO: -0, NaN, Inf */
    return 0.0;
  double d = floor(fabs(n.value()));
  if (n.value() < 0)
    d *= -1;

  return d;
}

#ifndef NDEBUG
#include <stdio.h>
void KJS::printInfo(ExecState *exec, const char *s, const Value &o, int lineno)
{
  if (o.isNull())
    fprintf(stderr, "KJS: %s: (null)", s);
  else {
    Value v = o;
    if (o.isA(ReferenceType))
      v = o.getValue(exec);

    UString name;
    switch ( v.type() ) {
    case UnspecifiedType:
      name = "Unspecified";
      break;
    case UndefinedType:
      name = "Undefined";
      break;
    case NullType:
      name = "Null";
      break;
    case BooleanType:
      name = "Boolean";
      break;
    case StringType:
      name = "String";
      break;
    case NumberType:
      name = "Number";
      break;
    case ObjectType:
      name = Object::dynamicCast(v).className();
      if (name.isNull())
        name = "(unknown class)";
      break;
    case ReferenceType:
      name = "Reference";
      break;
    case ListType:
      name = "List";
      break;
    case CompletionType:
      name = "Completion";
      break;
    default:
      break;
    }
    UString vString = v.toString(exec);
    if ( vString.size() > 50 )
      vString = vString.substr( 0, 50 ) + "...";
    // Can't use two UString::ascii() in the same fprintf call
    CString tempString( vString.cstring() );

    fprintf(stderr, "KJS: %s: %s : %s (%p)",
            s, tempString.c_str(), name.ascii(), (void*)v.imp());

    if (lineno >= 0)
      fprintf(stderr, ", line %d\n",lineno);
    else
      fprintf(stderr, "\n");
    if (!o.isNull())
      if (o.isA(ReferenceType)) {
        fprintf(stderr, "KJS: Was property '%s'\n", o.getPropertyName(exec).ascii());
        printInfo(exec,"of", o.getBase(exec));
      }
  }
}
#endif
