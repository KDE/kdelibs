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
  put("E", zeroRef(new KJSNumber(M_E)), attr);
#else
  put("E", zeroRef(new KJSNumber(::exp(1))), attr);
#endif

  put("sin", zeroRef(new KJSInternalFunction(&sin)));
}

double KJSMath::darg(const char *a)
{
  Ptr v = KJSARG(a);
  Ptr n = toNumber(v);
  return n->dVal();
}

KJSO* KJSMath::sin()
{
  double d = ::sin(darg("0"));
  KJSRETURN(zeroRef(new KJSNumber(d)));
}

