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

#include <math.h>
#include <stdlib.h>

#include "kjs.h"
#include "types.h"
#include "operations.h"
#include "math_object.h"
#include "lookup.h"
#include "object.h"

using namespace KJS;

const TypeInfo Math::info = { "Math", ObjectType,
			      &ObjectImp::info, 0, 0 };

Math::Math(const Object &objProto) : ObjectImp(ObjectClass)
{
  // ECMA 15.8
  setPrototype(objProto);

  put("E",       Number(exp(1.0)),             DontEnum|DontDelete|ReadOnly);
  put("LN10",    Number(log(10.0)),            DontEnum|DontDelete|ReadOnly);
  put("LN2",     Number(log(2.0)),             DontEnum|DontDelete|ReadOnly);
  put("LOG2E",   Number(1.0/log(2.0)),         DontEnum|DontDelete|ReadOnly);
  put("LOG10E",  Number(1.0/log(10.0)),        DontEnum|DontDelete|ReadOnly);
  put("PI",      Number(2.0 * asin(1.0)),      DontEnum|DontDelete|ReadOnly);
  put("SQRT1_2", Number(sqrt(0.5)),            DontEnum|DontDelete|ReadOnly);
  put("SQRT2",   Number(sqrt(2.0)),            DontEnum|DontDelete|ReadOnly);

  put("abs",     new MathFunc(Math::Abs,1),    DontEnum|DontDelete|ReadOnly);
  put("acos",    new MathFunc(Math::ACos,1),   DontEnum|DontDelete|ReadOnly);
  put("asin",    new MathFunc(Math::ASin,1),   DontEnum|DontDelete|ReadOnly);
  put("atan",    new MathFunc(Math::ATan,1),   DontEnum|DontDelete|ReadOnly);
  put("atan2",   new MathFunc(Math::ATan2,2),  DontEnum|DontDelete|ReadOnly);
  put("ceil",    new MathFunc(Math::Ceil,1),   DontEnum|DontDelete|ReadOnly);
  put("cos",     new MathFunc(Math::Cos,1),    DontEnum|DontDelete|ReadOnly);
  put("exp",     new MathFunc(Math::Exp,1),    DontEnum|DontDelete|ReadOnly);
  put("floor",   new MathFunc(Math::Floor,1),  DontEnum|DontDelete|ReadOnly);
  put("log",     new MathFunc(Math::Log,1),    DontEnum|DontDelete|ReadOnly);
  put("max",     new MathFunc(Math::Max,2),    DontEnum|DontDelete|ReadOnly);
  put("min",     new MathFunc(Math::Min,2),    DontEnum|DontDelete|ReadOnly);
  put("pow",     new MathFunc(Math::Pow,2),    DontEnum|DontDelete|ReadOnly);
  put("random",  new MathFunc(Math::Random,0), DontEnum|DontDelete|ReadOnly);
  put("round",   new MathFunc(Math::Round,1),  DontEnum|DontDelete|ReadOnly);
  put("sin",     new MathFunc(Math::Sin,1),    DontEnum|DontDelete|ReadOnly);
  put("sqrt",    new MathFunc(Math::Sqrt,1),   DontEnum|DontDelete|ReadOnly);
  put("tan",     new MathFunc(Math::Tan,1),    DontEnum|DontDelete|ReadOnly);
}

Completion MathFunc::execute(const List &args)
{
  KJSO v = args[0];
  Number n = v.toNumber();
  double arg = n.value();

  KJSO v2 = args[1];
  Number n2 = v2.toNumber();
  double arg2 = n2.value();
  double result;

  switch (id) {
  case Math::Abs:
    result = ( arg < 0 ) ? (-arg) : arg;
    break;
  case Math::ACos:
    result = ::acos(arg);
    break;
  case Math::ASin:
    result = ::asin(arg);
    break;
  case Math::ATan:
    result = ::atan(arg);
    break;
  case Math::ATan2:
    result = ::atan2(arg, arg2);
    break;
  case Math::Ceil:
    result = ::ceil(arg);
    break;
  case Math::Cos:
    result = ::cos(arg);
    break;
  case Math::Exp:
    result = ::exp(arg);
    break;
  case Math::Floor:
    result = ::floor(arg);
    break;
  case Math::Log:
    result = ::log(arg);
    break;
  case Math::Max: // ### variable args
    result = ( arg > arg2 ) ? arg : arg2;
    break;
  case Math::Min: // ### variable args
    result = ( arg < arg2 ) ? arg : arg2;
    break;
  case Math::Pow:
    // ECMA 15.8.2.1.13 (::pow takes care of most of the critera)
    if (KJS::isNaN(arg2))
      result = NaN;
    else if (arg2 == 0)
      result = 1;
    else if (KJS::isNaN(arg) && arg2 != 0)
      result = NaN;
    else if (::fabs(arg) > 1 && KJS::isPosInf(arg2))
      result = Inf;
    else if (::fabs(arg) > 1 && KJS::isNegInf(arg2))
      result = +0;
    else if (::fabs(arg) == 1 && KJS::isPosInf(arg2))
      result = NaN;
    else if (::fabs(arg) == 1 && KJS::isNegInf(arg2))
      result = NaN;
    else if (::fabs(arg) < 1 && KJS::isPosInf(arg2))
      result = +0;
    else if (::fabs(arg) < 1 && KJS::isNegInf(arg2))
      result = Inf;
    else
      result = ::pow(arg, arg2);
    break;
  case Math::Random:
    result = ::rand();
    result = result / RAND_MAX;
    break;
  case Math::Round:
    if (isNaN(arg))
      result = arg;
    if (isInf(arg) || isInf(-arg))
      result = arg;
    else if (arg == -0.5)
      result = 0;
    else
      result = (double)(arg >= 0.0 ? int(arg + 0.5) : int(arg - 0.5));
    break;
  case Math::Sin:
    result = ::sin(arg);
    break;
  case Math::Sqrt:
    result = ::sqrt(arg);
    break;
  case Math::Tan:
    result = ::tan(arg);
    break;

  default:
    result = 0.0;
    assert(0);
  }

  return Completion(ReturnValue, Number(result));
}
