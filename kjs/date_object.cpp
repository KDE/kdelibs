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

#include "kjs.h"
#include "date_object.h"

#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

namespace KJS {

  class DateObjectFunc : public InternalFunctionImp {
  public:
    DateObjectFunc(int i) : id(i) { };
    Completion execute(const List &);
    enum { Parse, UTC };
  private:
    int id;
  };

  class DateProtoFunc : public InternalFunctionImp {
  public:
    DateProtoFunc(int i, bool u);
    Completion execute(const List &);
    enum { ToString, ToDateString, ToTimeString, ToLocaleString,
	   ToLocaleDateString, ToLocaleTimeString, ValueOf, GetTime,
	   GetFullYear, GetMonth, GetDate, GetDay, GetHours, GetMinutes,
	   GetSeconds, GetMilliSeconds, GetTimezoneOffset, SetTime,
	   SetMilliSeconds, SetSeconds, SetMinutes, SetHours, SetDate,
	   SetMonth, SetFullYear,
	   // non-normative properties (Appendix B)
	   GetYear, SetYear, toGMTString };
  private:
    int id;
    bool utc;
  };

  // helper functions
  KJSO parseDate(const KJSO &s);
  KJSO timeClip(const KJSO &t);
};

using namespace KJS;

KJSO KJS::parseDate(const KJSO &)
{
  /* TODO */
  return Number(0);
}

KJSO KJS::timeClip(const KJSO &t)
{
  /* TODO */
  return t;
}

// ECMA 15.9.2
Completion DateObject::execute(const List &)
{
  time_t t = time(0L);
  UString s(ctime(&t));

  // return formatted string minus trailing \n
  return Completion(Normal, String(s.substr(0, s.size() - 1)));
}

// ECMA 15.9.3
Object DateObject::construct(const List &args)
{
  KJSO value;
  double utc;

  int numArgs = args.size();

  if (numArgs == 0) { // new Date() ECMA 15.9.3.3
    struct timeval tv;
    gettimeofday(&tv, 0L);
    utc = floor((double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0);
    value = Number(utc);
  } else if (numArgs == 1) {
    KJSO p = args[0].toPrimitive();
    if (p.isA(StringType))
      value = parseDate(p);
    else
      value = p.toNumber();
  } else {
    struct tm t;
    Number y = args[0].toNumber();
    /* TODO: check for NaN */
    int year = y.toInt32();
    t.tm_year = (year >= 0 && year <= 99) ? year : year - 1900;
    t.tm_mon = args[1].toInt32();
    t.tm_mday = (numArgs >= 3) ? args[2].toInt32() : 1;
    t.tm_hour = (numArgs >= 4) ? args[3].toInt32() : 0;
    t.tm_min = (numArgs >= 5) ? args[4].toInt32() : 0;
    t.tm_sec = (numArgs >= 6) ? args[5].toInt32() : 0;
    int ms = (numArgs >= 7) ? args[6].toInt32() : 0;
    value = Number(mktime(&t) * 1000.0 + ms);
  }

  return Object::create(DateClass, timeClip(value));
}

KJSO DateObject::get(const UString &p) const
{
  int id;

  if (p == "parse")
    id = DateObjectFunc::Parse;
  else if (p == "UTC")
    id = DateObjectFunc::UTC;
  else 
    return Imp::get(p);

  return Function(new DateObjectFunc(id));
}

// ECMA 15.9.4.2 - 3
Completion DateObjectFunc::execute(const List &args)
{
  KJSO result;

  if (id == Parse)
    result = parseDate(args[0]);
  else {
    struct tm t;
    int n = args.size();
    Number y = args[0].toNumber();
    /* TODO: check for NaN */
    int year = y.toInt32();
    t.tm_year = (year >= 0 && year <= 99) ? year : year - 1900;
    t.tm_mon = args[1].toInt32();
    t.tm_mday = (n >= 3) ? args[2].toInt32() : 1;
    t.tm_hour = (n >= 4) ? args[3].toInt32() : 0;
    t.tm_min = (n >= 5) ? args[4].toInt32() : 0;
    t.tm_sec = (n >= 6) ? args[5].toInt32() : 0;
    int ms = (n >= 7) ? args[6].toInt32() : 0;
    result = Number(mktime(&t) * 1000.0 + ms);
  }

  return Completion(Normal,result);
}

// ECMA 15.9.4
DatePrototype::DatePrototype(const Object& proto)
  : ObjectImp(DateClass, Number(NaN), proto)
{
  // The constructor will be added later in DateObject's constructor
}

KJSO DatePrototype::get(const UString &p) const
{
  int id;

  if (p == "toString")
    id = DateProtoFunc::ToString;
  else if (p == "toDateString")
    id = DateProtoFunc::ToDateString;
  else if (p == "toTimeString")
    id = DateProtoFunc::ToTimeString;
  else if (p == "toLocaleString")
    id = DateProtoFunc::ToLocaleString;
  else if (p == "toLocaleDateString")
    id = DateProtoFunc::ToLocaleDateString;
  else if (p == "toLocaleTimeString")
    id = DateProtoFunc::ToLocaleTimeString;
  else if (p == "valueOf")
    id = DateProtoFunc::ValueOf;
  else if (p == "getTime")
    id = DateProtoFunc::GetTime;
  else if (p == "getFullYear" || p == "getUTCFullYear")
    id = DateProtoFunc::GetFullYear;
  else if (p == "getMonth" || p == "getUTCMonth")
    id = DateProtoFunc::GetMonth;
  else if (p == "getDate" || p == "getUTCDate")
    id = DateProtoFunc::GetDate;
  else if (p == "getDay" || p == "getUTCDay")
    id = DateProtoFunc::GetDay;
  else if (p == "getHours" || p == "getUTCHours")
    id = DateProtoFunc::GetHours;
  else if (p == "getMinutes" || p == "getUTCMinutes")
    id = DateProtoFunc::GetMinutes;
  else if (p == "getSeconds" || p == "getUTCSeconds")
    id = DateProtoFunc::GetSeconds;
  else if (p == "getMilliSeconds" || p == "getUTCMilliSeconds")
    id = DateProtoFunc::GetMilliSeconds;
  else if (p == "getTimezoneOffset")
    id = DateProtoFunc::GetTimezoneOffset;
  else if (p == "setTime")
    id = DateProtoFunc::SetTime;
  else if (p == "setMilliSeconds" || p == "setUTCMilliSeconds")
    id = DateProtoFunc::SetMilliSeconds;
  else if (p == "setSeconds" || p == "setUTCSeconds")
    id = DateProtoFunc::SetSeconds;
  else if (p == "setMinutes" || p == "setUTCMinutes")
    id = DateProtoFunc::SetMinutes;
  else if (p == "setHours" || p == "setUTCHours")
    id = DateProtoFunc::SetHours;
  else if (p == "setDate" || p == "setUTCDate")
    id = DateProtoFunc::SetDate;
  else if (p == "setMonth" || p == "setUTCMonth")
    id = DateProtoFunc::SetMonth;
  else if (p == "setFullYear" || p == "setUTCFullYear")
    id = DateProtoFunc::SetFullYear;
  // non-normative
  else if (p == "getYear")
    id = DateProtoFunc::GetYear;
  else
    return Undefined();

  bool utc = (p.find("UTC") >= 0) ? true : false;
  return Function(new DateProtoFunc(id, utc));
}

DateProtoFunc::DateProtoFunc(int i, bool u) : id(i), utc(u) 
{
}

Completion DateProtoFunc::execute(const List &)
{
  KJSO result;
  UString s;
  Object thisObj = Object::dynamicCast(thisValue());
  KJSO v = thisObj.internalValue();
  double milli = v.toNumber().value();
  time_t tv = (time_t) floor(milli / 1000.0);
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
  case ToLocaleString:
  case ToLocaleDateString:
  case ToLocaleTimeString:
    /* TODO */
    result = Undefined();
    break;
  case ValueOf:
    result = Number(milli);
    break;
  case GetTime:
    if (thisObj.getClass() == DateClass)
      result = Number(milli);
    else
      result = Error::create(TypeError);
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
  case GetTimezoneOffset:
  case SetTime:
  case SetMilliSeconds:
  case SetSeconds:
  case SetMinutes:
  case SetHours:
  case SetDate:
  case SetMonth:
  case SetFullYear:
    /* TODO */
    result = Undefined();
    break;
  }

  return Completion(Normal, result);
}
