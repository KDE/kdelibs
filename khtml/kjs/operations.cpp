/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include <iostream.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "global.h"
#include "operations.h"

using namespace KJS;

// ECMA 9.1
KJSO *KJS::toPrimitive(KJSO *obj, Type preferred)
{
  if (!obj->isA(Object)) {
    return obj->ref();
  }

  return obj->defaultValue();
  /* TODO: is there still any need to throw a runtime error _here_ ? */
}

// ECMA 9.2
KJSO *KJS::toBoolean(KJSO *obj)
{
  bool b;

  switch (obj->type())
    {
    case Undefined:
    case Null:
    case Error:
      b = false;
      break;
    case Boolean:
      return obj->ref();
    case Number:
      b = !((obj->dVal() == 0) /* || (obj->iVal() == N0) */ ||
	(obj->dVal() == NaN));
      break;
    case String:
      b = (obj->sVal().length() > 0);
      break;
    case Object:
      b = true;
      break;
    default:
      b = false;
      assert(!"unhandled object type in switch");
    }

  return new KJSBoolean(b);
}

// ECMA 9.3
KJSO *KJS::toNumber(KJSO *obj)
{
  double d = 0;
  Ptr tmp, res;

  switch (obj->type())
    {
    case Undefined:
    case Error:
      d = NaN;
      break;
    case Null:
      d = 0;
      break;
    case Boolean:
      d = obj->bVal() ? 1 : 0;
      break;
    case Number:
      return obj->ref();
    case String:
      /* TODO */
      break;
    case Object:
      tmp = toPrimitive(obj, Number);
      res = toNumber(tmp);
      return res->ref();
    default:
      cerr << "type = " << obj->type() << endl;
      assert(!"toNumber: unhandled switch case");
      exit(1);
    }

  return new KJSNumber(d);
}

// helper function for toInteger, toInt32, toUInt32 and toUInt16
double KJS::round(KJSO *obj)
{
  Ptr n = toNumber(obj);
  if (n->dVal() == 0.0)   /* TODO: -0, NaN, Inf */
    return 0.0;
  double d = floor(fabs(n->dVal()));
  if (n->dVal() < 0)
    d *= -1;

  return d;
}

// ECMA 9.4
KJSO *KJS::toInteger(KJSO *obj)
{
  return new KJSNumber(round(obj));
}

// ECMA 9.5
int KJS::toInt32(KJSO *obj)
{
  double d = round(obj);
  double d32 = fmod(d, D32);

  if (d32 >= D16)
    d32 -= D32;

  return static_cast<int>(d32);
}

// ECMA 9.6
unsigned int KJS::toUInt32(KJSO *obj)
{
  double d = round(obj);
  double d32 = fmod(d, D32);

  return static_cast<unsigned int>(d32);
}

// ECMA 9.7
unsigned short KJS::toUInt16(KJSO *obj)
{
  double d = round(obj);
  double d16 = fmod(d, D16);

  return static_cast<unsigned short>(d16);
}

// ECMA 9.8
KJSO *KJS::toString(KJSO *obj)
{
  const char *c;
  double d;
  char buffer[40];
  Ptr res, tmp;

  switch (obj->type())
    {
    case Undefined:
    case Error:
      c = "undefined";
      break;
    case Null:
      c = "null";
      break;
    case Boolean:
      c = obj->bVal() ? "true" : "false";
      break;
    case Number:
      d = obj->dVal();
      // truncate decimal digits on round values
      if (d != (double)((int)d))
	sprintf(buffer, "%f", d);
      else
	sprintf(buffer, "%d", (int)d);
      c = buffer;
      break;
    case String:
      return obj->ref();
    case Object:
      tmp = toPrimitive(obj, String);
      res = toString(tmp);
      return res->ref();
    default:
      assert(!"toString: unhandled switch case");
      exit(1);
    }
  res = new KJSString(c);
  return res->ref();
}

// ECMA 9.9
KJSO *KJS::toObject(KJSO *obj)
{
  KJSO *o = 0L;

  switch (obj->type())
    {
    case Undefined:
    case Error:
      return new KJSError(ErrUndefToObject, o);
    case Null:
      return new KJSError(ErrNullToObject, o);
    case Boolean:
      o = new KJSBoolean(obj->bVal());
      break;
    case Number:
      o = new KJSNumber(obj->dVal());
      break;
    case String:
      o = new KJSString(obj->sVal().ascii());
      break;
    case Object:
    case DeclaredFunction:
    case InternalFunction:
    case AnonymousFunction:
      return obj->ref();
    default:
      assert(!"toObject: unhandled switch case");
      exit(1);
    }

  return o;
}


// ECMA 11.9.3
bool KJS::equal(KJSO *v1, KJSO *v2)
{
  Type t1 = v1->type();
  Type t2 = v2->type();

  if (t1 == t2) {
    if (t1 == Undefined || t1 == Null)
      return true;
    if (t1 == Number)
      return (v1->dVal() == v2->dVal()); /* TODO: NaN, -0 ? */
    if (t1 == String)
      return (v1->sVal() == v2->sVal());
    if (t1 == Boolean)
      return (v1->bVal() == v2->bVal());
    return (v1 == v2);
  }

  // different types
  if ((t1 == Null && t2 == Undefined) || (t1 == Undefined && t2 == Null))
    return true;
  if (t1 == Number && t2 == String) {
    Ptr n2 = toNumber(v2);
    return equal(v1, n2);
  }
  if ((t1 == String && t2 == Number) || t1 == Boolean) {
    Ptr n1 = toNumber(v1);
    return equal(n1, v2);
  }
  if (t2 == Boolean) {
    Ptr n2 = toNumber(v2);
    return equal(v1, n2);
  }
  if ((t1 == String || t1 == Number) && t2 == Object) {
    Ptr p2 = toPrimitive(v2);
    return equal(v1, p2);
  }
  if (t1 == Object && (t2 == String || t2 == Number)) {
    Ptr p1 = toPrimitive(v1);
    return equal(p1, v2);
  }

  return false;
}
