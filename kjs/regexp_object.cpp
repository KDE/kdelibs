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

#include <stdio.h>

#include "kjs.h"
#include "operations.h"
#include "regexp.h"
#include "regexp_object.h"

using namespace KJS;

// ECMA 15.9.2
KJSO* RegExpObject::execute(const List &)
{
  return newCompletion(Normal, zeroRef(newUndefined()));
}

// ECMA 15.9.3
Object* RegExpObject::construct(const List &args)
{
  /* TODO: regexp arguments */
  Ptr p = toString(args[0]);
  Ptr f = toString(args[1]);
  UString flags = f->stringVal();

  Object *obj = Object::create(RegExpClass);

  bool global = (flags.find("g") >= 0);
  bool ignoreCase = (flags.find("i") >= 0);
  bool multiline = (flags.find("m") >= 0);
  /* TODO: throw an error on invalid flags */

  obj->put("global", zeroRef(newBoolean(global)));
  obj->put("ignoreCase", zeroRef(newBoolean(ignoreCase)));
  obj->put("multiline", zeroRef(newBoolean(multiline)));

  obj->put("source", zeroRef(newString(p->stringVal())));
  obj->put("lastIndex", 0, DontDelete | DontEnum);

  obj->setRegExp(new RegExp(p->stringVal() /* TODO flags */));

  return obj;
}

// ECMA 15.9.4
RegExpPrototype::RegExpPrototype(Object *proto)
  : Object(RegExpClass, zeroRef(newString("")), proto)
{
  // The constructor will be added later in RegExpObject's constructor
}

KJSO *RegExpPrototype::get(const UString &p)
{
  int id = -1;
  if (p == "exec")
    id = RegExpProtoFunc::Exec;
  else if (p == "test")
    id = RegExpProtoFunc::Test;
  else if (p == "toString")
    id = RegExpProtoFunc::ToString;

  if (id >= 0)
    return new RegExpProtoFunc(id);
  else
    return KJSO::get(p);
}

KJSO* RegExpProtoFunc::execute(const List &args)
{
  Ptr result;

  if (!thisValue()->isClass(RegExpClass)) {
    result = newError(TypeError);
    return newCompletion(ReturnValue, result);
  }

  Object *thisObj = static_cast<Object*>(thisValue());

  Ptr s, lastIndex, tmp;
  UString str;
  int length, i;
  switch (id) {
  case Exec:
  case Test:
    s = toString(args[0]);
    length = s->stringVal().size();
    lastIndex = thisObj->get("lastIndex");
    i = toInt32(lastIndex);
    tmp = thisObj->get("global");
    if (tmp->boolVal() == false)
      i = 0;
    if (i < 0 || i > length) {
      thisObj->put("lastIndex", 0);
      result = newNull();
      break;
    }
    str = thisObj->regExp()->match(s->stringVal(), i);
    if (id == Test) {
      result = newBoolean(!(str == ""));
      break;
    }
    /* TODO complete */
    result = newString(str);
    break;
  case ToString:
    s = thisObj->get("source");
    str = "/";
    str += s->stringVal();
    str += "/";
    result = newString(str);
    break;
  }

  return newCompletion(Normal, result);
}
