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

#include <iostream.h>

#include "kjs.h"
#include "object.h"

namespace KJS {
  const double NaN = 0.0/0.;
  const double Inf = 1.0/.0;
  // TODO: -0
};

using namespace KJS;

KJSO *KJSO::executeCall(KJSO *, KJSArgList *)
{
  return (*call)(0L);
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

// ECMA 10.1.8
KJSArguments::KJSArguments(KJSFunction *func, KJSArgList *args)
{
  assert(func);
  // TODO:
  // put("Prototype", _Object.prototype_ );
  put("callee", func, DontEnum);
  int iarg = args->numArgs();
  put("length", new KJSNumber(iarg), DontEnum);
  for (int i = 0; i < iarg; i++) {
    /* TODO */
  }
}


KJSGlobal::KJSGlobal()
{
  put("NaN", new KJSNumber(NaN));
  put("Infinity", new KJSNumber(Inf));

  put("Math", new KJSMath(), DontEnum);

  // TODO: add function properties
  put("eval", new KJSInternalFunction(&eval));
}

KJSO* KJSGlobal::eval(KJSO *)
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

  codeType = type;

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

int KJSArgList::numArgs() const
{
  if (!first)
    return 0;

  int num = 1;
  KJSArg *arg = first;
  while (arg = arg->next)
    num++;
}

KJSMath::KJSMath()
{
  //  put("E", new KJSNumber(2.7182818284590452354));
}
