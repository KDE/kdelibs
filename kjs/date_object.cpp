// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003 Apple Computer, Inc.
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
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
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
#ifdef HAVE_SYS_TIMEB_H
#include <sys/timeb.h>
#endif

#include <errno.h>

#ifdef HAVE_SYS_PARAM_H
#  include <sys/param.h>
#endif // HAVE_SYS_PARAM_H

#include <math.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#  include <strings.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <ctype.h>
#include <assert.h>

#include "date_object.h"
#include "error_object.h"
#include "operations.h"

#include "date_object.lut.h"

using namespace KJS;

// come constants
const time_t invalidDate = LONG_MIN;
const double hoursPerDay = 24;
const double minutesPerHour = 60;
const double secondsPerMinute = 60;
const double msPerSecond = 1000;
const double msPerMinute = msPerSecond * secondsPerMinute;
const double msPerHour = msPerMinute * minutesPerHour;
const double msPerDay = msPerHour * hoursPerDay;
static const char * const weekdayName[7] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
static const char * const monthName[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

static UString formatDate(struct tm &tm)
{
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "%s %s %02d %04d",
            weekdayName[(tm.tm_wday + 6) % 7],
            monthName[tm.tm_mon], tm.tm_mday, tm.tm_year + 1900);
    return buffer;
}

static UString formatDateUTCVariant(struct tm &tm)
{
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "%s, %02d %s %04d",
            weekdayName[(tm.tm_wday + 6) % 7],
            tm.tm_mday, monthName[tm.tm_mon], tm.tm_year + 1900);
    return buffer;
}

static UString formatTime(struct tm &tm)
{
    char buffer[100];
    if (tm.tm_gmtoff == 0) {
        snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d GMT", tm.tm_hour, tm.tm_min, tm.tm_sec);
    } else {
        int offset = tm.tm_gmtoff;
        if (offset < 0) {
            offset = -offset;
        }
        snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d GMT%c%02d%02d",
                tm.tm_hour, tm.tm_min, tm.tm_sec,
                tm.tm_gmtoff < 0 ? '-' : '+', offset / (60*60), (offset / 60) % 60);
    }
    return UString(buffer);
}

static int day(double t)
{
  return int(floor(t / msPerDay));
}

static double dayFromYear(int year)
{
  return 365.0 * (year - 1970)
    + floor((year - 1969) / 4.0)
    - floor((year - 1901) / 100.0)
    + floor((year - 1601) / 400.0);
}

// depending on whether it's a leap year or not
static int daysInYear(int year)
{
  if (year % 4 != 0)
    return 365;
  else if (year % 400 == 0)
    return 366;
  else if (year % 100 == 0)
    return 365;
  else
    return 366;
}

// time value of the start of a year
double timeFromYear(int year)
{
  return msPerDay * dayFromYear(year);
}

// year determined by time value
int yearFromTime(double t)
{
  // ### there must be an easier way
  // initial guess
  int y = 1970 + int(t / (365.25 * msPerDay));
  // adjustment
  if (timeFromYear(y) > t) {
    do {
      --y;
    } while (timeFromYear(y) > t);
  } else {
    while (timeFromYear(y + 1) < t)
      ++y;
  }

  return y;
}

// 0: Sunday, 1: Monday, etc.
int weekDay(double t)
{
  int wd = (day(t) + 4) % 7;
  if (wd < 0)
    wd += 7;
  return wd;
}

static double timeZoneOffset(const struct tm *t)
{
#if defined BSD || defined(__linux__) || defined(__APPLE__)
  return -(t->tm_gmtoff / 60);
#else
#  if defined(__BORLANDC__)
// FIXME consider non one-hour DST change
#error please add daylight savings offset here!
  return _timezone / 60 - (t->tm_isdst > 0 ? 60 : 0);
#  else
  return timezone / 60 - (t->tm_isdst > 0 ? 60 : 0 );
#  endif
#endif
}

// Converts a list of arguments sent to a Date member function into milliseconds, updating
// ms (representing milliseconds) and t (representing the rest of the date structure) appropriately.
//
// Format of member function: f([hour,] [min,] [sec,] [ms])
static void fillStructuresUsingTimeArgs(ExecState *exec, const List &args, int maxArgs, double *ms, struct tm *t)
{
    double milliseconds = 0;
    int idx = 0;
    int numArgs = args.size();
    
    // JS allows extra trailing arguments -- ignore them
    if (numArgs > maxArgs)
        numArgs = maxArgs;

    // hours
    if (maxArgs >= 4 && idx < numArgs) {
        t->tm_hour = 0;
        milliseconds += args[idx++].toInt32(exec) * msPerHour;
    }

    // minutes
    if (maxArgs >= 3 && idx < numArgs) {
        t->tm_min = 0;
        milliseconds += args[idx++].toInt32(exec) * msPerMinute;
    }
    
    // seconds
    if (maxArgs >= 2 && idx < numArgs) {
        t->tm_sec = 0;
        milliseconds += args[idx++].toInt32(exec) * msPerSecond;
    }
    
    // milliseconds
    if (idx < numArgs) {
        milliseconds += roundValue(exec, args[idx]);
    } else {
        milliseconds += *ms;
    }
    
    *ms = milliseconds;
}

// Converts a list of arguments sent to a Date member function into years, months, and milliseconds, updating
// ms (representing milliseconds) and t (representing the rest of the date structure) appropriately.
//
// Format of member function: f([years,] [months,] [days])
static void fillStructuresUsingDateArgs(ExecState *exec, const List &args, int maxArgs, double *ms, struct tm *t)
{
  int idx = 0;
  int numArgs = args.size();
  
  // JS allows extra trailing arguments -- ignore them
  if (numArgs > maxArgs)
    numArgs = maxArgs;
  
  // years
  if (maxArgs >= 3 && idx < numArgs) {
    t->tm_year = args[idx++].toInt32(exec) - 1900;
  }
  
  // months
  if (maxArgs >= 2 && idx < numArgs) {
    t->tm_mon = args[idx++].toInt32(exec);
  }
  
  // days
  if (idx < numArgs) {
    t->tm_mday = 0;
    *ms += args[idx].toInt32(exec) * msPerDay;
  }
}

// ------------------------------ DateInstanceImp ------------------------------

const ClassInfo DateInstanceImp::info = {"Date", 0, 0, 0};

DateInstanceImp::DateInstanceImp(ObjectImp *proto)
  : ObjectImp(proto)
{
}

// ------------------------------ DatePrototypeImp -----------------------------

const ClassInfo DatePrototypeImp::info = {"Date", 0, &dateTable, 0};

/* Source for date_object.lut.h
   We use a negative ID to denote the "UTC" variant.
@begin dateTable 61
  toString		DateProtoFuncImp::ToString		DontEnum|Function	0
  toUTCString		DateProtoFuncImp::ToUTCString		DontEnum|Function	0
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

Value DatePrototypeImp::get(ExecState *exec, const Identifier &propertyName) const
{
  return lookupGetFunction<DateProtoFuncImp, ObjectImp>( exec, propertyName, &dateTable, this );
}

// ------------------------------ DateProtoFuncImp -----------------------------

DateProtoFuncImp::DateProtoFuncImp(ExecState *exec, int i, int len)
  : InternalFunctionImp(
    static_cast<FunctionPrototypeImp*>(exec->interpreter()->builtinFunctionPrototype().imp())
    ), id(abs(i)), utc(i<0)
  // We use a negative ID to denote the "UTC" variant.
{
  Value protect(this);
  putDirect(lengthPropertyName, len, DontDelete|ReadOnly|DontEnum);
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
  CString oldlocale = setlocale(LC_TIME,NULL);
  if (!oldlocale.c_str())
    oldlocale = setlocale(LC_ALL, NULL);
  Value v = thisObj.internalValue();
  double milli = v.toNumber(exec);
  // special case: time value is NaN
  if (isNaN(milli)) {
    switch (id) {
    case ToString:
    case ToDateString:
    case ToTimeString:
    case ToGMTString:
    case ToUTCString:
    case ToLocaleString:
    case ToLocaleDateString:
    case ToLocaleTimeString:
      return String("Invalid Date");
    case ValueOf:
    case GetTime:
    case GetYear:
    case GetFullYear:
    case GetMonth:
    case GetDate:
    case GetDay:
    case GetHours:
    case GetMinutes:
    case GetSeconds:
    case GetMilliSeconds:
    case GetTimezoneOffset:
      return Number(NaN);
    }
  }

  // check whether time value is outside time_t's usual range
  // make the necessary transformations if necessary
  int realYearOffset = 0;
  double milliOffset = 0.0;
  if (milli < 0 || milli >= timeFromYear(2038)) {
    // ### ugly and probably not very precise
    int realYear = yearFromTime(milli);
    int base = daysInYear(realYear) == 365 ? 2001 : 2000;
    milliOffset = timeFromYear(base) - timeFromYear(realYear);
    milli += milliOffset;
    realYearOffset = realYear - base;
  }

  time_t tv = (time_t) floor(milli / 1000.0);
  double ms = milli - tv * 1000.0;

  struct tm *t;
  if ( (id == DateProtoFuncImp::ToGMTString) ||
       (id == DateProtoFuncImp::ToUTCString) )
    t = gmtime(&tv);
  else if (id == DateProtoFuncImp::ToString)
    t = localtime(&tv);
  else if (utc)
    t = gmtime(&tv);
  else
    t = localtime(&tv);

  // we had an out of range year. use that one (plus/minus offset
  // found by calculating tm_year) and fix the week day calculation
  if (realYearOffset != 0) {
    t->tm_year += realYearOffset;
    milli -= milliOffset;
    // our own weekday calculation. beware of need for local time.
    double m = milli;
    if (!utc)
      m -= timeZoneOffset(t) * msPerMinute;
    t->tm_wday = weekDay(m);
  }

  // trick gcc. We don't want the Y2K warnings.
  const char xFormat[] = "%x";
  const char cFormat[] = "%c";

  switch (id) {
  case ToString:
    result = String(formatDate(*t) + " " + formatTime(*t));
    break;
  case ToDateString:
    result = String(formatDate(*t));
    break;
  case ToTimeString:
    result = String(formatTime(*t));
    break;
  case ToGMTString:
  case ToUTCString:
    result = String(formatDateUTCVariant(*t) + " " + formatTime(*t));
    break;
  case ToLocaleString:
    strftime(timebuffer, bufsize, cFormat, t);
    result = String(timebuffer);
    break;
  case ToLocaleDateString:
    strftime(timebuffer, bufsize, xFormat, t);
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
    // IE returns the full year even in getYear.
    if ( exec->interpreter()->compatMode() != Interpreter::IECompat )
      result = Number(t->tm_year);
    else
      result = Number(1900 + t->tm_year);
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
    result = Number(timeZoneOffset(t));
    break;
  case SetTime:
    milli = roundValue(exec,args[0]);
    result = Number(milli);
    thisObj.setInternalValue(result);
    break;
  case SetMilliSeconds:
    fillStructuresUsingTimeArgs(exec, args, 1, &ms, t);
    break;
  case SetSeconds:
    fillStructuresUsingTimeArgs(exec, args, 2, &ms, t);
    break;
  case SetMinutes:
    fillStructuresUsingTimeArgs(exec, args, 3, &ms, t);
    break;
  case SetHours:
    fillStructuresUsingTimeArgs(exec, args, 4, &ms, t);
    break;
  case SetDate:
    fillStructuresUsingDateArgs(exec, args, 1, &ms, t);
    break;
  case SetMonth:
    fillStructuresUsingDateArgs(exec, args, 2, &ms, t);    
    break;
  case SetFullYear:
    fillStructuresUsingDateArgs(exec, args, 3, &ms, t);
    break;
  case SetYear:
    int y = args[0].toInt32(exec);
    if (y < 1900) {
      if (y >= 0 && y <= 99) {
        t->tm_year = y;
      } else {
        fillStructuresUsingDateArgs(exec, args, 3, &ms, t);
      }
    } else {
      t->tm_year = y - 1900;
    }
    break;
  }

  if (id == SetYear || id == SetMilliSeconds || id == SetSeconds ||
      id == SetMinutes || id == SetHours || id == SetDate ||
      id == SetMonth || id == SetFullYear ) {
    result = Number(makeTime(t, ms, utc));
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
  putDirect(prototypePropertyName, dateProto, DontEnum|DontDelete|ReadOnly);

  static const Identifier parsePropertyName("parse");
  putDirect(parsePropertyName, new DateObjectFuncImp(exec,funcProto,DateObjectFuncImp::Parse, 1), DontEnum);
  static const Identifier UTCPropertyName("UTC");
  putDirect(UTCPropertyName,   new DateObjectFuncImp(exec,funcProto,DateObjectFuncImp::UTC,   7),   DontEnum);

  // no. of arguments for constructor
  putDirect(lengthPropertyName, 7, ReadOnly|DontDelete|DontEnum);
}

bool DateObjectImp::implementsConstruct() const
{
  return true;
}

// ECMA 15.9.3
Object DateObjectImp::construct(ExecState *exec, const List &args)
{
  int numArgs = args.size();

#ifdef KJS_VERBOSE
  fprintf(stderr,"DateObjectImp::construct - %d args\n", numArgs);
#endif
  double value;

  if (numArgs == 0) { // new Date() ECMA 15.9.3.3
#ifdef HAVE_SYS_TIMEB_H
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
    value = utc;
  } else if (numArgs == 1) {
    Value prim = args[0].toPrimitive(exec);
    if (prim.isA(StringType))
      value = parseDate(prim.toString(exec));
    else
      value = prim.toNumber(exec);
  } else {
    if (isNaN(args[0].toNumber(exec))
        || isNaN(args[1].toNumber(exec))
        || (numArgs >= 3 && isNaN(args[2].toNumber(exec)))
        || (numArgs >= 4 && isNaN(args[3].toNumber(exec)))
        || (numArgs >= 5 && isNaN(args[4].toNumber(exec)))
        || (numArgs >= 6 && isNaN(args[5].toNumber(exec)))
        || (numArgs >= 7 && isNaN(args[6].toNumber(exec)))) {
      value = NaN;
    } else {
      struct tm t;
      memset(&t, 0, sizeof(t));
      int year = args[0].toInt32(exec);
      t.tm_year = (year >= 0 && year <= 99) ? year : year - 1900;
      t.tm_mon = args[1].toInt32(exec);
      t.tm_mday = (numArgs >= 3) ? args[2].toInt32(exec) : 1;
      t.tm_hour = (numArgs >= 4) ? args[3].toInt32(exec) : 0;
      t.tm_min = (numArgs >= 5) ? args[4].toInt32(exec) : 0;
      t.tm_sec = (numArgs >= 6) ? args[5].toInt32(exec) : 0;
      t.tm_isdst = -1;
      int ms = (numArgs >= 7) ? args[6].toInt32(exec) : 0;
      value = makeTime(&t, ms, false);
    }
  }

  Object proto = exec->interpreter()->builtinDatePrototype();
  Object ret(new DateInstanceImp(proto.imp()));
  ret.setInternalValue(Number(timeClip(value)));
  return ret;
}

bool DateObjectImp::implementsCall() const
{
  return true;
}

// ECMA 15.9.2
Value DateObjectImp::call(ExecState* /*exec*/, Object &/*thisObj*/, const List &/*args*/)
{
#ifdef KJS_VERBOSE
  fprintf(stderr,"DateObjectImp::call - current time\n");
#endif
  time_t t = time(0L);
  // FIXME: not threadsafe
  struct tm *tm = localtime(&t);
  return String(formatDate(*tm) + " " + formatTime(*tm));
}

// ------------------------------ DateObjectFuncImp ----------------------------

DateObjectFuncImp::DateObjectFuncImp(ExecState* /*exec*/, FunctionPrototypeImp *funcProto,
                                     int i, int len)
  : InternalFunctionImp(funcProto), id(i)
{
  Value protect(this);
  putDirect(lengthPropertyName, len, DontDelete|ReadOnly|DontEnum);
}

bool DateObjectFuncImp::implementsCall() const
{
  return true;
}

// ECMA 15.9.4.2 - 3
Value DateObjectFuncImp::call(ExecState *exec, Object &/*thisObj*/, const List &args)
{
  if (id == Parse) {
    return Number(parseDate(args[0].toString(exec)));
  } else { // UTC
    int n = args.size();
    if (isNaN(args[0].toNumber(exec))
        || isNaN(args[1].toNumber(exec))
        || (n >= 3 && isNaN(args[2].toNumber(exec)))
        || (n >= 4 && isNaN(args[3].toNumber(exec)))
        || (n >= 5 && isNaN(args[4].toNumber(exec)))
        || (n >= 6 && isNaN(args[5].toNumber(exec)))
        || (n >= 7 && isNaN(args[6].toNumber(exec)))) {
      return Number(NaN);
    }

    struct tm t;
    memset(&t, 0, sizeof(t));
    int year = args[0].toInt32(exec);
    t.tm_year = (year >= 0 && year <= 99) ? year : year - 1900;
    t.tm_mon = args[1].toInt32(exec);
    t.tm_mday = (n >= 3) ? args[2].toInt32(exec) : 1;
    t.tm_hour = (n >= 4) ? args[3].toInt32(exec) : 0;
    t.tm_min = (n >= 5) ? args[4].toInt32(exec) : 0;
    t.tm_sec = (n >= 6) ? args[5].toInt32(exec) : 0;
    int ms = (n >= 7) ? args[6].toInt32(exec) : 0;
    return Number(makeTime(&t, ms, true));
  }
}

// -----------------------------------------------------------------------------


double KJS::parseDate(const UString &u)
{
#ifdef KJS_VERBOSE
  fprintf(stderr,"KJS::parseDate %s\n",u.ascii());
#endif
  double /*time_t*/ seconds = KRFCDate_parseDate( u );

  return seconds == invalidDate ? NaN : seconds * 1000.0;
}

///// Awful duplication from krfcdate.cpp - we don't link to kdecore

static double ymdhms_to_seconds(int year, int mon, int day, int hour, int minute, int second)
{
    //printf("year=%d month=%d day=%d hour=%d minute=%d second=%d\n", year, mon, day, hour, minute, second);

    double ret = (day - 32075)       /* days */
            + 1461L * (year + 4800L + (mon - 14) / 12) / 4
            + 367 * (mon - 2 - (mon - 14) / 12 * 12) / 12
            - 3 * ((year + 4900L + (mon - 14) / 12) / 100) / 4
            - 2440588;
    ret = 24*ret + hour;     /* hours   */
    ret = 60*ret + minute;   /* minutes */
    ret = 60*ret + second;   /* seconds */

    return ret;
}

// we follow the recommendation of rfc2822 to consider all
// obsolete time zones not listed here equivalent to "-0000"
static const struct {
#ifdef _WIN32
    char tzName[4];
#else
    const char tzName[4];
#endif
    int tzOffset;
} known_zones[] = {
    { "UT", 0 },
    { "GMT", 0 },
    { "EST", -300 },
    { "EDT", -240 },
    { "CST", -360 },
    { "CDT", -300 },
    { "MST", -420 },
    { "MDT", -360 },
    { "PST", -480 },
    { "PDT", -420 },
    { { 0, 0, 0, 0 }, 0 }
};

double KJS::makeTime(struct tm *t, double ms, bool utc)
{
    int utcOffset;
    if (utc) {
	time_t zero = 0;
#if defined BSD || defined(__linux__) || defined(__APPLE__)
	struct tm t3;
       	localtime_r(&zero, &t3);
        utcOffset = t3.tm_gmtoff;
        t->tm_isdst = t3.tm_isdst;
#else
        (void)localtime(&zero);
#  if defined(__BORLANDC__)
        utcOffset = - _timezone;
#  else
        utcOffset = - timezone;
#  endif
        t->tm_isdst = 0;
#endif
    } else {
        utcOffset = 0;
        t->tm_isdst = -1;
    }

    double yearOffset = 0.0;
    if (t->tm_year < (1970 - 1900) || t->tm_year > (2038 - 1900)) {
      // we'll fool mktime() into believing that this year is within
      // its normal, portable range (1970-2038) by setting tm_year to
      // 2000 or 2001 and adding the difference in milliseconds later.
      // choice between offset will depend on whether the year is a
      // leap year or not.
      int y = t->tm_year + 1900;
      int baseYear = daysInYear(y) == 365 ? 2001 : 2000;
      const double baseTime = timeFromYear(baseYear);
      yearOffset = timeFromYear(y) - baseTime;
      t->tm_year = baseYear - 1900;
    }

    // Determine if we passed over a DST change boundary
    if (!utc) {
      time_t tval = mktime(t) + utcOffset + int((ms + yearOffset)/1000);
      struct tm t3;
      localtime_r(&tval, &t3);
      t->tm_isdst = t3.tm_isdst;
    }

    return (mktime(t) + utcOffset) * 1000.0 + ms + yearOffset;
}

// returns 0-11 (Jan-Dec); -1 on failure
static int findMonth(const char *monthStr)
{
  assert(monthStr);
  static const char haystack[37] = "janfebmaraprmayjunjulaugsepoctnovdec";
  char needle[4];
  for (int i = 0; i < 3; ++i) {
    if (!*monthStr)
      return -1;
    needle[i] = tolower(*monthStr++);
  }
  needle[3] = '\0';
  const char *str = strstr(haystack, needle);
  if (str) {
    int position = str - haystack;
    if (position % 3 == 0) {
      return position / 3;
    }
  }
  return -1;
}

double KJS::KRFCDate_parseDate(const UString &_date)
{
     // This parse a date in the form:
     //     Wednesday, 09-Nov-99 23:12:40 GMT
     // or
     //     Sat, 01-Jan-2000 08:00:00 GMT
     // or
     //     Sat, 01 Jan 2000 08:00:00 GMT
     // or
     //     01 Jan 99 22:00 +0100    (exceptions in rfc822/rfc2822)
     // ### non RFC formats, added for Javascript:
     //     [Wednesday] January 09 1999 23:12:40 GMT
     //     [Wednesday] January 09 23:12:40 GMT 1999
     //
     // We ignore the weekday
     //
     double result = -1;
     int offset = 0;
     bool have_tz = false;
     char *newPosStr;
     const char *dateString = _date.ascii();
     int day = 0;
     int month = -1; // not set yet
     int year = 0;
     int hour = 0;
     int minute = 0;
     int second = 0;
     bool have_time = false;

     // Skip leading space
     while(*dateString && isspace(*dateString))
     	dateString++;

     const char *wordStart = dateString;
     // Check contents of first words if not number
     while(*dateString && !isdigit(*dateString))
     {
        if ( isspace(*dateString) && dateString - wordStart >= 3 )
        {
          month = findMonth(wordStart);
          while(*dateString && isspace(*dateString))
             dateString++;
          wordStart = dateString;
        }
        else
           dateString++;
     }
     // missing delimiter between month and day (like "January29")?
     if (month == -1 && dateString && wordStart != dateString) {
       month = findMonth(wordStart);
       // TODO: emit warning about dubious format found
     }

     while(*dateString && isspace(*dateString))
     	dateString++;

     if (!*dateString)
     	return invalidDate;

     // ' 09-Nov-99 23:12:40 GMT'
     errno = 0;
     day = strtol(dateString, &newPosStr, 10);
     if (errno)
       return invalidDate;
     dateString = newPosStr;

     if (!*dateString)
     	return invalidDate;

     if (day < 1)
       return invalidDate;
     if (day > 31) {
       // ### where is the boundary and what happens below?
       if (*dateString == '/' && day >= 1000) {
         // looks like a YYYY/MM/DD date
         if (!*++dateString)
           return invalidDate;
         year = day;
         month = strtol(dateString, &newPosStr, 10) - 1;
         if (errno)
           return invalidDate;
         dateString = newPosStr;
         if (*dateString++ != '/' || !*dateString)
           return invalidDate;
         day = strtol(dateString, &newPosStr, 10);
         if (errno)
           return invalidDate;
         dateString = newPosStr;
       } else {
         return invalidDate;
       }
     } else if (*dateString == '/' && day <= 12 && month == -1)
     {
     	dateString++;
        // This looks like a MM/DD/YYYY date, not an RFC date.....
        month = day - 1; // 0-based
        day = strtol(dateString, &newPosStr, 10);
        if (errno)
          return invalidDate;
        dateString = newPosStr;
        if (*dateString == '/')
          dateString++;
        if (!*dateString)
          return invalidDate;
        //printf("month=%d day=%d dateString=%s\n", month, day, dateString);
     }
     else
     {
       if (*dateString == '-')
         dateString++;

       while(*dateString && isspace(*dateString))
         dateString++;

       if (*dateString == ',')
         dateString++;

       if ( month == -1 ) // not found yet
       {
         month = findMonth(dateString);
         if (month == -1)
           return invalidDate;

         while(*dateString && (*dateString != '-') && !isspace(*dateString))
           dateString++;

         if (!*dateString)
           return invalidDate;

         // '-99 23:12:40 GMT'
         if ((*dateString != '-') && (*dateString != '/') && !isspace(*dateString))
           return invalidDate;
         dateString++;
       }

       if ((month < 0) || (month > 11))
         return invalidDate;
     }

     // '99 23:12:40 GMT'
     if (year <= 0 && *dateString) {
       year = strtol(dateString, &newPosStr, 10);
       if (errno)
         return invalidDate;
     }

     // Don't fail if the time is missing.
     if (*newPosStr)
     {
        // ' 23:12:40 GMT'
        if (!isspace(*newPosStr)) {
           if ( *newPosStr == ':' ) // Ah, so there was no year, but the number was the hour
               year = -1;
           else
               return invalidDate;
        } else // in the normal case (we parsed the year), advance to the next number
            dateString = ++newPosStr;

        hour = strtol(dateString, &newPosStr, 10);

        // Do not check for errno here since we want to continue
        // even if errno was set becasue we are still looking
        // for the timezone!
        // read a number? if not this might be a timezone name
        if (newPosStr != dateString) {
          have_time = true;
          dateString = newPosStr;

          if ((hour < 0) || (hour > 23))
            return invalidDate;

          if (!*dateString)
            return invalidDate;

          // ':12:40 GMT'
          if (*dateString++ != ':')
            return invalidDate;

          minute = strtol(dateString, &newPosStr, 10);
          if (errno)
            return invalidDate;
          dateString = newPosStr;

          if ((minute < 0) || (minute > 59))
            return invalidDate;

          // ':40 GMT'
          if (*dateString && *dateString != ':' && !isspace(*dateString))
            return invalidDate;

          // seconds are optional in rfc822 + rfc2822
          if (*dateString ==':') {
            dateString++;

            second = strtol(dateString, &newPosStr, 10);
            if (errno)
              return invalidDate;
            dateString = newPosStr;

            if ((second < 0) || (second > 59))
              return invalidDate;
          }

          while(*dateString && isspace(*dateString))
            dateString++;
        }
     } else {
       dateString = newPosStr;
     }

     // don't fail if the time zone is missing, some
     // broken mail-/news-clients omit the time zone
     if (*dateString) {

       if ( (dateString[0] == 'G' && dateString[1] == 'M' && dateString[2] == 'T')
            || (dateString[0] == 'U' && dateString[1] == 'T' && dateString[2] == 'C') )
       {
         dateString += 3;
         have_tz = true;
       }

       while (*dateString && isspace(*dateString))
         ++dateString;

       if (strncasecmp(dateString, "GMT", 3) == 0) {
         dateString += 3;
       }
       if ((*dateString == '+') || (*dateString == '-')) {
         offset = strtol(dateString, &newPosStr, 10);
         if (errno)
           return invalidDate;
         dateString = newPosStr;

         if ((offset < -9959) || (offset > 9959))
            return invalidDate;

         int sgn = (offset < 0)? -1:1;
         offset = abs(offset);
         if ( *dateString == ':' ) { // GMT+05:00
           int offset2 = strtol(dateString, &newPosStr, 10);
           if (errno)
             return invalidDate;
           dateString = newPosStr;
           offset = (offset*60 + offset2)*sgn;
         }
         else
           offset = ((offset / 100)*60 + (offset % 100))*sgn;
         have_tz = true;
       } else {
         for (int i=0; known_zones[i].tzName != 0; i++) {
           if (0 == strncasecmp(dateString, known_zones[i].tzName, strlen(known_zones[i].tzName))) {
             offset = known_zones[i].tzOffset;
             dateString += strlen(known_zones[i].tzName);
             have_tz = true;
             break;
           }
         }
       }
     }

     while(*dateString && isspace(*dateString))
        dateString++;

     if ( *dateString && year == -1 ) {
       year = strtol(dateString, &newPosStr, 10);
       if (errno)
         return invalidDate;
       dateString = newPosStr;
     }

     while (isspace(*dateString))
       dateString++;

#if 0
     // Trailing garbage
     if (*dateString != '\0')
       return invalidDate;
#endif

     // Y2K: Solve 2 digit years
     if ((year >= 0) && (year < 50))
         year += 2000;

     if ((year >= 50) && (year < 100))
         year += 1900;  // Y2K

     if (!have_tz) {
       // fall back to midnight, local timezone
       struct tm t;
       memset(&t, 0, sizeof(tm));
       t.tm_mday = day;
       t.tm_mon = month;
       t.tm_year = year - 1900;
       t.tm_isdst = -1;
       if (have_time) {
         t.tm_sec = second;
         t.tm_min = minute;
         t.tm_hour = hour;
       }

       // better not use mktime() as it can't handle the full year range
       return makeTime(&t, 0, false) / 1000.0;
     }

     result = ymdhms_to_seconds(year, month+1, day, hour, minute, second) - offset*60;
     return result;
}


double KJS::timeClip(double t)
{
  if (isInf(t))
    return NaN;
  double at = fabs(t);
  if (at > 8.64E15)
    return NaN;
  return floor(at) * (t != at ? -1 : 1);
}

