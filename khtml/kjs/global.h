/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

namespace KJS {
  enum Type { // main types
              Undefined,
	      Null,
	      Boolean,
	      Number,
	      String,
	      Object,
	      Reference,
	      ListType,
              Completion,
	      // extended types
	      Property,
	      Scope,
	      InternalFunction,
	      DeclaredFunction,
	      AnonymousFunction,
	      ArgList,
	      Activation
};

  enum Attribute { None       = 0,
		   ReadOnly   = 1 >> 1,
		   DontEnum   = 1 >> 2,
		   DontDelete = 1 >> 3,
		   Internal   = 1 >> 4 };

  enum Class { Test };

  enum Hint { NoneHint, StringHint, NumberHint };

  enum Compl { Normal, Break, Continue, ReturnValue };

  enum CodeType { GlobalCode,
		  EvalCode,
		  FunctionCode,
		  AnonymousCode,
		  HostCode };

  enum Operator { OpEqual,
		  OpEqEq,
		  OpNotEq,
		  OpPlusEq,
		  OpMinusEq,
		  OpMultEq,
		  OpDivEq,
                  OpPlusPlus,
		  OpMinusMinus,
		  OpLess,
		  OpLessEq,
		  OpGreater,
		  OpGreaterEq,
		  OpAndEq,
		  OpXOrEq,
		  OpOrEq,
		  OpModEq,
                  OpAnd,
                  OpOr,
		  OpBitAnd,
		  OpBitXOr,
		  OpBitOr,
		  OpLShift,
		  OpRShift,
		  OpURShift
  };

  enum FunctionAttribute { ImplicitNone, ImplicitThis, ImplicitParents };

  // XChar2b and QChar compatible character representation
  struct UnicodeChar {
    UnicodeChar() : hi(0), lo(0) { }
    UnicodeChar(unsigned char h , unsigned char l) : hi(h), lo(l) { }
    unsigned short unicode() const { return hi << 8 | lo; }
    unsigned char hi;
    unsigned char lo;
  };

  extern const double NaN;
  extern const double Inf;
};

#include <iostream.h>
#define Debug(s) { cerr << s << endl; }
#endif
