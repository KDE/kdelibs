/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
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

using namespace KJS;

KJSO *ObjectObject::execute(Context *context)
{
  Ptr result;
  int numArgs = context->numArgs();

  List argList;
  if (numArgs == 0) {
    result = construct(argList);
  } else {
    KJSO *arg = context->arg(0);
    if (arg->isA(NullType) || arg->isA(UndefinedType)) {
      argList.append(arg);
      result = construct(argList);
    } else
      result = toObject(arg);
  }
  return newCompletion(Normal, result);
}

// ECMA 15.2.2
Object* ObjectObject::construct(const List &args)
{
  // if no arguments have been passed ...
  if (args.size() == 0)
    return Object::create(ObjectClass);

  KJSO *arg = args.begin();
  if (arg->isA(ObjectType)) {
    /* TODO: handle host objects */
    Object *obj = static_cast<Object*>(arg->ref());
    return obj;
  }

  switch (arg->type()) {
  case StringType:
    return Object::create(StringClass, arg);
  case BooleanType:
    return Object::create(BooleanClass, arg);
  case NumberType:
    return Object::create(NumberClass, arg);
  default:
    assert(!"unhandled switch case in ObjectConstructor");
  case NullType:
  case UndefinedType:
    return Object::create(ObjectClass);
  }
}

ObjectPrototype::ObjectPrototype()
  : Object(ObjectClass)
{
  // the spec says that [[Property]] should be `null'.
  // Not sure if Null or C's NULL is needed.
}

KJSO *ObjectPrototype::get(const UString &p)
{
  if (p == "toString")
    return new ObjectProtoFunc(ToString);
  else if (p == "valueOf")
    return new ObjectProtoFunc(ValueOf);
  else
    return KJSO::get(p);
}

ObjectProtoFunc::ObjectProtoFunc(int i)
  : id(i)
{
  setPrototype(KJScript::global()->funcProto);
}

// ECMA 15.2.4.2 + 15.2.4.3
KJSO *ObjectProtoFunc::execute(Context *)
{
  Ptr result;
  KJSO *thisVal = KJScript::context()->thisValue;

  /* TODO: what to do with non-objects. Is this possible at all ? */
  if (!thisVal->isA(ObjectType)) {
    result = newString("[no object]");
    return newCompletion(ReturnValue, result);
  }

  Object *thisObj = static_cast<Object*>(thisVal);

  // valueOf()
  if (id == ObjectPrototype::ValueOf)
    /* TODO: host objects*/
    return newCompletion(Normal, thisObj);

  // toString()
  UString str;
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
  result = newString(str);

  return newCompletion(Normal, result);
}
