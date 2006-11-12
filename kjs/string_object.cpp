// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "value.h"
#include "object.h"
#include "types.h"
#include "interpreter.h"
#include "operations.h"
#include "regexp.h"
#include "regexp_object.h"
#include "string_object.h"
#include "error_object.h"
#include <stdio.h>
#include "string_object.lut.h"

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_BITYPES_H
#include <sys/bitypes.h> /* For uintXX_t on Tru64 */
#endif

using namespace KJS;

// ------------------------------ StringInstanceImp ----------------------------

const ClassInfo StringInstanceImp::info = {"String", 0, 0, 0};

StringInstanceImp::StringInstanceImp(ObjectImp *proto)
  : ObjectImp(proto)
{
  setInternalValue(String(""));
}

StringInstanceImp::StringInstanceImp(ObjectImp *proto, const UString &string)
  : ObjectImp(proto)
{
  setInternalValue(String(string));
}

Value StringInstanceImp::get(ExecState *exec, const Identifier &propertyName) const
{
  if (propertyName == lengthPropertyName)
    return Number(internalValue().toString(exec).size());

  bool ok;
  const unsigned index = propertyName.toArrayIndex(&ok);
  if (ok) {
    const UString s = internalValue().toString(exec);
    const unsigned length = s.size();
    if (index < length) {
      const UChar c = s[index];
      return String(UString(&c, 1));
    }
  }

  return ObjectImp::get(exec, propertyName);
}

void StringInstanceImp::put(ExecState *exec, const Identifier &propertyName, const Value &value, int attr)
{
  if (propertyName == lengthPropertyName)
    return;
  ObjectImp::put(exec, propertyName, value, attr);
}

bool StringInstanceImp::hasProperty(ExecState *exec, const Identifier &propertyName) const
{
  if (propertyName == lengthPropertyName)
    return true;

  bool ok;
  unsigned index = propertyName.toULong(&ok);
  if (ok && index < (unsigned)internalValue().toString(exec).size())
    return true;

  return ObjectImp::hasProperty(exec, propertyName);
}

bool StringInstanceImp::deleteProperty(ExecState *exec, const Identifier &propertyName)
{
  if (propertyName == lengthPropertyName)
    return false;

  bool ok;
  unsigned index = propertyName.toULong(&ok);
  if (ok && index < (unsigned)internalValue().toString(exec).size())
    return false;

  return ObjectImp::deleteProperty(exec, propertyName);
}

ReferenceList StringInstanceImp::propList(ExecState *exec, bool recursive)
{
  ReferenceList properties = ObjectImp::propList(exec,recursive);

  UString str = internalValue().toString(exec);
  for (int i = 0; i < str.size(); i++)
    if (!ObjectImp::hasProperty(exec,Identifier::from(i)))
      properties.append(Reference(this, i));

  return properties;
}

// ------------------------------ StringPrototypeImp ---------------------------
const ClassInfo StringPrototypeImp::info = {"String", &StringInstanceImp::info, &stringTable, 0};
/* Source for string_object.lut.h
@begin stringTable 28
  toString		StringProtoFuncImp::ToString	DontEnum|Function	0
  valueOf		StringProtoFuncImp::ValueOf	DontEnum|Function	0
  charAt		StringProtoFuncImp::CharAt	DontEnum|Function	1
  charCodeAt		StringProtoFuncImp::CharCodeAt	DontEnum|Function	1
  concat		StringProtoFuncImp::Concat	DontEnum|Function	1
  indexOf		StringProtoFuncImp::IndexOf	DontEnum|Function	1
  lastIndexOf		StringProtoFuncImp::LastIndexOf	DontEnum|Function	1
  match			StringProtoFuncImp::Match	DontEnum|Function	1
  replace		StringProtoFuncImp::Replace	DontEnum|Function	2
  search		StringProtoFuncImp::Search	DontEnum|Function	1
  slice			StringProtoFuncImp::Slice	DontEnum|Function	2
  split			StringProtoFuncImp::Split	DontEnum|Function	2
  substr		StringProtoFuncImp::Substr	DontEnum|Function	2
  substring		StringProtoFuncImp::Substring	DontEnum|Function	2
  toLowerCase		StringProtoFuncImp::ToLowerCase	DontEnum|Function	0
  toUpperCase		StringProtoFuncImp::ToUpperCase	DontEnum|Function	0
  toLocaleLowerCase	StringProtoFuncImp::ToLocaleLowerCase DontEnum|Function	0
  toLocaleUpperCase     StringProtoFuncImp::ToLocaleUpperCase DontEnum|Function	0
#
# Under here: html extension, should only exist if KJS_PURE_ECMA is not defined
# I guess we need to generate two hashtables in the .lut.h file, and use #ifdef
# to select the right one... TODO. #####
  big			StringProtoFuncImp::Big		DontEnum|Function	0
  small			StringProtoFuncImp::Small	DontEnum|Function	0
  blink			StringProtoFuncImp::Blink	DontEnum|Function	0
  bold			StringProtoFuncImp::Bold	DontEnum|Function	0
  fixed			StringProtoFuncImp::Fixed	DontEnum|Function	0
  italics		StringProtoFuncImp::Italics	DontEnum|Function	0
  strike		StringProtoFuncImp::Strike	DontEnum|Function	0
  sub			StringProtoFuncImp::Sub		DontEnum|Function	0
  sup			StringProtoFuncImp::Sup		DontEnum|Function	0
  fontcolor		StringProtoFuncImp::Fontcolor	DontEnum|Function	1
  fontsize		StringProtoFuncImp::Fontsize	DontEnum|Function	1
  anchor		StringProtoFuncImp::Anchor	DontEnum|Function	1
  link			StringProtoFuncImp::Link	DontEnum|Function	1
@end
*/
// ECMA 15.5.4
StringPrototypeImp::StringPrototypeImp(ExecState * /*exec*/,
                                       ObjectPrototypeImp *objProto)
  : StringInstanceImp(objProto)
{
  Value protect(this);
  // The constructor will be added later, after StringObjectImp has been built
  putDirect(lengthPropertyName, NumberImp::zero(), DontDelete|ReadOnly|DontEnum);

}

Value StringPrototypeImp::get(ExecState *exec, const Identifier &propertyName) const
{
  return lookupGetFunction<StringProtoFuncImp, StringInstanceImp>( exec, propertyName, &stringTable, this );
}

// ------------------------------ StringProtoFuncImp ---------------------------

StringProtoFuncImp::StringProtoFuncImp(ExecState *exec, int i, int len)
  : InternalFunctionImp(
    static_cast<FunctionPrototypeImp*>(exec->lexicalInterpreter()->builtinFunctionPrototype().imp())
    ), id(i)
{
  Value protect(this);
  putDirect(lengthPropertyName, len, DontDelete|ReadOnly|DontEnum);
}

bool StringProtoFuncImp::implementsCall() const
{
  return true;
}

// ECMA 15.5.4.2 - 15.5.4.20
Value StringProtoFuncImp::call(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;

  // toString and valueOf are no generic functions.
  if (id == ToString || id == ValueOf) {
    KJS_CHECK_THIS( StringInstanceImp, thisObj );

    return String(thisObj.internalValue().toString(exec));
  }

  int n, m;
  UString u2, u3;
  double dpos;
  int pos, p0, i;
  double d = 0.0;

  UString s = thisObj.toString(exec);

  int len = s.size();
  Value a0 = args[0];
  Value a1 = args[1];

  switch (id) {
  case ToString:
  case ValueOf:
    // handled above
    break;
  case CharAt:
    pos = a0.type() == UndefinedType ? 0 : a0.toInteger(exec);
    if (pos < 0 || pos >= len)
      s = "";
    else
      s = s.substr(pos, 1);
    result = String(s);
    break;
  case CharCodeAt:
    pos = a0.type() == UndefinedType ? 0 : a0.toInteger(exec);
    if (pos < 0 || pos >= len)
      d = NaN;
    else {
      UChar c = s[pos];
      d = (c.high() << 8) + c.low();
    }
    result = Number(d);
    break;
  case Concat: {
    ListIterator it = args.begin();
    for ( ; it != args.end() ; ++it) {
        s += it->dispatchToString(exec);
    }
    result = String(s);
    break;
  }
  case IndexOf:
    u2 = a0.toString(exec);
    if (a1.type() == UndefinedType)
      pos = 0;
    else
      pos = a1.toInteger(exec);
    d = s.find(u2, pos);
    result = Number(d);
    break;
  case LastIndexOf:
    u2 = a0.toString(exec);
    d = a1.toNumber(exec);
    if (a1.type() == UndefinedType || KJS::isNaN(d))
      dpos = len;
    else {
      dpos = d;
      if (dpos < 0)
        dpos = 0;
      else if (dpos > len)
        dpos = len;
    }
    result = Number(s.rfind(u2, int(dpos)));
    break;
  case Match:
  case Search: {
    RegExp *reg, *tmpReg = 0;
    RegExpImp *imp = 0;
    if (a0.isA(ObjectType) && a0.toObject(exec).inherits(&RegExpImp::info))
    {
      imp = static_cast<RegExpImp *>( a0.toObject(exec).imp() );
      reg = imp->regExp();
    }
    else
    { /*
       *  ECMA 15.5.4.12 String.prototype.search (regexp)
       *  If regexp is not an object whose [[Class]] property is "RegExp", it is
       *  replaced with the result of the expression new RegExp(regexp).
       */
      reg = tmpReg = new RegExp(a0.toString(exec), RegExp::None);
    }
    RegExpObjectImp* regExpObj = static_cast<RegExpObjectImp*>(exec->interpreter()->builtinRegExp().imp());
    int **ovector = regExpObj->registerRegexp(reg, s);
    reg->prepareMatch(s);
    UString mstr = reg->match(s, -1, &pos, ovector);
    if (id == Search) {
      result = Number(pos);
    } else { // Match
      if (mstr.isNull()) {
        result = Null(); // no match
      } else if ((reg->flags() & RegExp::Global) == 0) {
	// case without 'g' flag is handled like RegExp.prototype.exec
	regExpObj->setSubPatterns(reg->subPatterns());
	result = regExpObj->arrayOfMatches(exec,mstr);
      } else {
	// return array of matches
	List list;
	while (pos >= 0) {
	  list.append(String(mstr));
	  pos += mstr.isEmpty() ? 1 : mstr.size();
	  delete [] *ovector;
	  mstr = reg->match(s, pos, &pos, ovector);
	}
	result = exec->lexicalInterpreter()->builtinArray().construct(exec, list);
      }
    }
    reg->doneMatch();
    delete tmpReg;
    break;
  }
  case Replace:
    if (a0.type() == ObjectType && a0.toObject(exec).inherits(&RegExpImp::info)) {
      RegExpImp* imp = static_cast<RegExpImp *>( a0.toObject(exec).imp() );
      RegExp *reg = imp->regExp();
      bool global = false;
      Value tmp = imp->get(exec,"global");
      if (tmp.type() != UndefinedType && tmp.toBoolean(exec) == true)
        global = true;

      RegExpObjectImp* regExpObj = static_cast<RegExpObjectImp*>(exec->lexicalInterpreter()->builtinRegExp().imp());
      int lastIndex = 0;
      Object o1;
      // Test if 2nd arg is a function (new in JS 1.3)
      if ( a1.type() == ObjectType && a1.toObject(exec).implementsCall() )
        o1 = a1.toObject(exec);
      else
        u3 = a1.toString(exec); // 2nd arg is the replacement string

      UString out;
      
      // This is either a loop (if global is set) or a one-way (if not).
      reg->prepareMatch(s);
      do {
        int **ovector = regExpObj->registerRegexp( reg, s );
        UString mstr = reg->match(s, lastIndex, &pos, ovector);
        regExpObj->setSubPatterns(reg->subPatterns());
        if (pos == -1)
          break;
          
        len = mstr.size();

        UString rstr;
        // Prepare replacement
        if (!o1.isValid())
        {
          rstr = u3;
          bool ok;
          // check if u3 matches $1 or $2 etc
          for (int i = 0; (i = rstr.find(UString("$"), i)) != -1; i++) {
            if (i+1<rstr.size() && rstr[i+1] == '$') {  // "$$" -> "$"
              rstr = rstr.substr(0,i) + "$" + rstr.substr(i+2);
              continue;
            }
            // Assume number part is one char exactly
            unsigned long pos = rstr.substr(i+1,1).toULong(&ok, false /* tolerate empty string */);
            if (ok && pos <= (unsigned)reg->subPatterns()) {
              rstr = rstr.substr(0,i)
                     + s.substr((*ovector)[2*pos],
                                (*ovector)[2*pos+1]-(*ovector)[2*pos])
                     + rstr.substr(i+2);
              i += (*ovector)[2*pos+1]-(*ovector)[2*pos] - 1; // -1 offsets i++
            }
          }
        } else // 2nd arg is a function call. Spec from http://devedge.netscape.com/library/manuals/2000/javascript/1.5/reference/string.html#1194258
        {
          List l;
          l.append(String(mstr)); // First arg: complete matched substring
          // Then the submatch strings
          for ( unsigned int sub = 1; sub <= reg->subPatterns() ; ++sub )
            l.append( String( s.substr((*ovector)[2*sub],
                               (*ovector)[2*sub+1]-(*ovector)[2*sub]) ) );
          l.append(Number(pos)); // The offset within the string where the match occurred
          l.append(String(s)); // Last arg: the string itself. Can't see the difference with the 1st arg!
          Object thisObj = exec->interpreter()->globalObject();
          rstr = o1.call( exec, thisObj, l ).toString(exec);
        }

        // Append the stuff we skipped over to get to the match --
        // that would be [lastIndex, pos) of the original..
        if (pos != lastIndex)
          out += s.substr(lastIndex, pos - lastIndex);

        // Append the replacement..
        out += rstr;
        
        lastIndex = pos + len; // Skip over the matched stuff...
      } while (global);

      // Append the rest of the string to the output...
      if (lastIndex == 0 && out.size() == 0) // Don't copy stuff if nothing changed
        out = s;
      else
        out += s.substr(lastIndex, s.size() - lastIndex);

      reg->doneMatch();

      result = String(out);
    } else { // First arg is a string
      u2 = a0.toString(exec);
      pos = s.find(u2);
      len = u2.size();
      // Do the replacement
      if (pos == -1)
        result = String(s);
      else {
        u3 = s.substr(0, pos) + a1.toString(exec) +
             s.substr(pos + len);
        result = String(u3);
      }
    }
    break;
  case Slice: // http://developer.netscape.com/docs/manuals/js/client/jsref/string.htm#1194366 or 15.5.4.13
    {
        // The arg processing is very much like ArrayProtoFunc::Slice
        int begin = args[0].toUInt32(exec);
        int end = len;
        if (args[1].type() != UndefinedType) {
          end = args[1].toInteger(exec);
        }
        int from = begin < 0 ? len + begin : begin;
        int to = end < 0 ? len + end : end;
        if (to > from && to > 0 && from < len) {
          if (from < 0) {
            from = 0;
          }
          if (to > len) {
            to = len;
          }
          result = String(s.substr(from, to - from));
        } else {
          result = String("");
        }
        break;
    }
    case Split: {
    Object constructor = exec->lexicalInterpreter()->builtinArray();
    Object res = Object::dynamicCast(constructor.construct(exec,List::empty()));
    result = res;
    i = p0 = 0;
    uint32_t limit = (a1.type() != UndefinedType) ? a1.toUInt32(exec) : 0xFFFFFFFFU;
    if (a0.type() == ObjectType && Object::dynamicCast(a0).inherits(&RegExpImp::info)) {
      Object obj0 = Object::dynamicCast(a0);
      RegExp reg(obj0.get(exec,"source").toString(exec));
      reg.prepareMatch(s);
      if (s.isEmpty() && !reg.match(s, 0).isNull()) {
	// empty string matched by regexp -> empty array
        reg.doneMatch();
	res.put(exec, lengthPropertyName, Number(0), DontDelete|ReadOnly|DontEnum);
	break;
      }
      pos = 0;
      while (static_cast<uint32_t>(i) != limit && pos < s.size()) {
	// TODO: back references
        int mpos;
        int *ovector = 0L;
	UString mstr = reg.match(s, pos, &mpos, &ovector);
        delete [] ovector; ovector = 0L;
	if (mpos < 0)
	  break;
	pos = mpos + (mstr.isEmpty() ? 1 : mstr.size());
	if (mpos != p0 || !mstr.isEmpty()) {
	  res.put(exec,i, String(s.substr(p0, mpos-p0)));
	  p0 = mpos + mstr.size();
	  i++;
	}
      }
      reg.doneMatch();
    } else {
      u2 = a0.toString(exec);
      if (u2.isEmpty()) {
	if (s.isEmpty()) {
	  // empty separator matches empty string -> empty array
	  put(exec,lengthPropertyName, Number(0));
	  break;
	} else {
	  while (static_cast<uint32_t>(i) != limit && i < s.size()-1)
	    res.put(exec,i++, String(s.substr(p0++, 1)));
	}
      } else {
	while (static_cast<uint32_t>(i) != limit && (pos = s.find(u2, p0)) >= 0) {
	  res.put(exec,i, String(s.substr(p0, pos-p0)));
	  p0 = pos + u2.size();
	  i++;
	}
      }
    }
    // add remaining string, if any
    if (static_cast<uint32_t>(i) != limit)
      res.put(exec,i++, String(s.substr(p0)));
    res.put(exec,lengthPropertyName, Number(i));
    }
    break;
  case Substr: {
    n = a0.toInteger(exec);
    m = a1.toInteger(exec);
    int d, d2;
    if (n >= 0)
      d = n;
    else
      d = maxInt(len + n, 0);
    if (a1.type() == UndefinedType)
      d2 = len - d;
    else
      d2 = minInt(maxInt(m, 0), len - d);
    result = String(s.substr(d, d2));
    break;
  }
  case Substring: {
    double start = a0.toNumber(exec);
    double end = a1.toNumber(exec);
    if (KJS::isNaN(start))
      start = 0;
    if (KJS::isNaN(end))
      end = 0;
    if (start < 0)
      start = 0;
    if (end < 0)
      end = 0;
    if (start > len)
      start = len;
    if (end > len)
      end = len;
    if (a1.type() == UndefinedType)
      end = len;
    if (start > end) {
      double temp = end;
      end = start;
      start = temp;
    }
    result = String(s.substr((int)start, (int)end-(int)start));
    }
    break;
  case ToLowerCase:
  case ToLocaleLowerCase: // FIXME: To get this 100% right we need to detect Turkish and change I to lowercase i without a dot.
    for (i = 0; i < len; i++)
      s[i] = s[i].toLower();
    result = String(s);
    break;
  case ToUpperCase:
  case ToLocaleUpperCase: // FIXME: To get this 100% right we need to detect Turkish and change i to uppercase I with a dot.
    for (i = 0; i < len; i++)
      s[i] = s[i].toUpper();
    result = String(s);
    break;
#ifndef KJS_PURE_ECMA
  case Big:
    result = String("<big>" + s + "</big>");
    break;
  case Small:
    result = String("<small>" + s + "</small>");
    break;
  case Blink:
    result = String("<blink>" + s + "</blink>");
    break;
  case Bold:
    result = String("<b>" + s + "</b>");
    break;
  case Fixed:
    result = String("<tt>" + s + "</tt>");
    break;
  case Italics:
    result = String("<i>" + s + "</i>");
    break;
  case Strike:
    result = String("<strike>" + s + "</strike>");
    break;
  case Sub:
    result = String("<sub>" + s + "</sub>");
    break;
  case Sup:
    result = String("<sup>" + s + "</sup>");
    break;
  case Fontcolor:
    result = String("<font color=\"" + a0.toString(exec) + "\">" + s + "</font>");
    break;
  case Fontsize:
    result = String("<font size=\"" + a0.toString(exec) + "\">" + s + "</font>");
    break;
  case Anchor:
    result = String("<a name=\"" + a0.toString(exec) + "\">" + s + "</a>");
    break;
  case Link:
    result = String("<a href=\"" + a0.toString(exec) + "\">" + s + "</a>");
    break;
#endif
  }

  return result;
}

// ------------------------------ StringObjectImp ------------------------------

StringObjectImp::StringObjectImp(ExecState *exec,
                                 FunctionPrototypeImp *funcProto,
                                 StringPrototypeImp *stringProto)
  : InternalFunctionImp(funcProto)
{
  Value protect(this);
  // ECMA 15.5.3.1 String.prototype
  putDirect(prototypePropertyName, stringProto, DontEnum|DontDelete|ReadOnly);

  putDirect("fromCharCode", new StringObjectFuncImp(exec,funcProto), DontEnum);

  // no. of arguments for constructor
  putDirect(lengthPropertyName, NumberImp::one(), ReadOnly|DontDelete|DontEnum);
}


bool StringObjectImp::implementsConstruct() const
{
  return true;
}

// ECMA 15.5.2
Object StringObjectImp::construct(ExecState *exec, const List &args)
{
  ObjectImp *proto = exec->lexicalInterpreter()->builtinStringPrototype().imp();
  if (args.size() == 0)
    return Object(new StringInstanceImp(proto));
  return Object(new StringInstanceImp(proto, args.begin()->dispatchToString(exec)));
}

bool StringObjectImp::implementsCall() const
{
  return true;
}

// ECMA 15.5.1
Value StringObjectImp::call(ExecState *exec, Object &/*thisObj*/, const List &args)
{
  if (args.isEmpty())
    return String("");
  else {
    Value v = args[0];
    return String(v.toString(exec));
  }
}

// ------------------------------ StringObjectFuncImp --------------------------

// ECMA 15.5.3.2 fromCharCode()
StringObjectFuncImp::StringObjectFuncImp(ExecState* /*exec*/, FunctionPrototypeImp *funcProto)
  : InternalFunctionImp(funcProto)
{
  Value protect(this);
  putDirect(lengthPropertyName, NumberImp::one(), DontDelete|ReadOnly|DontEnum);
}

bool StringObjectFuncImp::implementsCall() const
{
  return true;
}

Value StringObjectFuncImp::call(ExecState *exec, Object &/*thisObj*/, const List &args)
{
  UString s;
  if (args.size()) {
    UChar *buf = new UChar[args.size()];
    UChar *p = buf;
    ListIterator it = args.begin();
    while (it != args.end()) {
      unsigned short u = it->toUInt16(exec);
      *p++ = UChar(u);
      it++;
    }
    s = UString(buf, args.size(), false);
  } else
    s = "";

  return String(s);
}
