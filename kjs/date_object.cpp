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
#include <locale.h>

#include "date_object.h"
#include "error_object.h"

using namespace KJS;


// ------------------------------ DatePrototypeImp -----------------------------

// ECMA 15.9.4

DatePrototypeImp::DatePrototypeImp(ExecState *exec,
                                   ObjectPrototypeImp *objectProto,
                                   FunctionPrototypeImp *funcProto)
  : ObjectImp(objectProto)
{
  Value protect(this);
  setInternalValue(Number(NaN));

  // The constructor will be added later in DateObject's constructor

  put(exec,"toString",           new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::ToString,false,           0), DontEnum);
  put(exec,"toUTCString",        new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::ToString,true,            0), DontEnum);
  put(exec,"toDateString",       new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::ToDateString,false,       0), DontEnum);
  put(exec,"toTimeString",       new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::ToTimeString,false,       0), DontEnum);
  put(exec,"toLocaleString",     new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::ToLocaleString,false,     0), DontEnum);
  put(exec,"toLocaleDateString", new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::ToLocaleDateString,false, 0), DontEnum);
  put(exec,"toLocaleTimeString", new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::ToLocaleTimeString,false, 0), DontEnum);
  put(exec,"valueOf",            new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::ValueOf,false,            0), DontEnum);
  put(exec,"getTime",            new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetTime,false,            0), DontEnum);
  put(exec,"getFullYear",        new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetFullYear,false,        0), DontEnum);
  put(exec,"getUTCFullYear",     new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetFullYear,true,         0), DontEnum);
  put(exec,"toGMTString",        new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::ToGMTString,false,        0), DontEnum);
  put(exec,"getMonth",           new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetMonth,false,           0), DontEnum);
  put(exec,"getUTCMonth",        new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetMonth,true,            0), DontEnum);
  put(exec,"getDate",            new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetDate,false,            0), DontEnum);
  put(exec,"getUTCDate",         new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetDate,true,             0), DontEnum);
  put(exec,"getDay",             new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetDay,false,             0), DontEnum);
  put(exec,"getUTCDay",          new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetDay,true,              0), DontEnum);
  put(exec,"getHours",           new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetHours,false,           0), DontEnum);
  put(exec,"getUTCHours",        new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetHours,true,            0), DontEnum);
  put(exec,"getMinutes",         new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetMinutes,false,         0), DontEnum);
  put(exec,"getUTCMinutes",      new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetMinutes,true,          0), DontEnum);
  put(exec,"getSeconds",         new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetSeconds,false,         0), DontEnum);
  put(exec,"getUTCSeconds",      new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetSeconds,true,          0), DontEnum);
  put(exec,"getMilliseconds",    new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetMilliSeconds,false,    0), DontEnum);
  put(exec,"getUTCMilliseconds", new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetMilliSeconds,true,     0), DontEnum);
  put(exec,"getTimezoneOffset",  new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetTimezoneOffset,false,  0), DontEnum);
  put(exec,"setTime",            new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::SetTime,false,            1), DontEnum);
  put(exec,"setMilliseconds",    new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::SetMilliSeconds,false,    1), DontEnum);
  put(exec,"setUTCMilliseconds", new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::SetMilliSeconds,true,     1), DontEnum);
  put(exec,"setSeconds",         new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::SetSeconds,false,         2), DontEnum);
  put(exec,"setUTCSeconds",      new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::SetSeconds,true,          2), DontEnum);
  put(exec,"setMinutes",         new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::SetMinutes,false,         3), DontEnum);
  put(exec,"setUTCMinutes",      new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::SetMinutes,true,          3), DontEnum);
  put(exec,"setHours",           new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::SetHours,false,           4), DontEnum);
  put(exec,"setUTCHours",        new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::SetHours,true,            4), DontEnum);
  put(exec,"setDate",            new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::SetDate,false,            1), DontEnum);
  put(exec,"setUTCDate",         new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::SetDate,true,             1), DontEnum);
  put(exec,"setMonth",           new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::SetMonth,false,           2), DontEnum);
  put(exec,"setUTCMonth",        new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::SetMonth,true,            2), DontEnum);
  put(exec,"setFullYear",        new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::SetFullYear,false,        3), DontEnum);
  put(exec,"setUTCFullYear",     new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::SetFullYear,true,         3), DontEnum);
  put(exec,"setYear",            new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::SetYear,false,            1), DontEnum);
  // non-normative
  put(exec,"getYear",            new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::GetYear,false,            0), DontEnum);
  put(exec,"toGMTString",        new DateProtoFuncImp(exec,funcProto,DateProtoFuncImp::ToGMTString,false,        0), DontEnum);
}

// ------------------------------ DateProtoFuncImp -----------------------------

DateProtoFuncImp::DateProtoFuncImp(ExecState *exec, FunctionPrototypeImp *funcProto,
                                   int i, bool u, int len)
  : InternalFunctionImp(funcProto), id(i), utc(u)
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
    if (thisObj.inherits(&DateInstanceImp::info)) {
      result = Number(milli);
    }
    else {
      Object err = Error::create(exec,TypeError);
      exec->setException(err);
      result = err;
    }
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
    result = Undefined();
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
    if (thisObj.inherits(&DateInstanceImp::info)) {
      milli = roundValue(exec,args[0]);
      result = Number(milli);
      thisObj.setInternalValue(result);
    }
    else {
      Object err = Error::create(exec,TypeError);
      exec->setException(err);
      result = err;
    }
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

// ------------------------------ DateInstanceImp ------------------------------

const ClassInfo DateInstanceImp::info = {"Date", 0, 0, 0};

DateInstanceImp::DateInstanceImp(const Object &proto)
  : ObjectImp(proto)
{
}
