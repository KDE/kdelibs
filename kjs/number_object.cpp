// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003 Peter Kelly (pmk@post.com)
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "value.h"
#include "object.h"
#include "types.h"
#include "interpreter.h"
#include "operations.h"
#include "number_object.h"
#include "error_object.h"
#include "dtoa.h"

#include "number_object.lut.h"

#include <assert.h>
#include <math.h>

using namespace KJS;

// ------------------------------ NumberInstanceImp ----------------------------

const ClassInfo NumberInstanceImp::info = {"Number", 0, 0, 0};

NumberInstanceImp::NumberInstanceImp(ObjectImp *proto)
  : ObjectImp(proto)
{
}
// ------------------------------ NumberPrototypeImp ---------------------------

// ECMA 15.7.4

NumberPrototypeImp::NumberPrototypeImp(ExecState *exec,
                                       ObjectPrototypeImp *objProto,
                                       FunctionPrototypeImp *funcProto)
  : NumberInstanceImp(objProto)
{
  Value protect(this);
  setInternalValue(NumberImp::zero());

  // The constructor will be added later, after NumberObjectImp has been constructed

  putDirect(toStringPropertyName,new NumberProtoFuncImp(exec,funcProto,NumberProtoFuncImp::ToString,
							1,toStringPropertyName),DontEnum);
  putDirect(toLocaleStringPropertyName,new NumberProtoFuncImp(exec,funcProto,NumberProtoFuncImp::ToLocaleString,
							      0,toLocaleStringPropertyName),DontEnum);
  putDirect(valueOfPropertyName,new NumberProtoFuncImp(exec,funcProto,NumberProtoFuncImp::ValueOf,
						       0,valueOfPropertyName),DontEnum);
  putDirect("toFixed", new NumberProtoFuncImp(exec,funcProto,NumberProtoFuncImp::ToFixed,
					      1,"toFixed"),DontEnum);
  putDirect("toExponential",new NumberProtoFuncImp(exec,funcProto,NumberProtoFuncImp::ToExponential,
						   1,"toExponential"),DontEnum);
  putDirect("toPrecision",new NumberProtoFuncImp(exec,funcProto,NumberProtoFuncImp::ToPrecision,
						 1,"toPrecision"),DontEnum);
}


// ------------------------------ NumberProtoFuncImp ---------------------------

NumberProtoFuncImp::NumberProtoFuncImp(ExecState * /*exec*/, FunctionPrototypeImp *funcProto,
                                       int i, int len, const Identifier &_ident)
  : InternalFunctionImp(funcProto), id(i)
{
  Value protect(this);
  putDirect(lengthPropertyName, len, DontDelete|ReadOnly|DontEnum);
  ident = _ident;
}


bool NumberProtoFuncImp::implementsCall() const
{
  return true;
}

static UString integer_part_noexp(double d)
{
  int decimalPoint;
  int signDummy;
  char *result = kjs_dtoa(d, 0, 0, &decimalPoint, &signDummy, NULL);
  int length = strlen(result);

  // sign for non-zero, negative numbers
  UString str = d < 0 ? "-" : "";
  if (decimalPoint == 9999) {
    str += UString(result);
  } else if (decimalPoint <= 0) {
    str += UString("0");
  } else {
    char *buf;

    if (length <= decimalPoint) {
      buf = (char*)malloc(decimalPoint+1);
      strcpy(buf,result);
      memset(buf+length,'0',decimalPoint-length);
    } else {
      buf = (char*)malloc(decimalPoint+1);
      strncpy(buf,result,decimalPoint);
    }

    buf[decimalPoint] = '\0';
    str += UString(buf);
    free(buf);
  }

  kjs_freedtoa(result);

  return str;
}

static UString char_sequence(char c, int count)
{
  char *buf = (char*)malloc(count+1);
  memset(buf,c,count);
  buf[count] = '\0';
  UString s(buf);
  free(buf);
  return s;
}

// ECMA 15.7.4.2 - 15.7.4.7
Value NumberProtoFuncImp::call(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;

  // no generic function. "this" has to be a Number object
  KJS_CHECK_THIS( NumberInstanceImp, thisObj );

  // execute "toString()" or "valueOf()", respectively
  Value v = thisObj.internalValue();
  switch (id) {
  case ToString: {
    int radix = 10;
    if (!args.isEmpty() && args[0].type() != UndefinedType)
      radix = args[0].toInteger(exec);
    if (radix < 2 || radix > 36 || radix == 10)
      result = String(v.toString(exec));
    else {
      const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
      // INT_MAX results in 1024 characters left of the dot with radix 2
      // give the same space on the right side. safety checks are in place
      // unless someone finds a precise rule.
      char s[2048 + 3];
      double x = v.toNumber(exec);
      if (isNaN(x) || isInf(x))
        return String(UString::from(x));
      // apply algorithm on absolute value. add sign later.
      bool neg = false;
      if (x < 0.0) {
        neg = true;
        x = -x;
      }
      // convert integer portion
      double f = floor(x);
      double d = f;
      char *dot = s + sizeof(s) / 2;
      char *p = dot;
      *p = '\0';
      do {
        *--p = digits[int(fmod(d, double(radix)))];
        d /= radix;
      } while ((d <= -1.0 || d >= 1.0) && p > s);
      // any decimal fraction ?
      d = x - f;
      const double eps = 0.001; // TODO: guessed. base on radix ?
      if (d < -eps || d > eps) {
        *dot++ = '.';
        do {
          d *= radix;
          *dot++ = digits[int(d)];
          d -= int(d);
        } while ((d < -eps || d > eps) && dot - s < int(sizeof(s)) - 1);
        *dot = '\0';
      }
      // add sign if negative
      if (neg)
        *--p = '-';
      result = String(p);
    }
    break;
  }
  case ToLocaleString: /* TODO */
    result = String(v.toString(exec));
    break;
  case ValueOf:
    result = Number(v.toNumber(exec));
    break;
  case ToFixed:
  {
    // FIXME: firefox works for all values, not just 0..20.  This includes
    // NaN, infinity, undefined, etc.  This is just a hack to pass our regression
    // suite.
    Value fractionDigits = args[0];
    int f = -1;
    double fd = fractionDigits.toNumber(exec);
    if (isNaN(fd)) {
      f = 0;
    } else if (!isInf(fd)) {
      f = int(fd);
    }
    if (f < 0 || f > 20) {
      Object err = Error::create(exec,RangeError);
      exec->setException(err);
      return err;
    }

    double x = v.toNumber(exec);
    if (isNaN(x))
      return String("NaN");

    UString s = "";
    if (x < 0) {
      s += "-";
      x = -x;
    }

    if (x >= 1e21)
      return String(s+UString::from(x));

    double n = floor(x*pow(10.0,f));
    if (fabs(n/pow(10.0,f)-x) > fabs((n+1)/pow(10.0,f)-x))
      n++;

    UString m = integer_part_noexp(n);

    int k = m.size();
    if (m.size() < f) {
      UString z = "";
      for (int i = 0; i < f+1-k; i++)
	z += "0";
      m = z + m;
      k = f + 1;
      assert(k == m.size());
    }
    if (k-f < m.size())
      return String(s+m.substr(0,k-f)+"."+m.substr(k-f));
    else
      return String(s+m.substr(0,k-f));
  }
  case ToExponential: {
    double x = v.toNumber(exec);

    if (isNaN(x) || isInf(x))
      return String(UString::from(x));

    int f = 1;
    Value fractionDigits = args[0];
    if (args.size() > 0) {
      f = fractionDigits.toInteger(exec);
      if (f < 0 || f > 20) {
        Object err = Error::create(exec,RangeError);
        exec->setException(err);
        return err;
      }
    }

    int decimalAdjust = 0;
    if (!fractionDigits.isA(UndefinedType)) {
      double logx = floor(log10(x));
      x /= pow(10.0,logx);
      double fx = floor(x*pow(10.0,f))/pow(10.0,f);
      double cx = ceil(x*pow(10.0,f))/pow(10.0,f);

      if (fabs(fx-x) < fabs(cx-x))
	x = fx;
      else
	x = cx;

      decimalAdjust = int(logx);
    }

    char buf[80];
    int decimalPoint;
    int sign;

    if (isNaN(x))
      return String("NaN");

    char *result = kjs_dtoa(x, 0, 0, &decimalPoint, &sign, NULL);
    int length = strlen(result);
    decimalPoint += decimalAdjust;

    int i = 0;
    if (sign) {
      buf[i++] = '-';
    }

    if (decimalPoint == 999) {
      strcpy(buf + i, result);
    } else {
      buf[i++] = result[0];

      if (fractionDigits.isA(UndefinedType))
	f = length-1;

      if (length > 1 && f > 0) {
	buf[i++] = '.';
	int haveFDigits = length-1;
	if (f < haveFDigits) {
	  strncpy(buf+i,result+1, f);
	  i += f;
	}
	else {
	  strcpy(buf+i,result+1);
	  i += length-1;
	  for (int j = 0; j < f-haveFDigits; j++)
	    buf[i++] = '0';
	}
      }

      buf[i++] = 'e';
      buf[i++] = (decimalPoint >= 0) ? '+' : '-';
      // decimalPoint can't be more than 3 digits decimal given the
      // nature of float representation
      int exponential = decimalPoint - 1;
      if (exponential < 0) {
	exponential = exponential * -1;
      }
      if (exponential >= 100) {
	buf[i++] = '0' + exponential / 100;
      }
      if (exponential >= 10) {
	buf[i++] = '0' + (exponential % 100) / 10;
      }
      buf[i++] = '0' + exponential % 10;
      buf[i++] = '\0';
    }

    assert(i <= 80);

    kjs_freedtoa(result);

    return String(UString(buf));
  }
  case ToPrecision:
  {
    int e = 0;
    UString m;

    int p = args[0].toInteger(exec);
    double x = v.toNumber(exec);
    if (args[0].isA(UndefinedType) || isNaN(x) || isInf(x))
      return String(v.toString(exec));

    UString s = "";
    if (x < 0) {
      s = "-";
      x = -x;
    }

    if (p < 1 || p > 21) {
      Object err = Error::create(exec, RangeError,
				 "toPrecision() argument must be between 1 and 21");
      exec->setException(err);
      return err;
    }

    if (x != 0) {
      // suggestions for a better algorithm welcome!
      e = int(log10(x));
      double n = floor(x/pow(10.0,e-p+1));
      if (n < pow(10.0,p-1)) {
	// first guess was not good
	e = e - 1;
	n = floor(x/pow(10.0,e-p+1));
	if (n >= pow(10.0,p)) {
	  // violated constraint. try something else.
	  n = pow(10.0,p-1);
	  e = int(log10(x/n)) + p - 1;
	}
      }

      if (fabs((n+1)*pow(10.0,e-p+1)-x) < fabs(n*pow(10.0,e-p+1)-x))
	n++;
      assert(pow(10.0,p-1) <= n);
      assert(n < pow(10.0,p));

      m = integer_part_noexp(n);
      if (e < -6 || e >= p) {
	if (m.size() > 1)
	  m = m.substr(0,1)+"."+m.substr(1);
	if (e >= 0)
	  return String(s+m+"e+"+UString::from(e));
	else
	  return String(s+m+"e-"+UString::from(-e));
      }
    }
    else {
      m = char_sequence('0',p);
      e = 0;
    }

    if (e == p-1) {
      return String(s+m);
    }
    else if (e >= 0) {
      if (e+1 < m.size())
	return String(s+m.substr(0,e+1)+"."+m.substr(e+1));
      else
	return String(s+m.substr(0,e+1));
    }
    else {
      return String(s+"0."+char_sequence('0',-(e+1))+m);
    }
  }
  }

  return result;
}

// ------------------------------ NumberObjectImp ------------------------------

const ClassInfo NumberObjectImp::info = {"Function", &InternalFunctionImp::info, &numberTable, 0};

/* Source for number_object.lut.h
@begin numberTable 5
  NaN			NumberObjectImp::NaNValue	DontEnum|DontDelete|ReadOnly
  NEGATIVE_INFINITY	NumberObjectImp::NegInfinity	DontEnum|DontDelete|ReadOnly
  POSITIVE_INFINITY	NumberObjectImp::PosInfinity	DontEnum|DontDelete|ReadOnly
  MAX_VALUE		NumberObjectImp::MaxValue	DontEnum|DontDelete|ReadOnly
  MIN_VALUE		NumberObjectImp::MinValue	DontEnum|DontDelete|ReadOnly
@end
*/
NumberObjectImp::NumberObjectImp(ExecState * /*exec*/,
                                 FunctionPrototypeImp *funcProto,
                                 NumberPrototypeImp *numberProto)
  : InternalFunctionImp(funcProto)
{
  Value protect(this);
  // Number.Prototype
  putDirect(prototypePropertyName, numberProto, DontEnum|DontDelete|ReadOnly);

  // no. of arguments for constructor
  putDirect(lengthPropertyName, NumberImp::one(), ReadOnly|DontDelete|DontEnum);
}

Value NumberObjectImp::get(ExecState *exec, const Identifier &propertyName) const
{
  return lookupGetValue<NumberObjectImp, InternalFunctionImp>( exec, propertyName, &numberTable, this );
}

Value NumberObjectImp::getValueProperty(ExecState *, int token) const
{
  // ECMA 15.7.3
  switch(token) {
  case NaNValue:
    return Number(NaN);
  case NegInfinity:
    return Number(-Inf);
  case PosInfinity:
    return Number(Inf);
  case MaxValue:
    return Number(1.7976931348623157E+308);
  case MinValue:
    return Number(5E-324);
  }
  return Null();
}

bool NumberObjectImp::implementsConstruct() const
{
  return true;
}


// ECMA 15.7.1
Object NumberObjectImp::construct(ExecState *exec, const List &args)
{
  ObjectImp *proto = exec->lexicalInterpreter()->builtinNumberPrototype().imp();
  Object obj(new NumberInstanceImp(proto));

  Number n;
  if (args.isEmpty())
    n = Number(0);
  else
    n = args[0].toNumber(exec);

  obj.setInternalValue(n);

  return obj;
}

bool NumberObjectImp::implementsCall() const
{
  return true;
}

// ECMA 15.7.2
Value NumberObjectImp::call(ExecState *exec, Object &/*thisObj*/, const List &args)
{
  if (args.isEmpty())
    return Number(0);
  else
    return Number(args[0].toNumber(exec));
}
