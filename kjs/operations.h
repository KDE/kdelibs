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
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */

#ifndef _KJS_OPERATIONS_H_
#define _KJS_OPERATIONS_H_

#include "value.h"

namespace KJS {

  class ExecState;

  /**
   * @return True if d is not a number (platform support required).
   */
  bool isNaN(double d);
  /**
   * @return True if d is infinite (platform support required).
   */
  bool isInf(double d);
  bool isPosInf(double d);
  bool isNegInf(double d);
  bool equal(ExecState *exec, const Value& v1, const Value& v2);
  bool strictEqual(ExecState *exec, const Value &v1, const Value &v2);
  /**
   * This operator performs an abstract relational comparision of the two
   * arguments that can be of arbitrary type. If possible, conversions to the
   * string or number type will take place before the comparison.
   *
   * @return 1 if v1 is "less-than" v2, 0 if the relation is "greater-than-or-
   * equal". -1 if the result is undefined.
   */
  int relation(ExecState *exec, const Value& v1, const Value& v2);
  int maxInt(int d1, int d2);
  int minInt(int d1, int d2);
  /**
   * Additive operator. Either performs an addition or substraction of v1
   * and v2.
   * @param oper '+' or '-' for an addition or substraction, respectively.
   * @return The result of the operation.
   */
  Value add(ExecState *exec, const Value &v1, const Value &v2, char oper);
  /**
   * Multiplicative operator. Either multiplies/divides v1 and v2 or
   * calculates the remainder from an division.
   * @param oper '*', '/' or '%' for a multiplication, division or
   * modulo operation.
   * @return The result of the operation.
   */
  Value mult(ExecState *exec, const Value &v1, const Value &v2, char oper);

}

#endif
