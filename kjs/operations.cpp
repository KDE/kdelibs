/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
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

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "object.h"
#include "operations.h"

namespace KJS {
  const double D16 = 65536.0;
  const double D31 = 2147483648.0;
  const double D32 = 4294967296.0;
};

using namespace KJS;


// ECMA 9.1
KJSO *KJS::toPrimitive(KJSO *obj, Type preferred)
{
  if (!obj->isObject()) {
    return obj->ref();
  }

  return obj->defaultValue(preferred);
  /* TODO: is there still any need to throw a runtime error _here_ ? */
}

// ECMA 9.2
KJSO *KJS::toBoolean(KJSO *obj)
{
  bool b;

  switch (obj->type())
    {
    case UndefinedType:
    case NullType:
      b = false;
      break;
    case BooleanType:
      return obj->ref();
    case NumberType:
      b = !((obj->doubleVal() == 0) /* || (obj->iVal() == N0) */ ||
	(obj->doubleVal() == NaN));
      break;
    case StringType:
      b = (obj->stringVal().size() > 0);
      break;
    case ObjectType:
      b = true;
      break;
    default:
      b = false;
      assert(!"unhandled object type in switch");
    }

  return KJSO::newBoolean(b);
}

// ECMA 9.3
KJSO *KJS::toNumber(KJSO *obj)
{
  double d = 0;
  Ptr tmp, res;

  switch (obj->type())
    {
    case UndefinedType:
      d = NaN;
      break;
    case NullType:
      d = 0;
      break;
    case BooleanType:
      d = obj->boolVal() ? 1 : 0;
      break;
    case NumberType:
      return obj->ref();
    case StringType:
      d = obj->stringVal().toDouble();
      break;
    case ObjectType:
      tmp = toPrimitive(obj, NumberType);
      res = toNumber(tmp);
      return res->ref();
    default:
      assert(!"toNumber: unhandled switch case");
      exit(1);
    }

  return KJSO::newNumber(d);
}

// helper function for toInteger, toInt32, toUInt32 and toUInt16
double KJS::round(KJSO *obj)
{
  Ptr n = toNumber(obj);
  if (n->doubleVal() == 0.0)   /* TODO: -0, NaN, Inf */
    return 0.0;
  double d = floor(fabs(n->doubleVal()));
  if (n->doubleVal() < 0)
    d *= -1;

  return d;
}

// ECMA 9.4
KJSO *KJS::toInteger(KJSO *obj)
{
  return KJSO::newNumber(round(obj));
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
  UString c;
  double d;
  Ptr res, tmp;

  switch (obj->type())
    {
    case UndefinedType:
      c = "undefined";
      break;
    case NullType:
      c = "null";
      break;
    case BooleanType:
      c = obj->boolVal() ? "true" : "false";
      break;
    case NumberType:
      d = obj->doubleVal();
      c = UString::from(d);
      break;
    case StringType:
      return obj->ref();
    case ObjectType:
    case HostType:
    case ConstructorType:
    case InternalFunctionType:
      if (obj->hasProperty("toString")) {
	tmp = obj->get("toString");
	// TODO: check for implementsCall() ?
	res = tmp->executeCall(obj, 0L);
      } else {
	tmp = toPrimitive(obj, StringType);
	res = toString(tmp);
      }
      return res->ref();
    default:
      assert(!"toString: unhandled switch case");
      exit(1);
    }
  res = KJSO::newString(c);
  return res->ref();
}

// ECMA 9.9
KJSO *KJS::toObject(KJSO *obj)
{
  KJSO *o = 0L;

  switch (obj->type())
    {
    case UndefinedType:
      return KJSO::newError(TypeError);
    case NullType:
      return KJSO::newError(TypeError);
    case BooleanType:
      o = Object::create(BooleanClass,
			    zeroRef(KJSO::newBoolean(obj->boolVal())));
      break;
    case NumberType:
      o = Object::create(NumberClass,
			    zeroRef(KJSO::newNumber(obj->doubleVal())));
      break;
    case StringType:
      o = Object::create(StringClass,
			    zeroRef(KJSO::newString(obj->stringVal())));
      break;
    case ObjectType:
    case HostType:
    case DeclaredFunctionType:
    case InternalFunctionType:
    case AnonymousFunctionType:
    case ConstructorType:
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
    if (t1 == UndefinedType || t1 == NullType)
      return true;
    if (t1 == NumberType)
      return (v1->doubleVal() == v2->doubleVal()); /* TODO: NaN, -0 ? */
    if (t1 == StringType)
      return (v1->stringVal() == v2->stringVal());
    if (t1 == BooleanType)
      return (v1->boolVal() == v2->boolVal());
    return (v1 == v2);
  }

  // different types
  if ((t1 == NullType && t2 == UndefinedType) || (t1 == UndefinedType && t2 == NullType))
    return true;
  if (t1 == NumberType && t2 == StringType) {
    Ptr n2 = toNumber(v2);
    return equal(v1, n2);
  }
  if ((t1 == StringType && t2 == NumberType) || t1 == BooleanType) {
    Ptr n1 = toNumber(v1);
    return equal(n1, v2);
  }
  if (t2 == BooleanType) {
    Ptr n2 = toNumber(v2);
    return equal(v1, n2);
  }
  if ((t1 == StringType || t1 == NumberType) && t2 == ObjectType) {
    Ptr p2 = toPrimitive(v2);
    return equal(v1, p2);
  }
  if (t1 == ObjectType && (t2 == StringType || t2 == NumberType)) {
    Ptr p1 = toPrimitive(v1);
    return equal(p1, v2);
  }

  return false;
}

double KJS::max(double d1, double d2)
{
  /* TODO: check for NaN */
  return (d1 > d2) ? d1 : d2;
}

double KJS::min(double d1, double d2)
{
  /* TODO: check for NaN */
  return (d1 < d2) ? d1 : d2;
}
