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
#include "types.h"
#include "string_object.h"

using namespace KJS;

KJSO StringObject::get(const UString &p) const
{
  if (p == "fromCharCode")
    return Function(new StringObjectFunc());
  else
    return Imp::get(p);
}

// ECMA 15.5.1
Completion StringObject::execute(const List &args)
{
  KJSO v;
  String s;

  if (args.isEmpty())
    s = String("");
  else {
    v = args[0];
    s = v.toString();
  }

  return Completion(Normal, s);
}

// ECMA 15.5.2
Object StringObject::construct(const List &args)
{
  String s;
  if (args.size() > 0)
    s = args.begin()->toString();
  else
    s = String("");

  return Object::create(StringClass, s);
}

// ECMA 15.5.3.2 fromCharCode()
Completion StringObjectFunc::execute(const List &args)
{
  UString s;
  if (args.size()) {
    UChar *buf = new UChar(args.size());
    UChar *p = buf;
    ListIterator it = args.begin();
    while (it != args.end()) {
      unsigned short u = it->toUInt16();
      *p++ = UChar(u);
      it++;
    }
    s = UString(buf, args.size(), false);
  } else
    s = "";
  
  return Completion(Normal, String(s));
}

// ECMA 15.5.4
StringPrototype::StringPrototype(const Object& proto)
  : ObjectImp(StringClass, String(""), proto)
{
  // The constructor will be added later in StringObject's constructor
}

KJSO StringPrototype::get(const UString &p) const
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
    return Imp::get(p);

  return Function(new StringProtoFunc(id));
}

StringProtoFunc::StringProtoFunc(int i)
  : id(i)
{
  setPrototype(Global::current().functionPrototype());
}

// ECMA 15.5.4.2 - 15.5.4.20
Completion StringProtoFunc::execute(const List &args)
{
  KJSO result;

  Object thisObj = Object::dynamicCast(thisValue());

  // toString and valueOf are no generic function.
  if (id == ToString || id == ValueOf) {
    if (thisObj.isNull() || thisObj.getClass() != StringClass) {
      result = Error::create(TypeError);
      return Completion(ReturnValue, result);
    }
  }

  String s2;
  Number n, m;
  UString u;
  int pos;
  double d, d2;
  KJSO v = thisObj.internalValue();
  String s = v.toString();
  int len = (int) s.value().size();
  KJSO a0 = args[0];
  KJSO a1 = args[1];

  switch (id) {
  case ToString:
  case ValueOf:
    result = v.toString();
    break;
  case CharAt:
    n = a0.toInteger();
    pos = (int) n.value();
    if (pos < 0 || pos >= len)
      u = "";
    else
      u = s.value().substr(pos, 1);
    result = String(u);
    break;
  case CharCodeAt:
    n = a0.toInteger();
    pos = (int) n.value();
    if (pos < 0 || pos >= len)
      d = NaN;
    else {
      UChar c = s.value()[pos];
      d = (c.hi << 8) + c.lo;
    }
    result = Number(d);
    break;
  case IndexOf:
    s2 = a0.toString();
    n = a1.toInteger();
    if (n.isA(UndefinedType))
      pos = 0;
    else
      pos = n.intValue();
    d = s.value().find(s2.value(), pos);
    result = Number(d);
    break;
  case LastIndexOf:
    s2 = a0.toString();
    n = a1.toInteger();
    if (n.isA(UndefinedType))
      pos = len;
    else
      pos = n.intValue();
    d = s.value().rfind(s2.value(), pos);
    result = Number(d);
    break;
  case Substr:
    n = a0.toInteger();
    m = a1.toInteger();
    if (n.value() >= 0)
      d = n.value();
    else
      d = max(len + n.value(), 0);
    if (a1.isA(UndefinedType))
      d2 = len - d;
    else
      d2 = min(max(m.value(), 0), len - d);
    result = String(s.value().substr((int)d, (int)d2));
    break;
  case Substring:
    n = a0.toInteger();
    m = a1.toInteger();
    d = min(max(n.value(), 0), len);
    if (a1.isA(UndefinedType))
      d2 = len - d;
    else {
      d2 = min(max(m.value(), 0), len);
      d2 = max(d2-d, 0);
    }
    result = String(s.value().substr((int)d, (int)d2));
    break;
  }

  return Completion(Normal, result);
}
