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
#include "array_object.h"

using namespace KJS;

// ECMA 15.6.1
Completion ArrayObject::execute(const List &args)
{
  // equivalent to 'new Array(....)'
  KJSO result = construct(args);

  return Completion(Normal, result);
}

// ECMA 15.6.2
Object ArrayObject::construct(const List &args)
{
  Object result = Object::create(ArrayClass);

  unsigned int len;
  ListIterator it = args.begin();
  // a single argument might denote the array size
  if (args.size() == 1 && it->isA(NumberType))
    len = it->toUInt32();
  else {
    // initialize array
    len = args.size();
    for (unsigned int u = 0; it != args.end(); it++, u++)
      result.put(UString::from(u), *it);
  }

  // array size
  result.put("length", len, DontEnum | DontDelete);

  return result;
}

// ECMA 15.6.4
ArrayPrototype::ArrayPrototype(const Object& proto)
  : ObjectImp(ArrayClass, Null(), proto)
{
  // The constructor will be added later in ArrayObject's constructor

  put("length", 0u);
}

KJSO ArrayPrototype::get(const UString &p) const
{
  int id; 
  if(p == "toString")
    id = ArrayProtoFunc::ToString;
  else if(p == "toLocaleString")
    id = ArrayProtoFunc::ToLocaleString;
  else if(p == "concat")
    id = ArrayProtoFunc::Concat;
  else if (p == "join")
    id = ArrayProtoFunc::Join;
  else if(p == "pop")
    id = ArrayProtoFunc::Pop;
  else if(p == "push")
    id = ArrayProtoFunc::Push;
  else if(p == "reverse")
    id = ArrayProtoFunc::Reverse;
  else if(p == "shift")
    id = ArrayProtoFunc::Shift;
  else if(p == "slice")
    id = ArrayProtoFunc::Slice;
  else if(p == "sort")
    id = ArrayProtoFunc::Sort;
  else if(p == "splice")
    id = ArrayProtoFunc::Splice;
  else if(p == "unshift")
    id = ArrayProtoFunc::UnShift;
  else 
    return Imp::get(p);

  return Function(new ArrayProtoFunc(id));
}

// ECMA 15.4.4
Completion ArrayProtoFunc::execute(const List &args)
{
  KJSO result, obj;
  Object thisObj = Object::dynamicCast(thisValue());
  unsigned int length = thisObj.get("length").toUInt32();
  UString str = "";
  UString seperator = ",";

  switch (id) {
  case ToLocaleString:
    /* TODO */
    // fall trough
  case ToString:
    if (!thisObj.getClass() == ArrayClass) {
      result = Error::create(TypeError);
      break;
    }
    // fall trough
  case Join:
    if (!args[0].isA(UndefinedType))
      seperator = args[0].toString().value();
    for (unsigned int k = 0; k < length; k++) {
      if (k >= 1)
	str += seperator;
      obj = thisObj.get(UString::from(k));
      if (!obj.isA(UndefinedType) && !obj.isA(NullType))
	str += obj.toString().value();
    }
    result = String(str);
    break;
  case Pop:
    if (length == 0) {
      thisObj.put("length", Number(length));
      result = Undefined();
    } else {
      str = UString::from(length - 1);
      result = thisObj.get(str);
      thisObj.deleteProperty(str);
      thisObj.put("length", length - 1);
    }
    break;
  case Push:
    for (int n = 0; n < args.size(); n++)
      thisObj.put(UString::from(length + n), args[n]);
    length += args.size();
    thisObj.put("length", length);
    result = Number(length);
    break;
    /* TODO */
  default:
    result = Undefined();
  }

  return Completion(Normal,result);
}
