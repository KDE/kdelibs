// -*- c-basic-offset: 2 -*-
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
 *
 *  $Id$
 */

#include <math.h>
#include <stdlib.h>
#include <assert.h>

#include "value.h"
#include "object.h"
#include "types.h"
#include "interpreter.h"
#include "operations.h"
#include "math_object.h"

using namespace KJS;

// ------------------------------ MathObjectImp --------------------------------

MathObjectImp::MathObjectImp(ExecState *exec,
                             ObjectPrototypeImp *objProto,
                             FunctionPrototypeImp *funcProto)
  : ObjectImp(objProto)
{
  Value protect(this);
  // ECMA 15.8

  put(exec,"E",       Number(exp(1.0)),             DontEnum|DontDelete|ReadOnly);
  put(exec,"LN10",    Number(log(10.0)),            DontEnum|DontDelete|ReadOnly);
  put(exec,"LN2",     Number(log(2.0)),             DontEnum|DontDelete|ReadOnly);
  put(exec,"LOG2E",   Number(1.0/log(2.0)),         DontEnum|DontDelete|ReadOnly);
  put(exec,"LOG10E",  Number(1.0/log(10.0)),        DontEnum|DontDelete|ReadOnly);
  put(exec,"PI",      Number(2.0 * asin(1.0)),      DontEnum|DontDelete|ReadOnly);
  put(exec,"SQRT1_2", Number(sqrt(0.5)),            DontEnum|DontDelete|ReadOnly);
  put(exec,"SQRT2",   Number(sqrt(2.0)),            DontEnum|DontDelete|ReadOnly);

  put(exec,"abs",     new MathFuncImp(exec,funcProto,MathFuncImp::Abs,1),    DontEnum|DontDelete|ReadOnly);
  put(exec,"acos",    new MathFuncImp(exec,funcProto,MathFuncImp::ACos,1),   DontEnum|DontDelete|ReadOnly);
  put(exec,"asin",    new MathFuncImp(exec,funcProto,MathFuncImp::ASin,1),   DontEnum|DontDelete|ReadOnly);
  put(exec,"atan",    new MathFuncImp(exec,funcProto,MathFuncImp::ATan,1),   DontEnum|DontDelete|ReadOnly);
  put(exec,"atan2",   new MathFuncImp(exec,funcProto,MathFuncImp::ATan2,2),  DontEnum|DontDelete|ReadOnly);
  put(exec,"ceil",    new MathFuncImp(exec,funcProto,MathFuncImp::Ceil,1),   DontEnum|DontDelete|ReadOnly);
  put(exec,"cos",     new MathFuncImp(exec,funcProto,MathFuncImp::Cos,1),    DontEnum|DontDelete|ReadOnly);
  put(exec,"exp",     new MathFuncImp(exec,funcProto,MathFuncImp::Exp,1),    DontEnum|DontDelete|ReadOnly);
  put(exec,"floor",   new MathFuncImp(exec,funcProto,MathFuncImp::Floor,1),  DontEnum|DontDelete|ReadOnly);
  put(exec,"log",     new MathFuncImp(exec,funcProto,MathFuncImp::Log,1),    DontEnum|DontDelete|ReadOnly);
  put(exec,"max",     new MathFuncImp(exec,funcProto,MathFuncImp::Max,2),    DontEnum|DontDelete|ReadOnly);
  put(exec,"min",     new MathFuncImp(exec,funcProto,MathFuncImp::Min,2),    DontEnum|DontDelete|ReadOnly);
  put(exec,"pow",     new MathFuncImp(exec,funcProto,MathFuncImp::Pow,2),    DontEnum|DontDelete|ReadOnly);
  put(exec,"random",  new MathFuncImp(exec,funcProto,MathFuncImp::Random,0), DontEnum|DontDelete|ReadOnly);
  put(exec,"round",   new MathFuncImp(exec,funcProto,MathFuncImp::Round,1),  DontEnum|DontDelete|ReadOnly);
  put(exec,"sin",     new MathFuncImp(exec,funcProto,MathFuncImp::Sin,1),    DontEnum|DontDelete|ReadOnly);
  put(exec,"sqrt",    new MathFuncImp(exec,funcProto,MathFuncImp::Sqrt,1),   DontEnum|DontDelete|ReadOnly);
  put(exec,"tan",     new MathFuncImp(exec,funcProto,MathFuncImp::Tan,1),    DontEnum|DontDelete|ReadOnly);
}

// ------------------------------ MathObjectImp --------------------------------

MathFuncImp::MathFuncImp(ExecState *exec, FunctionPrototypeImp *funcProto, int i, int l)
  : InternalFunctionImp(funcProto), id(i)
{
  Value protect(this);
  put(exec,"length",Number(l),DontDelete|ReadOnly|DontEnum);
}

bool MathFuncImp::implementsCall() const
{
  return true;
}

Value MathFuncImp::call(ExecState *exec, Object &/*thisObj*/, const List &args)
{
  Value v = args[0];
  Number n = v.toNumber(exec);
  double arg = n.value();

  Value v2 = args[1];
  Number n2 = v2.toNumber(exec);
  double arg2 = n2.value();
  double result;

  switch (id) {
  case Abs:
    result = ( arg < 0 ) ? (-arg) : arg;
    break;
  case ACos:
    result = ::acos(arg);
    break;
  case ASin:
    result = ::asin(arg);
    break;
  case ATan:
    result = ::atan(arg);
    break;
  case ATan2:
    result = ::atan2(arg, arg2);
    break;
  case Ceil:
    result = ::ceil(arg);
    break;
  case Cos:
    result = ::cos(arg);
    break;
  case Exp:
    result = ::exp(arg);
    break;
  case Floor:
    result = ::floor(arg);
    break;
  case Log:
    result = ::log(arg);
    break;
  case Max: // TODO: support variable args
    result = ( arg > arg2 ) ? arg : arg2;
    break;
  case Min: // TODO: support variable args
    result = ( arg < arg2 ) ? arg : arg2;
    break;
  case Pow:
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
  case Random:
    result = ::rand();
    result = result / RAND_MAX;
    break;
  case Round:
    if (isNaN(arg))
      result = arg;
    if (isInf(arg) || isInf(-arg))
      result = arg;
    else if (arg == -0.5)
      result = 0;
    else
      result = (double)(arg >= 0.0 ? int(arg + 0.5) : int(arg - 0.5));
    break;
  case Sin:
    result = ::sin(arg);
    break;
  case Sqrt:
    result = ::sqrt(arg);
    break;
  case Tan:
    result = ::tan(arg);
    break;

  default:
    result = 0.0;
    assert(0);
  }

  return Number(result);
}
