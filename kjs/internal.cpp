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
 */

#include "internal.h"

#include <assert.h>
#include <stdio.h>

#include "kjs.h"
#include "object.h"
#include "types.h"
#include "operations.h"
#include "regexp.h"
#include "nodes.h"
#include "lexer.h"
#include "collector.h"
#include "debugger.h"

#include "global_object.h"
#include "object_object.h"
#include "function_object.h"
#include "array_object.h"
#include "bool_object.h"
#include "string_object.h"
#include "number_object.h"
#include "math_object.h"
#include "date_object.h"
#include "regexp_object.h"
#include "error_object.h"

#define I18N_NOOP(s) s

extern int kjsyyparse();

using namespace KJS;

const TypeInfo UndefinedImp::info = { "Undefined", UndefinedType, 0, 0, 0 };
const TypeInfo NullImp::info = { "Null", NullType, 0, 0, 0 };
const TypeInfo NumberImp::info = { "Number", NumberType, 0, 0,0  };
const TypeInfo StringImp::info = { "String", StringType, 0, 0, 0 };
const TypeInfo BooleanImp::info = { "Boolean", BooleanType, 0, 0, 0 };
const TypeInfo CompletionImp::info = { "Completion", CompletionType, 0, 0, 0 };
const TypeInfo ReferenceImp::info = { "Reference", ReferenceType, 0, 0, 0 };
const TypeInfo ArgumentsImp::info = { "Arguments", ArgumentsType, &ObjectImp::info, 0, 0 };

// ------------------------------ UndefinedImp ---------------------------------

UndefinedImp *UndefinedImp::staticUndefined = 0;

UndefinedImp::UndefinedImp()
{
}

KJSO UndefinedImp::toPrimitive(Type) const
{
  return (Imp*)this;
}

Boolean UndefinedImp::toBoolean() const
{
  return Boolean(false);
}

Number UndefinedImp::toNumber() const
{
  return Number(NaN);
}

String UndefinedImp::toString() const
{
  return String("undefined");
}

Object UndefinedImp::toObject() const
{
  return Error::createObject(TypeError, I18N_NOOP("Undefined value"));
}

// ------------------------------ NullImp --------------------------------------

NullImp *NullImp::staticNull = 0;

NullImp::NullImp()
{
}

KJSO NullImp::toPrimitive(Type) const
{
  return (Imp*)this;
}

Boolean NullImp::toBoolean() const
{
  return Boolean(false);
}

Number NullImp::toNumber() const
{
  return Number(0);
}

String NullImp::toString() const
{
  return String("null");
}

Object NullImp::toObject() const
{
  return Error::createObject(TypeError, I18N_NOOP("Null value"));
}

// ------------------------------ BooleanImp -----------------------------------

BooleanImp* BooleanImp::staticTrue = 0;
BooleanImp* BooleanImp::staticFalse = 0;

KJSO BooleanImp::toPrimitive(Type) const
{
  return (Imp*)this;
}

Boolean BooleanImp::toBoolean() const
{
  return Boolean((BooleanImp*)this);
}

Number BooleanImp::toNumber() const
{
  return Number(val ? 1 : 0);
}

String BooleanImp::toString() const
{
  return String(val ? "true" : "false");
}

Object BooleanImp::toObject() const
{
  return Object::create(BooleanClass, Boolean((BooleanImp*)this));
}

// ------------------------------ NumberImp ------------------------------------

NumberImp::NumberImp(double v)
  : val(v)
{
}

KJSO NumberImp::toPrimitive(Type) const
{
  return (Imp*)this;
}

Boolean NumberImp::toBoolean() const
{
  bool b = !((val == 0) /* || (iVal() == N0) */ || isNaN(val));

  return Boolean(b);
}

Number NumberImp::toNumber() const
{
  return Number((NumberImp*)this);
}

String NumberImp::toString() const
{
  return String(UString::from(val));
}

Object NumberImp::toObject() const
{
  return Object::create(NumberClass, Number((NumberImp*)this));
}

// ------------------------------ StringImp ------------------------------------

StringImp::StringImp(const UString& v)
  : val(v)
{
}

KJSO StringImp::toPrimitive(Type) const
{
  return (Imp*)this;
}

Boolean StringImp::toBoolean() const
{
  return Boolean(val.size() > 0);
}

Number StringImp::toNumber() const
{
  return Number(val.toDouble());
}

String StringImp::toString() const
{
  return String((StringImp*)this);
}

Object StringImp::toObject() const
{
  return Object::create(StringClass, String((StringImp*)this));
}

// ------------------------------ ReferenceImp ---------------------------------

ReferenceImp::ReferenceImp(const KJSO& b, const UString& p)
  : base(b), prop(p)
{
}

void ReferenceImp::mark(Imp*)
{
  Imp::mark();
  Imp *im = base.imp();
  if (im && !im->marked())
    im->mark();
}

// ------------------------------ CompletionImp --------------------------------

CompletionImp::CompletionImp(Compl c, const KJSO& v, const UString& t)
  : comp(c), val(v), tar(t)
{
}

void CompletionImp::mark(Imp*)
{
  Imp::mark();
  Imp *im = val.imp();
  if (im && !im->marked())
    im->mark();
}

// ------------------------------ RegExpImp ------------------------------------

RegExpImp::RegExpImp()
  : ObjectImp(RegExpClass), reg(0L)
{
}

RegExpImp::~RegExpImp()
{
  delete reg;
}

// ------------------------------ Reference ------------------------------------

Reference::Reference(const KJSO& b, const UString &p)
  : KJSO(new ReferenceImp(b, p))
{
}

Reference::~Reference()
{
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

// ------------------------------ Context --------------------------------------

// ECMA 10.2
Context::Context(CodeType type, Context *_callingContext,
		 FunctionImp *func, const List *args, Imp *thisV)
{
  KJSO glob = KJScriptImp::current()->globalObject();
  codeType = type;
  callingCon = _callingContext;

  // create and initialize activation object (ECMA 10.1.6)
  if (type == FunctionCode || type == AnonymousCode || type == HostCode) {
    activation = new ActivationImp(func, args);
    variable = activation;
  } else {
    activation = KJSO();
    variable = glob;
  }

  // ECMA 10.2
  switch(type) {
    case EvalCode:
      if (callingCon) {
	scopeChain = callingCon->copyOfChain();
	variable = callingCon->variableObject();
	thisVal = callingCon->thisValue();
	break;
      } // else same as GlobalCode
    case GlobalCode:
      scopeChain = new List();
      scopeChain->append(glob);
      thisVal = glob.imp();
      break;
    case FunctionCode:
    case AnonymousCode:
      if (type == FunctionCode) {
	scopeChain = ((DeclaredFunctionImp*)func)->scopeChain()->copy();
	scopeChain->prepend(activation);
      } else {
	scopeChain = new List();
	scopeChain->append(activation);
	scopeChain->append(glob);
      }
      variable = activation; /* TODO: DontDelete ? (ECMA 10.2.3) */
      if (thisV->type() >= ObjectType) {
	thisVal = thisV;
      }
      else
	thisVal = glob.imp();
      break;
    case HostCode:
      if (thisV->type() >= ObjectType)
	thisVal = thisV;
      else
	thisVal = glob;
      variable = activation; /* TODO: DontDelete (ECMA 10.2.4) */
      scopeChain = new List();
      scopeChain->append(activation);
      if (func->hasAttribute(ImplicitThis))
	scopeChain->append(KJSO(thisVal));
      if (func->hasAttribute(ImplicitParents)) {
	/* TODO ??? */
      }
      scopeChain->append(glob);
      break;
    }
}

Context::~Context()
{
  if (codeType == FunctionCode || codeType == AnonymousCode || codeType == HostCode)
    static_cast<ActivationImp*>(activation.imp())->cleanup();
  delete scopeChain;
}

void Context::pushScope(const KJSO &s)
{
  scopeChain->prepend(s);
}

void Context::popScope()
{
  scopeChain->removeFirst();
}

List* Context::copyOfChain()
{
  return scopeChain->copy();
}

// ------------------------------ DeclaredFunctionImp --------------------------

const TypeInfo DeclaredFunctionImp::info = { "Function", DeclaredFunctionType,
					 &ConstructorImp::info, 0, 0 };

DeclaredFunctionImp::DeclaredFunctionImp(const UString &n,
					 FunctionBodyNode *b, const List *sc)
  : ConstructorImp(n), body(b), scopes(sc->copy())
{
}

DeclaredFunctionImp::~DeclaredFunctionImp()
{
  delete scopes;
  delete body;
}

// step 2 of ECMA 13.2.1. rest in FunctionImp::executeCall()
Completion DeclaredFunctionImp::execute(const List &)
{
 /* TODO */

  Debugger *dbg = KJScriptImp::current()->debugger();
  //  int oldSourceId = -1;
  if (dbg) {
    // ###    oldSourceId = dbg->sourceId();
    //    dbg->setSourceId(body->sourceId());
  }

  // ### use correct script & context
  Completion result = body->execute(KJScriptImp::current(),KJScriptImp::current()->context());

  if (dbg) {
    // ###    dbg->setSourceId(oldSourceId);
  }

  if (result.complType() == Throw || result.complType() == ReturnValue)
      return result;
  return Completion(Normal, Undefined()); /* TODO: or ReturnValue ? */
}

// ECMA 13.2.2 [[Construct]]
Object DeclaredFunctionImp::construct(const List &args)
{
  Object obj(ObjectClass);
  KJSO p = get("prototype");
  if (p.isObject())
    obj.setPrototype(p);
  else
    obj.setPrototype(KJScriptImp::current()->objectPrototype());

  KJSO res = executeCall(obj.imp(), &args);

  Object v = Object::dynamicCast(res);
  if (v.isNull())
    return obj;
  else
    return v;
}

void DeclaredFunctionImp::processVarDecls()
{
  // ### use given script & context
  body->processVarDecls(KJScriptImp::current(),KJScriptImp::current()->context());
}

// ------------------------------ AnonymousFunction ----------------------------

AnonymousFunction::AnonymousFunction()
  : Function(0L)
{
  /* TODO */
}

Completion AnonymousFunction::execute(const List &)
{
 /* TODO */
  return Completion(Normal, Null());
}

// ECMA 10.1.8
ArgumentsImp::ArgumentsImp(FunctionImp *func, const List *args)
  : ObjectImp(UndefClass)
{
  setPrototype(KJScriptImp::current()->objectPrototype());
  put("callee", Function(func), DontEnum);
  if (args) {
    put("length", Number(args->size()), DontEnum);
    ListIterator arg = args->begin();
    for (int i = 0; arg != args->end(); arg++, i++) {
      put(UString::from(i), *arg, DontEnum);
    }
  }
}

// ------------------------------ ActivationImp --------------------------------

const TypeInfo ActivationImp::info = { "Activation", ActivationType, 0, 0, 0 };

// ECMA 10.1.6
ActivationImp::ActivationImp(FunctionImp *f, const List *args)
{
  KJSO aobj(new ArgumentsImp(f, args));
  put("arguments", aobj, DontDelete);
  /* TODO: this is here to get myFunc.arguments and myFunc.a1 going.
     I can't see this described in the spec but it's possible in browsers. */
  func = 0;
  if (!f->name().isNull()) {
    f->put("arguments", aobj);
    f->pushArgs(aobj);
    func = f;
  }
}

void ActivationImp::cleanup()
{
  if (func)
    func->popArgs();
}

// ------------------------------ Parser ---------------------------------------

ProgramNode *Parser::progNode = 0;
int Parser::sid = 0;

ProgramNode *Parser::parse(const UChar *code, unsigned int length, int sourceId,
			   int *errType, int *errLine, UString *errMsg)
{
  if (errType)
    *errType = -1;
  if (errLine)
    *errLine = -1;
  if (errMsg)
    *errMsg = 0;

  assert(Lexer::curr());
  Lexer::curr()->setCode(code, length);
  progNode = 0;
  sid = sourceId;
  int parseError = kjsyyparse();
  ProgramNode *prog = progNode;
  progNode = 0;
  sid = -1;

  //  Node::setFirstNode(firstNode());
  //  setFirstNode(Node::firstNode());

  if (parseError) {
    // ###
    int eline = Lexer::curr()->lineNo();
    if (errType)
      *errType = 99; /* TODO */
    if (errLine)
      *errLine = eline;
    if (errMsg)
      *errMsg = "Parse error at line " + UString::from(eline);
#ifndef NDEBUG
    fprintf(stderr, "KJS: JavaScript parse error at line %d.\n", eline);
#endif
    if (prog)
      delete prog;
    return 0;
  }

  return prog;
}

// ------------------------------ DebuggerImp ----------------------------------

DebuggerImp::DebuggerImp(Debugger *_dbg)
{
  //  dmode = Debugger::Disabled;
  dbg = _dbg;
}

DebuggerImp::~DebuggerImp()
{
}

// called from the scripting engine each time a statement node is hit.
bool DebuggerImp::hitStatement(KJScriptImp *script, Context *context,
			       int sid, int line0, int line1)
{
  // ### parse second line to debugger
  ExecutionContext ec(context);
  dbg->atLine(script->scr,sid,line0,&ec);

  return true;
}

// ------------------------------ KJScriptImp ----------------------------------

KJScriptImp* KJScriptImp::curr = 0L;
KJScriptImp* KJScriptImp::hook = 0L;
int          KJScriptImp::instances = 0;
int          KJScriptImp::running = 0;
int          KJScriptImp::nextSid = 1;

KJScriptImp::KJScriptImp(KJScript *s, KJSO global)
  : scr(s),
    initialized(false),
    glob(0L),
    dbg(0L),
    retVal(0L)
{
  instances++;
  KJScriptImp::curr = this;
#ifdef KJS_DEBUG_GLOBAL
  fprintf(stderr,"KJScriptImp::KJScriptImp this=curr=%p\n", this);
#endif
  // are we the first interpreter instance ? Initialize some stuff
  if (instances == 1)
    globalInit();
  glob = global;
  clearException();
  lex = new Lexer();
  debugEnabled = false;
}

KJScriptImp::~KJScriptImp()
{
#ifdef KJS_DEBUG_GLOBAL
  fprintf(stderr,"KJScriptImp::~KJScriptImp this=curr=%p\n", this);
#endif
  KJScriptImp::curr = this;

  clear();

  delete lex;
  lex = 0L;

  KJScriptImp::curr = 0L;
#ifdef KJS_DEBUG_GLOBAL
  fprintf(stderr,"KJScriptImp::~KJScriptImp (this=%p. Setting curr to 0L)\n", this);
#endif
  // are we the last of our kind ? Free global stuff.
  if (instances == 1)
    globalClear();
  instances--;
}

void KJScriptImp::globalInit()
{
  UndefinedImp::staticUndefined = new UndefinedImp();
  UndefinedImp::staticUndefined->ref();
  NullImp::staticNull = new NullImp();
  NullImp::staticNull->ref();
  BooleanImp::staticTrue = new BooleanImp(true);
  BooleanImp::staticTrue->ref();
  BooleanImp::staticFalse = new BooleanImp(false);
  BooleanImp::staticFalse->ref();
}

void KJScriptImp::globalClear()
{
  UndefinedImp::staticUndefined->deref();
  UndefinedImp::staticUndefined = 0L;
  NullImp::staticNull->deref();
  NullImp::staticNull = 0L;
  BooleanImp::staticTrue->deref();
  BooleanImp::staticTrue = 0L;
  BooleanImp::staticFalse->deref();
  BooleanImp::staticFalse = 0L;
}

void KJScriptImp::mark()
{
  if (exVal && !exVal->marked())
    exVal->mark();
  if (retVal && !retVal->marked())
    retVal->mark();
  UndefinedImp::staticUndefined->mark();
  NullImp::staticNull->mark();
  BooleanImp::staticTrue->mark();
  BooleanImp::staticFalse->mark();
  if (glob.imp())
    glob.imp()->mark();
}

void KJScriptImp::init()
{
#ifdef KJS_DEBUG_GLOBAL
  fprintf(stderr,"KJScriptImp::init() this=curr=%p\n", this);
#endif
  KJScriptImp::curr = this;

  clearException();
  retVal = 0L;

  if (!initialized) {
    // add this interpreter to the global chain
    // as a root set for garbage collection
    if (hook) {
      prev = hook;
      next = hook->next;
      hook->next->prev = this;
      hook->next = this;
    } else {
      hook = next = prev = this;
    }

    if (!glob.imp())
      glob = new GlobalImp();
    initGlobal(glob.imp());
    con = new Context();
    recursion = 0;
    errMsg = "";
    initialized = true;
  }
}

void KJScriptImp::clear()
{

  if ( recursion ) {
#ifndef NDEBUG
      fprintf(stderr, "KJS: ignoring clear() while running\n");
#endif
      return;
  }
  KJScriptImp *old = curr;
  if (initialized) {
    KJScriptImp::curr = this;
#ifdef KJS_DEBUG_GLOBAL
  fprintf(stderr,"KJScriptImp::clear() this=%p, curr temporarily set\n", this);
#endif

    clearException();
    retVal = 0L;

    delete con; con = 0L;

    Collector::collect();

    // remove from global chain (see init())
    next->prev = prev;
    prev->next = next;
    hook = next;
    if (hook == this)
      hook = 0L;

    initialized = false;
  }

  if (old != this)
      KJScriptImp::curr = old;
  else
      KJScriptImp::curr = 0L;

#ifdef KJS_DEBUG_GLOBAL
  fprintf(stderr,"KJScriptImp::clear() this=%p, curr is now %p\n", this, curr);
#endif
}

bool KJScriptImp::evaluate(const UChar *code, unsigned int length, const KJSO &thisV,
			   bool onlyCheckSyntax)
{
  init();

  //  if (debugger())
  // ###    debugger()->setSourceId(sid);
  if (recursion > 7) {
    fprintf(stderr, "KJS: breaking out of recursion\n");
    return true;
  } else if (recursion > 0) {
#ifndef NDEBUG
    fprintf(stderr, "KJS: entering recursion level %d\n", recursion);
#endif
  }

  int sid = nextSourceId();

  ProgramNode *progNode = Parser::parse(code,length,sid,&errType,&errLine,&errMsg);
  if (dbg) {
    UString parsedSource = UString(code,length);
    dbg->sourceParsed(scr,sid,parsedSource,errLine);
  }
  if (!progNode)
    return false;

  if (onlyCheckSyntax)
    return true;

  clearException();

  KJSO oldVar;
  if (!thisV.isNull()) {
    context()->setThisValue(thisV);
    context()->pushScope(thisV);
    oldVar = context()->variableObject();
    context()->setVariableObject(thisV);
  }

  running++;
  recursion++;
  // ### use correct script & context
  Completion res = progNode->execute(this,con);
  recursion--;
  running--;

  if (hadException()) {
    KJSO err = exception();
    errType = 99; /* TODO */
    errLine = err.get("line").toInt32();
    errMsg = err.get("name").toString().value() + ". ";
    errMsg += err.get("message").toString().value();
    //###    if (dbg)
    //      dbg->setSourceId(err.get("sid").toInt32());
    clearException();
  } else {
    errType = 0;
    errLine = -1;
    errMsg = "";

    // catch return value
    retVal = 0L;
    if (res.complType() == ReturnValue || !thisV.isNull())
	retVal = res.value().imp();
  }

  if (!thisV.isNull()) {
    context()->popScope();
    context()->setVariableObject(oldVar);
  }

  delete progNode;

  return !errType;
}

bool KJScriptImp::call(const KJSO &scope, const UString &func, const List &args)
{
  init();
  KJSO callScope(scope);
  if (callScope.isNull())
    callScope = KJScriptImp::current()->globalObject().imp();
  if (!callScope.hasProperty(func)) {
#ifndef NDEBUG
      fprintf(stderr, "KJS: couldn't resolve function name %s. call() failed\n",
	      func.ascii());
#endif
      return false;
  }
  KJSO v = callScope.get(func);
  if (!v.derivedFrom(ConstructorType)) {
#ifndef NDEBUG
      fprintf(stderr, "KJS: %s is not a function. call() failed.\n", func.ascii());
#endif
      return false;
  }
  running++;
  recursion++;
  static_cast<ConstructorImp*>(v.imp())->executeCall(scope.imp(), &args);
  recursion--;
  running--;
  return !hadException();
}

bool KJScriptImp::call(const KJSO &func, const KJSO &thisV,
		       const List &args, const List &extraScope)
{
  init();
  if(!func.implementsCall())
    return false;

  running++;
  recursion++;
  retVal = func.executeCall(thisV, &args, &extraScope).imp();
  recursion--;
  running--;

  return !hadException();
}

void KJScriptImp::setException(Imp *e)
{
  assert(curr);
  curr->exVal = e;
  curr->exMsg = "Exception"; // not very meaningful but we use !0L to test
}

void KJScriptImp::setException(const char *msg)
{
  assert(curr);
  curr->exVal = 0L;		// will be created later on exception()
  curr->exMsg = msg;
}

KJSO KJScriptImp::exception()
{
  assert(curr);
  if (!curr->exMsg)
    return Undefined();
  if (curr->exVal)
    return curr->exVal;
  return Error::create(GeneralError, curr->exMsg);
}

void KJScriptImp::clearException()
{
  assert(curr);
  curr->exMsg = 0L;
  curr->exVal = 0L;
}

void KJScriptImp::setDebugger(Debugger *debugger)
{
  dbg = debugger;
}

void KJScriptImp::initGlobal(Imp *global)
{
  // constructor properties. prototypes as Global's member variables first.
  Object objProto = new ObjectPrototype();
  Object funcProto = new FunctionPrototype(objProto);
  Object arrayProto(new ArrayPrototype(objProto,funcProto));
  Object stringProto(new StringPrototype(objProto,funcProto));
  Object booleanProto(new BooleanPrototype(objProto,funcProto));
  Object numberProto(new NumberPrototype(objProto,funcProto));
  Object dateProto(new DatePrototype(objProto,funcProto));
  Object regexpProto(new RegExpPrototype(objProto,funcProto));
  Object errorProto(new ErrorPrototype(objProto,funcProto));

  objProto.get("toString").setPrototype(funcProto);
  objProto.get("valueOf").setPrototype(funcProto);
  global->setPrototype(objProto);

  // these are internal kjs properties
  global->put("[[Object.prototype]]", objProto,ReadOnly|DontDelete|DontEnum);
  global->put("[[Function.prototype]]", funcProto,ReadOnly|DontDelete|DontEnum);
  global->put("[[Array.prototype]]", arrayProto,ReadOnly|DontDelete|DontEnum);
  global->put("[[String.prototype]]", stringProto,ReadOnly|DontDelete|DontEnum);
  global->put("[[Boolean.prototype]]", booleanProto,ReadOnly|DontDelete|DontEnum);
  global->put("[[Number.prototype]]", numberProto,ReadOnly|DontDelete|DontEnum);
  global->put("[[Date.prototype]]", dateProto,ReadOnly|DontDelete|DontEnum);
  global->put("[[RegExp.prototype]]", regexpProto,ReadOnly|DontDelete|DontEnum);
  global->put("[[Error.prototype]]", errorProto,ReadOnly|DontDelete|DontEnum);

  Object objectObj(new ObjectObject(funcProto, objProto));
  Object funcObj(new FunctionObject(funcProto));
  Object arrayObj(new ArrayObject(funcProto, arrayProto));
  Object stringObj(new StringObject(funcProto, stringProto));
  Object boolObj(new BooleanObject(funcProto, booleanProto));
  Object numObj(new NumberObject(funcProto, numberProto));
  Object dateObj(new DateObject(funcProto, dateProto));
  Object regObj(new RegExpObject(funcProto, regexpProto));
  Object errObj(new ErrorObject(funcProto, errorProto));

  // ECMA 15.3.4.1
  funcProto.put("constructor", funcObj, DontEnum);

  global->put("Object", objectObj, DontEnum);
  global->put("Function", funcObj, DontEnum);
  global->put("Array", arrayObj, DontEnum);
  global->put("Boolean", boolObj, DontEnum);
  global->put("String", stringObj, DontEnum);
  global->put("Number", numObj, DontEnum);
  global->put("Date", dateObj, DontEnum);
  global->put("RegExp", regObj, DontEnum);
  global->put("Error", errObj, DontEnum);

  objProto.setConstructor(objectObj);
  funcProto.setConstructor(funcObj);
  arrayProto.setConstructor(arrayObj);
  booleanProto.setConstructor(boolObj);
  stringProto.setConstructor(stringObj);
  numberProto.setConstructor(numObj);
  dateProto.setConstructor(dateObj);
  regexpProto.setConstructor(regObj);
  errorProto.setConstructor(errObj);

  global->put("NaN", Number(NaN), DontEnum);
  global->put("Infinity", Number(Inf), DontEnum);
  global->put("undefined", Undefined(), DontEnum);

  // built-in functions
  global->put("eval",       new GlobalFunc(GlobalFunc::Eval,       1), DontEnum);
  global->put("parseInt",   new GlobalFunc(GlobalFunc::ParseInt,   2), DontEnum);
  global->put("parseFloat", new GlobalFunc(GlobalFunc::ParseFloat, 1), DontEnum);
  global->put("isNaN",      new GlobalFunc(GlobalFunc::IsNaN,      1), DontEnum);
  global->put("isFinite",   new GlobalFunc(GlobalFunc::IsFinite,   1), DontEnum);
  global->put("escape",     new GlobalFunc(GlobalFunc::Escape,     1), DontEnum);
  global->put("unescape",   new GlobalFunc(GlobalFunc::UnEscape,   1), DontEnum);

  // built-in objects
  global->put("Math",       new Math(objProto),                        DontEnum);
}

KJSO KJScriptImp::objectPrototype() const
{
  return glob.get("[[Object.prototype]]");
}

KJSO KJScriptImp::functionPrototype() const
{
  return glob.get("[[Function.prototype]]");
}

// ------------------------------ PropList -------------------------------------

bool PropList::contains(const UString &name)
{
  PropList *p = this;
  while (p) {
    if (name == p->name)
      return true;
    p = p->next;
  }
  return false;
}

// -----------------------------------------------------------------------------

// ECMA 15.3.5.3 [[HasInstance]]
// see comment in header file
KJSO KJS::hasInstance(const KJSO &F, const KJSO &V)
{
  if (V.isObject()) {
    KJSO prot = F.get("prototype");
    if (!prot.isObject())
      return Error::create(TypeError, "Invalid prototype encountered "
			   "in instanceof operation.");
    Imp *v = V.imp();
    while ((v = v->prototype())) {
      if (v == prot.imp())
	return Boolean(true);
    }
  }
  return Boolean(false);
}

#ifndef NDEBUG
#include <stdio.h>
void KJS::printInfo( const char *s, const KJSO &o, int lineno )
{
    if (o.isNull())
      fprintf(stderr, "KJS: %s: (null)", s);
    else {
      KJSO v = o;
      if (o.isA(ReferenceType))
	  v = o.getValue();
      fprintf(stderr, "KJS: %s: %s : %s (%p)",
	      s,
	      v.toString().value().ascii(),
	      v.imp()->typeInfo()->name,
	      (void*)v.imp());
    if (lineno >= 0)
      fprintf(stderr, ", line %d\n",lineno);
    else
      fprintf(stderr, "\n");
    if (!o.isNull())
      if (o.isA(ReferenceType)) {
	  fprintf(stderr, "KJS: Was property '%s'\n", o.getPropertyName().ascii());
	  printInfo("of", o.getBase());
      }
    }
}
#endif
