/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
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

#define KJSARG(x) KJSWorld::context->activation->get((x))
#define KJSRETURN(x) return new KJSCompletion(Normal,(x))

using namespace KJS;

KJSMath::KJSMath()
{
  const int attr = DontEnum | DontDelete | ReadOnly;
#ifdef M_E
  put("E", (double)M_E, attr);
#else
  put("E", exp(1), attr);
#endif

#ifdef M_LN2
  put("LN2", (double)M_LN2, attr);
#else
  put("LN2", log(2), attr);
#endif

#ifdef M_LN10
  put("LN10", (double)M_LN10, attr);
#else
  put("LN10", log(10), attr);
#endif

#ifdef M_LOG2E
  put("LOG2E", (double)M_LOG2E, attr);
#else
  put("LOG2E", 1.0/log(2), attr);
#endif

#ifdef M_LOG10E
  put("LOG10E", (double)M_LOG10E, attr);
#else
  put("LOG10E", 1.0/log(10), attr);
#endif

#ifdef M_PI
  put("PI", (double)M_PI, attr);
#else
  put("PI", 2.0 * asin(1), attr);
#endif

#ifdef M_SQRT1_2
  put("SQRT1_2", (double)M_SQRT1_2, attr);
#else
  put("SQRT1_2", sqrt(0.5), attr);
#endif

#ifdef M_SQRT2
  put("SQRT2", (double)M_SQRT2, attr);
#else
  put("SQRT2", sqrt(2), attr);
#endif

  put("abs", new KJSMathFunc(IDAbs), attr, true);
  put("acos", new KJSMathFunc(IDACos), attr, true);
  put("asin", new KJSMathFunc(IDASin), attr, true);
  put("atan", new KJSMathFunc(IDATan), attr, true);
  put("atan2", new KJSMathFunc(IDATan2), attr, true);
  put("ceil", new KJSMathFunc(IDCeil), attr, true);
  put("cos", new KJSMathFunc(IDCos), attr, true);
  put("exp", new KJSMathFunc(IDExp), attr, true);
  put("floor", new KJSMathFunc(IDFloor), attr, true);
  put("log", new KJSMathFunc(IDLog), attr, true);
  put("max", new KJSMathFunc(IDMax), attr, true);
  put("min", new KJSMathFunc(IDMin), attr, true);
  put("random", new KJSMathFunc(IDRandom), attr, true);
  put("round", new KJSMathFunc(IDRound), attr, true);
  put("sin", new KJSMathFunc(IDSin), attr, true);
  put("sqrt", new KJSMathFunc(IDSqrt), attr, true);
  put("tan", new KJSMathFunc(IDTan), attr, true);
}

KJSO* KJSMathFunc::execute(KJSContext *context)
{
  Ptr v = context->activation->get("0");
  Ptr n = toNumber(v);
  double arg = n->dVal();

  Ptr v2 = context->activation->get("1");
  Ptr n2 = toNumber(v2);
  double arg2 = n2->dVal();
  double result;
  int temp;

  switch (id) {
  case IDAbs:
    result = ( arg < 0 ) ? (-arg) : arg;
    break;
  case IDACos:
    result = ::acos(arg);
    break;
  case IDASin:
    result = ::asin(arg);
    break;
  case IDATan:
    result = ::atan(arg);
    break;
  case IDATan2:
    result = ::atan2(arg, arg2);
    break;
  case IDCeil:
    result = ::ceil(arg);
    break;
  case IDCos:
    result = ::cos(arg);
    break;
  case IDExp:
    result = ::exp(arg);
    break;
  case IDFloor:
    result = ::floor(arg);
    break;
  case IDLog:
    result = ::log(arg);
    break;
  case IDMax:
    result = ( arg > arg2 ) ? arg : arg2;
    break;
  case IDMin:
    result = ( arg < arg2 ) ? arg : arg2;
    break;
  case IDRandom:
    result = ::rand();
    result = result / RAND_MAX;
    break;
  case IDRound:
    temp =  (int) (arg + 0.5);
    result = temp;
    break;
  case IDSin:
    result = ::sin(arg);
    break;
  case IDSqrt:
    result = ::sqrt(arg);
    break;
  case IDTan:
    result = ::tan(arg);
    break;

  default:
    assert((result = 0));
  }

  KJSRETURN(zeroRef(new KJSNumber(result)));
}
