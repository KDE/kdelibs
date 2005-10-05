// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2002 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2004 Apple Computer, Inc.
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

#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "array_object.h"
#include "bool_object.h"
#include "collector.h"
#include "context.h"
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
  /* work around some strict alignment requirements
     for double variables on some architectures (e.g. PA-RISC) */
  typedef union { unsigned char b[8]; double d; } kjs_double_t;

#ifdef WORDS_BIGENDIAN
  static const kjs_double_t NaN_Bytes = { { 0x7f, 0xf8, 0, 0, 0, 0, 0, 0 } };
  static const kjs_double_t Inf_Bytes = { { 0x7f, 0xf0, 0, 0, 0, 0, 0, 0 } };
#elif defined(arm)
  static const kjs_double_t NaN_Bytes = { { 0, 0, 0xf8, 0x7f, 0, 0, 0, 0 } };
  static const kjs_double_t Inf_Bytes = { { 0, 0, 0xf0, 0x7f, 0, 0, 0, 0 } };
#else
  static const kjs_double_t NaN_Bytes = { { 0, 0, 0, 0, 0, 0, 0xf8, 0x7f } };
  static const kjs_double_t Inf_Bytes = { { 0, 0, 0, 0, 0, 0, 0xf0, 0x7f } };
#endif

  const double NaN = NaN_Bytes.d;
  const double Inf = Inf_Bytes.d;
}

#ifdef KJS_THREADSUPPORT
static pthread_once_t interpreterLockOnce = PTHREAD_ONCE_INIT;
static pthread_mutex_t interpreterLock;
static int interpreterLockCount = 0;

static void initializeInterpreterLock()
{
  pthread_mutexattr_t attr;

  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);

  pthread_mutex_init(&interpreterLock, &attr);
}
#endif

static inline void lockInterpreter()
{
#ifdef KJS_THREADSUPPORT
  pthread_once(&interpreterLockOnce, initializeInterpreterLock);
  pthread_mutex_lock(&interpreterLock);
  interpreterLockCount++;
#endif
}

static inline void unlockInterpreter()
{
#ifdef KJS_THREADSUPPORT
  interpreterLockCount--;
  pthread_mutex_unlock(&interpreterLock);
#endif
}



// ------------------------------ UndefinedImp ---------------------------------

UndefinedImp *UndefinedImp::staticUndefined = 0;

Value UndefinedImp::toPrimitive(ExecState* /*exec*/, Type) const
{
  return Value((ValueImp*)this);
}

bool UndefinedImp::toBoolean(ExecState* /*exec*/) const
{
  return false;
}

double UndefinedImp::toNumber(ExecState* /*exec*/) const
{
  return NaN;
}

UString UndefinedImp::toString(ExecState* /*exec*/) const
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

Value NullImp::toPrimitive(ExecState* /*exec*/, Type) const
{
  return Value((ValueImp*)this);
}

bool NullImp::toBoolean(ExecState* /*exec*/) const
{
  return false;
}

double NullImp::toNumber(ExecState* /*exec*/) const
{
  return 0.0;
}

UString NullImp::toString(ExecState* /*exec*/) const
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

Value BooleanImp::toPrimitive(ExecState* /*exec*/, Type) const
{
  return Value((ValueImp*)this);
}

bool BooleanImp::toBoolean(ExecState* /*exec*/) const
{
  return val;
}

double BooleanImp::toNumber(ExecState* /*exec*/) const
{
  return val ? 1.0 : 0.0;
}

UString BooleanImp::toString(ExecState* /*exec*/) const
{
  return val ? "true" : "false";
}

Object BooleanImp::toObject(ExecState *exec) const
{
  List args;
  args.append(const_cast<BooleanImp*>(this));
  return Object::dynamicCast(exec->lexicalInterpreter()->builtinBoolean().construct(exec,args));
}

// ------------------------------ StringImp ------------------------------------

Value StringImp::toPrimitive(ExecState* /*exec*/, Type) const
{
  return Value((ValueImp*)this);
}

bool StringImp::toBoolean(ExecState* /*exec*/) const
{
  return (val.size() > 0);
}

double StringImp::toNumber(ExecState* /*exec*/) const
{
  return val.toDouble();
}

UString StringImp::toString(ExecState* /*exec*/) const
{
  return val;
}

Object StringImp::toObject(ExecState *exec) const
{
  List args;
  args.append(const_cast<StringImp*>(this));
  return Object(static_cast<ObjectImp *>(exec->lexicalInterpreter()->builtinString().construct(exec, args).imp()));
}

// ------------------------------ NumberImp ------------------------------------

NumberImp *NumberImp::staticNaN;

ValueImp *NumberImp::create(int i)
{
    if (SimpleNumber::fits(i))
        return SimpleNumber::make(i);
    NumberImp *imp = new NumberImp(static_cast<double>(i));
    imp->setGcAllowedFast();
    return imp;
}

ValueImp *NumberImp::create(double d)
{
    if (SimpleNumber::fits(d))
        return SimpleNumber::make((int)d);
    if (isNaN(d))
        return staticNaN;
    NumberImp *imp = new NumberImp(d);
    imp->setGcAllowedFast();
    return imp;
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
  if (val == 0.0) // +0.0 or -0.0
    return "0";
  return UString::from(val);
}

Object NumberImp::toObject(ExecState *exec) const
{
  List args;
  args.append(const_cast<NumberImp*>(this));
  return Object::dynamicCast(exec->lexicalInterpreter()->builtinNumber().construct(exec,args));
}

bool NumberImp::toUInt32(unsigned& uint32) const
{
  uint32 = (unsigned)val;
  return (double)uint32 == val;
}

double SimpleNumber::negZero = -0.0;

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

bool LabelStack::push(const Identifier &id)
{
  if (id.isEmpty() || contains(id))
    return false;

  StackElem *newtos = new StackElem;
  newtos->id = id;
  newtos->prev = tos;
  tos = newtos;
  return true;
}

bool LabelStack::contains(const Identifier &id) const
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

// ------------------------------ ContextImp -----------------------------------


// ECMA 10.2
ContextImp::ContextImp(Object &glob, InterpreterImp *interpreter, Object &thisV, int _sourceId, CodeType type,
                       ContextImp *callingCon, FunctionImp *func, const List *args)
  : _interpreter(interpreter), _function(func), _arguments(args)
{
  m_codeType = type;
  _callingContext = callingCon;
  tryCatch = 0;

  sourceId = _sourceId;
  line0 = 1;
  line1 = 1;

  if (func && func->inherits(&DeclaredFunctionImp::info))
    functionName = static_cast<DeclaredFunctionImp*>(func)->name();
  else
    functionName = Identifier::null();

  // create and initialize activation object (ECMA 10.1.6)
  if (type == FunctionCode) {
    activation = Object(new ActivationImp(func,*args));
    variable = activation;
  } else {
    activation = Object();
    variable = glob;
  }

  // ECMA 10.2
  switch(type) {
    case EvalCode:
      if (_callingContext) {
	scope = _callingContext->scopeChain();
#ifndef KJS_PURE_ECMA
	if (thisV.imp() != glob.imp())
	  scope.push(thisV.imp()); // for deprecated Object.prototype.eval()
#endif
	variable = _callingContext->variableObject();
	thisVal = _callingContext->thisValue();
	break;
      } // else same as GlobalCode
    case GlobalCode:
      scope.clear();
      scope.push(glob.imp());
#ifndef KJS_PURE_ECMA
      if (thisV.isValid())
          thisVal = thisV;
      else
#endif
          thisVal = glob;
      break;
    case FunctionCode:
      scope = func->scope();
      scope.push(activation.imp());
      variable = activation; // TODO: DontDelete ? (ECMA 10.2.3)
      thisVal = thisV;
      break;
    }

  _interpreter->setContext(this);
}

ContextImp::~ContextImp()
{
  _interpreter->setContext(_callingContext);
}

void ContextImp::mark()
{
  for (ContextImp *context = this; context; context = context->_callingContext) {
    context->scope.mark();
  }
}

bool ContextImp::inTryCatch() const
{
  const ContextImp *c = this;
  while (c && !c->tryCatch)
    c = c->_callingContext;
  return (c && c->tryCatch);
}

// ---------------------------- SourceCode -------------------------------------

void SourceCode::cleanup()
{
  if (interpreter && interpreter->debugger())
    interpreter->debugger()->sourceUnused(interpreter->globalExec(),sid);
  if (interpreter)
    interpreter->removeSourceCode(this);
  delete this;
}

// ------------------------------ Parser ---------------------------------------

FunctionBodyNode *Parser::progNode = 0;
int Parser::sid = 0;
SourceCode *Parser::source = 0;

FunctionBodyNode *Parser::parse(const UChar *code, unsigned int length, SourceCode **src,
				int *errLine, UString *errMsg)
{
  if (errLine)
    *errLine = -1;
  if (errMsg)
    *errMsg = 0;

  Lexer::curr()->setCode(code, length);
  progNode = 0;
  sid++;

  source = new SourceCode(sid);
  source->ref();
  *src = source;

  // Enable this (and the #define YYDEBUG in grammar.y) to debug a parse error
  //extern int kjsyydebug;
  //kjsyydebug=1;
  int parseError = kjsyyparse();
  if (Lexer::curr()->hadError())
    parseError = 1;
  Lexer::curr()->doneParsing();
  FunctionBodyNode *prog = progNode;
  progNode = 0;
  //sid = -1;
  source = 0;

  if (parseError) {
    int eline = Lexer::curr()->lineNo();
    if (errLine)
      *errLine = eline;
    if (errMsg)
      *errMsg = "Parse error at line " + UString::from(eline);
#ifdef KJS_VERBOSE
    fprintf( stderr, "%s\n", UString(code,length).ascii() );
#endif
#ifndef NDEBUG
    fprintf(stderr, "KJS: JavaScript parse error at line %d.\n", eline);
#endif
    delete prog;
    return 0;
  }
#ifdef KJS_VERBOSE
  fprintf( stderr, "%s\n", prog->toCode().ascii() );
#endif

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
  NumberImp::staticNaN = new NumberImp(NaN);
  NumberImp::staticNaN->ref();
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
  NumberImp::staticNaN->deref();
  NumberImp::staticNaN->setGcAllowed();
  NumberImp::staticNaN = 0;
}

InterpreterImp::InterpreterImp(Interpreter *interp, const Object &glob)
  : m_interpreter(interp),
    global(glob),
    dbg(0),
    m_compatMode(Interpreter::NativeMode),
    _context(0),
    recursion(0),
    sources(0)
{
  // add this interpreter to the global chain
  // as a root set for garbage collection
  lockInterpreter();
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
  unlockInterpreter();

  globExec = new ExecState(m_interpreter,0);

  // initialize properties of the global object
  initGlobalObject();
}

void InterpreterImp::lock()
{
  lockInterpreter();
}

void InterpreterImp::unlock()
{
  unlockInterpreter();
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

  b_Object = Object(new ObjectObjectImp(globExec, objProto, funcProto));
  b_Function = Object(new FunctionObjectImp(globExec, funcProto));
  b_Array = Object(new ArrayObjectImp(globExec, funcProto, arrayProto));
  b_String = Object(new StringObjectImp(globExec, funcProto, stringProto));
  b_Boolean = Object(new BooleanObjectImp(globExec, funcProto, booleanProto));
  b_Number = Object(new NumberObjectImp(globExec, funcProto, numberProto));
  b_Date = Object(new DateObjectImp(globExec, funcProto, dateProto));
  b_RegExp = Object(new RegExpObjectImp(globExec, funcProto, regexpProto));
  b_Error = Object(new ErrorObjectImp(globExec, funcProto, errorProto));

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
  funcProto->put(globExec,constructorPropertyName, b_Function, DontEnum);

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
  objProto->put(globExec, constructorPropertyName, b_Object, DontEnum | DontDelete | ReadOnly);
  funcProto->put(globExec, constructorPropertyName, b_Function, DontEnum | DontDelete | ReadOnly);
  arrayProto->put(globExec, constructorPropertyName, b_Array, DontEnum | DontDelete | ReadOnly);
  booleanProto->put(globExec, constructorPropertyName, b_Boolean, DontEnum | DontDelete | ReadOnly);
  stringProto->put(globExec, constructorPropertyName, b_String, DontEnum | DontDelete | ReadOnly);
  numberProto->put(globExec, constructorPropertyName, b_Number, DontEnum | DontDelete | ReadOnly);
  dateProto->put(globExec, constructorPropertyName, b_Date, DontEnum | DontDelete | ReadOnly);
  regexpProto->put(globExec, constructorPropertyName, b_RegExp, DontEnum | DontDelete | ReadOnly);
  errorProto->put(globExec, constructorPropertyName, b_Error, DontEnum | DontDelete | ReadOnly);
  b_evalErrorPrototype.put(globExec, constructorPropertyName, b_evalError, DontEnum | DontDelete | ReadOnly);
  b_rangeErrorPrototype.put(globExec, constructorPropertyName, b_rangeError, DontEnum | DontDelete | ReadOnly);
  b_referenceErrorPrototype.put(globExec, constructorPropertyName, b_referenceError, DontEnum | DontDelete | ReadOnly);
  b_syntaxErrorPrototype.put(globExec, constructorPropertyName, b_syntaxError, DontEnum | DontDelete | ReadOnly);
  b_typeErrorPrototype.put(globExec, constructorPropertyName, b_typeError, DontEnum | DontDelete | ReadOnly);
  b_uriErrorPrototype.put(globExec, constructorPropertyName, b_uriError, DontEnum | DontDelete | ReadOnly);

  // built-in values
  global.put(globExec, "NaN",        Number(NaN), DontEnum|DontDelete);
  global.put(globExec, "Infinity",   Number(Inf), DontEnum|DontDelete);
  global.put(globExec, "undefined",  Undefined(), DontEnum|DontDelete);

  // built-in functions
#ifdef KJS_PURE_ECMA // otherwise as deprecated Object.prototype property
  global.put(globExec,"eval",
	     Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::Eval,1,"eval")), DontEnum);
#endif
  global.put(globExec,"parseInt",
	     Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::ParseInt,2,"parseInt")), DontEnum);
  global.put(globExec,"parseFloat",
	     Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::ParseFloat,1,"parseFloat")), DontEnum);
  global.put(globExec,"isNaN",
	     Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::IsNaN,1,"isNaN")), DontEnum);
  global.put(globExec,"isFinite",
	     Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::IsFinite,1,"isFinite")), DontEnum);
  global.put(globExec,"decodeURI",
	     Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::DecodeURI,1,"decodeURI")),
	     DontEnum);
  global.put(globExec,"decodeURIComponent",
	     Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::DecodeURIComponent,1,"decodeURIComponent")),
	     DontEnum);
  global.put(globExec,"encodeURI",
	     Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::EncodeURI,1,"encodeURI")),
	     DontEnum);
  global.put(globExec,"encodeURIComponent",
	     Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::EncodeURIComponent,1,"encodeURIComponent")),
	     DontEnum);
  global.put(globExec,"escape",
	     Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::Escape,1,"escape")), DontEnum);
  global.put(globExec,"unescape",
	     Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::UnEscape,1,"unescape")), DontEnum);
#ifndef NDEBUG
  global.put(globExec,"kjsprint",
	     Object(new GlobalFuncImp(globExec,funcProto,GlobalFuncImp::KJSPrint,1,"kjsprint")), DontEnum);
#endif

  // built-in objects
  global.put(globExec,"Math", Object(new MathObjectImp(globExec,objProto)), DontEnum);
}

InterpreterImp::~InterpreterImp()
{
  if (dbg)
    dbg->detach(m_interpreter);
  for (SourceCode *s = sources; s; s = s->next)
    s->interpreter = 0;
  delete globExec;
  globExec = 0L;
  clear();
}

void InterpreterImp::clear()
{
  //fprintf(stderr,"InterpreterImp::clear\n");
  // remove from global chain (see init())
  lockInterpreter();
  next->prev = prev;
  prev->next = next;
  s_hook = next;
  if (s_hook == this)
  {
    // This was the last interpreter
    s_hook = 0L;
    globalClear();
  }
  unlockInterpreter();
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
  if (NumberImp::staticNaN && !NumberImp::staticNaN->marked())
    NumberImp::staticNaN->mark();
  if (BooleanImp::staticTrue && !BooleanImp::staticTrue->marked())
    BooleanImp::staticTrue->mark();
  if (BooleanImp::staticFalse && !BooleanImp::staticFalse->marked())
    BooleanImp::staticFalse->mark();
  //fprintf( stderr, "InterpreterImp::mark this=%p global.imp()=%p\n", this, global.imp() );
  if (global.imp())
    global.imp()->mark();
  if (m_interpreter)
    m_interpreter->mark();
  if (_context)
    _context->mark();
}

bool InterpreterImp::checkSyntax(const UString &code, int *errLine, UString *errMsg)
{
  // Parser::parse() returns 0 in a syntax error occurs, so we just check for that
  SourceCode *source;
  FunctionBodyNode *progNode = Parser::parse(code.data(),code.size(),&source,errLine,errMsg);
  source->deref();
  bool ok = (progNode != 0);
  delete progNode;
  return ok;
}

bool InterpreterImp::checkSyntax(const UString &code)
{
  // Parser::parse() returns 0 in a syntax error occurs, so we just check for that
  SourceCode *source;
  FunctionBodyNode *progNode = Parser::parse(code.data(),code.size(),&source,0,0);
  source->deref();
  bool ok = (progNode != 0);
  delete progNode;
  return ok;
}

Completion InterpreterImp::evaluate(const UString &code, const Value &thisV)
{
  lockInterpreter();

  // prevent against infinite recursion
  if (recursion >= 20) {
    Completion result = Completion(Throw,Error::create(globExec,GeneralError,"Recursion too deep"));
    unlockInterpreter();
    return result;
  }

  // parse the source code
  int errLine;
  UString errMsg;
  SourceCode *source;
  FunctionBodyNode *progNode = Parser::parse(code.data(),code.size(),&source,&errLine,&errMsg);

  // notify debugger that source has been parsed
  if (dbg) {
    bool cont = dbg->sourceParsed(globExec,source->sid,code,errLine);
    if (!cont) {
      source->deref();
      if (progNode)
	delete progNode;
      unlockInterpreter();
      return Completion(Break);
    }
  }

  addSourceCode(source);

  // no program node means a syntax error occurred
  if (!progNode) {
    Object err = Error::create(globExec,SyntaxError,errMsg.ascii(),errLine);
    err.put(globExec,"sid",Number(source->sid));
    globExec->setException(err); // required to notify the debugger
    globExec->clearException();
    source->deref();
    unlockInterpreter();
    return Completion(Throw,err);
  }
  source->deref();

  globExec->clearException();

  recursion++;
  progNode->ref();

  Object &globalObj = globalObject();
  Object thisObj = globalObject();

  if (thisV.isValid()) {
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
    ContextImp ctx(globalObj, this, thisObj, source->sid);
    ExecState newExec(m_interpreter,&ctx);

    // create variables (initialized to undefined until var statements
    // with optional initializers are executed)
    progNode->processVarDecls(&newExec);

    ctx.setLines(progNode->firstLine(),progNode->firstLine());
    bool abort = false;
    if (dbg) {
      if (!dbg->enterContext(&newExec)) {
	// debugger requested we stop execution
	dbg->imp()->abort();
	abort = true;
      }
    }

    if (!abort) {
      ctx.setLines(progNode->lastLine(),progNode->lastLine());
      res = progNode->execute(&newExec);
      if (dbg && !dbg->exitContext(&newExec,res)) {
	// debugger requested we stop execution
	dbg->imp()->abort();
	unlockInterpreter();
	res = Completion(ReturnValue,Undefined());
      }
    }
  }

  if (progNode->deref())
    delete progNode;
  recursion--;

  if (globExec->hadException()) {
    res = Completion(Throw,globExec->exception());
    globExec->clearException();
  }

  unlockInterpreter();
  return res;
}

void InterpreterImp::setDebugger(Debugger *d)
{
  if (d == dbg)
    return;
  // avoid recursion
  Debugger *old = dbg;
  dbg = d;
  if ( old )
    old->detach(m_interpreter);
}

void InterpreterImp::addSourceCode(SourceCode *code)
{
  assert(!code->next);
  assert(!code->interpreter);
  code->next = sources;
  code->interpreter = this;
  sources = code;
}

void InterpreterImp::removeSourceCode(SourceCode *code)
{
  assert(code);
  assert(sources);

  if (code == sources) {
    sources = sources->next;
    return;
  }

  SourceCode *prev = sources;
  SourceCode *cur = sources->next;
  while (cur != code) {
    assert(cur);
    prev = cur;
    cur = cur->next;
  }

  prev->next = cur->next;
}

// ------------------------------ InternalFunctionImp --------------------------

const ClassInfo InternalFunctionImp::info = {"Function", 0, 0, 0};

InternalFunctionImp::InternalFunctionImp(FunctionPrototypeImp *funcProto)
  : ObjectImp(funcProto)
{
}

InternalFunctionImp::InternalFunctionImp(ExecState *exec)
  : ObjectImp(static_cast<FunctionPrototypeImp*>(exec->interpreter()->builtinFunctionPrototype().imp()))
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

  Value prot = get(exec,prototypePropertyName);
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
  double n = v.toNumber(exec);
  if (isNaN(n) || isInf(n))
    return n;
  double an = fabs(n);
  if (an == 0.0)
    return n;
  double d = floor(an);
  if (n < 0)
    d *= -1;

  return d;
}

#ifndef NDEBUG
#include <stdio.h>
void KJS::printInfo(ExecState *exec, const char *s, const Value &o, int lineno)
{
  if (!o.isValid())
    fprintf(stderr, "KJS: %s: (null)", s);
  else {
    Value v = o;
    unsigned int arrayLength = 0;
    bool hadExcep = exec->hadException();

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
    case ObjectType: {
      Object obj = Object::dynamicCast(v);
      name = obj.className();
      if (name.isNull())
        name = "(unknown class)";
      if ( obj.inherits(&ArrayInstanceImp::info) )
        arrayLength = obj.get(exec,lengthPropertyName).toUInt32(exec);
      }
      break;
    }
    UString vString;
    // Avoid calling toString on a huge array (e.g. 4 billion elements, in mozilla/js/js1_5/Array/array-001.js)
    if ( arrayLength > 100 )
      vString = UString( "[ Array with " ) + UString::from( arrayLength ) + " elements ]";
    else
      vString = v.toString(exec);
    if ( !hadExcep )
      exec->clearException();
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
  }
}
#endif
