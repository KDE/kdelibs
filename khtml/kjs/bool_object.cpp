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

#define KJSARG(x) KJSWorld::context->activation->get((x))
#define KJSRETURN(x) return new KJSCompletion(Normal,(x))

using namespace KJS;

BooleanObject::BooleanObject(KJSPrototype *objProto, KJSPrototype *funcProto)
{
  KJSPrototype *boolProto = new BooleanPrototype(objProto, funcProto);
  KJSConstructor *ctor = new BooleanConstructor(boolProto, funcProto);
  setConstructor(ctor);
  boolProto->setConstructor(ctor);
  setPrototype(boolProto);
  
  boolProto->deref();
  ctor->deref();

  put("length", zeroRef(new KJSNumber(1)), DontEnum);
}

// ECMA 15.6.1
KJSO* BooleanObject::execute()
{
  Ptr v, b;
  Ptr length = KJSWorld::context->activation->get("length");
  int numArgs = (int) length->dVal();

  if (numArgs == 0)
    b = new KJSBoolean(false);
  else {
    v = KJSARG("0");
    b = toBoolean(v);
  }

  KJSRETURN(b);
}

BooleanConstructor::BooleanConstructor(KJSPrototype *bp, KJSPrototype *fp)
  : boolProto(bp)
{
  setPrototype(fp);
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
  result->setPrototype(boolProto);
  result->setInternalValue(b);

  return result;
}

// ECMA 15.6.4
BooleanPrototype::BooleanPrototype(KJSPrototype *objProto,
				   KJSPrototype *funcProto)
{
  // properties of the Boolean Prototype Object. The constructor will be
  // added later in BooleanObject's constructor
  setClass(BooleanClass);
  setInternalValue(zeroRef(new KJSBoolean(false)));
  setPrototype(objProto);

  const int attr = DontEnum | DontDelete | ReadOnly;
  put("toString", zeroRef(new BooleanProtoFunc(IDBool2S, funcProto)), attr);
  put("valueOf", zeroRef(new BooleanProtoFunc(IDBoolvalOf, funcProto)), attr);
}

BooleanProtoFunc::BooleanProtoFunc(int i, KJSPrototype *funcProto)
  : id(i)
{
  setPrototype(funcProto);
}

// ECMA 15.6.4.2 + 15.6.4.3
KJSO *BooleanProtoFunc::execute()
{
  Ptr result;
  KJSO *thisVal = KJSWorld::context->thisValue;

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
