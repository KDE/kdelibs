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
#include "object_object.h"

#define KJSRETURN(x) return new KJSCompletion(Normal,(x))

using namespace KJS;

ObjectObject::ObjectObject(KJSGlobal *glob)
{
  KJSConstructor *ctor = new ObjectConstructor(glob);
  setConstructor(ctor);
  glob->objProto->setConstructor(ctor);

  // setting these properties in ObjectPrototype's constructor would
  // have been preferable but we need a pointer to Function.prototype
  const int attr = DontEnum | DontDelete | ReadOnly;
  glob->objProto->put("toString",
		      zeroRef(new ObjectProtoFunc(IDObj2S, glob)), attr);
  glob->objProto->put("valueOf",
		      zeroRef(new ObjectProtoFunc(IDObjValOf, glob)), attr);
  setPrototype(glob->objProto);

  ctor->deref();

  put("length", 1, DontEnum);
}

KJSO *ObjectObject::execute(KJSContext *context)
{
  Ptr result;
  Ptr length = context->activation->get("length");
  int numArgs = (int) length->dVal();

  KJSList argList;
  if (numArgs == 0) {
    result = construct(&argList);
  } else {
    KJSO *arg = context->activation->get("0");
    if (arg->isA(Null) || arg->isA(Undefined)) {
      argList.append(arg);
      result = construct(&argList);
    } else
      result = toObject(arg);
  }
  KJSRETURN(result);
}

ObjectConstructor::ObjectConstructor(KJSGlobal *glob)
  : global(glob)
{
  setPrototype(glob->funcProto);
}

// ECMA 15.2.2
KJSObject* ObjectConstructor::construct(KJSList *args)
{
  // if no arguments have been passed ...
  if (args->size() == 0)
    return KJSObject::create(ObjectClass);

  KJSO *arg = args->begin();
  if (arg->isA(Object)) {
    /* TODO: handle host objects */
    KJSObject *obj = static_cast<KJSObject*>(arg->ref());
    return obj;
  }

  switch (arg->type()) {
  case String:
    return KJSObject::create(StringClass, arg);
  case Boolean:
    return KJSObject::create(BooleanClass, arg);
  case Number:
    return KJSObject::create(NumberClass, arg);
  default:
    assert(!"unhandled switch case in ObjectConstructor");
  case Null:
  case Undefined:
    return KJSObject::create(ObjectClass);
  }
}

ObjectPrototype::ObjectPrototype()
{
  setClass(BooleanClass);

  // the spec says that [[Property]] should be `null'.
  // Not sure if KJSNull or C's NULL is needed.
}

ObjectProtoFunc::ObjectProtoFunc(int i, KJSGlobal *global)
  : id(i)
{
  setPrototype(global->funcProto);
}

// ECMA 15.2.4.2 + 15.2.4.3
KJSO *ObjectProtoFunc::execute(KJSContext *)
{
  Ptr result;
  KJSO *thisVal = KJScript::context()->thisValue;

  /* TODO: what to do with non-objects. Is this possible at all ? */
  if (!thisVal->isA(Object)) {
    result = new KJSString("[no object]");
    return new KJSCompletion(ReturnValue, result);
  }

  KJSObject *thisObj = static_cast<KJSObject*>(thisVal);

  // valueOf()
  if (id == IDObjValOf)
    /* TODO: host objects*/
    return new KJSCompletion(Normal, thisObj);

  // toString()
  const char *str;
  switch(thisObj->getClass()) {
  case StringClass:
    str = "[object String]";
    break;
  case BooleanClass:
    str = "[object Boolean]";
    break;
  case NumberClass:
    str = "[object Number]";
    break;
  case ObjectClass:
    str = "[object Object]";
    break;
  default:
    str = "[undefined object]";
  }
  result = new KJSString(str);

  return new KJSCompletion(Normal, result);
}
