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
#include "operations.h"
#include "math_object.h"

using namespace KJS;

KJSO *Math::get(const UString &p)
{
  /* TODO: temporarily removed until an experiment is finnished ;) */
  return KJSO::get(p);
}

KJSO* MathFunc::execute(Context *context)
{
  Ptr v = context->arg(0);
  Ptr n = toNumber(v);
  double arg = n->doubleVal();

  Ptr v2 = context->arg(1);
  Ptr n2 = toNumber(v2);
  double arg2 = n2->doubleVal();
  double result;
  int temp;

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
  case Math::Max:
    result = ( arg > arg2 ) ? arg : arg2;
    break;
  case Math::Min:
    result = ( arg < arg2 ) ? arg : arg2;
    break;
  case Math::Random:
    result = ::rand();
    result = result / RAND_MAX;
    break;
  case Math::Round:
    temp =  (int) (arg + 0.5);
    result = temp;
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
    assert((result = 0));
  }

  return newCompletion(Normal, (zeroRef(newNumber(result))));
}
