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
#include "math_object.h"


namespace KJS {
  const double NaN = 0.0/0.;
  const double Inf = 1.0/.0;
  // TODO: -0

#ifdef KJS_DEBUG_MEM
  int KJSO::count = 0;
  KJSO* KJSO::firstObject = 0L;
  int KJSO::lastId = 0;
#endif
};

using namespace KJS;

#ifdef KJS_DEBUG_MEM
const char *typeName[] = {
  "Undefined",
  "Null",
  "Boolean",
  "Number",
  "String",
  "Object",
  "Reference",
  "ListType",
  "Completion",
  "Property",
  "Scope",
  "InternalFunction",
  "DeclaredFunction",
  "AnonymousFunction",
  "Activation"
};
#endif

KJSO *KJS::zeroRef(KJSO *obj)
{
  obj->refCount = 0;
  return obj;
}

void KJSO::init()
{
  proto = 0L; prop = 0L; call = 0L;
  refCount = 1;

#ifdef KJS_DEBUG_MEM
  count++;
  if (firstObject)
    firstObject->prevObject = this;
  nextObject = firstObject;
  prevObject = 0L;
  firstObject = this;
  objId = ++lastId;
  printf("++ count: %d id: %d\n", count, objId);
#endif
}

KJSO::~KJSO()
{ /* TODO: delete String object ???*/
  KJSProperty *tmp, *p = prop;
  while (p) {
    tmp = p;
    p = p->next;
    delete tmp;
  }

#ifdef KJS_DEBUG_MEM
  if (prevObject)
    prevObject->nextObject = nextObject;
  if (nextObject)
    nextObject->prevObject = prevObject;
  if (firstObject == this)
    firstObject = nextObject;

  --count;
#endif
}

// [[call]]
KJSO *KJSO::executeCall(KJSO *thisV, KJSArgList *args)
{
  KJSFunction *func = static_cast<KJSFunction*>(this);

  KJSContext *save = KJSWorld::context;

  CodeType ctype = func->codeType();
  KJSWorld::context = new KJSContext(ctype, save, func, args, thisV);

  // assign user supplied arguments to parameters
  if (args)
    func->processParameters(args);

  Ptr compl = func->execute();

  delete KJSWorld::context;
  KJSWorld::context = save;

  if (compl->isValueCompletion())
    return compl->complValue()->ref();
  else
    return new KJSUndefined();
}

// ECMA 8.7.1
KJSO *KJSO::getBase()
{
  if (!isA(Reference))
    return new KJSError(ErrBaseNoRef, this);

  return base->ref();
}

// ECMA 8.7.2
CString KJSO::getPropertyName()
{
  if (!isA(Reference))
    // the spec wants a runtime error here. But getValue() and putValue()
    // will catch this case on their own earlier. When returning a Null
    // string we should be on the safe side.
    return CString();

  return propname;
}

// ECMA 8.7.3
KJSO *KJSO::getValue()
{
  if (!isA(Reference)) {
    return this->ref();
  }
  Ptr o = getBase();
  if (o->isA(Null))
    return new KJSError(ErrBaseIsNull, this);

  return o->get(getPropertyName());
}

// ECMA 8.7.4
ErrorCode KJSO::putValue(KJSO *v)
{
  if (!isA(Reference))
    return ErrNoReference;

  Ptr o = getBase();
  if (o->isA(Null)) {
    KJSWorld::global->put(getPropertyName(), v);
  } else
    o->put(getPropertyName(), v);

  return ErrOK;
}

KJSReference::KJSReference(KJSO *b, const CString &s)
{
  base = b->ref();
  propname = s;
}

KJSReference::~KJSReference()
{
  base->deref();
}

// ECMA 10.1.7 (draft April 98, 10.1.6 previously)
KJSActivation::KJSActivation(KJSFunction *f, KJSArgList *args)
{
  char buffer[10];
  KJSArg *arg;

  func = f;

  /* TODO: solve deleting problem due to circular reference */
  // put("arguments", this, DontDelete | DontEnum);
  if (func->hasProperty("arguments"))
    put("OldArguments", func->get("arguments"));
  put("callee", func, DontEnum);

  if (args) {
    int iarg = args->count();
    put("length", zeroRef(new KJSNumber(iarg)), DontEnum);
    arg = args->firstArg();
    for (int i = 0; i < iarg && i < 100; i++) {
      sprintf(buffer, "%d", i);
      put(buffer, arg->object());
      arg = arg->nextArg();
    }
  }
  /* TODO: solve deleting problem due to circular reference */
  //  func->put("arguments", this);
}

// ECMA 10.1.6
KJSActivation::~KJSActivation()
{
  if (!hasProperty("OldArguments"))
    func->deleteProperty("arguments");
  else
    func->put("arguments", get("OldArguments")); /* TODO: deep copy ? */
}

#if 0
// ECMA 10.1.8
KJSArguments::KJSArguments(KJSFunction *func, KJSArgList *args)
{
  char buffer[10];
  KJSArg *arg;

  assert(func);
  // TODO:
  // put("Prototype", _Object.prototype_ );
  put("callee", func, DontEnum);
  if (args) {
    int iarg = args->count();
    put("length", zeroRef(new KJSNumber(iarg)), DontEnum);
    arg = args->firstArg();
    for (int i = 0; i < iarg && i < 100; i++) {
      sprintf(buffer, "%d", i);
      put(buffer, arg->object());
      arg = arg->nextArg();
    }
    /* TODO: length != num. of arguments */
  }
}
#endif

KJSGlobal::KJSGlobal()
{
  put("NaN", zeroRef(new KJSNumber(NaN)));
  put("Infinity", zeroRef(new KJSNumber(Inf)));

  put("Math", zeroRef(new KJSMath()), DontEnum);

  // TODO: add function properties
  //  put("eval", new KJSInternalFunction(&eval));
}

KJSO* KJSGlobal::eval()
{
  Debug("KJSGLOBAL::eval");
  return new KJSNumber(11);
}

// ECMA 10.2
KJSContext::KJSContext(CodeType type, KJSContext *callingContext,
		       KJSFunction *func, KJSArgList *args, KJSO *thisV)
{
  KJSGlobal *glob = KJSWorld::global;
  assert(glob);

  // create and initialize activation object (ECMA 10.1.6)
  if (type == FunctionCode || type == AnonymousCode || type == HostCode) {
    activation = new KJSActivation(func, args);
    variable = activation;
  } else {
    activation = 0L;
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
      if (thisV->isA(Object)) {
	thisValue = thisV;
      }
      else
	thisValue = glob;
      break;
    case HostCode:
      if (thisV->isA(Object))
	thisValue = thisV;
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
  KJSScope *s = scopeChain;
  while (s) {
    KJSScope *tmp = s;
    s = s->next;
    tmp->deref();
  }
  if (activation)
    activation->deref();
    // delete activation;
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
void KJSO::dump(int level)
{
  if (level == 0) {
    cout << "-------------------------" << endl;
    cout << "Properties: " << endl;
    cout << "-------------------------" << endl;
  }
  if (prop) {
    KJSProperty *pr = prop;
    while (pr) {
      for (int i = 0; i < level; i++)
	cout << "  ";
      cout << pr->name.ascii() << endl;
      if (pr->object->prop && !(pr->name == "callee")) {
	pr->object->dump(level+1);
      }
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
 if (level == 0)
    cout << "-------------------------" << endl;
}

KJSArgList::~KJSArgList()
{
  KJSArg *tmp, *a = first;
  while (a) {
    tmp = a;
    a = a->nextArg();
    delete tmp;
  }
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
    for(int i = 0; i < param->count() && i < 100; i++)
      if (arg) {
	variable->put(param->at(i), arg->object());
	arg = arg->nextArg();
      } else
	variable->put(param->at(i), zeroRef(new KJSUndefined()));
  }
}

KJSDeclaredFunction::KJSDeclaredFunction(KJSParamList *p, StatementNode *b)
  : block(b)
{
  param = p;
}

KJSO* KJSDeclaredFunction::execute()
{
 /* TODO */
  return block->evaluate();
}

KJSError::KJSError(ErrorCode e, Node *n)
  : errNo(e)
{
  line = n ? n->lineNo() : -1;

  cerr << "Runtime error " << (int) e << " at line " << line << endl;
}

// can we make any use of the object reference ?
KJSError::KJSError(ErrorCode e, KJSO *)
  : errNo(e)
{
  cerr << "Runtime error " << (int) e << endl;
}
