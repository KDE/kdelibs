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
#include "string_object.h"

using namespace KJS;

/* TODO: fromCharCode() */

// ECMA 15.8.1
KJSO* StringObject::execute(const List &args)
{
  Ptr v, s;

  if (args.isEmpty())
    s = newString("");
  else {
    v = args[0];
    s = toString(v);
  }

  return newCompletion(Normal, s);
}

// ECMA 15.8.2
Object* StringObject::construct(const List &args)
{
  Ptr s;
  if (args.size() > 0)
    s = toString(args.begin());
  else
    s = newString("");

  return Object::create(StringClass, s);
}

// ECMA 15.8.4
StringPrototype::StringPrototype(Object *proto)
  : Object(StringClass, zeroRef(newString("")), proto)
{
  // The constructor will be added later in StringObject's constructor
}

KJSO *StringPrototype::get(const UString &p)
{
  int id;

  if (p == "toString")
    id = StringProtoFunc::ToString;
  else if (p == "valueOf")
    id = StringProtoFunc::ValueOf;
  else if (p == "charAt")
    id = StringProtoFunc::CharAt;
  else if (p == "charCodeAt")
    id = StringProtoFunc::CharCodeAt;
  else if (p == "indexOf")
    id = StringProtoFunc::IndexOf;
  else if (p == "lastIndexOf")
    id = StringProtoFunc::LastIndexOf;
  else if (p == "substr")
    id = StringProtoFunc::Substr;
  else if (p == "substring")
    id = StringProtoFunc::Substring;
  else
    return KJSO::get(p);

  return new StringProtoFunc(id);
}

StringProtoFunc::StringProtoFunc(int i)
  : id(i)
{
  setPrototype(KJScript::global()->funcProto);
}

// ECMA 15.8.4.2 - 15.8.4.20
KJSO *StringProtoFunc::execute(const List &args)
{
  Ptr result;
  KJSO *thisVal = thisValue();

  Object *thisObj = static_cast<Object*>(thisVal);

  // toString and valueOf are no generic function.
  if (id == ToString || id == ValueOf) {
    if ((!thisVal->isA(ObjectType)) || (thisObj->getClass() != StringClass)) {
      result = newError(ErrInvalidThis, this);
      return newCompletion(ReturnValue, result);
    }
  }

  Ptr n, m, s2;
  UString u;
  int pos;
  double d, d2;
  Ptr v = thisObj->internalValue();
  Ptr s = toString(v);
  int len = (int) s->stringVal().size();
  Ptr a0 = args[0];
  Ptr a1 = args[1];

  switch (id) {
  case ToString:
  case ValueOf:
    result = v->ref();
    break;
  case CharAt:
    n = toInteger(a0);
    pos = (int) n->doubleVal();
    if (pos < 0 || pos >= len)
      u = "";
    else
      u = s->stringVal().substr(pos, 1);
    result = newString(u);
    break;
  case CharCodeAt:
    n = toInteger(a0);
    pos = (int) n->doubleVal();
    if (pos < 0 || pos >= len)
      d = NaN;
    else {
      UChar c = s->stringVal()[pos];
      d = (c.hi << 8) + c.lo;
    }
    result = newNumber(d);
    break;
  case IndexOf:
    s2 = toString(a0);
    n = toInteger(a1);
    if (n->isA(UndefinedType))
      pos = 0;
    else
      pos = n->intVal();
    d = s->stringVal().find(s2->stringVal(), pos);
    result = newNumber(d);
    break;
  case LastIndexOf:
    s2 = toString(a0);
    n = toInteger(a1);
    if (n->isA(UndefinedType))
      pos = len;
    else
      pos = n->intVal();
    d = s->stringVal().rfind(s2->stringVal(), pos);
    result = newNumber(d);
    break;
  case Substr:
    n = toInteger(a0);
    m = toInteger(a1);
    if (n->doubleVal() >= 0)
      d = n->doubleVal();
    else
      d = max(len + n->doubleVal(), 0);
    if (a1->isA(UndefinedType))
      d2 = len - d;
    else
      d2 = min(max(m->doubleVal(), 0), len - d);
    result = newString(s->stringVal().substr((int)d, (int)d2));
    break;
  case Substring:
    n = toInteger(a0);
    m = toInteger(a1);
    d = min(max(n->doubleVal(), 0), len);
    if (a1->isA(UndefinedType))
      d2 = len - d;
    else {
      d2 = min(max(m->doubleVal(), 0), len);
      d2 = max(d2-d, 0);
    }
    result = newString(s->stringVal().substr((int)d, (int)d2));
    break;
  }

  return newCompletion(Normal, result);
}
