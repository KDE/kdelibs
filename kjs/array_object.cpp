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

  /* TODO: put() properties */
}

#if 0
ArrayProtoFunc::ArrayProtoFunc(int i, const Global& global)
  : id(i)
{
  setPrototype(global->funcProto);
}

// ECMA 15.4.4
Completion ArrayProtoFunc::execute(const List &args)
{
  /* TODO */
}
#endif
