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

#include "kjs.h"
#include "operations.h"
#include "bool_object.h"

#define KJSRETURN(x) return new KJSCompletion(Normal,(x))

using namespace KJS;

BooleanObject::BooleanObject(KJSGlobal *global)
{
  //  KJSPrototype *boolProto = new BooleanPrototype(objProto, funcProto);
  KJSConstructor *ctor = new BooleanConstructor(global);
  setConstructor(ctor);
  global->boolProto->setConstructor(ctor);
  setPrototype(global->boolProto);
  
  //  boolProto->deref();
  ctor->deref();

  put("length", zeroRef(new KJSNumber(1)), DontEnum);
}

// ECMA 15.6.1
KJSO* BooleanObject::execute(KJSContext *context)
{
  Ptr v, b;
  Ptr length = context->activation->get("length");
  int numArgs = (int) length->dVal();

  if (numArgs == 0)
    b = new KJSBoolean(false);
  else {
    v = context->activation->get("0");
    b = toBoolean(v);
  }

  KJSRETURN(b);
}

BooleanConstructor::BooleanConstructor(KJSGlobal *glob)
  : global(glob)
{
  setPrototype(glob->funcProto);
}

// ECMA 15.6.2
KJSObject* BooleanConstructor::construct(KJSArgList *args)
{
  Ptr b;
  if (args->count() > 0)
    b = toBoolean(args->firstArg()->object());
  else
    b = new KJSBoolean(false);

  KJSObject *result = new KJSObject();
  result->setClass(BooleanClass);
  result->setPrototype(global->boolProto);
  result->setInternalValue(b);

  return result;
}

// ECMA 15.6.4
BooleanPrototype::BooleanPrototype(KJSGlobal *global)
{
  // properties of the Boolean Prototype Object. The constructor will be
  // added later in BooleanObject's constructor
  setClass(BooleanClass);
  setInternalValue(zeroRef(new KJSBoolean(false)));
  setPrototype(global->objProto);

  const int attr = DontEnum | DontDelete | ReadOnly;
  put("toString", zeroRef(new BooleanProtoFunc(IDBool2S, global)), attr);
  put("valueOf", zeroRef(new BooleanProtoFunc(IDBoolvalOf, global)), attr);
}

BooleanProtoFunc::BooleanProtoFunc(int i, KJSGlobal *global)
  : id(i)
{
  setPrototype(global->funcProto);
}

// ECMA 15.6.4.2 + 15.6.4.3
KJSO *BooleanProtoFunc::execute(KJSContext *context)
{
  Ptr result;
  KJSO *thisVal = context->thisValue;

  KJSObject *thisObj = static_cast<KJSObject*>(thisVal);

  // no generic function. "this" has to be a Boolean object
  if ((!thisVal->isA(Object)) || (thisObj->getClass() != BooleanClass)) {
    result = new KJSError(ErrInvalidThis, this);
    return new KJSCompletion(ReturnValue, result);
  }

  // execute "toString()" or "valueOf()", respectively
  Ptr v = thisObj->internalValue();
  if (id == IDBool2S)
    result = toString(v);
  else
    result = v->ref();

  return new KJSCompletion(Normal, result);
}
