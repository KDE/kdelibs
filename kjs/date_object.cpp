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
#include <ctype.h>

#include "date_object.h"
#include "error_object.h"
#include "operations.h"

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
  : DateInstanceImp(Object(objectProto))
{
  Value protect(this);
  setInternalValue(Number(NaN));
  // The constructor will be added later, after DateObjectImp has been built
}

Value DatePrototypeImp::get(ExecState *exec, const UString &propertyName) const
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
  CString oldlocale = setlocale(LC_TIME,NULL);
  if (!oldlocale.c_str())
    oldlocale = setlocale(LC_ALL, NULL);
  Value v = thisObj.internalValue();
  double milli = v.toNumber(exec);
  time_t tv = (time_t) floor(milli / 1000.0);
  int ms = int(milli - tv * 1000.0);

  // As long as we're using time_t we need to 'truncate' to avoid 'wrapping'.
  // Real long term solutions include: writing our own 64-bit-based date/time class,
  // using wxWindow's datetime.cpp (in wxBase), using QDateTime... or shifting
  // to a time_t range by substracting a big enough number of years....
  if (sizeof(time_t) == 4)
  {
    // If time_t is signed, the bigger it can be is 2^31-1
    if ( (time_t)-1 < 0 ) {
      if ( floor(milli / 1000.0) > ((double)((uint)1<<31)-1) ) {
#ifdef KJS_VERBOSE
        fprintf(stderr, "date above time_t limit. Year seems to be %d\n", (int)(milli/(1000.0*365.25*86400)+1970));
#endif
        tv = ((uint)1<<31)-1;
        ms = 0;
      }
    }
    else
      // time_t is unsigned, the bigger it can be is 2^32-1, aka (uint)-1
      if ( floor(milli / 1000.0) > ((double)(uint)-1) ) {
#ifdef KJS_VERBOSE
        fprintf(stderr, "date above time_t limit. Year seems to be %d\n", (int)(milli/(1000.0*365.25*86400)+1970));
#endif
        tv = (uint)-1;
        ms = 0;
      }
  }

  struct tm *t;
  if (utc)
    t = gmtime(&tv);
  else
    t = localtime(&tv);

  switch (id) {
  case ToString:
  case ToDateString:
  case ToTimeString:
  case ToGMTString:
  case ToUTCString:
    setlocale(LC_TIME,"C");
    if (id == DateProtoFuncImp::ToDateString) {
      strftime(timebuffer, bufsize, "%x",t);
    } else if (id == DateProtoFuncImp::ToTimeString) {
      strftime(timebuffer, bufsize, "%X",t);
    } else { // ToString, toGMTString & toUTCString
      t = (id == ToString ? localtime(&tv) : gmtime(&tv));
      strftime(timebuffer, bufsize, "%a, %d %b %Y %H:%M:%S %z", t);
    }
    setlocale(LC_TIME,oldlocale.c_str());
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
    // IE returns the full year even in getYear.
    // Let's do that too, it's more sensible.
    if ( exec->interpreter()->compatMode() == Interpreter::NetscapeCompat )
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
#if defined BSD || defined(__APPLE__)
    result = Number(-(t->tm_gmtoff / 60) + (t->tm_isdst > 0 ? 60 : 0));
#else
#  if defined(__BORLANDC__)
#error please add daylight savings offset here!
    result = Number(_timezone / 60 - (t->tm_isdst > 0 ? 60 : 0));
#  else
    result = Number((timezone / 60 - (t->tm_isdst > 0 ? 60 : 0 )));
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
  put(exec,"prototype", Object(dateProto), DontEnum|DontDelete|ReadOnly);

  put(exec,"parse", Object(new DateObjectFuncImp(exec,funcProto,DateObjectFuncImp::Parse, 1)), DontEnum);
  put(exec,"UTC",   Object(new DateObjectFuncImp(exec,funcProto,DateObjectFuncImp::UTC,   7)),   DontEnum);

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
  int numArgs = args.size();

#ifdef KJS_VERBOSE
  fprintf(stderr,"DateObjectImp::construct - %d args\n", numArgs);
#endif
  Value value;

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
    UString s = args[0].toString(exec);
    double d = s.toDouble();
    if (isNaN(d))
      value = parseDate(s);
    else
      value = Number(d);
  } else {
    struct tm t;
    memset(&t, 0, sizeof(t));
    int year = args[0].toInt32(exec);
    // TODO: check for NaN
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

  Object proto = exec->interpreter()->builtinDatePrototype();
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
#ifdef KJS_VERBOSE
  fprintf(stderr,"DateObjectImp::call - current time\n");
#endif
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
  else { // UTC
    struct tm t;
    memset(&t, 0, sizeof(t));
    int n = args.size();
    int year = args[0].toInt32(exec);
    // TODO: check for NaN
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
#ifdef KJS_VERBOSE
  fprintf(stderr,"KJS::parseDate %s\n",u.ascii());
#endif
  double /*time_t*/ seconds = KRFCDate_parseDate( u );
#ifdef KJS_VERBOSE
  fprintf(stderr,"KRFCDate_parseDate returned seconds=%g\n",seconds);
  bool withinLimits = true;
  if ( sizeof(time_t) == 4 )
  {
    int limit = ((time_t)-1 < 0) ? 2038 : 2115;
    if ( seconds > (limit-1970) * 365.25 * 86400 ) {
      fprintf(stderr, "date above time_t limit. Year seems to be %d\n", (int)(seconds/(365.25*86400)+1970));
      withinLimits = false;
    }
  }
  if ( withinLimits ) {
    time_t lsec = (time_t)seconds;
    fprintf(stderr, "this is: %s\n", ctime(&lsec));
  }
#endif

  if ( seconds == -1 )
    return Undefined();
  else
    return Number(seconds * 1000.0);
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

static const char haystack[37]="janfebmaraprmayjunjulaugsepoctnovdec";

// we follow the recommendation of rfc2822 to consider all
// obsolete time zones not listed here equivalent to "-0000"
static const struct {
    const char *tzName;
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
    { 0, 0 }
};

int KJS::local_timeoffset()
{
     static int local_offset = -1;

     if ( local_offset != -1 ) return local_offset;

     time_t local = time(0);
     struct tm* tm_local = gmtime(&local);
     local_offset = local-mktime(tm_local);
     if(tm_local->tm_isdst)
       local_offset += 3600;

     return local_offset;
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
     // ### non RFC format, added for Javascript:
     //     [Wednesday] January 09 1999 23:12:40 GMT
     //
     // We ignore the weekday
     //
     double result = 0;
     int offset = 0;
     bool have_tz = false;
     char *newPosStr;
     const char *dateString = _date.ascii();
     int day = 0;
     char monthStr[4];
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
          monthStr[0] = tolower(*wordStart++);
          monthStr[1] = tolower(*wordStart++);
          monthStr[2] = tolower(*wordStart++);
          monthStr[3] = '\0';
          //fprintf(stderr,"KJS::parseDate found word starting with '%s'\n", monthStr);
          const char *str = strstr(haystack, monthStr);
          if (str)
            month = (str-haystack)/3; // Jan=00, Feb=01, Mar=02, ..
          while(*dateString && isspace(*dateString))
             dateString++;
          wordStart = dateString;
        }
        else
           dateString++;
     }

     while(*dateString && isspace(*dateString))
     	dateString++;

     if (!*dateString)
     	return result;  // Invalid date

     // ' 09-Nov-99 23:12:40 GMT'
     day = strtol(dateString, &newPosStr, 10);
     dateString = newPosStr;

     if ((day < 1) || (day > 31))
     	return result; // Invalid date;
     if (!*dateString)
     	return result;  // Invalid date

     if (*dateString == '/' && day <= 12 && month == -1)
     {
     	dateString++;
        // This looks like a MM/DD/YYYY date, not an RFC date.....
        month = day - 1; // 0-based
        day = strtol(dateString, &newPosStr, 10);
        dateString = newPosStr;
        if (*dateString == '/')
          dateString++;
        if (!*dateString)
          return result;  // Invalid date
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
         for(int i=0; i < 3;i++)
         {
           if (!*dateString || (*dateString == '-') || isspace(*dateString))
             return result;  // Invalid date
           monthStr[i] = tolower(*dateString++);
         }
         monthStr[3] = '\0';

         newPosStr = (char*)strstr(haystack, monthStr);

         if (!newPosStr)
           return result;  // Invalid date

         month = (newPosStr-haystack)/3; // Jan=00, Feb=01, Mar=02, ..

         if ((month < 0) || (month > 11))
           return result;  // Invalid date

         while(*dateString && (*dateString != '-') && !isspace(*dateString))
           dateString++;

         if (!*dateString)
           return result;  // Invalid date

         // '-99 23:12:40 GMT'
         if ((*dateString != '-') && (*dateString != '/') && !isspace(*dateString))
           return result;  // Invalid date
         dateString++;
       }

       if ((month < 0) || (month > 11))
         return result;  // Invalid date
     }

     // '99 23:12:40 GMT'
     year = strtol(dateString, &newPosStr, 10);
     dateString = newPosStr;

     // Y2K: Solve 2 digit years
     if ((year >= 0) && (year < 50))
         year += 2000;

     if ((year >= 50) && (year < 100))
         year += 1900;  // Y2K

     if ((year < 1900) || (year > 2500))
     	return result; // Invalid date

     // Don't fail if the time is missing.
     if (*dateString)
     {
        // ' 23:12:40 GMT'
        if (!isspace(*dateString++))
           return false;  // Invalid date

        have_time = true;
        hour = strtol(dateString, &newPosStr, 10);
        dateString = newPosStr;

        if ((hour < 0) || (hour > 23))
           return result; // Invalid date

        if (!*dateString)
           return result;  // Invalid date

        // ':12:40 GMT'
        if (*dateString++ != ':')
           return result;  // Invalid date

        minute = strtol(dateString, &newPosStr, 10);
        dateString = newPosStr;

        if ((minute < 0) || (minute > 59))
           return result; // Invalid date

        // ':40 GMT'
        if (*dateString && *dateString != ':' && !isspace(*dateString))
           return result;  // Invalid date

        // seconds are optional in rfc822 + rfc2822
        if (*dateString ==':') {
           dateString++;

           second = strtol(dateString, &newPosStr, 10);
           dateString = newPosStr;

           if ((second < 0) || (second > 59))
              return result; // Invalid date
        }

        while(*dateString && isspace(*dateString))
           dateString++;
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

       if ((*dateString == '+') || (*dateString == '-')) {
         offset = strtol(dateString, &newPosStr, 10);
         dateString = newPosStr;

         if ((offset < -9959) || (offset > 9959))
           return result; // Invalid date

         int sgn = (offset < 0)? -1:1;
         offset = abs(offset);
         if ( *dateString == ':' ) { // GMT+05:00
           int offset2 = strtol(dateString, &newPosStr, 10);
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
             have_tz = true;
             break;
           }
         }
       }
     }

#if 0
     if (sizeof(time_t) == 4)
     {
         if ((time_t)-1 < 0)
         {
            if (year >= 2038)
            {
               year = 2038;
               month = 0;
               day = 1;
               hour = 0;
               minute = 0;
               second = 0;
            }
         }
         else
         {
            if (year >= 2115)
            {
               year = 2115;
               month = 0;
               day = 1;
               hour = 0;
               minute = 0;
               second = 0;
            }
         }
     }
#endif

     if (!have_time && !have_tz) {
       // fall back to midnight, local timezone
       struct tm t;
       memset(&t, 0, sizeof(tm));
       t.tm_mday = day;
       t.tm_mon = month;
       t.tm_year = year - 1900;
       t.tm_isdst = -1;
       return mktime(&t);
     }

     if(!have_tz)
       offset = local_timeoffset();
     else
       offset *= 60;

     result = ymdhms_to_seconds(year, month+1, day, hour, minute, second);

     // avoid negative time values
     if ((offset > 0) && (offset > result))
        offset = 0;

     result -= offset;

     // If epoch 0 return epoch +1 which is Thu, 01-Jan-70 00:00:01 GMT
     // This is so that parse error and valid epoch 0 return values won't
     // be the same for sensitive applications...
     if (result < 1) result = 1;

     return result;
}


Value KJS::timeClip(const Value &t)
{
  /* TODO */
  return t;
}

