/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
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

#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#include <assert.h>

#include "object.h"

namespace KJS {

  enum CodeType { GlobalCode,
		  EvalCode,
		  FunctionCode,
		  AnonymousCode,
		  HostCode };

  enum FunctionAttribute { ImplicitNone, ImplicitThis, ImplicitParents };

  class Context;
  class ParamList;

  /**
   * @short Base class for Function objects.
   */
  class Function : public KJSO {
  public:
    Function() { attr = ImplicitNone; }
    void processParameters(const List *);
    virtual KJSO* execute(const List &) = 0;
    virtual bool hasAttribute(FunctionAttribute a) const { return (attr & a); }
    virtual CodeType codeType() const = 0;
    KJSO *thisValue() const;
  protected:
    FunctionAttribute attr;
    ParamList *param;
  };

  /**
   * @short Abstract base class for internal functions.
   */
  class InternalFunction : public Function {
  public:
    InternalFunction() { param = 0L; }
    /**
     * @return @ref InternalFunctionType
     */
    virtual Type type() const { return InternalFunctionType; }
    virtual KJSO* execute(const List &) = 0;
    /**
     * @return @ref HostCode
     */
    CodeType codeType() const { return HostCode; }
  };

  /**
   * @short Constructor object for use with the 'new' operator
   */
  class Constructor : public InternalFunction {
  public:
    Constructor();
    Constructor(Object *proto, int len);
    /**
     * @return @ref ConstructorType
     */
    virtual Type type() const { return ConstructorType; }
    virtual KJSO* execute(const List &);
    virtual Object* construct(const List &args) = 0;
  };

  /**
   * @short Print to stdout for debugging purposes.
   */
  class DebugPrint : public InternalFunction {
  public:
    KJSO* execute(const List &args);
  };

}; // namespace

#endif
