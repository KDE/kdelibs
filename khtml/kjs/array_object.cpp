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

#include "kjs.h"
#include "operations.h"
#include "array_object.h"

#define KJSRETURN(x) return new KJSCompletion(Normal,(x))

using namespace KJS;

ArrayObject::ArrayObject(KJSGlobal *global)
{
  KJSConstructor *ctor = new ArrayConstructor(global);
  setConstructor(ctor);
  global->arrayProto->setConstructor(ctor);
  setPrototype(global->arrayProto);

  ctor->deref();

  put("length", zeroRef(new KJSNumber(1)), DontEnum);
}

// ECMA 15.6.1
KJSO* ArrayObject::execute(KJSContext *context)
{
  // equivalent to 'new Array(....)'
  KJSArgList argList;
  Ptr length = context->activation->get("length");
  unsigned int numArgs = (unsigned int) length->dVal();
  char str[20];
  for (unsigned int u = 0; u < numArgs; u++) {
    sprintf(str, "%u", u);
    argList.append(context->activation->get(str));
  }
  Ptr result = construct(&argList);

  KJSRETURN(result);
}

ArrayConstructor::ArrayConstructor(KJSGlobal *glob)
  : global(glob)
{
  setPrototype(glob->funcProto);
}

// ECMA 15.6.2
KJSObject* ArrayConstructor::construct(KJSArgList *args)
{
  KJSObject *result = new KJSObject();
  result->setClass(ArrayClass);
  result->setPrototype(global->arrayProto);

  unsigned int len;
  KJSArg *arg = args->firstArg();
  // a single argument might denote the array size
  if (args->count() == 1 && arg->object()->isA(Number))
    len = toUInt32(arg->object());
  else {
    // initialize array
    len = args->count();
    char str[20];
    for (unsigned int u = 0; u < len; u++) {
      sprintf(str, "%u", u);
      result->put(str, arg->object());
      arg = arg->nextArg();
    }
  }

  // array size
  result->put("length", zeroRef(new KJSNumber(len)));

  return result;
}

// ECMA 15.6.4
ArrayPrototype::ArrayPrototype(KJSGlobal *global)
{
  // properties of the Array Prototype Object. The constructor will be
  // added later in ArrayObject's constructor
  setClass(ArrayClass);
  setPrototype(global->objProto);

  put("length", zeroRef(new KJSNumber(0)));

  /* TODO: put() properties */
}

#if 0
ArrayProtoFunc::ArrayProtoFunc(int i, KJSGlobal *global)
  : id(i)
{
  setPrototype(global->funcProto);
}

// ECMA 15.4.4
KJSO *ArrayProtoFunc::execute(KJSContext *context)
{
  /* TODO */
}
#endif
