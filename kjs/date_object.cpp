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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifndef HAVE_SYS_TIMEB_H
#define HAVE_SYS_TIMEB_H 0
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#  include <time.h>
# endif
#endif
#if HAVE_SYS_TIMEB_H
#include <sys/timeb.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#  include <sys/param.h>
#endif // HAVE_SYS_PARAM_H

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "date_object.h"
#include "error_object.h"

#include "date_object.lut.h"

using namespace KJS;

// ------------------------------ DateInstanceImp ------------------------------

const ClassInfo DateInstanceImp::info = {"Date", 0, 0, 0};

DateInstanceImp::DateInstanceImp(const Object &proto)
  : ObjectImp(proto)
{
}

// ------------------------------ DatePrototypeImp -----------------------------

const ClassInfo DatePrototypeImp::info = {"Date", 0, &dateTable, 0};

/* Source for date_object.lut.h
   We use a negative ID to denote the "UTC" variant.
@begin dateTable 61
  toString		DateProtoFuncImp::ToString		DontEnum|Function	0
  toUTCString		-DateProtoFuncImp::ToString		DontEnum|Function	0
  toDateString		DateProtoFuncImp::ToDateString		DontEnum|Function	0
  toTimeString		DateProtoFuncImp::ToTimeString		DontEnum|Function	0
  toLocaleString	DateProtoFuncImp::ToLocaleString	DontEnum|Function	0
  toLocaleDateString	DateProtoFuncImp::ToLocaleDateString	DontEnum|Function	0
  toLocaleTimeString	DateProtoFuncImp::ToLocaleTimeString	DontEnum|Function	0
  valueOf		DateProtoFuncImp::ValueOf		DontEnum|Function	0
  getTime		DateProtoFuncImp::GetTime		DontEnum|Function	0
  getFullYear		DateProtoFuncImp::GetFullYear		DontEnum|Function	0
  getUTCFullYear	-DateProtoFuncImp::GetFullYear		DontEnum|Function	0
  toGMTString		DateProtoFuncImp::ToGMTString		DontEnum|Function	0
  getMonth		DateProtoFuncImp::GetMonth		DontEnum|Function	0
  getUTCMonth		-DateProtoFuncImp::GetMonth		DontEnum|Function	0
  getDate		DateProtoFuncImp::GetDate		DontEnum|Function	0
  getUTCDate		-DateProtoFuncImp::GetDate		DontEnum|Function	0
  getDay		DateProtoFuncImp::GetDay		DontEnum|Function	0
  getUTCDay		-DateProtoFuncImp::GetDay		DontEnum|Function	0
  getHours		DateProtoFuncImp::GetHours		DontEnum|Function	0
  getUTCHours		-DateProtoFuncImp::GetHours		DontEnum|Function	0
  getMinutes		DateProtoFuncImp::GetMinutes		DontEnum|Function	0
  getUTCMinutes		-DateProtoFuncImp::GetMinutes		DontEnum|Function	0
  getSeconds		DateProtoFuncImp::GetSeconds		DontEnum|Function	0
  getUTCSeconds		-DateProtoFuncImp::GetSeconds		DontEnum|Function	0
  getMilliseconds	DateProtoFuncImp::GetMilliSeconds	DontEnum|Function	0
  getUTCMilliseconds	-DateProtoFuncImp::GetMilliSeconds	DontEnum|Function	0
  getTimezoneOffset	DateProtoFuncImp::GetTimezoneOffset	DontEnum|Function	0
  setTime		DateProtoFuncImp::SetTime		DontEnum|Function	1
  setMilliseconds	DateProtoFuncImp::SetMilliSeconds	DontEnum|Function	1
  setUTCMilliseconds	-DateProtoFuncImp::SetMilliSeconds	DontEnum|Function	1
  setSeconds		DateProtoFuncImp::SetSeconds		DontEnum|Function	2
  setUTCSeconds		-DateProtoFuncImp::SetSeconds		DontEnum|Function	2
  setMinutes		DateProtoFuncImp::SetMinutes		DontEnum|Function	3
  setUTCMinutes		-DateProtoFuncImp::SetMinutes		DontEnum|Function	3
  setHours		DateProtoFuncImp::SetHours		DontEnum|Function	4
  setUTCHours		-DateProtoFuncImp::SetHours		DontEnum|Function	4
  setDate		DateProtoFuncImp::SetDate		DontEnum|Function	1
  setUTCDate		-DateProtoFuncImp::SetDate		DontEnum|Function	1
  setMonth		DateProtoFuncImp::SetMonth		DontEnum|Function	2
  setUTCMonth		-DateProtoFuncImp::SetMonth		DontEnum|Function	2
  setFullYear		DateProtoFuncImp::SetFullYear		DontEnum|Function	3
  setUTCFullYear	-DateProtoFuncImp::SetFullYear		DontEnum|Function	3
  setYear		DateProtoFuncImp::SetYear		DontEnum|Function	1
  getYear		DateProtoFuncImp::GetYear		DontEnum|Function	0
  toGMTString		DateProtoFuncImp::ToGMTString		DontEnum|Function	0
@end
*/
// ECMA 15.9.4

DatePrototypeImp::DatePrototypeImp(ExecState *,
                                   ObjectPrototypeImp *objectProto)
  : DateInstanceImp(objectProto)
{
  Value protect(this);
  setInternalValue(Number(NaN));
  // The constructor will be added later, after DateObjectImp has been built
}

Value DatePrototypeImp::get(ExecState *exec, const UString &propertyName) const
{
  return lookupGetFunction<DateProtoFuncImp, DatePrototypeImp, ObjectImp>( exec, propertyName, &dateTable, this );
}

// ------------------------------ DateProtoFuncImp -----------------------------

DateProtoFuncImp::DateProtoFuncImp(ExecState *exec, int i, int len)
  : InternalFunctionImp(
    static_cast<FunctionPrototypeImp*>(exec->interpreter()->builtinFunctionPrototype().imp())
    ), id(abs(i)), utc(i<0)
  // We use a negative ID to denote the "UTC" variant.
{
  Value protect(this);
  put(exec,"length",Number(len),DontDelete|ReadOnly|DontEnum);
}

bool DateProtoFuncImp::implementsCall() const
{
  return true;
}

Value DateProtoFuncImp::call(ExecState *exec, Object &thisObj, const List &args)
{
  if ((id == ToString || id == ValueOf || id == GetTime || id == SetTime) &&
      !thisObj.inherits(&DateInstanceImp::info)) {
    // non-generic function called on non-date object

    // ToString and ValueOf are generic according to the spec, but the mozilla
    // tests suggest otherwise...
    Object err = Error::create(exec,TypeError);
    exec->setException(err);
    return err;
  }


  Value result;
  UString s;
  const int bufsize=100;
  char timebuffer[bufsize];
  char *oldlocale = setlocale(LC_TIME,NULL);
  if (!oldlocale)
    oldlocale = setlocale(LC_ALL, NULL);
  Value v = thisObj.internalValue();
  double milli = v.toNumber(exec).value();
  time_t tv = (time_t) floor(milli / 1000.0);
  int ms = int(milli - tv * 1000.0);

  struct tm *t;
  if (utc)
    t = gmtime(&tv);
  else
    t = localtime(&tv);

  switch (id) {
  case ToString:
    s = ctime(&tv);
    result = String(s.substr(0, s.size() - 1));
    break;
  case ToDateString:
  case ToTimeString:
  case ToGMTString:
    setlocale(LC_TIME,"C");
    if (id == DateProtoFuncImp::ToDateString) {
      strftime(timebuffer, bufsize, "%x",t);
    } else if (id == DateProtoFuncImp::ToTimeString) {
      strftime(timebuffer, bufsize, "%X",t);
    } else {
      t = gmtime(&tv);
      strftime(timebuffer, bufsize, "%a, %d-%b-%y %H:%M:%S %Z", t);
    }
    setlocale(LC_TIME,oldlocale);
    result = String(timebuffer);
    break;
  case ToLocaleString:
    strftime(timebuffer, bufsize, "%c", t);
    result = String(timebuffer);
    break;
  case ToLocaleDateString:
    strftime(timebuffer, bufsize, "%x", t);
    result = String(timebuffer);
    break;
  case ToLocaleTimeString:
    strftime(timebuffer, bufsize, "%X", t);
    result = String(timebuffer);
    break;
  case ValueOf:
    result = Number(milli);
    break;
  case GetTime:
    result = Number(milli);
    break;
  case GetYear:
    result = Number(t->tm_year);
    break;
  case GetFullYear:
    result = Number(1900 + t->tm_year);
    break;
  case GetMonth:
    result = Number(t->tm_mon);
    break;
  case GetDate:
    result = Number(t->tm_mday);
    break;
  case GetDay:
    result = Number(t->tm_wday);
    break;
  case GetHours:
    result = Number(t->tm_hour);
    break;
  case GetMinutes:
    result = Number(t->tm_min);
    break;
  case GetSeconds:
    result = Number(t->tm_sec);
    break;
  case GetMilliSeconds:
    result = Number(ms);
    break;
  case GetTimezoneOffset:
#if defined BSD || defined(__APPLE__)
    result = Number(-( t->tm_gmtoff / 60 ) + ( t->tm_isdst ? 60 : 0 ));
#else
#  if defined(__BORLANDC__)
#error please add daylight savings offset here!
    result = Number(_timezone / 60 - (_daylight ? 60 : 0));
#  else
    result = Number(( timezone / 60 - ( daylight ? 60 : 0 )));
#  endif
#endif
    break;
  case SetTime:
    milli = roundValue(exec,args[0]);
    result = Number(milli);
    thisObj.setInternalValue(result);
    break;
  case SetMilliSeconds:
    ms = args[0].toInt32(exec);
    break;
  case SetSeconds:
    t->tm_sec = args[0].toInt32(exec);
    break;
  case SetMinutes:
    t->tm_min = args[0].toInt32(exec);
    break;
  case SetHours:
    t->tm_hour = args[0].toInt32(exec);
    break;
  case SetDate:
    t->tm_mday = args[0].toInt32(exec);
    break;
  case SetMonth:
    t->tm_mon = args[0].toInt32(exec);
    break;
  case SetFullYear:
    t->tm_year = args[0].toInt32(exec) - 1900;
    break;
  case SetYear:
    t->tm_year = args[0].toInt32(exec) >= 1900 ? args[0].toInt32(exec) - 1900 : args[0].toInt32(exec);
    break;
  }

  if (id == SetYear || id == SetMilliSeconds || id == SetSeconds ||
      id == SetMinutes || id == SetHours || id == SetDate ||
      id == SetMonth || id == SetFullYear ) {
    result = Number(mktime(t) * 1000.0 + ms);
    thisObj.setInternalValue(result);
  }

  return result;
}

// ------------------------------ DateObjectImp --------------------------------

// TODO: MakeTime (15.9.11.1) etc. ?

DateObjectImp::DateObjectImp(ExecState *exec,
                             FunctionPrototypeImp *funcProto,
                             DatePrototypeImp *dateProto)
  : InternalFunctionImp(funcProto)
{
  Value protect(this);
  // ECMA 15.9.4.1 Date.prototype
  put(exec,"prototype",dateProto,DontEnum|DontDelete|ReadOnly);

  put(exec,"parse", new DateObjectFuncImp(exec,funcProto,DateObjectFuncImp::Parse, 1), DontEnum);
  put(exec,"UTC",   new DateObjectFuncImp(exec,funcProto,DateObjectFuncImp::UTC,   7),   DontEnum);

  // no. of arguments for constructor
  put(exec,"length", Number(7), ReadOnly|DontDelete|DontEnum);
}

bool DateObjectImp::implementsConstruct() const
{
  return true;
}

// ECMA 15.9.3
Object DateObjectImp::construct(ExecState *exec, const List &args)
{
  Value value;

  int numArgs = args.size();

  if (numArgs == 0) { // new Date() ECMA 15.9.3.3
#if HAVE_SYS_TIMEB_H
#  if defined(__BORLANDC__)
    struct timeb timebuffer;
    ftime(&timebuffer);
#  else
    struct _timeb timebuffer;
    _ftime(&timebuffer);
#  endif
    double utc = floor((double)timebuffer.time * 1000.0 + (double)timebuffer.millitm);
#else
    struct timeval tv;
    gettimeofday(&tv, 0L);
    double utc = floor((double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0);
#endif
    value = Number(utc);
  } else if (numArgs == 1) {
    Value p = args[0].toPrimitive(exec);
    if (p.type() == StringType)
      value = parseDate(p.toString(exec));
    else
      value = p.toNumber(exec);
  } else {
    struct tm t;
    memset(&t, 0, sizeof(t));
    Number y = args[0].toNumber(exec);
    // TODO: check for NaN
    int year = y.toInt32(exec);
    t.tm_year = (year >= 0 && year <= 99) ? year : year - 1900;
    t.tm_mon = args[1].toInt32(exec);
    t.tm_mday = (numArgs >= 3) ? args[2].toInt32(exec) : 1;
    t.tm_hour = (numArgs >= 4) ? args[3].toInt32(exec) : 0;
    t.tm_min = (numArgs >= 5) ? args[4].toInt32(exec) : 0;
    t.tm_sec = (numArgs >= 6) ? args[5].toInt32(exec) : 0;
    t.tm_isdst = -1;
    int ms = (numArgs >= 7) ? args[6].toInt32(exec) : 0;
    value = Number(mktime(&t) * 1000.0 + ms);
  }

  ObjectImp *proto = static_cast<ObjectImp*>(exec->interpreter()->builtinDatePrototype().imp());
  Object ret(new DateInstanceImp(proto));
  ret.setInternalValue(timeClip(value));
  return ret;
}

bool DateObjectImp::implementsCall() const
{
  return true;
}

// ECMA 15.9.2
Value DateObjectImp::call(ExecState */*exec*/, Object &/*thisObj*/, const List &/*args*/)
{
  time_t t = time(0L);
  UString s(ctime(&t));

  // return formatted string minus trailing \n
  return String(s.substr(0, s.size() - 1));
}

// ------------------------------ DateObjectFuncImp ----------------------------

DateObjectFuncImp::DateObjectFuncImp(ExecState *exec, FunctionPrototypeImp *funcProto,
                                     int i, int len)
  : InternalFunctionImp(funcProto), id(i)
{
  Value protect(this);
  put(exec,"length",Number(len),DontDelete|ReadOnly|DontEnum);
}

bool DateObjectFuncImp::implementsCall() const
{
  return true;
}

// ECMA 15.9.4.2 - 3
Value DateObjectFuncImp::call(ExecState *exec, Object &/*thisObj*/, const List &args)
{
  if (id == Parse) {
    if (args[0].type() == StringType)
      return parseDate(args[0].toString(exec));
    else
      return Undefined();
  }
  else {
    struct tm t;
    memset(&t, 0, sizeof(t));
    int n = args.size();
    Number y = args[0].toNumber(exec);
    // TODO: check for NaN
    int year = y.toInt32(exec);
    t.tm_year = (year >= 0 && year <= 99) ? year : year - 1900;
    t.tm_mon = args[1].toInt32(exec);
    t.tm_mday = (n >= 3) ? args[2].toInt32(exec) : 1;
    t.tm_hour = (n >= 4) ? args[3].toInt32(exec) : 0;
    t.tm_min = (n >= 5) ? args[4].toInt32(exec) : 0;
    t.tm_sec = (n >= 6) ? args[5].toInt32(exec) : 0;
    int ms = (n >= 7) ? args[6].toInt32(exec) : 0;
    return Number(mktime(&t) * 1000.0 + ms);
  }
}

// -----------------------------------------------------------------------------


Value KJS::parseDate(const String &s)
{
  UString u = s.value();
  int firstSlash = u.find('/');
  if ( firstSlash == -1 )
  {
    // TODO parse dates like "December 25, 1995 23:15:00"
    fprintf(stderr,"KJS::parseDate parsing for this format isn't implemented\n%s", u.ascii());
    return Number(0);
  }
  else
  {
    // Found 12/31/2099 on some website -> obviously MM/DD/YYYY
    int month = u.substr(0,firstSlash).toULong();
    int secondSlash = u.find('/',firstSlash+1);
    //fprintf(stdout,"KJS::parseDate firstSlash=%d, secondSlash=%d\n", firstSlash, secondSlash);
    if ( secondSlash == -1 )
    {
      fprintf(stderr,"KJS::parseDate parsing for this format isn't implemented\n%s", u.ascii());
      return Number(0);
    }
    int day = u.substr(firstSlash+1,secondSlash-firstSlash-1).toULong();
    int year = u.substr(secondSlash+1).toULong();
    //fprintf(stdout,"KJS::parseDate day=%d, month=%d, year=%d\n", day, month, year);
    struct tm t;
    memset( &t, 0, sizeof(t) );
    year = (year > 2037) ? 2037 : year; // mktime is limited to 2037 !!!
    t.tm_year = (year >= 0 && year <= 99) ? year : year - 1900;
    t.tm_mon = month-1; // mktime wants 0-11 for some reason
    t.tm_mday = day;
    time_t seconds = mktime(&t);
    if ( seconds == -1 )
    {
      fprintf(stderr,"KJS::parseDate mktime returned -1.\n%s", u.ascii());
      return Undefined();
    }
    else
      return Number(seconds * 1000.0);
  }
}

Value KJS::timeClip(const Value &t)
{
  /* TODO */
  return t;
}

