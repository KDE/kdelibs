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
#include "error_object.h"

using namespace KJS;

const char *ErrorObject::errName[] = {
  "No Error",
  "Error",
  "EvalError",
  "RangeError",
  "ReferenceError",
  "SyntaxError",
  "TypeError",
  "URIError"
};

ErrorObject::ErrorObject(Object *proto, ErrorType t)
  : Constructor(proto, 1), errType(t)
{
  const char *n = errName[errType];

  put("name", zeroRef(newString(n)));
  put("message", zeroRef(newString("bla bla")));
}
      
// ECMA 15.9.2
KJSO* ErrorObject::execute(const List &args)
{
  // "Error()" gives the sames result as "new Error()"
  return construct(args);
}

// ECMA 15.9.3
Object* ErrorObject::construct(const List &args)
{
  if (args.isEmpty() == 1 || args[0]->isA(UndefinedType))
    return Object::create(ErrorClass, zeroRef(newUndefined()));

  Ptr message = toString(args[0]);
  return Object::create(ErrorClass, message);
}

// ECMA 15.9.4
ErrorPrototype::ErrorPrototype(Object *proto)
  : Object(ErrorClass, zeroRef(newUndefined()), proto)
{
  // The constructor will be added later in ErrorObject's constructor
}

KJSO *ErrorPrototype::get(const UString &p)
{
  const char *s;

  /* TODO: are these properties dynamic, i.e. should we put() them ? */
  if (p == "name")
    s = "Error";
  else if (p == "message")
    s = "Error message.";
  else if (p == "toString")
    return new ErrorProtoFunc();
  else
    return KJSO::get(p);

  return newString(s);
}

KJSO *ErrorProtoFunc::execute(const List &)
{
  // toString()
  const char *s = "Error message.";
  
  return newCompletion(Normal, zeroRef(newString(s)));
}

