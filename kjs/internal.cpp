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

#include "internal.h"

#include <assert.h>
#include <stdio.h>
#include <iostream.h>

#include "kjs.h"
#include "object.h"
#include "types.h"
#include "regexp.h"
#include "nodes.h"
#include "lexer.h"
#include "collector.h"

extern int kjsyyparse();

using namespace KJS;

const TypeInfo UndefinedImp::info = { "Undefined", UndefinedType, 0, 0, 0 };
const TypeInfo NullImp::info = { "Null", NullType, 0, 0, 0 };
const TypeInfo NumberImp::info = { "Number", NumberType, 0, 0,0  };
const TypeInfo StringImp::info = { "String", StringType, 0, 0, 0 };
const TypeInfo BooleanImp::info = { "Boolean", BooleanType, 0, 0, 0 };
const TypeInfo CompletionImp::info = { "Completion", CompletionType, 0, 0, 0 };
const TypeInfo ReferenceImp::info = { "Reference", ReferenceType, 0, 0, 0 };

UndefinedImp::UndefinedImp()
{
}

KJSO UndefinedImp::toPrimitive(Type) const
{
  return KJSO(new UndefinedImp);
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
  return Error::createObject(TypeError);
}

NullImp::NullImp()
{
}

KJSO NullImp::toPrimitive(Type) const
{
  return KJSO(new NullImp);
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
  return Error::createObject(TypeError);
}

KJSO BooleanImp::toPrimitive(Type) const
{
  return KJSO(new BooleanImp(val));
}

Boolean BooleanImp::toBoolean() const
{
  return Boolean(val);
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
  return Object::create(BooleanClass, Boolean(val));
}

NumberImp::NumberImp(double v)
  : val(v)
{
}

KJSO NumberImp::toPrimitive(Type) const
{
  return KJSO(new NumberImp(val));
}

Boolean NumberImp::toBoolean() const
{
  bool b = !((val == 0) /* || (iVal() == N0) */ ||
	     (val == NaN));

  return Boolean(b);
}

Number NumberImp::toNumber() const
{
  return Number(val);
}

String NumberImp::toString() const
{
  UString s = UString::from(val);
  return String(s);
}

Object NumberImp::toObject() const
{
  return Object::create(NumberClass, Number(val));
}

StringImp::StringImp(const UString& v)
  : val(v)
{
}

KJSO StringImp::toPrimitive(Type) const
{
  return KJSO(new StringImp(val));
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
  //  return String(this);
  return String(val);
}

Object StringImp::toObject() const
{
  return Object::create(StringClass, String(val));
}

ReferenceImp::ReferenceImp(const KJSO& b, const UString& p)
  : base(b), prop(p)
{
}

CompletionImp::CompletionImp(Compl c, const KJSO& v, const UString& t)
  : comp(c), val(v), tar(t)
{
}

RegExpImp::RegExpImp()
  : ObjectImp(RegExpClass), reg(0L)
{
}

RegExpImp::~RegExpImp()
{
  delete reg;
}

// ECMA 10.2
Context::Context(CodeType type, Context *callingContext,
		 FunctionImp *func, const List *args, Imp *thisV)
  : err(false)
{
  Global glob(Global::current());

  // create and initialize activation object (ECMA 10.1.6)
  if (type == FunctionCode || type == AnonymousCode || type == HostCode) {
    activation = KJSO(new ActivationImp(func, args));
    variable = activation;
  } else {
    activation = KJSO();
    variable = glob;
  }

  // ECMA 10.2
  switch(type) {
    case EvalCode:
      if (callingContext) {
	scopeChain = callingContext->copyOfChain();
	variable = callingContext->variableObject();
	thisVal = callingContext->thisValue();
	break;
      } // else same as GlobalCode
    case GlobalCode:
      scopeChain = new List();
      scopeChain->append(glob);
      variable = glob;
      thisVal = glob.imp();
      break;
    case FunctionCode:
    case AnonymousCode:
      scopeChain = new List();
      scopeChain->append(activation);
      scopeChain->append(glob);
      variable = activation; /* TODO: DontDelete ? (ECMA 10.2.3) */
      if (thisV->type() == ObjectType) {
	thisVal = thisV;
      }
      else
	thisVal = glob.imp();
      break;
    case HostCode:
      if (thisV->type() == ObjectType)
	thisVal = thisV;
      else
	thisVal = glob.imp();
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
  delete scopeChain;
#if 0
  scopeChain->deref();

#endif
}

Context *Context::current()
{
  return KJScriptImp::curr ? KJScriptImp::curr->con : 0L;
}

void Context::setCurrent(Context *c)
{
  KJScriptImp::current()->con = c;
}

void Context::pushScope(KJSO *s)
{
  scopeChain->prepend(*s);
}

void Context::popScope()
{
  scopeChain->removeFirst();
}

List* Context::copyOfChain()
{
  /* TODO */
  return scopeChain;
}


AnonymousFunction::AnonymousFunction()
  : Function(0L)
{
  /* TODO */
}

DeclaredFunctionImp::DeclaredFunctionImp(ParamList *p, StatementNode *b)
  : block(b)
{
  param = p;
}

Completion DeclaredFunctionImp::execute(const List &)
{
 /* TODO */
  return block->execute();
}

Object DeclaredFunctionImp::construct(const List &args)
{
  Object obj(ObjectClass);
  KJSO p = get("prototype");
  if (p.isObject())
    obj.setPrototype(p);
  else
    obj.setPrototype(Global::current().objectPrototype());

  KJSO res = executeCall(obj.imp(), &args);

  Object v = Object::dynamicCast(res);
  if (v.isNull())
    return obj;
  else
    return v;
}

Completion AnonymousFunction::execute(const List &)
{
 /* TODO */
  return Completion(Normal, Null());
}

const TypeInfo ActivationImp::info = { "Activation", ActivationType, 0, 0, 0 };

// ECMA 10.1.7 (draft April 98, 10.1.6 previously)
ActivationImp::ActivationImp(FunctionImp *f, const List *args)
{
  func = f;

  /* TODO: solve deleting problem due to circular reference */
  // put("arguments", this, DontDelete | DontEnum);
  if (func->hasProperty("arguments"))
    put("OldArguments", func->get("arguments"));
  put("callee", Function(func), DontEnum);
  if (args) {
    put("length", Number(args->size()), DontEnum);
    ListIterator arg = args->begin();
    for (int i = 0; arg != args->end(); arg++, i++) {
      put(UString::from(i), *arg);
    }
  }
  /* TODO: solve deleting problem due to circular reference */
  //  func->put("arguments", this);
}

// ECMA 10.1.6
ActivationImp::~ActivationImp()
{
  /* TODO */
#if 0
  if (!hasProperty("OldArguments"))
    func->deleteProperty("arguments");
  else
    func->put("arguments", get("OldArguments")); /* TODO: deep copy ? */
#endif
}

KJScriptImp* KJScriptImp::curr = 0L;

KJScriptImp::KJScriptImp()
  : initialized(false),
    glob(0L)
{
  KJScriptImp::curr = this;
  lex = new Lexer();
}

KJScriptImp::~KJScriptImp()
{
  clear();

  KJScriptImp::curr = this;

  delete lex;
  lex = 0L;
}

KJScriptImp* KJScriptImp::current()
{
  return curr;
}

void KJScriptImp::init()
{
  KJScriptImp::curr = this;

  if (!initialized) {
    if (!Collector::current())
      collector = Collector::init();
    else
      collector = Collector::current();
    glob.init();
    con = new Context();
    initialized = true;
  } else
    Collector::attach(collector);
}

void KJScriptImp::clear()
{
  if (initialized) {
    KJScriptImp::curr = this;

    Node::deleteAllNodes();

    delete con; con = 0L;
    delete collector; collector = 0L;

    initialized = false;
  }
}

bool KJScriptImp::evaluate(const QChar *code, unsigned int length)
{
  init();

  Lexer::curr()->setCode((UChar*)code, length);
  int parseError = kjsyyparse();

  if (parseError) {
    fprintf(stderr, "JavaScript parse error.\n");
    Node::deleteAllNodes();
    return false;
  }

  Context *context = Context::current();
  context->clearError();

  KJSO res = Node::progNode()->evaluate();

  if (context->hadError()) {
    /* TODO */
    errType = 99;
    errMsg = "Error";
    context->clearError();
  } else {
    errType = 0;
    errMsg = "";
  }

  if (Node::progNode())
    KJS::Node::progNode()->deleteStatements();

  return (errType == 0);
}
