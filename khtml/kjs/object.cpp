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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef KJS_DEBUG_MEM
#include <stdio.h>
#endif

#include "kjs.h"
#include "object.h"
#include "nodes.h"
#include "object_object.h"
#include "function_object.h"
#include "array_object.h"
#include "bool_object.h"
#include "string_object.h"
#include "math_object.h"

namespace KJS {

#ifdef WORDS_BIGENDIAN
  unsigned char NaN_Bytes[] = { 0x7f, 0xf8, 0, 0, 0, 0, 0, 0 };
  unsigned char Inf_Bytes[] = { 0x7f, 0xf0, 0, 0, 0, 0, 0, 0 };
#else
  unsigned char NaN_Bytes[] = { 0, 0, 0, 0, 0, 0, 0xf8, 0x7f };
  unsigned char Inf_Bytes[] = { 0, 0, 0, 0, 0, 0, 0xf0, 0x7f };
#endif

  const double NaN = *(const double*) NaN_Bytes;
  const double Inf = *(const double*) Inf_Bytes;
  // TODO: -0
  const double D16 = 65536.0;
  const double D31 = 2147483648.0;
  const double D32 = 4294967296.0;

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
  "Host",
  "Reference",
  "List",
  "Completion",
  "Property",
  "Scope",
  "InternalFunction",
  "DeclaredFunction",
  "AnonymousFunction",
  "Constructor",
  "Activation",
  "Error"
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
  constr = 0L;
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

  if (constr)
    constr->deref();

  if (proto)
    proto->deref();

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
KJSO *KJSO::executeCall(KJSO *thisV, KJSList *args)
{
  // second part of our hack to allow code like "abc".charAt(0).
  if (thisV->isA(String)) {
    KJSObject *tmp = new KJSObject();
    Ptr str = new KJSString(thisV->sVal());
    tmp->setClass(StringClass);
    tmp->setInternalValue(str);
    tmp->setPrototype(KJScript::global()->stringProto);
    thisV = tmp;
  }

  KJSFunction *func = static_cast<KJSFunction*>(this);

  KJSContext *save = KJScript::context();

  CodeType ctype = func->codeType();
  KJScript::setContext(new KJSContext(ctype, save, func, args, thisV));

  // assign user supplied arguments to parameters
  if (args)
    func->processParameters(args);

  Ptr comp = func->execute(KJScript::context());

  delete KJScript::context();
  KJScript::setContext(save);

  if (comp->isValueCompletion())
    return comp->complValue();
  else
    return new KJSUndefined();
}

void KJSO::setConstructor(KJSConstructor *c)
{
  assert(c);
  c->ref();
  constr = c;
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
    KJScript::global()->put(getPropertyName(), v);
  } else {
    // are we writing into an array ?
    if (o->isA(Object) && (((KJSObject*)(KJSO*)o)->getClass() == ArrayClass))
      o->putArrayElement(getPropertyName(), v);
    else
      o->put(getPropertyName(), v);
  }

  return ErrOK;
}

// [[construct]] property
KJSObject *KJSO::construct(KJSList *args)
{
  assert(constr);
  /* TODO: pass `undefined' arguments if needed */
  return constr->construct(args);
}

void KJSO::setPrototype(KJSPrototype *p)
{
  assert(p);
  p->ref();
  proto = p;
  put("prototype", p, DontEnum | DontDelete | ReadOnly);
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
KJSActivation::KJSActivation(KJSFunction *f, KJSList *args)
{
  func = f;

  /* TODO: solve deleting problem due to circular reference */
  // put("arguments", this, DontDelete | DontEnum);
  if (func->hasProperty("arguments"))
    put("OldArguments", func->get("arguments"));
  put("callee", func, DontEnum);
  if (args) {
    put("length", args->size(), DontEnum);
    KJSListIterator arg = args->begin();
    for (int i = 0; arg != args->end(); arg++, i++) {
      put(int2String(i), arg);
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
  KJSArg *arg;

  assert(func);
  // TODO:
  // put("Prototype", _Object.prototype_ );
  put("callee", func, DontEnum);
  if (args) {
    int iarg = args->size();
    put("length", iarg, DontEnum);
    arg = args->firstArg();
    for (int i = 0; i < iarg && i < 100; i++) {
      put(int2String(i), arg->object());
      arg = arg->nextArg();
    }
    /* TODO: length != num. of arguments */
  }
}
#endif

KJSGlobal::KJSGlobal()
{
  // value properties
  put("NaN", NaN);
  put("Infinity", Inf);

  // function properties

  // constructor properties. prototypes as Global's member variables first.
  objProto = new ObjectPrototype();
  funcProto = new FunctionPrototype();
  arrayProto = new ArrayPrototype(this);
  stringProto = new StringPrototype(this);
  boolProto = new BooleanPrototype(this);

  put("Object", zeroRef(new ObjectObject(this)), DontEnum);
  put("Array", zeroRef(new ArrayObject(this)), DontEnum);
  put("String", zeroRef(new StringObject(this)), DontEnum);
  put("Boolean", zeroRef(new BooleanObject(this)), DontEnum);

  // other properties
  put("Math", zeroRef(new KJSMath()), DontEnum);

  objProto->deref();
  funcProto->deref();
  arrayProto->deref();
  boolProto->deref();
}

// ECMA 10.2
KJSContext::KJSContext(CodeType type, KJSContext *callingContext,
		       KJSFunction *func, KJSList *args, KJSO *thisV)
{
  KJSGlobal *glob = KJScript::global();
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
      scopeChain = new KJSScopeChain();
      scopeChain->append(glob);
      variable = glob;
      thisValue = glob;
      break;
    case FunctionCode:
    case AnonymousCode:
      scopeChain = new KJSScopeChain();
      scopeChain->append(activation);
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
      scopeChain = new KJSScopeChain();
      scopeChain->append(activation);
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
  scopeChain->deref();

  if (activation)
    activation->deref();
    // delete activation;
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

// ECMA 10.1.3
void KJSFunction::processParameters(KJSList *args)
{
  KJSO *variable = KJScript::context()->variableObject();

  assert(args);

  if (param) {
    KJSListIterator it = args->begin();
    for(int i = 0; i < param->count() && i < 100; i++)
      if (it != args->end()) {
	variable->put(param->at(i), it);
	it++;
      } else
	variable->put(param->at(i), zeroRef(new KJSUndefined()));
  }
}

KJSDeclaredFunction::KJSDeclaredFunction(KJSParamList *p, StatementNode *b)
  : block(b)
{
  param = p;
}

KJSO* KJSDeclaredFunction::execute(KJSContext *)
{
 /* TODO */
  return block->evaluate();
}

KJSO* KJSAnonymousFunction::execute(KJSContext *)
{
 /* TODO */
  return 0L;
}

KJSError::KJSError(ErrorCode e, Node *n)
  : errNo(e)
{
  line = n ? n->lineNo() : -1;

  if (!KJScript::error()) {
    ref();
    KJScript::setError(this);
  }

  cerr << "Runtime error " << (int) e << " at line " << line << endl;
}

// can we make any use of the object reference ?
KJSError::KJSError(ErrorCode e, KJSO *)
  : errNo(e)
{
  if (!KJScript::error()) {
    ref();
    KJScript::setError(this);
  }

  cerr << "Runtime error " << (int) e << endl;
}

KJSList::KJSList()
{
  hook = new KJSListNode(0L, 0L, 0L);
  hook->next = hook;
  hook->prev = hook;
}

KJSList::~KJSList()
{
  clear();
  delete hook;
}

void KJSList::append(KJSO *obj)
{
  KJSListNode *n = new KJSListNode(obj->ref(), hook->prev, hook);
  hook->prev->next = n;
  hook->prev = n;
}

void KJSList::prepend(KJSO *obj)
{
  KJSListNode *n = new KJSListNode(obj->ref(), hook, hook->next);
  hook->next->prev = n;
  hook->next = n;
}

void KJSList::removeFirst()
{
  erase(begin());
}

void KJSList::removeLast()
{
  KJSListIterator it(end());
  erase(--it);
}

void KJSList::clear()
{
  KJSListNode *n = hook->next;
  while (n != hook) {
    n = n->next;
    delete n->prev;
  }

  hook->next = hook;
  hook->prev = hook;
}

void KJSList::erase(KJSListIterator it)
{
  KJSListNode *n = it.node;

  if (n != hook) {
    n->next->prev = n->prev;
    n->prev->next = n->next;
    delete n;
  }
}

int KJSList::size() const
{
  int s = 0;
  KJSListNode *node = hook;
  while ((node = node->next) != hook)
    s++;

  return s;
}

KJSO *HostObject::get(const CString &) const
{
  return new KJSUndefined();
}

void HostObject::put(const CString &, KJSO *, int)
{
  cout << "Ignoring put() in HostObject" << endl;
}
