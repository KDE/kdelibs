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
  put("E", zeroRef(new KJSNumber((double)M_E)), attr);
#else
  put("E", zeroRef(new KJSNumber(::exp(1))), attr);
#endif

  put("sin", new KJSMathFunc(IDSin), attr, true);
  put("cos", new KJSMathFunc(IDCos), attr, true);
}

KJSO* KJSMathFunc::execute()
{
  Ptr v = KJSARG("0");
  Ptr n = toNumber(v);
  double arg = n->dVal();
  double result;

  switch (id) {
  case IDSin:
    result = ::sin(arg);
    break;
  case IDCos:
    result = ::cos(arg);
    break;
  default:
    assert((result = 0));
  }

  KJSRETURN(zeroRef(new KJSNumber(result)));
}
