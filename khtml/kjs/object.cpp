/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
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

#include <stdio.h>
#include <iostream.h>

#include "kjs.h"
#include "object.h"
#include "nodes.h"

namespace KJS {
  const double NaN = 0.0/0.;
  const double Inf = 1.0/.0;
  // TODO: -0
};

using namespace KJS;

// [[call]]
KJSO *KJSO::executeCall(KJSO *, KJSArgList *args)
{
  KJSFunction *func = static_cast<KJSFunction*>(this);

  KJSContext *save = KJSWorld::context;
  CodeType ctype;
  switch(func->type()) {
    case InternalFunction:
      ctype = HostCode;
      break;
    case DeclaredFunction:
      ctype = FunctionCode;
      break;
    case AnonymousFunction:
      ctype = AnonymousCode;
      break;
    default:
      assert(!"KJSO::executeCall(): unhandled switch case");
  }

  KJSWorld::context = new KJSContext(ctype, save, func, args);
  
  // assign user supplied arguments to parameter
  func->processParameters(args);

  KJSO *compl = func->execute();

  KJSWorld::context = save;

  if(compl->isValueCompletion())
    return compl->complValue();
  else
    return new KJSUndefined();
}

// ECMA 8.7.1
KJSO *KJSO::getBase()
{
  if (!isA(Reference)) {
    /* TODO: runtime error */
    cerr << "KJSO::getBase(): RUNTIME ERROR" << endl;
    exit(1);
  }

  return base;
}

// ECMA 8.7.2
CString KJSO::getPropertyName()
{
  if (!isA(Reference)) {
    /* TODO: runtime error */
    cerr << "KJSO::getPropertyName(): RUNTIME ERROR" << endl;
    exit(1);
  }

  return propname;
}

// ECMA 8.7.3
KJSO *KJSO::getValue()
{
  if (!isA(Reference)) {
    return this;
  }
  KJSO *o = getBase();
  if (o->isA(Null)) {
    /* TODO: runtime error */
    cerr << "KJSO::getValue(): RUNTIME ERROR" << endl;
    exit(1);
  }

  return o->get(getPropertyName());
}

// ECMA 8.7.4
void KJSO::putValue(KJSO *v)
{
  if (!isA(Reference)) {
    /* TODO: runtime error */
    cerr << "KJSO::putValue(): RUNTIME ERROR" << endl;
    exit(1);
  }
  KJSO *o = getBase();
  if (o->isA(Null)) {
    KJSWorld::global->put(getPropertyName(), v);
  } else
    o->put(getPropertyName(), v);
}

KJSReference::KJSReference(KJSO *b, const CString &s)
{
  base = b;
  propname = s;
}

// ECMA 10.1.6
KJSActivation::KJSActivation(KJSFunction *f, KJSArgList *args)
{
  func = f;

  if (func->hasProperty("arguments"))
    put("OldArguments", func->get("arguments")); /* TODO: deep copy ? */

  func->put("arguments", args);
}

// ECMA 10.1.6
KJSActivation::~KJSActivation()
{
  if (hasProperty("OldArguments"))
    func->deleteProperty("arguments");
  else
    func->put("arguments", get("OldArguments")); /* TODO: deep copy ? */
}

#include "operations.h"
// ECMA 10.1.8
KJSArguments::KJSArguments(KJSFunction *func, KJSArgList *args)
{
  char buffer[10];
  KJSArg *arg;

  assert(func);
  // TODO:
  // put("Prototype", _Object.prototype_ );
  put("callee", func, DontEnum);
  // what is this needed for anyway ?
  if(args) {
    int iarg = args->count();
    put("length", new KJSNumber(iarg), DontEnum);
    arg = args->firstArg();
    for (int i = 0; i < iarg && i < 100; i++) {
      sprintf(buffer, "%d", i);
      put(buffer, arg->object());
      arg = arg->nextArg();
    }
    /* TODO: length != num. of arguments */
  }
}


KJSGlobal::KJSGlobal()
{
  put("NaN", new KJSNumber(NaN));
  put("Infinity", new KJSNumber(Inf));

  // TODO: add function properties
  put("eval", new KJSInternalFunction(&eval));
}

KJSO* KJSGlobal::eval()
{
  Debug("KJSGLOBAL::eval");
  return new KJSNumber(11);
}

// ECMA 10.2
KJSContext::KJSContext(CodeType type, KJSContext *callingContext,
		       KJSFunction *func, KJSArgList *args)
{
  KJSGlobal *glob = KJSWorld::global;
  assert(glob);

  // create and initialize activation object (ECMA 10.1.6)
  if (type == FunctionCode || type == AnonymousCode || type == HostCode) {
    activation = new KJSActivation(func, args);
    variable = activation;
    arguments = new KJSArguments(func, args);
  } else {
    activation = 0L;
    arguments = 0L;
    variable = glob;
  }

  // ECMA 10.2
  switch(type) {
    case EvalCode:
      if (callingContext) {
	scopeChain = callingContext->copyOfChain();
	variable = callingContext->variableObject();
	thisValue = callingContext->thisValue;
	break;
      } // else same as GlobalCode
    case GlobalCode:
      scopeChain = new KJSScope(glob);
      variable = glob;
      thisValue = glob;
      break;
    case FunctionCode:
    case AnonymousCode:
      scopeChain = new KJSScope(activation);
      scopeChain->append(glob);
      variable = activation; /* TODO: DontDelete ? (ECMA 10.2.3) */
      if (callingContext->thisValue->isA(Object))
	thisValue = callingContext->thisValue;
      else
	thisValue = glob;
      break;
    case HostCode:
      if (callingContext->thisValue->isA(Object))
	thisValue = callingContext->thisValue;
      else
	thisValue = glob;
      variable = activation; /* TODO: DonDelete (ECMA 10.2.4) */
      scopeChain = new KJSScope(activation);
      if (func->hasAttribute(ImplicitThis))
	scopeChain->append(thisValue);
      if (func->hasAttribute(ImplicitParents)) {
	/* TODO ??? */
      }
      scopeChain->append(glob);
      break;
    }
}

KJSContext::~KJSContext()
{
  /* TODO: deleting */
}

void KJSContext::insertScope(KJSO *s)
{
  if (!scopeChain) {
    // chain was empty so far
    scopeChain = new KJSScope(s);
    scopeChain->next = 0L;
    return;
  }

  // insert at first position
  KJSScope *scope = new KJSScope(s);
  scope->next = scopeChain;
  scopeChain = scope;
}

// debugging info
void KJSO::dump()
{
  cout << "Properties: " << endl;
  if (prop) {
    KJSProperty *pr = prop;
    while (pr) {
      cout << "  " << pr->name.ascii() << endl;
      pr = pr->next;
    }
  } else
    cout << "   None." << endl;

#if 0
   if(prototype()) {
    cout << "from prototype:" << endl;
    prototype()->dump();
  }
#endif
}

KJSArgList *KJSArgList::append(KJSO *o)
{
  if (!first) {
    first = new KJSArg(o);
    return this;
  }

  KJSArg *arg = first;
  while (arg->next)
     arg = arg->next;

  arg->next = new KJSArg(o);

  return this;
}

int KJSArgList::count() const
{
  if (!first)
    return 0;

  int num = 1;
  KJSArg *arg = first;
  while ((arg = arg->next))
    num++;

  return num;
}

// ECMA 10.1.3
void KJSFunction::processParameters(KJSArgList *args)
{
  KJSO *variable = KJSWorld::context->variableObject();

  assert(args);
  if (param) {
    KJSArg *arg = args->firstArg();
    for(int i = 0; i < param->count(); i++)
      if (arg) {
	variable->put(param->at(i), arg->object());
	arg = arg->nextArg();
      } else
	variable->put(param->at(i), new KJSUndefined());
  }
}

KJSDeclaredFunction::KJSDeclaredFunction(const CString &i, KJSParamList *p,
					 StatementNode *b)
  : block(b)
{
  param = p;
}

KJSO* KJSDeclaredFunction::execute()
{
 /* TODO */
  return block->evaluate();
}
