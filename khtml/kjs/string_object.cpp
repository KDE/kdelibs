/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>

#include "kjs.h"
#include "operations.h"
#include "string_object.h"

#define KJSRETURN(x) return new KJSCompletion(Normal,(x))

using namespace KJS;

StringObject::StringObject(KJSGlobal *global)
{
  KJSConstructor *ctor = new StringConstructor(global);
  setConstructor(ctor);
  global->stringProto->setConstructor(ctor);
  setPrototype(global->stringProto);
  
  ctor->deref();

  put("length", zeroRef(new KJSNumber(1)), DontEnum);
}

// ECMA 15.8.1
KJSO* StringObject::execute(KJSContext *context)
{
  Ptr v, s;
  Ptr length = context->activation->get("length");
  int numArgs = (int) length->dVal();

  if (numArgs == 0)
    s = new KJSString("");
  else {
    v = context->activation->get("0");
    s = toString(v);
  }

  KJSRETURN(s);
}

StringConstructor::StringConstructor(KJSGlobal *glob)
  : global(glob)
{
  setPrototype(glob->funcProto);
  
  /* TODO: fromCharCode() */
}

// ECMA 15.8.2
KJSObject* StringConstructor::construct(KJSList *args)
{
  Ptr s;
  if (args->size() > 0)
    s = toString(args->begin());
  else
    s = new KJSString("");

  KJSObject *result = new KJSObject();
  result->setClass(StringClass);
  result->setPrototype(global->stringProto);
  result->setInternalValue(s);

  return result;
}

// ECMA 15.8.4
StringPrototype::StringPrototype(KJSGlobal *global)
{
  // properties of the String Prototype Object. The constructor will be
  // added later in StringObject's constructor
  setClass(StringClass);
  setInternalValue(zeroRef(new KJSString("")));
  setPrototype(global->objProto);

  const int attr = DontEnum | DontDelete | ReadOnly;
  put("toString",
      zeroRef(new StringProtoFunc(StringProtoFunc::ToString, global)), attr);
  put("valueOf",
      zeroRef(new StringProtoFunc(StringProtoFunc::ValueOf, global)), attr);
  put("charAt",
      zeroRef(new StringProtoFunc(StringProtoFunc::CharAt, global)), attr);
  put("charCodeAt",
      zeroRef(new StringProtoFunc(StringProtoFunc::CharCodeAt, global)), attr);
  put("indexOf",
      zeroRef(new StringProtoFunc(StringProtoFunc::IndexOf, global)), attr);
  put("lastIndexOf",
      zeroRef(new StringProtoFunc(StringProtoFunc::LastIndexOf, global)), attr);
  put("substr",
      zeroRef(new StringProtoFunc(StringProtoFunc::Substr, global)), attr);
  put("substring",
      zeroRef(new StringProtoFunc(StringProtoFunc::Substring, global)), attr);
}

StringProtoFunc::StringProtoFunc(int i, KJSGlobal *global)
  : id(i)
{
  setPrototype(global->funcProto);
}

// ECMA 15.8.4.2 - 15.8.4.20
KJSO *StringProtoFunc::execute(KJSContext *context)
{
  Ptr result;
  KJSO *thisVal = context->thisValue;

  KJSObject *thisObj = static_cast<KJSObject*>(thisVal);

  // toString and valueOf are no generic function.
  if (id == ToString || id == ValueOf) {
    if ((!thisVal->isA(Object)) || (thisObj->getClass() != StringClass)) {
      result = new KJSError(ErrInvalidThis, this);
      return new KJSCompletion(ReturnValue, result);
    }
  }

  Ptr n, m, s2;
  UString u;
  int pos;
  double d, d2;
  Ptr v = thisObj->internalValue();
  Ptr s = toString(v);
  int len = (int) s->sVal().size();
  Ptr a0 = context->activation->get("0");
  Ptr a1 = context->activation->get("1");

  switch (id) {
  case ToString:
  case ValueOf:
    result = v->ref();
    break;
  case CharAt:
    n = toInteger(a0);
    pos = (int) n->dVal();
    if (pos < 0 || pos >= len)
      u = "";
    else
      u = s->sVal().substr(pos, 1);
    result = new KJSString(u);
    break;
  case CharCodeAt:
    n = toInteger(a0);
    pos = (int) n->dVal();
    if (pos < 0 || pos >= len)
      d = NaN;
    else {
      UChar c = s->sVal()[pos];
      d = (c.hi >> 8) + c.lo;
    }
    result = new KJSNumber(d);
    break;
  case IndexOf:
    s2 = toString(a0);
    n = toInteger(a1);
    if (n->isA(Undefined))
      pos = 0;
    else
      pos = (int) n->dVal();
    d = s->sVal().find(s2->sVal(), pos);
    result = new KJSNumber(d);
    break;
  case LastIndexOf:
    s2 = toString(a0);
    n = toInteger(a1);
    if (n->isA(Undefined))
      pos = len;
    else
      pos = (int) n->dVal();
    d = s->sVal().rfind(s2->sVal(), pos);
    result = new KJSNumber(d);
    break;
  case Substr:
    n = toInteger(a0);
    m = toInteger(a1);
    if (n->dVal() >= 0)
      d = n->dVal();
    else
      d = max(len + n->dVal(), 0);
    if (a1->isA(Undefined))
      d2 = len - d;
    else
      d2 = min(max(m->dVal(), 0), len - d);
    result = new KJSString(s->sVal().substr((int)d, (int)d2));
    break;
  case Substring:
    n = toInteger(a0);
    m = toInteger(a1);
    d = min(max(n->dVal(), 0), len);
    if (a1->isA(Undefined))
      d2 = len - d;
    else {
      d2 = min(max(m->dVal(), 0), len);
      d2 = max(d2-d, 0);
    }
    result = new KJSString(s->sVal().substr((int)d, (int)d2));
    break;
  }

  return new KJSCompletion(Normal, result);
}
