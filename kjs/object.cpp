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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>

#include "kjs.h"
#include "object.h"
#include "types.h"
#include "operations.h"

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

KJSO::KJSO()
{
  init();
}

void KJSO::init()
{
  pprototype = 0L; prop = 0L;
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
  Property *tmp, *p = prop;
  while (p) {
    tmp = p;
    p = p->next;
    delete tmp;
  }

  if (pprototype)
    pprototype->deref();

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

KJSO *KJSO::newNull()
{
  return new Null();
}

KJSO *KJSO::newUndefined()
{
  return new Undefined();
}

KJSO *KJSO::newBoolean(bool b)
{
  return new Boolean(b);
}

KJSO *KJSO::newString(const UString &s)
{
  return new String(s);
}

KJSO *KJSO::newNumber(double d)
{
  return new Number(d);
}

KJSO *KJSO::newNumber(int i)
{
  return new Number(i);
}

KJSO *KJSO::newNumber(unsigned int u)
{
  return new Number(u);
}

KJSO *KJSO::newNumber(long unsigned int l)
{
  return new Number(l);
}

KJSO *KJSO::newCompletion(Compl c, KJSO *v, const UString &t)
{
  return new Completion(c, v, t);
}

KJSO *KJSO::newReference(KJSO *b, const UString &s)
{
  return new Reference(b, s);
}

KJSO *KJSO::newError(ErrorCode e, Node *n)
{
  return new Error(e, n);
}

KJSO *KJSO::newError(ErrorCode e, KJSO *o)
{
  return new Error(e, o);
}

bool KJSO::implementsCall() const
{
  return (type() == InternalFunctionType ||
	  type() == ConstructorType ||
	  type() == DeclaredFunctionType ||
	  type() == AnonymousFunctionType);
}

// [[call]]
KJSO *KJSO::executeCall(KJSO *thisV, const List *args)
{
  if (!args)
    args = List::empty();

  Function *func = static_cast<Function*>(this);

  Context *save = KJScript::context();

  CodeType ctype = func->codeType();
  KJScript::setContext(new Context(ctype, save, func, args, thisV));

  // assign user supplied arguments to parameters
  func->processParameters(args);

  Ptr comp = func->execute(*args);

  delete KJScript::context();
  KJScript::setContext(save);

  if (comp->isValueCompletion())
    return comp->complValue();
  else
    return newUndefined();
}

void KJSO::setConstructor(KJSO *c)
{
  assert(c);
  put("constructor", c, DontEnum | DontDelete | ReadOnly);
}

// ECMA 8.7.1
KJSO *KJSO::getBase()
{
  if (!isA(ReferenceType))
    return newError(ErrBaseNoRef, this);

  return value.base->ref();
}

// ECMA 8.7.2
UString KJSO::getPropertyName()
{
  if (!isA(ReferenceType))
    // the spec wants a runtime error here. But getValue() and putValue()
    // will catch this case on their own earlier. When returning a Null
    // string we should be on the safe side.
    return UString();

  return strVal;
}

// ECMA 8.7.3
KJSO *KJSO::getValue()
{
  if (!isA(ReferenceType)) {
    return this->ref();
  }
  Ptr o = getBase();
  if (o->isA(NullType))
    return newError(ErrBaseIsNull, this);

  return o->get(getPropertyName());
}

// ECMA 8.7.4
ErrorCode KJSO::putValue(KJSO *v)
{
  if (!isA(ReferenceType))
    return ErrNoReference;

  Ptr o = getBase();
  if (o->isA(NullType)) {
    KJScript::global()->put(getPropertyName(), v);
  } else {
    // are we writing into an array ?
    if (o->isA(ObjectType) && (((Object*)(KJSO*)o)->getClass() == ArrayClass))
      o->putArrayElement(getPropertyName(), v);
    else
      o->put(getPropertyName(), v);
  }

  return ErrOK;
}

void KJSO::setPrototype(Object *p)
{
  assert(p);
  p->ref();
  pprototype = p;
  put("prototype", p, DontEnum | DontDelete | ReadOnly);
}

// ECMA 8.6.2.1
KJSO *KJSO::get(const UString &p)
{
  if (prop) {
    Property *pr = prop;
    while (pr) {
      if (pr->name == p)
	return pr->object->ref();
      pr = pr->next;
    }
  }
  if (!prototype())
    return newUndefined();

  return prototype()->get(p);
}

// ECMA 8.6.2.2 (may be overriden)
void KJSO::put(const UString &p, KJSO *v)
{
  if (!canPut(p))
    return;

  Property *pr;

  if (prop) {
    pr = prop;
    while (pr) {
      if (pr->name == p) {
	// replace old value
	pr->object = v->ref();
	return;
      }
      pr = pr->next;
    }
  }

  // add new property
  pr = new Property(p, v);
  pr->next = prop;
  prop = pr;
}

// ECMA 8.6.2.2
void KJSO::put(const UString &p, KJSO *v, int /*attr*/)
{
  put(p, v);
  /* TODO: set attributes */
}

// provided for convenience.
void KJSO::put(const UString &p, double d, int attr)
{
  put(p, zeroRef(newNumber(d)), attr);
}

// provided for convenience.
void KJSO::put(const UString &p, int i, int attr)
{
  put(p, zeroRef(newNumber(i)), attr);
}

// provided for convenience.
void KJSO::put(const UString &p, unsigned int u, int attr)
{
  put(p, zeroRef(newNumber(u)), attr);
}

// ECMA 15.4.5.1
void KJSO::putArrayElement(const UString &p, KJSO *v)
{
  if (!canPut(p))
    return;

  if (hasProperty(p)) {
    if (p == "length") {
      Ptr len = get("length");
      unsigned int oldLen = toUInt32(len);
      unsigned int newLen = toUInt32(v);
      // shrink array
      for (unsigned int u = newLen; u < oldLen; u++) {
	UString p = int2String(u);
	if (hasProperty(p, false))
	  deleteProperty(p);
      }
      put("length", newLen);
      return;
    }
    //    put(p, v);
    } //  } else
    put(p, v);

  // array index ?
  unsigned int idx;
  if (!sscanf(p.cstring().c_str(), "%u", &idx)) /* TODO */
    return;

  // do we need to update/create the length property ?
  if (hasProperty("length", false)) {
    Ptr len = get("length");
    if (idx < toUInt32(len))
      return;
  }

  put("length", idx+1);
}

// ECMA 8.6.2.3
bool KJSO::canPut(const UString &p) const
{
  if (prop) {
    Property *pr = prop;
    while (pr) {
      if (pr->name == p)
	return !(pr->attribute & ReadOnly);
      pr = pr->next;
    }
  }
  if (!prototype())
    return true;

  return prototype()->canPut(p);
}

// ECMA 8.6.2.4
bool KJSO::hasProperty(const UString &p, bool recursive) const
{
  if (!prop)
    return false;

  Property *pr = prop;
  while (pr) {
    if (pr->name == p)
      return true;
    pr = pr->next;
  }

  if (!prototype() || !recursive)
    return false;

  return prototype()->hasProperty(p);
}

// ECMA 8.6.2.5
void KJSO::deleteProperty(const UString &p)
{
  if (prop) {
    Property *pr = prop;
    Property **prev = &prop;
    while (pr) {
      if (pr->name == p) {
	*prev = pr->next;
	delete pr;
	return;
      }
      prev = &(pr->next);
      pr = pr->next;
    }
  }
}

// ECMA 8.6.2.6 (new draft)
KJSO* KJSO::defaultValue(Type hint)
{
  Ptr o, s;

  if (hint == UndefinedType)
    hint = NumberType;

  if (hint == StringType)
    o = get("toString");
  else
    o = get("valueOf");

  if (o->implementsCall()) { // spec says "not primitive type" but ...
    s = o->executeCall(this, 0L);
    if (!s->isObject())
      return s->ref();
  }

  if (hint == StringType)
    o = get("valueOf");
  else
    o = get("toString");

  if (o->implementsCall()) {
    s = o->executeCall(this, 0L);
    if (!s->isObject())
      return s->ref();
  }

  return newError(ErrNoDefault, this);
}

Object::Object(Class c, KJSO *v, Object *p)
  : classType(c), objValue(v)
{
  if (p)
    setPrototype(p);
}

// factory
Object* Object::create(Class c, KJSO *val, Object *p)
{
  Global *global = KJScript::global();
  Object *obj = new Object();
  Object *prot;
  obj->setClass(c);
  obj->setInternalValue(val);

  if (p)
    prot = p;
  else
    switch (c) {
    case ObjectClass:
      prot = global->objProto;
      break;
    case ArrayClass:
      prot = global->arrayProto;
      break;
    case StringClass:
      prot = global->stringProto;
      break;
    case BooleanClass:
      prot = global->boolProto;
      break;
    case NumberClass:
      prot = global->numProto;
      break;
    case UndefClass:
      prot = 0L;
      break;
    }

  obj->setPrototype(prot);
  return obj;
}

// debugging info
void KJSO::dump(int level)
{
  if (level == 0) {
    printf("-------------------------\n");
    printf("Properties:\n");
    printf("-------------------------\n");
  }
  if (prop) {
    Property *pr = prop;
    while (pr) {
      for (int i = 0; i < level; i++)
	printf("  ");
      printf("%s\n", pr->name.cstring().c_str());
      if (pr->object->prop && !(pr->name == "callee")) {
	pr->object->dump(level+1);
      }
      pr = pr->next;
    }
  } else
    printf("   None.\n");

#if 0
   if(prototype()) {
    printf("from prototype:\n");
    prototype()->dump();
  }
#endif
 if (level == 0)
    printf("-------------------------\n");
}

KJSO *HostObject::get(const UString &)
{
  return newUndefined();
}

void HostObject::put(const UString &, KJSO *)
{
  //  cout << "Ignoring put() in HostObject" << endl;
}
