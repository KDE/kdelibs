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
#include "bool_object.h"

using namespace KJS;

// ECMA 15.6.1
KJSO* BooleanObject::execute(const List &args)
{
  Ptr v, b;

  if (args.isEmpty())
    b = newBoolean(false);
  else {
    v = args[0];
    b = toBoolean(v);
  }

  return newCompletion(Normal, b);
}

// ECMA 15.6.2
Object* BooleanObject::construct(const List &args)
{
  Ptr b;
  if (args.size() > 0)
    b = toBoolean(args.begin());
  else
    b = newBoolean(false);

  return Object::create(BooleanClass, b);
}

// ECMA 15.6.4
BooleanPrototype::BooleanPrototype(Object *proto)
  : Object(BooleanClass, zeroRef(newBoolean(false)), proto)
{
  // The constructor will be added later in BooleanObject's constructor
}

KJSO *BooleanPrototype::get(const UString &p)
{
  if (p == "toString")
    return new BooleanProtoFunc(ToString);
  else if (p == "valueOf")
    return new BooleanProtoFunc(ValueOf);
  else
    return KJSO::get(p);
}

BooleanProtoFunc::BooleanProtoFunc(int i)
  : id(i)
{
  setPrototype(KJScript::global()->funcProto);
}

// ECMA 15.6.4.2 + 15.6.4.3
KJSO *BooleanProtoFunc::execute(const List &)
{
  Ptr result;
  KJSO *thisVal = thisValue();

  Object *thisObj = static_cast<Object*>(thisVal);

  // no generic function. "this" has to be a Boolean object
  if ((!thisVal->isA(ObjectType)) || (thisObj->getClass() != BooleanClass)) {
    result = newError(TypeError);
    return newCompletion(ReturnValue, result);
  }

  // execute "toString()" or "valueOf()", respectively
  Ptr v = thisObj->internalValue();
  if (id == BooleanPrototype::ToString)
    result = toString(v);
  else
    result = v->ref();

  return newCompletion(Normal, result);
}
